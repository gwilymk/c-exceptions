#include "exceptions.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef MAX_TRY_DEPTH
#define MAX_TRY_DEPTH 128
#endif

jmp_buf exceptionStack__[MAX_TRY_DEPTH];
static const char *fileNames[MAX_TRY_DEPTH];
static int lineNumbers[MAX_TRY_DEPTH];

static int exceptionStackDepth = 0;

static struct
{
    int type;
    const char *message;
    bool handled;
} currentException = {
    .handled = true,
};

int try__(const char *fileName, int lineNumber)
{
    fileNames[exceptionStackDepth] = fileName;
    lineNumbers[exceptionStackDepth] = lineNumber;
    return exceptionStackDepth++;
}

void throw__(int type, const char *message)
{
    currentException.type = type;
    currentException.message = message;
    currentException.handled = false;
    if (exceptionStackDepth == 0)
    {
        fprintf(stderr, "Unhandled exception of type %i with messasge %s\n",
                type, message);
        exit(1);
    }
    longjmp(exceptionStack__[exceptionStackDepth - 1], type);
}

void rethrow__(void)
{
    endTry__();
    throw__(currentException.type, currentException.message);
}

const char *
catchMessage__(void)
{
    return currentException.message;
}

int catchType__(void)
{
    return currentException.type;
}

int catchHandled__(void)
{
    bool oldWasCatchHandled = currentException.handled;
    currentException.handled = true;

    return oldWasCatchHandled;
}

void endTry__(void)
{
    exceptionStackDepth--;
}

int getExceptionStackDepth__(void);
// Used in the testing framework to ensure that it is always working
int getExceptionStackDepth__(void)
{
    fprintf(stderr, "Exception stack:\n");
    for (int i = 0; i < exceptionStackDepth; i++)
    {
        fprintf(stderr, "%s:%d\n", fileNames[i], lineNumbers[i]);
    }

    return exceptionStackDepth;
}
