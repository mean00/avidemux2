"""
test case for re module
"""

import re
import testsuite
SUCCEED, FAIL, SYNTAX_ERROR = range(3)

def RAISE():
    raise("testing failed")

def main():
    print("begin re tests")

    assert(re.__name__ != None)
    assert(re.__doc__ != None)
    assert(re.__file__ != None)

    test_re_obj_search()
    test_re_obj_match()
    test_re_mod_search()
    test_re_mod_match()
    test_re_obj_split()
    test_re_mod_split()
    test_re_obj_findall()
    test_re_mod_findall()
    test_mat_obj_groups()
    test_mat_obj_start()
    test_mat_obj_end()
    test_mat_obj_span()

    print("OK: re tests passed")

def test_re_obj_search(verbose = None):
    """
    some tests borrowed from cpython
    testing re.compile(), reobj.search(), and matobj.group()
    """
    regex_tests = testsuite.search_regex_tests
    for t in regex_tests:
        pattern=s=outcome=repl=expected=None
        if len(t)==5:
            pattern, s, outcome, repl, expected = t
        elif len(t)==3:
            pattern, s, outcome = t
        else:
            raise ('Test tuples should have 3 or 5 fields',t)

        try:
            obj=re.compile(pattern)
        except:
            if outcome==SYNTAX_ERROR: continue    # Expected a syntax error
            else:
                # Regex syntax errors aren't yet reported, so for
                # the official test suite they'll be quietly ignored.
                pass
        try:
            matobj=obj.search(s)
        except:
            print('=== Unexpected exception:', obj, matobj, pattern, s)
            RAISE()

        if outcome==SYNTAX_ERROR:
            # This should have been a syntax error; forget it.
            pass
        elif outcome==FAIL:
            if matobj==None: pass   # No match, as expected
            else: print('=== Succeeded incorrectly', obj, matobj, pattern, s)
        elif outcome==SUCCEED:
            if matobj!=None:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                found=matobj.group(0)
                repl = repl.replace("found", str(found))
                for i in range(1,11):
                    if "g"+str(i) in repl:
                        gi = str(matobj.group(i))
                        repl = repl.replace("g"+str(i), gi)
                if len(t) == 5:
                    repl = repl.replace('+', '')
                    repl = repl.replace('\"', '')
                    if repl!=expected:
                        print( '=== grouping error', t, 
                                str(repl)+' should be '+str(expected))
                        RAISE()
            else:
                print ('=== Failed incorrectly', t)

def test_re_obj_match(verbose = None):
    """
    some tests borrowed from cpython
    testing re.compile(), reobj.match() and matobj.group()
    """
    regex_tests = testsuite.match_regex_tests
    for t in regex_tests:
        pattern=s=outcome=repl=expected=None
        if len(t)==5:
            pattern, s, outcome, repl, expected = t
        elif len(t)==3:
            pattern, s, outcome = t
        else:
            raise ('Test tuples should have 3 or 5 fields',t)

        try:
            obj=re.compile(pattern)
        except:
            if outcome==SYNTAX_ERROR: continue    # Expected a syntax error
            else:
                # Regex syntax errors aren't yet reported, so for
                # the official test suite they'll be quietly ignored.
                pass
        try:
            matobj=obj.match(s)
        except:
            print('=== Unexpected exception:', obj, matobj, pattern, s)

        if outcome==SYNTAX_ERROR:
            # This should have been a syntax error; forget it.
            pass
        elif outcome==FAIL:
            if matobj==None: pass   # No match, as expected
            else: print('=== Succeeded incorrectly', obj, matobj, pattern, s)
        elif outcome==SUCCEED:
            if matobj!=None:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                found=matobj.group(0)
                repl = repl.replace("found", str(found))
                for i in range(1,11):
                    if "g"+str(i) in repl:
                        gi = str(matobj.group(i))
                        repl = repl.replace("g"+str(i), gi)
                if len(t) == 5:
                    repl = repl.replace('+', '')
                    repl = repl.replace('\"', '')
                    if repl!=expected:
                        print( '=== grouping error', t, 
                                str(repl)+' should be '+str(expected))
                        RAISE()
            else:
                print ('=== Failed incorrectly', obj, matobj, pattern, s)

