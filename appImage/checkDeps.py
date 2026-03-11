import subprocess
import os
import sys
import re
import shutil

# CORE SYSTEM LIBS: Do NOT bundle these (they cause crashes on different distros)
EXCLUDED_LIBS = {
    'libc.so.6', 'libpthread.so.0', 'libdl.so.2', 'libm.so.6', 
    'librt.so.1', 'libutil.so.1', 'libgcc_s.so.1', 'libstdc++.so.6',
    'ld-linux-x86-64.so.2', 'linux-vdso.so.1', 'libglib-2.0.so.0'
}

def get_dependencies(file_path, lib_dir):
    """Returns a list of (lib_name, host_path) for a given binary."""
    env = os.environ.copy()
    # Force ldd to look at our bundle's lib dir first
    env["LD_LIBRARY_PATH"] = f"{lib_dir}:{env.get('LD_LIBRARY_PATH', '')}"
    
    deps = []
    try:
        result = subprocess.run(['ldd', file_path], capture_output=True, text=True, env=env)
        for line in result.stdout.splitlines():
            if "=> /" in line:
                match = re.search(r'\s*(.+?)\s*=>\s*(.+?)\s*\(', line)
                if match:
                    deps.append((match.group(1).strip(), match.group(2).strip()))
    except Exception:
        pass
    return deps

def fix_appdir_recursive(appdir_path):
    appdir_path = os.path.abspath(appdir_path)
    lib_dest = os.path.join(appdir_path, "usr/lib")
    os.makedirs(lib_dest, exist_ok=True)

    print(f"--- Starting Recursive Deep Scan: {appdir_path} ---")

    iteration = 1
    while True:
        print(f"\nScan Iteration {iteration}...")
        added_this_round = 0
        
        # 1. Find every binary/library in the entire AppDir
        all_targets = []
        for root, _, files in os.walk(appdir_path):
            for f in files:
                # Check for shared objects or files in bin folders
                if f.endswith(".so") or ".so." in f or "/bin/" in root:
                    all_targets.append(os.path.join(root, f))

        # 2. Check each target for host leaks
        for target in all_targets:
            dependencies = get_dependencies(target, lib_dest)
            
            for lib_name, host_path in dependencies:
                if lib_name in EXCLUDED_LIBS:
                    continue

                # If the resolved path is outside the AppDir, it's a leak
                if not host_path.startswith(appdir_path):
                    dest_file = os.path.join(lib_dest, lib_name)
                    
                    if not os.path.exists(dest_file):
                        print(f"📦 Found Leak: {lib_name} (needed by {os.path.basename(target)})")
                        shutil.copy2(host_path, dest_file)
                        added_this_round += 1

        if added_this_round == 0:
            print("✨ No more leaks found. Bundle is self-contained.")
            break
        
        iteration += 1

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 deep_fix.py <path_to_AppDir>")
    else:
        fix_appdir_recursive(sys.argv[1])
