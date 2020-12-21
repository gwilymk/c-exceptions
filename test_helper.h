/*
 * Copyright (c) 2020 Gwilym Kuiper <gw@ilym.me>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdbool.h>

typedef void (*testFunc__)(void);

void testAssert__(int assertion, const char *file, int line, const char *fstr1,
                  const char *fstr2, const char *fstr3, ...);
void registerTest(const char *name, testFunc__ test, int expectedException);
void registerSetup(const char *name, testFunc__ test, int expectedException);
void registerTeardown(const char *name, testFunc__ test, int expectedException);

#define TEST_ASSERT__(assertion, fstr1, fstr2, fstr3, ...)               \
    do                                                                   \
    {                                                                    \
        testAssert__(assertion, __FILE__, __LINE__, fstr1, fstr2, fstr3, \
                     __VA_ARGS__);                                       \
    } while (0)

#define TEST_CONCAT1__(a, b) a##b
#define TEST_CONCAT__(a, b) TEST_CONCAT1__(a, b)
#define TEST_REGISTER_TYPE__(name, prefix, exception)               \
    static void TEST_CONCAT__(prefix##_, __LINE__)(void);           \
    __attribute__((constructor)) static void TEST_CONCAT__(         \
        register##prefix##_, __LINE__)(void)                        \
    {                                                               \
        register##prefix(name, &TEST_CONCAT__(prefix##_, __LINE__), \
                         exception);                                \
    }                                                               \
    static void TEST_CONCAT__(prefix##_, __LINE__)()

#define TEST_EXPECTING(name, exception) \
    TEST_REGISTER_TYPE__(name, Test, exception)
#define TEST(name) TEST_EXPECTING(name, 0)

#define SETUP() TEST_REGISTER_TYPE__("setup", Setup, 0)
#define TEARDOWN() TEST_REGISTER_TYPE__("teardown", Teardown, 0)

#define FORMAT_STRING__(thing)   \
    _Generic((thing),            \
             unsigned char       \
             : "%u",             \
               char              \
             : "%c",             \
               int               \
             : "%d",             \
               unsigned int      \
             : "%d",             \
               long              \
             : "%d",             \
               char *            \
             : "%s",             \
               double            \
             : "%f",             \
               bool              \
             : "%s",             \
               unsigned long int \
             : "0x%16lx",        \
               default           \
             : "%s")

#define FORMAT__(thing)                  \
    _Generic((thing),                    \
             unsigned char               \
             : thing,                    \
               char                      \
             : thing,                    \
               int                       \
             : thing,                    \
               unsigned int              \
             : thing,                    \
               long                      \
             : thing,                    \
               char *                    \
             : thing,                    \
               double                    \
             : thing,                    \
               bool                      \
             : thing ? "true" : "false", \
               unsigned long int         \
             : thing,                    \
               default                   \
             : "unknown")

#define ASSERT_EQUAL(actual, expected)                            \
    do                                                            \
    {                                                             \
        __auto_type _actual = (actual);                           \
        __auto_type _expected = (expected);                       \
        TEST_ASSERT__(_actual == _expected,                       \
                      "Expected to be the same, actual = ",       \
                      FORMAT_STRING__(_actual), " expected = %s", \
                      FORMAT__(_actual), #expected);              \
    } while (0)

#define ASSERT_NOT_EQUAL(actual, expected)                        \
    do                                                            \
    {                                                             \
        __auto_type _actual = (actual);                           \
        __auto_type _expected = (expected);                       \
        TEST_ASSERT__(_actual != _expected,                       \
                      "Expected to be different, actual = ",      \
                      FORMAT_STRING__(_actual), " expected = %s", \
                      FORMAT__(_actual), #expected);              \
    } while (0)

#define ASSERT(assertion)                                   \
    do                                                      \
    {                                                       \
        __auto_type _assertion = (assertion);               \
        TEST_ASSERT__(!!_assertion, "Assertion failed! ",   \
                      FORMAT_STRING__(_assertion), " (%s)", \
                      FORMAT__(_assertion), #assertion);    \
    } while (0)