def test_re_mod_search(verbose = None):
    """
    some tests borrowed from cpython
    testing re.search(), and matobj.group()
    """
    regex_tests = testsuite.search_regex_tests
    for t in regex_tests:
        pattern=s=outcome=repl=expected=None
        if len(t)==5:
            pattern, s, outcome, repl, expected = t
        elif len(t)==3:
            pattern, s, outcome = t
        else:
            raise ('Test tuples should have 3 or 5 fields',t)

        try:
            matobj=re.search(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                pass
            else:
                print('=== Unexpected exception:', matobj, pattern, s)

        if outcome==FAIL:
            if matobj==None: pass   # No match, as expected
            else: print('=== Succeeded incorrectly', obj, matobj, pattern, s)
        elif outcome==SUCCEED:
            if matobj!=None:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                found=matobj.group(0)
                repl = repl.replace("found", str(found))
                for i in range(1,11):
                    if "g"+str(i) in repl:
                        gi = str(matobj.group(i))
                        repl = repl.replace("g"+str(i), gi)
                if len(t) == 5:
                    repl = repl.replace('+', '')
                    repl = repl.replace('\"', '')
                    if repl!=expected:
                        print( '=== grouping error', t, 
                                str(repl)+' should be '+str(expected))
                        RAISE()
            else:
                print ('=== Failed incorrectly', t)

def test_re_mod_match(verbose = None):
    """
    some tests borrowed from cpython
    testing re.match(), and matobj.group()
    """
    regex_tests = testsuite.match_regex_tests
    for t in regex_tests:
        pattern=s=outcome=repl=expected=None
        if len(t)==5:
            pattern, s, outcome, repl, expected = t
        elif len(t)==3:
            pattern, s, outcome = t
        else:
            raise ('Test tuples should have 3 or 5 fields',t)

        try:
            matobj=re.match(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                pass
            else:
                print('=== Unexpected exception:', matobj, pattern, s)

        if outcome==FAIL:
            if matobj==None: pass   # No match, as expected
            else: print('=== Succeeded incorrectly', matobj, pattern, s)
        elif outcome==SUCCEED:
            if matobj!=None:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                found=matobj.group(0)
                repl = repl.replace("found", str(found))
                for i in range(1,11):
                    if "g"+str(i) in repl:
                        gi = str(matobj.group(i))
                        repl = repl.replace("g"+str(i), gi)
                if len(t) == 5:
                    repl = repl.replace('+', '')
                    repl = repl.replace('\"', '')
                    if repl!=expected:
                        print( '=== grouping error', t, 
                                str(repl)+' should be '+str(expected))
                        RAISE()
            else:
                print ('=== Failed incorrectly', t)

def test_re_obj_split(verbose = None):
    """
    test re.compile(), and reobj.split()
    """
    regex_tests = testsuite.split_regex_tests
    for t in regex_tests:
        pattern, s, outcome, maxsplit, fields = t
        try:
            reobj = re.compile(pattern)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                pass
            else:
                print('=== Unexpected exception:', pattern, s, 
                                    outcome, maxsplit, fields)
        try:
            fldlst=reobj.split(s, maxsplit)
        except:
            if outcome == SYNTAX_ERROR:
                continue
            else:
                print('=== Unexpected exception:', pattern, s, 
                                    outcome, maxsplit, fields)

        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if fldlst:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                if verbose:
                    fldstr = fieldstr = ""
                    for item in fldlst:
                        fldstr = fldstr + str(item) + " | "
                    for item in fields:
                        fieldstr = fieldstr + str(item) + " | "
                    print(fldstr, "~~~", fieldstr)
                if len(fields) != len(fldlst):
                    print('=== Not coherent 1')
                    RAISE()

                for i in range(len(fields)):
                    if fields[i] != fldlst[i]:
                        if verbose:
                            print('=== Not coherent 2', pattern, s, 
                                    outcome, maxsplit, fields, i, 
                                    fields[i],'(',len(fields[i]),')', ' | ', 
                                    fldlst[i],'(',len(fldlst[i]),')')
                        else:
                            print('=== Not coherent 2')
                        RAISE()
            else:
                print ('=== Failed incorrectly', pattern, s, 
                        outcome, maxsplit, fields)

def test_re_mod_split(verbose = None):
    """
    test re.split()
    """
    regex_tests = testsuite.split_regex_tests
    for t in regex_tests:
        pattern, s, outcome, maxsplit, fields = t
        try:
            fldlst=re.split(pattern, s, maxsplit)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception:', pattern, s, 
                                    outcome, maxsplit, fields)

        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if fldlst:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                if verbose:
                    fldstr = fieldstr = ""
                    for item in fldlst:
                        fldstr = fldstr + str(item) + " | "
                    for item in fields:
                        fieldstr = fieldstr + str(item) + " | "
                    print(fldstr, "~~~", fieldstr)

                if len(fields) != len(fldlst):
                    print('=== Not coherent 1')
                    RAISE()

                for i in range(len(fields)):
                    if fields[i] != fldlst[i]:
                        if verbose:
                            print('=== Not coherent 2', pattern, s, 
                                    outcome, maxsplit, fields, i, 
                                    fields[i],'(',len(fields[i]),')', ' | ', 
                                    fldlst[i],'(',len(fldlst[i]),')')
                        else:
                            print('=== Not coherent 2')
                        RAISE()
            else:
                print ('=== Failed incorrectly', pattern, s, 
                        outcome, maxsplit, fields)

def test_re_obj_findall(verbose = None):
    """
    test re.compile(), and reobj.findall()
    """
    regex_tests = testsuite.findall_regex_tests
    for t in regex_tests:
        pattern, s, outcome, pos, fields = t
        try:
            reobj = re.compile(pattern)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                pass
            else:
                print('=== Unexpected exception:', pattern, s, 
                                    outcome, pos, fields)
        try:
            fldlst=reobj.findall(s, pos)
        except:
            if outcome == SYNTAX_ERROR:
                continue
            else:
                print('=== Unexpected exception:', pattern, s, 
                                    outcome, pos, fields)

        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if fldlst:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                if verbose:
                    fldstr = fieldstr = ""
                    for item in fldlst:
                        fldstr = fldstr + str(item) + " | "
                    for item in fields:
                        fieldstr = fieldstr + str(item) + " | "
                    print(fldstr, "~~~", fieldstr)

                if len(fields) != len(fldlst):
                    print('=== Not coherent 1')
                    RAISE()

                for i in range(len(fields)):
                    if fields[i] != fldlst[i]:
                        if verbose:
                            print('=== Not coherent 2', pattern, s, 
                                    outcome, maxsplit, fields, i, 
                                    fields[i],'(',len(fields[i]),')', ' | ', 
                                    fldlst[i],'(',len(fldlst[i]),')')
                        else:
                            print('=== Not coherent 2')
                        RAISE()
            else:
                print ('=== Failed incorrectly', pattern, s, 
                        outcome, pos, fields)

def test_re_mod_findall(verbose = None):
    """
    test re.findall()
    """
    regex_tests = testsuite.mod_findall_regex_tests
    for t in regex_tests:
        pattern, s, outcome, pos, fields = t    # pos is not used
        try:
            fldlst=re.findall(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception:', pattern, s, 
                                    outcome, pos, fields)

        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if fldlst:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                if verbose:
                    fldstr = fieldstr = ""
                    for item in fldlst:
                        fldstr = fldstr + str(item) + " | "
                    for item in fields:
                        fieldstr = fieldstr + str(item) + " | "
                    print(fldstr, "~~~", fieldstr)

                if len(fields) != len(fldlst):
                    print('=== Not coherent 1')
                    RAISE()

                for i in range(len(fields)):
                    if fields[i] != fldlst[i]:
                        if verbose:
                            print('=== Not coherent 2', pattern, s, 
                                    outcome, maxsplit, fields, i, 
                                    fields[i],'(',len(fields[i]),')', ' | ', 
                                    fldlst[i],'(',len(fldlst[i]),')')
                        else:
                            print('=== Not coherent 2')
                        RAISE()
            else:
                print ('=== Failed incorrectly', pattern, s, 
                        outcome, pos, fields)

def test_mat_obj_groups(verbose = None):
    """
    test re.search(), and matobj.groups()
    'verbose' is for debugging, when 'verbose' is true, print extra info
    """
    regex_tests = testsuite.matobj_groups_regex_tests
    for t in regex_tests:
        pattern, s, outcome, fields, grpidx, start, end = t
        try:
            matobj=re.search(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 1:', pattern, s, 
                                    outcome,fields)

        try:
            if outcome==SUCCEED: assert(matobj != None)
            fldlst = matobj.groups()
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 2:', pattern, s, 
                                    outcome,fields)
           
        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if fldlst and fields:
                # Matched, as expected, so now we compute the
                # result string and compare it to our expected result.
                if verbose:
                    fldstr = fieldstr = ""
                    for item in fldlst:
                        fldstr = fldstr + str(item) + " | "
                    for item in fields:
                        fieldstr = fieldstr + str(item) + " | "
                    print(fldstr, "~~~", fieldstr)

                if len(fields) != len(fldlst):
                    print('=== Not coherent 2')
                    RAISE()

                for i in range(len(fields)):
                    if fields[i] != fldlst[i]:
                        if verbose:
                            print('=== Not coherent', pattern, s, 
                                    outcome,fields, i, 
                                    fields[i],'(',len(fields[i]),')', ' | ', 
                                    fldlst[i],'(',len(fldlst[i]),')')
                        else:
                            print('=== Not coherent')
                        RAISE()
            elif not len(fldlst) and not len(fields):
                # output is empty, as expected
                if verbose:
                    print("output is empty, as expected")
                continue
            else:
                if verbose:
                    for item in fldlst:
                        print(item)
                    print("")
                    for item in fields: 
                        print(item)
                    print("")
                print ('=== Failed incorrectly', pattern, s, 
                        outcome,fields,fldlst)

