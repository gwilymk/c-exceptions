#include "exceptions.h"
#include "test_helper.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool returnFinallyRan;

SETUP()
{
    returnFinallyRan = false;
}

int getExceptionStackDepth__(void);
TEARDOWN()
{
    if (getExceptionStackDepth__() != 0)
    {
        printf("Exception stack depth ended at %i\n",
               getExceptionStackDepth__());
        exit(1);
    }
}

static void
throwException(int value)
{
    THROW(value, "unit test exception");
}

static void
throwsException(void)
{
    throwException(1);
}

TEST("can throw exceptions")
{
    volatile bool exceptionThrown = false;

    TRY { throwsException(); }
    CATCH(1) { exceptionThrown = true; }

    ASSERT(exceptionThrown);
}

TEST("can handle multiple catches")
{
    volatile int valueThrown = 0;

    TRY { throwException(3); }
    CATCH(1) { valueThrown = 1; }
    CATCH(2) { valueThrown = 2; }
    CATCH(3) { valueThrown = 3; }

    ASSERT_EQUAL(valueThrown, 3);
}

static void
doesntHandle(void)
{
    TRY { throwException(3); }
    CATCH(1) {}
}

TEST("unhandled exceptions will bubble")
{
    volatile bool handled = false;

    TRY { doesntHandle(); }
    CATCH(3) { handled = true; }

    ASSERT(handled);
}

TEST("handles no exception thrown")
{
    volatile bool handled = false;

    TRY {}
    CATCH(88) { handled = true; }

    ASSERT(handled == false);
}

TEST("code in try is executed upto throw")
{
    volatile bool step1 = false;
    volatile bool step2 = false;

    TRY
    {
        step1 = true;
        THROW(5, "testing try statement");
        step2 = true;
    }
    CATCH(5) {}

    ASSERT(step1);
    ASSERT(!step2);
}

TEST("can do nested try catches")
{
    volatile bool handled = false;

    TRY
    {
        TRY
        {
            TRY { throwException(55); }
            CATCH(44) { ASSERT(false); }
        }
        CATCH(55) { throwException(44); }
    }
    CATCH(44) { handled = true; }

    ASSERT(handled);
}

TEST("finally block executes if there are no exceptions")
{
    volatile bool finallyRan = false;
    TRY {}
    FINALLY { finallyRan = true; }

    ASSERT(finallyRan);
}

TEST("finally block executes if there was an exception")
{
    volatile bool finallyRan = false;
    volatile bool catchBlockHit = false;

    TRY { THROW(5, "some error"); }
    CATCH(5) { catchBlockHit = true; }
    FINALLY { finallyRan = true; }

    ASSERT(finallyRan);
    ASSERT(catchBlockHit);
}

TEST("finally block executes if it doesn't catch the exception")
{
    volatile bool finallyRan = false;
    volatile bool catchBlockHit = false;

    TRY
    {
        TRY { THROW(5, "some error"); }
        FINALLY { finallyRan = true; }
    }
    CATCH(5) { catchBlockHit = true; }

    ASSERT(finallyRan);
    ASSERT(catchBlockHit);
}

TEST("rethrow allows throwing again in a catch block")
{
    volatile bool firstCatchHit = false;
    volatile bool secondCatchHit = false;

    TRY
    {
        TRY { THROW(5, "some error"); }
        CATCH(5)
        {
            firstCatchHit = true;
            RETHROW;
        }
    }
    CATCH(5) { secondCatchHit = true; }

    ASSERT(firstCatchHit);
    ASSERT(secondCatchHit);
}

TEST("TRY is allowed on its own")
{
    volatile bool tryCalled = false;

    TRY { tryCalled = true; }

    ASSERT(tryCalled);
}

TEST("can catch all exceptions")
{
    volatile Exception errorCode = {.type = 0};

    TRY { THROW(44, "some message"); }
    CATCH(88) { ASSERT(false); }
    CATCH_ALL(code) { errorCode = code; }

    ASSERT_EQUAL(errorCode.type, 44);
}

TEST("can have catch alls and finally's")
{
    volatile Exception errorCode = {};
    volatile bool finallyRun = false;

    // TODO: Unfortunately, this is how it will have to be done for now
    // untill I can come up with a better way of doing it...
    TRY
    {
        TRY { THROW(44, "some message"); }
        FINALLY { finallyRun = true; }
    }
    CATCH_ALL(code) { errorCode = code; }

    ASSERT_EQUAL(errorCode.type, 44);
    ASSERT(finallyRun);
}

TEST_EXPECTING("can expect exceptions", 5)
{
    throwException(5);
}

TEST("Can have both a catch all and a finally")
{
    volatile Exception errorCode = {.type = -1};
    volatile bool finallyRan;
    TRY { THROW(44, "Some message"); }
    CATCH_ALL(code) { errorCode = code; }
    FINALLY { finallyRan = errorCode.type == 44; }

    ASSERT(finallyRan);
}

static int
return5AndSetFinallyRan(void)
{
    TRY { RETURN(5); }
    FINALLY { returnFinallyRan = true; }

    return 0;
}

TEST("Finally runs if return within a TRY block")
{
    ASSERT_EQUAL(return5AndSetFinallyRan(), 5);
    ASSERT(returnFinallyRan);
}

static int
return5WithinCatchAndSetFinallyRan(void)
{
    TRY { THROW(32, "unit test exception"); }
    CATCH(32) { RETURN(5); }
    FINALLY { returnFinallyRan = true; }

    return 0;
}

TEST("Finally runs if return within CATCH block")
{
    ASSERT_EQUAL(return5WithinCatchAndSetFinallyRan(), 5);
    ASSERT(returnFinallyRan);
}

static int
return5WithinTryBlockAndDontSetFinallyRan(void)
{
    TRY
    {
        RETURN(5);
        returnFinallyRan = true;
    }
    FINALLY {}

    return 0;
}

TEST("Bits after the return statement don't run")
{
    ASSERT_EQUAL(return5WithinTryBlockAndDontSetFinallyRan(), 5);
    ASSERT(!returnFinallyRan);
}

static int
claimToReturnAnIntButInsteadThrow(void)
{
    THROW(5, "unit test exception");
    return 0;
}

static int
methodInReturnThrowsAnExceptionAndThenReturn5(void)
{
    TRY { RETURN(claimToReturnAnIntButInsteadThrow()); }
    CATCH(5) {}
    FINALLY { returnFinallyRan = true; }

    return 5;
}

TEST("Method inside the return statement which throw work as expected")
{
    ASSERT_EQUAL(methodInReturnThrowsAnExceptionAndThenReturn5(), 5);
    ASSERT(returnFinallyRan);
}
