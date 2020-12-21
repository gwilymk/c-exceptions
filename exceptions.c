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