def test_mat_obj_start(verbose = None):
    """
    test re.search(), and matobj.start()
    'verbose' is for debugging, when 'verbose' is true, print extra info
    """
    regex_tests = testsuite.matobj_groups_regex_tests
    for t in regex_tests:
        pattern, s, outcome, fields, grpidx, start, end = t
        try:
            matobj=re.search(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 1:', pattern, s, 
                                    outcome,fields)

        try:
            if outcome==SUCCEED: assert(matobj != None)
            fldlst = matobj.groups()
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 2:', pattern, s, 
                                    outcome,fields)
           
        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if grpidx > 0:
                if matobj.start(grpidx) == start:
                    pass
                else:
                    if verbose:
                        print ('=== Failed incorrectly', pattern, s, 
                            outcome,fields,fldlst)
                    raise("testing failed")


def test_mat_obj_end(verbose = None):
    """
    test re.search(), and matobj.end()
    'verbose' is for debugging, when 'verbose' is true, print extra info
    """
    regex_tests = testsuite.matobj_groups_regex_tests
    for t in regex_tests:
        pattern, s, outcome, fields, grpidx, start, end = t
        try:
            matobj=re.search(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 1:', pattern, s, 
                                    outcome,fields)

        try:
            if outcome==SUCCEED: assert(matobj != None)
            fldlst = matobj.groups()
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 2:', pattern, s, 
                                    outcome,fields)
           
        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if grpidx > 0:
                if matobj.end(grpidx) == end:
                    pass
                else:
                    if verbose:
                        print ('=== Failed incorrectly', pattern, s, 
                            outcome,fields,fldlst, matobj.end(grpidx), end)
                    raise("testing failed")

def test_mat_obj_span(verbose = None):
    """
    test re.search(), and matobj.span()
    'verbose' is for debugging, when 'verbose' is true, print extra info
    """
    regex_tests = testsuite.matobj_groups_regex_tests
    for t in regex_tests:
        pattern, s, outcome, fields, grpidx, start, end = t
        try:
            matobj=re.search(pattern, s)
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 1:', pattern, s, 
                                    outcome,fields)

        try:
            if outcome==SUCCEED: assert(matobj != None)
            fldlst = matobj.groups()
        except:
            if outcome==SYNTAX_ERROR:
                # This should have been a syntax error; forget it.
                continue
            else:
                print('=== Unexpected exception 2:', pattern, s, 
                                    outcome,fields)
           
        if outcome==FAIL:
            pass    # No match, as expected
        elif outcome==SUCCEED:
            if (grpidx > 0):
                spstart, spend = matobj.span(grpidx)
                if spstart == start and spend == end:
                    pass
                else:
                    if verbose:
                        print ('=== Failed incorrectly', pattern, s, 
                            outcome,fields,fldlst)
                    raise("testing failed")

main()

