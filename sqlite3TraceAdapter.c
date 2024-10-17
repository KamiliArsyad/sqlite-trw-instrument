#include "sqlite3TraceAdapter.h"
#include <stdlib.h>
#include <string.h>

#define COLUMN_OP_NAME "Column"
#define ROW_ID_OP_NAME "RowId"

static const char* cursorOperations[9] = {
    "Next",
    "Rewind",
    "Prev",
    "SeekRowId",
    "Found",
    "NotFound",
    "IsUnique",
    "NotExists",
    "NoConflict"
};

int isCursorMovement(u8 opCode)
{
    int match = 0;
    for (int i = 0; i < sizeof(cursorOperations) / sizeof(cursorOperations[0]); i++)
    {
        match |= strcmp(cursorOperations[i], sqlite3OpcodeName(opCode)) == 0;
        if (match) return 1;
    }

    return 0;
}

int isColumnOp(u8 opCode)
{
    return strcmp(sqlite3OpcodeName(opCode), COLUMN_OP_NAME) == 0;
}

int isRowIdOp(u8 opCode)
{
    return strcmp(sqlite3OpcodeName(opCode), ROW_ID_OP_NAME) == 0;
}

int checkVdbeOp(VdbeOp *op, vdbeOpCheckPredicate predicate)
{
    return predicate(op->opcode);
}

TraceState* initTraceState()
{
    TraceState *traceState = malloc(sizeof(TraceState));
    if (!traceState) return NULL;

    traceState->readOp = NULL;
    traceState->writeOp = NULL;
    traceState->rowId = -1;

    return traceState;
}

void freeTraceState(TraceState *traceState)
{
    free(traceState);
}

// ------------------------------------------
// ---- Actual Interceptor Implementation ---
// ------------------------------------------

__thread TraceState *currentTraceState = NULL;

void sqlite3TraceInterceptor(VdbeOp *pOp)
{
    if (!pOp) return;

    if (checkVdbeOp(pOp, isCursorMovement))
    {
        if (currentTraceState == NULL) return;

        if (currentTraceState->readOp != NULL)
        {
            printTransactionOp(currentTraceState->readOp, stdout);
        }

        if (currentTraceState->writeOp != NULL)
        {
            printTransactionOp(currentTraceState->writeOp, stdout);
        }

        freeTraceState(currentTraceState);
        currentTraceState = NULL;
    } else if (checkVdbeOp(pOp, isColumnOp))
    {
        // A read operation is done
        if (currentTraceState == NULL)
        {
            currentTraceState = initTraceState();
        }

        currentTraceState->readOp = trackRead(getThreadId(), currentTraceState->rowId);
    }
}

void setRowId(int rowId)
{
    if (currentTraceState == NULL)
    {
        currentTraceState = initTraceState();
    }

    currentTraceState->rowId = rowId;
}