/* The basic outline for this came from "Learning C the Hard Way" */
#include "minunit.h"
#include <dlfcn.h>
#include <stdio.h>
#include "lib.h"


typedef int (*lib_function)(const char *data);
char *lib_file = "build/libjoza.so";
void *lib = NULL;
FILE *LOG_FILE = NULL;

#define FUNC "ifind"
int check_func_ifind(uint32_t arr[], size_t n, uint32_t X, size_t expected)
{
    size_t (*func)(uint32_t arr[], size_t, uint32_t) = dlsym(lib, FUNC);
    check(func != NULL, "Did not find %s function in the library %s: %s", FUNC, lib_file, dlerror());

    size_t rc = func(arr, n, X);
    if (n == 0) {
        check(rc == expected, "Function %s return %d for data: [] %d %d", FUNC, rc, n, X);
    } else if (n == 1) {
        check(rc == expected, "Function %s return %d for data: [%d] %d %d", FUNC, rc, arr[0], n, X);
    } else if (n == 0) {
        check(rc == expected, "Function %s return %d for data: [%d,%d] %d %d", FUNC, rc, arr[0], arr[1], n, X);
    } else if (n == 3) {
        check(rc == expected, "Function %s return %d for data: [%d,%d,%d] %d %d", FUNC, rc, arr[0], arr[1], arr[2], n, X);
    } else if (n >= 3) {
        check(rc == expected, "Function %s return %d for data: [%d,%d,%d,...] %d %d", FUNC, rc, arr[0], arr[1], arr[2], n, X);
    }

    return 1;
error:
    return 0;
}
#undef FUNC

int check_function(const char *func_to_run, const char *data, int expected)
{
    lib_function func = dlsym(lib, func_to_run);
    check(func != NULL, "Did not find %s function in the library %s: %s", func_to_run, lib_file, dlerror());

    int rc = func(data);
    check(rc == expected, "Function %s return %d for data: %s", func_to_run, rc, data);

    return 1;
error:
    return 0;
}

char *test_dlopen()
{
    lib = dlopen(lib_file, RTLD_NOW);
    mu_assert(lib != NULL, "Failed to open the library to test.");

    return NULL;
}

char *test_functions()
{
    uint32_t arr3[3] = {10, 20, 30};
    size_t len3 = 3, len2 = 2;
    uint32_t arr2[2] = {10, 20};

    mu_assert(check_func_ifind(arr2, len2, 0, 0), "find before 1st element");
    mu_assert(check_func_ifind(arr2, len2, 10, 0), "find 1st element");
    mu_assert(check_func_ifind(arr2, len2, 15, 1), "find before 2nd element");
    mu_assert(check_func_ifind(arr2, len2, 20, 1), "find 2nd element");
    mu_assert(check_func_ifind(arr2, len2, 35, 2), "find after 2nd element");

    mu_assert(check_func_ifind(arr3, len3, 0, 0), "find before 1st element");
    mu_assert(check_func_ifind(arr3, len3, 10, 0), "find 1st element");
    mu_assert(check_func_ifind(arr3, len3, 15, 1), "find before 2nd element");
    mu_assert(check_func_ifind(arr3, len3, 20, 1), "find 2nd element");
    mu_assert(check_func_ifind(arr3, len3, 25, 2), "find before 3nd element");
    mu_assert(check_func_ifind(arr3, len3, 30, 2), "find 3nd element");
    mu_assert(check_func_ifind(arr3, len3, 35, 3), "find after 3nd element");
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

char *all_tests()
{
    mu_suite_start();

    mu_run_test(test_dlopen);
    mu_run_test(test_functions);
    mu_run_test(test_failures);
    mu_run_test(test_dlclose);

    return NULL;
}

RUN_TESTS(all_tests);
