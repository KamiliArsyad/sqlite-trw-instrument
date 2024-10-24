#include "mvtracer.h"
#include <stdlib.h>
#include <stdio.h>

Value* createValue(const void* val, const valToStringFunc func)
{
    Value *res = malloc(sizeof(Value));
    if (!res)
    {
        return NULL;
    }

    res->val = val;
    res->func = func;

    return res;
}

static void destroyValue(Value* val)
{
    if (val)
    {
        free(val);
    }
}

static TransactionOp* createTransactionOp(const OpType type, const int transactionId, const unsigned long objectId,
                                          const Value* writeVal)
{
    TransactionOp* transactionOp = malloc(sizeof(TransactionOp));
    if (!transactionOp)
    {
        return NULL;
    }

    transactionOp->type = type;
    transactionOp->transactionId = transactionId;
    transactionOp->objectId = objectId;
    transactionOp->writeVal = writeVal;

    return transactionOp;
}

static void destroyTransactionOp(TransactionOp* transactionOp)
{
    if (transactionOp)
    {
        if (transactionOp->writeVal != NULL
            && transactionOp->writeVal->val != NULL)
        {
            destroyValue(transactionOp->writeVal);
        }
        free(transactionOp);
    }
}

// ------------ PUBLIC API -----------------
TransactionOp* trackRead(int transactionId, unsigned long objectId)
{
    return createTransactionOp(READ, transactionId, objectId, createValue(NULL, NULL));
}

TransactionOp* trackWrite(int transactionId, unsigned long objectId, Value *value)
{
    return createTransactionOp(WRITE, transactionId, objectId, value);
}

TransactionOp *trackBegin(int transactionId)
{
    return createTransactionOp(BEGIN, transactionId, NULL, NULL);
}

TransactionOp *trackEnd(int transactionId)
{
    return createTransactionOp(COMMIT, transactionId, NULL, NULL);
}

void printTransactionOp(TransactionOp* transactionOp, FILE* pOut)
{
    if (!transactionOp)
    {
        printf("Invalid TransactionOp pointer.\n");
        return;
    }

    if (!pOut)
    {
        // Nothing to print, just destroy.
        destroyTransactionOp(transactionOp);
        return;
    }

    // Print operation type
    const char* opTypeStr = "UNKNOWN";
    switch (transactionOp->type)
    {
    case BEGIN:
        opTypeStr = "BEGIN";
        break;
    case COMMIT:
        opTypeStr = "COMMIT";
        break;
    case WRITE:
        opTypeStr = "WRITE";
        break;
    case READ:
        opTypeStr = "READ";
        break;
    default:
        break;
    }

    static const char *baseFormat = "\n$$Op: %s\t Tx: %d";
    static const char *objFormat = "\t Obj: %d";
    static const char *writeFormat = " \t wVal: %s";

    char formattedStr[512]; // Ensure it's large enough to fit the full string

    int offset = snprintf(formattedStr, sizeof(formattedStr), baseFormat, opTypeStr, transactionOp->transactionId);

    // Print object ID if it's not a BEGIN or COMMIT operation
    if (transactionOp->type == WRITE || transactionOp->type == READ) {
        offset += snprintf(formattedStr + offset, sizeof(formattedStr) - offset, objFormat, transactionOp->objectId);
    }

    // Print write value if it's a WRITE operation
    if (transactionOp->type == WRITE && transactionOp->writeVal->func != NULL) {
        const char* valueStr = transactionOp->writeVal->func(transactionOp->writeVal->val);
        offset += snprintf(formattedStr + offset, sizeof(formattedStr) - offset, writeFormat, valueStr ? valueStr : "<NULL>");
    }

    // Finally, add the newline and write to output
    snprintf(formattedStr + offset, sizeof(formattedStr) - offset, "$$\n");
    fprintf(pOut, "%s", formattedStr);

    destroyTransactionOp(transactionOp);
}

const char* intToString(const void* val)
{
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", *(int*)val);
    return buffer;
}

const char* floatToString(const void* val)
{
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", *(float*)val);
    return buffer;
}

const char* stringToString(const void* val)
{
    return val;
}