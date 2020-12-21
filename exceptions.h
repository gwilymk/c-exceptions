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

/**
 * @file exceptions.h
 * @author Gwilym Kuiper
 * @brief Provides basic exception handling macros
 */
#pragma once

#include <setjmp.h>
#include <stdnoreturn.h>

extern jmp_buf exceptionStack__[];

/**
 * Holds information about a thrown exception.
 *
 * This is the type of the argument in the CATCH_ALL(e) block.
 *
 * This library allows for lightweight exceptions in C using setjmp/longjmp.
 * The support is very basic but provides most of the features you would
 * want in an exception handling context. For example:
 *
 * @code{.c}
 * TRY {
 *   doSomethingDangerous()
 * } CATCH(SOME_EXCEPTION) {
 *   // This code only runs if doSomethingDangerous throws SOME_EXCEPTION
 * } FINALLY {
 *   // This code will always run
 * }
 * @endcode
 *
 * Exceptions can be thrown using the THROW macro. In order to catch any
 * exception, a CATCH_ALL macro is provided.
 *
 * @code{.c}
 * TRY {
 *   // ...
 * } CATCH_ALL(e) {
 *   // ...
 * } FINALLY {
 *   // ...
 * }
 * @endcode
 *
 * A limitation is that you cannot return until after the exception handling
 * block, instead, you must use the RETURN macro as follows:
 *
 * @code{.c}
 * TRY {
 *   // ...
 *   RETURN(myFunction());
 * } FINALLY {
 *   // cleanup
 * }
 * @endcode
 *
 * The code within the FINALLY block will be excuted before returning. This
 * does not work within nested TRY blocks.
 *
 * RETURN also works within CATCH blocks but are not defined within FINALLY
 * blocks and their use within FINALLY blocks will cause the TRY / CATCH state
 * to become corrupt.
 *
 * See https://gwilym.dev/2020/12/the-c-preprocessor-is-awesome-part-iii/ for implementation details.
 */
typedef struct
{
    /** The type of the exception (i.e. the code passed to THROW()) */
    int type;
    /** The message (i.e. the massage passed to THROW()) */
    const char *message;
} Exception;

int try__(const char *filename, int lineNumber);
int catchHandled__(void);
const char *catchMessage__(void);
noreturn void throw__(int value, const char *message);
noreturn void rethrow__(void);
void endTry__(void);

/**
 * Throw an exception of type t with message m
 *
 * @param t The type of exception that is being thrown
 * @param m The message.  Throw will longjmp to the nearest try block.
 * The message is never freed so ensure that a constant char is used
 * as the message. If there is no TRY statement higher up, then this
 * call will abort execution.
 */
#define THROW(t, m) throw__(t, m)

/**
 * Can only be used within a CATCH or a CATCH_ALL block. Will throw the current
 * exception again.
 *
 * BUG: Rethrow will skip the execution of the FINALLY block.
 */
#define RETHROW rethrow__();

typedef struct
{
    int tryAttempt;
    int runFourTimes;
    void *returnTo;
    void *continueLabel;
} TryData__;

/**
 * Start an exception block
 */
#define TRY                                                           \
    for (TryData__ tryData__ =                                        \
             {setjmp(exceptionStack__[try__(__FILE__, __LINE__)]), 0, \
              (void *)0, (void *)0};                                  \
         tryData__.runFourTimes <= 3; tryData__.runFourTimes++)       \
        if (tryData__.runFourTimes == 0)                              \
        {                                                             \
            __label__ continueLabel;                                  \
            tryData__.continueLabel = &&continueLabel;                \
        continueLabel:;                                               \
        }                                                             \
        else if (tryData__.runFourTimes == 3)                         \
        {                                                             \
            if (!catchHandled__())                                    \
            {                                                         \
                RETHROW;                                              \
            }                                                         \
            endTry__();                                               \
            if (tryData__.returnTo)                                   \
            {                                                         \
                goto *tryData__.returnTo;                             \
            }                                                         \
        }                                                             \
        else if (tryData__.runFourTimes == 1 && tryData__.tryAttempt == 0)

/**
 * Catch a specific type of exception.
 *
 * @param value The exception to catch
 */
#define CATCH(value)                                                           \
    else if (tryData__.runFourTimes == 1 && tryData__.tryAttempt == (value) && \
             (catchHandled__() || 1))

/**
 * Will catch every exception type and store in a variable named by the
 * parameter e. The variable will be of type Exception
 *
 * @param e The name of the exception variable
 */
#define CATCH_ALL(e)                                                         \
    else if (tryData__.runFourTimes == 1 && tryData__.tryAttempt > 0 &&      \
             (catchHandled__() ||                                            \
              1)) for (volatile Exception e = {.type = tryData__.tryAttempt, \
                                               .message = catchMessage__()}; \
                       (e).type != -1; (e).type = -1)

/**
 * This block will get called regardless of whether or not an exception was
 * thrown.
 */
#define FINALLY else if (tryData__.runFourTimes == 2)

/**
 * This must be used if you wish to return a value within a TRY or CATCH block.
 * It will ensure that the FINALLY block gets run first.
 *
 * @param x The value to return
 */
#define RETURN(x)                           \
    do                                      \
    {                                       \
        __label__ returnPoint;              \
        __auto_type retValue = (x);         \
        tryData__.returnTo = &&returnPoint; \
        goto *tryData__.continueLabel;      \
    returnPoint:                            \
        return retValue;                    \
    } while (0)

/**
 * Possible exception types.
 *
 * More will be added to this list as they are needed.
 */
typedef enum
{
    BAD_OBJECT_TYPE_EXCEPTION = 1,   /** Expected a different object type */
    CANNOT_POP_STACK_EXCEPTION,      /** Cannot pop the stack */
    GC_OUT_OF_SPACE_EXCEPTION,       /** GC ran out of space */
    OUT_OF_RANGE_EXCEPTION,          /** Operation failed bounds check */
    CALL_STACK_EXCEEDED_EXCEPTION,   /** Stack overflow */
    RANDOM_SEEDING_FAILED_EXCEPTION, /** Failed to read the random seed */
} Exceptions;
