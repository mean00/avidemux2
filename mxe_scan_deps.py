#
# pip install pefile
# python3 mxe_scan_deps.py avidemux64 --sources /opt/mxe/usr/x86_64-w64-mingw32.shared/qt6/bin /opt/mxe/usr/x86_64-w64-mingw32.shared/bin/
#
import os
import shutil
import argparse
import pefile
import sys
from collections import deque

class Colors:
    HEADER, INFO, SUCCESS, WARNING, FAIL, ENDC, BOLD = '\033[95m', '\033[94m', '\033[92m', '\033[93m', '\033[91m', '\033[0m', '\033[1m'

SYSTEM_DLLS = {
    "kernel32.dll", "user32.dll", "gdi32.dll", "shell32.dll", "advapi32.dll",
    "msvcrt.dll", "ws2_32.dll", "ole32.dll", "rpcrt4.dll", "shlwapi.dll",
    "comctl32.dll", "winmm.dll", "opengl32.dll", "glu32.dll", "crypt32.dll",
    "imm32.dll", "wsock32.dll", "setupapi.dll", "iphlpapi.dll", "msi.dll",
    "comdlg32.dll", "version.dll", "winhttp.dll", "wininet.dll", "netapi32.dll",
    "mpr.dll", "psapi.dll", "dnsapi.dll", "uxtheme.dll", "dwmapi.dll", "ntdll.dll",
    "imagehlp.dll", "powrprof.dll", "bcrypt.dll", "d3d9.dll","d3d11.dll", "d3d12.dll", 
    "dwrite.dll", "dxgi.dll", "secur32.dll", "oleaut32.dll", "authz.dll", "userenv.dll",
    "shcore.dll","wtsapi32.dll"
}

def is_system_dll(name):
    return name.lower() in SYSTEM_DLLS or name.lower().startswith(("api-ms-win-", "ext-ms-win-"))

def get_imports(file_path):
    deps = []
    try:
        pe = pefile.PE(file_path, fast_load=False)
        if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
            for entry in pe.DIRECTORY_ENTRY_IMPORT:
                if entry.dll:
                    deps.append(entry.dll.decode('utf-8'))
    except Exception:
        pass # Skip non-PE or corrupted
    return deps

def build_source_index(paths):
    idx = {}
    for p in paths:
        if not os.path.isdir(p): continue
        for root, _, files in os.walk(p):
            for f in files:
                if f.lower().endswith('.dll'):
                    # Map both the name and a version-stripped name
                    idx[f.lower()] = os.path.abspath(os.path.join(root, f))
    return idx

def run_resolver(target_root, source_paths):
    target_root = os.path.abspath(target_root)
    source_index = build_source_index(source_paths)
    
    queue = deque()
    processed_paths = set()
    
    print(f"{Colors.HEADER}Target Directory: {target_root}{Colors.ENDC}")

    # Seed from target
    for root, _, files in os.walk(target_root):
        for f in files:
            if f.lower().endswith(('.exe', '.dll')):
                queue.append(os.path.abspath(os.path.join(root, f)))

    while queue:
        current_path = queue.popleft()
        if current_path in processed_paths: continue
        
        current_name = os.path.basename(current_path)
        print(f"\n{Colors.INFO}[Checking]{Colors.ENDC} {current_name}")
        
        deps = get_imports(current_path)
        for dep in deps:
            if is_system_dll(dep): continue
            
            dep_l = dep.lower()
            # PHYSICAL CHECK: Does this file exist EXACTLY in the target root?
            destination_path = os.path.join(target_root, dep)
            
            # We also check for 'lib' prefixed version in the target root
            lib_variant = f"lib{dep_l}" if not dep_l.startswith("lib") else dep_l
            destination_path_lib = os.path.join(target_root, lib_variant)

            if os.path.exists(destination_path) or os.path.exists(destination_path_lib):
                actual_found = destination_path if os.path.exists(destination_path) else destination_path_lib
                print(f"   {Colors.SUCCESS}[OK]{Colors.ENDC} {os.path.basename(actual_found)} exists.")
                if os.path.abspath(actual_found) not in processed_paths:
                    queue.append(os.path.abspath(actual_found))
                continue

            # If not in target root, search MXE index
            print(f"   {Colors.WARNING}[Missing]{Colors.ENDC} {dep} -> searching MXE...")
            
            match_path = None
            # Check variations in source index
            for v in [dep_l, f"lib{dep_l}", dep_l.replace("lib", "", 1)]:
                if v in source_index:
                    match_path = source_index[v]
                    break
            
            # Special logic for versioned libs (e.g. x264.dll requested, libx264-165.dll exists)
            if not match_path:
                base_name = dep_l.split('.')[0].replace("lib", "")
                for key, full_path in source_index.items():
                    if base_name in key:
                        match_path = full_path
                        break

            if match_path:
                final_dest = os.path.join(target_root, os.path.basename(match_path))
                print(f"      {Colors.SUCCESS}[COPYING]{Colors.ENDC} {os.path.basename(match_path)}")
                shutil.copy2(match_path, final_dest)
                queue.append(os.path.abspath(final_dest))
            else:
                # Last ditch: check if it's an internal lib elsewhere in the tree
                found_internal = None
                for root, _, files in os.walk(target_root):
                    if dep_l in [f.lower() for f in files]:
                        found_internal = os.path.join(root, dep)
                        break
                
                if found_internal:
                    final_dest = os.path.join(target_root, dep)
                    print(f"      {Colors.WARNING}[PROMOTING]{Colors.ENDC} Internal {dep}")
                    shutil.copy2(found_internal, final_dest)
                    queue.append(os.path.abspath(final_dest))
                else:
                    print(f"   {Colors.FAIL}[FATAL] {dep} not found anywhere.{Colors.ENDC}")
                    sys.exit(1)

        processed_paths.add(current_path)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("target")
    parser.add_argument("--sources", nargs='+')
    args = parser.parse_args()
    run_resolver(args.target, args.sources)


