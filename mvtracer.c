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
    transactionOp->writeVal = writeVal == NULL ? NULL : *writeVal;

    return transactionOp;
}

static TransactionOp* createTransactionOp(const OpType type, const int transactionId, const unsigned long objectId)
{
    return createTransactionOp(type, transactionId, objectId, createValue(NULL, NULL));
}


static void destroyTransactionOp(TransactionOp* transactionOp)
{
    if (transactionOp)
    {
        if (transactionOp->writeVal.val != NULL)
        {
            destroyValue(&(transactionOp->writeVal));
        }
        free(transactionOp);
    }
}

// Public API
TransactionOp* trackRead(const int transactionId, const unsigned long objectId)
{
    return createTransactionOp(READ, transactionId, objectId);
}

TransactionOp* trackWrite(const int transactionId, const unsigned long objectId, const Value *value)
{
    return createTransactionOp(WRITE, transactionId, objectId, value);
}

void printTransactionOp(TransactionOp* transactionOp)
{
    if (!transactionOp)
    {
        printf("Invalid TransactionOp pointer.\n");
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

    printf("Transaction Operation:\n");
    printf("  Type: %s\n", opTypeStr);
    printf("  Transaction ID: %d\n", transactionOp->transactionId);
    printf("  Object ID: %lu\n", transactionOp->objectId);

    // Print write value if it's a WRITE operation
    if (transactionOp->type == WRITE && transactionOp->writeVal.func != NULL)
    {
        const char* valueStr = transactionOp->writeVal.func(transactionOp->writeVal.val);
        printf("  Write Value: %s\n", valueStr ? valueStr : "<NULL>");
    }

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