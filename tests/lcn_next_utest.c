/* The basic outline for this came from "Learning C the Hard Way" */
#include "minunit.h"
#include <dlfcn.h>
#include <stdio.h>
#include "lib.h"
#include "lcn.h"


typedef int (*lib_function)(const char *data);
char *lib_file = "build/libjoza.so";
void *lib = NULL;
FILE *LOG_FILE = NULL;

#define FUNC "lcn_next"
int check_func_lcn_next(lcn_t arr[], chan_idx_t n, lcn_t key, chan_idx_lcn_t expected)
{
    chan_idx_lcn_t (*func)(lcn_t arr[], chan_idx_t, lcn_t) = dlsym(lib, FUNC);
    check(func != NULL, "Did not find %s function in the library %s: %s", FUNC, lib_file, dlerror());

    chan_idx_lcn_t rc = func(arr, n, key);
    if (n == 0) {
        check(rc.index == expected.index && rc.key == expected.key,
              "Function %s returns {%d,%d} for data: [] %d %d", FUNC, rc.index, rc.key, n, key);
    }
    else if (n == 1) {
        check(rc.index == expected.index && rc.key == expected.key,
              "Function %s returns {%d,%d} for data: [%d] %d %d", FUNC, rc.index, rc.key, arr[0], n, key);
    }
    else if (n == 2) {
        check(rc.index == expected.index && rc.key == expected.key,
              "Function %s returns {%d,%d} for data: [%d,%d] %d %d", FUNC, rc.index, rc.key, arr[0], arr[1], n, key);
    }
    else if (n == 3) {
        check(rc.index == expected.index && rc.key == expected.key,
              "Function %s returns {%d,%d} for data: [%d,%d,%d] %d %d", FUNC, rc.index, rc.key, arr[0], arr[1], arr[2], n, key);

    }
    else if (n >= 3) {
        check(rc.index == expected.index && rc.key == expected.key,
              "Function %s returns {%d,%d} for data: [%d,%d,%d,...] %d %d", FUNC, rc.index, rc.key, arr[0], arr[1], arr[2], n, key);
    }

    return 1;
error:
    return 0;
}
#undef FUNC

char *test_dlopen()
{
    lib = dlopen(lib_file, RTLD_NOW);
    mu_assert(lib != NULL, "Failed to open the library to test.");

    return NULL;
}

char *test_functions()
{
    lcn_t arr3[3] = {10, 20, 30};
    size_t len3 = 3, len2 = 2;
    lcn_t arr2[2] = {10, 20};
    chan_idx_lcn_t expected;

    expected.index = 0;
    expected.key = 1;
    mu_assert(check_func_lcn_next(arr2, len2, 1, expected), "find before 1st element");
    expected.index = 1;
    expected.key = 11;
    mu_assert(check_func_lcn_next(arr2, len2, 10, expected), "find 1st element");
    expected.index = 1;
    expected.key = 15;
    mu_assert(check_func_lcn_next(arr2, len2, 15, expected), "find before 2nd element");
    expected.index = 2;
    expected.key = 21;
    mu_assert(check_func_lcn_next(arr2, len2, 20, expected), "find 2nd element");
    expected.index = 2;
    expected.key = 35;
    mu_assert(check_func_lcn_next(arr2, len2, 35, expected), "find after 2nd element");

    //mu_assert(check_func_lcn_next(arr3, len3, 0, 0), "find before 1st element");
    //mu_assert(check_func_lcn_next(arr3, len3, 10, 0), "find 1st element");
    //mu_assert(check_func_lcn_next(arr3, len3, 15, 1), "find before 2nd element");
    //mu_assert(check_func_lcn_next(arr3, len3, 20, 1), "find 2nd element");
    //mu_assert(check_func_lcn_next(arr3, len3, 25, 2), "find before 3nd element");
    //mu_assert(check_func_lcn_next(arr3, len3, 30, 2), "find 3nd element");
    //mu_assert(check_func_lcn_next(arr3, len3, 35, 3), "find after 3nd element");
    return NULL;
}

char *test_failures()
{
    // mu_assert(check_function("fail_on_purpose", "Hello", 1), "fail_on_purpose should fail.");

    return NULL;
}

char *test_dlclose()
{
    int rc = dlclose(lib);
    mu_assert(rc == 0, "Failed to close lib.");

    return NULL;
}

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_dlopen);
    mu_run_test(test_functions);
    mu_run_test(test_failures);
    mu_run_test(test_dlclose);

    return NULL;
}

RUN_TESTS(all_tests);
