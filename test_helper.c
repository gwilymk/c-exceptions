#define _GNU_SOURCE
#include "exceptions.h"
#include "test_helper.h"

// For asprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <stdint.h>

#define ASSERTION_FAILED_EXCEPTION (INT32_MAX - 3)

typedef struct Test
{
    const char *name;
    int expectedException;
    testFunc__ test;
    struct Test *next;
} Test;

static Test *tests;
static Test *setups;
static Test *teardowns;

static const char *testName;

void testAssert__(int assertion, const char *file, int line, const char *fstr1,
                  const char *fstr2, const char *fstr3, ...)
{
    if (!assertion)
    {
        printf("Assertion failed in test \"%s\" on %s:%i\n", testName, file,
               line);
        char *format;
        if (asprintf(&format, "%s%s%s\n", fstr1, fstr2, fstr3) == -1)
        {
            fprintf(stderr, "Failed to create format string\n");
            exit(0);
        }
        va_list va;
        va_start(va, fstr3);
        vprintf(format, va);
        va_end(va);

        free(format);

        THROW(ASSERTION_FAILED_EXCEPTION, "Assertion failed");
    }
}

void registerTest(const char *name, testFunc__ testFn, int expectedException)
{
    Test *test = malloc(sizeof(Test));
    test->name = name;
    test->test = testFn;
    test->next = tests;
    test->expectedException = expectedException;
    tests = test;
}

void registerSetup(const char *name, testFunc__ setupFn, int expectedException)
{
    Test *setup = malloc(sizeof(Test));
    setup->name = name;
    setup->test = setupFn;
    setup->next = setups;
    setup->expectedException = expectedException;
    setups = setup;
}

void registerTeardown(const char *name, testFunc__ teardownFn, int expectedException)
{
    Test *teardown = malloc(sizeof(Test));
    teardown->name = name;
    teardown->test = teardownFn;
    teardown->next = teardowns;
    teardown->expectedException = expectedException;
    teardowns = teardown;
}

static void
cleanupTests(Test *tests)
{
    Test *test = tests;
    while (test)
    {
        Test *nextTest = test->next;
        free(test);
        test = nextTest;
    }
}

static void
runAll(Test *tests)
{
    Test *test = tests;
    while (test)
    {
        test->test();
        test = test->next;
    }
}

static Exception
runSafely(Test *test)
{
    TRY
    {
        printf("Running test %s\n", test->name);
        testName = test->name;
        test->test();
    }
    CATCH_ALL(e) { RETURN(e); }
    FINALLY { printf("Done\n"); }

    return (Exception){.type = 0, .message = "no exception thrown"};
}

int main(void)
{
    int numTests = 0;
    Test *test = tests;

    while (test)
    {
        runAll(setups);
        Exception exception = runSafely(test);

        if (exception.type == ASSERTION_FAILED_EXCEPTION)
        {
            printf("Assertion failed in test %s\n", test->name);
            exit(1);
        }
        else if (test->expectedException != exception.type)
        {
            printf("Unexpected exception thrown in test"
                   "\"%s\" of type %i with message \"%s\"\n",
                   test->name, exception.type, exception.message);
            exit(1);
        }

        runAll(teardowns);
        test = test->next;
        numTests++;
    }

    cleanupTests(tests);
    cleanupTests(setups);
    cleanupTests(teardowns);

    return 0;
}
