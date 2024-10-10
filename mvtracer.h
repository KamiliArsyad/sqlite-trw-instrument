#ifndef MVTRACER_H
#define MVTRACER_H
#include <stdio.h>

#endif //MVTRACER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef const char* (*valToStringFunc)(const void*);

typedef struct
{
    const void* val;
    valToStringFunc func;
} Value;

/**
 * Types of transaction operations.
 */
typedef enum
{
    BEGIN,
    COMMIT,
    WRITE,
    READ
} OpType;

typedef struct
{
    OpType type;
    int transactionId;
    unsigned long objectId;
    Value* writeVal;
} TransactionOp;

/**
*
*/
TransactionOp *trackRead(int transactionId, unsigned long objectId);

/**
*
*/
TransactionOp *trackWrite(int transactionId, unsigned long objectId, Value *value);

TransactionOp *trackBegin(int transactionId);

TransactionOp *trackEnd(int transactionId);

Value* createValue(const void* val, valToStringFunc func);

/**
 * WARNING: This operation **REMOVES** the object in the input.
 * Prints to `pOut` a transaction in the format:
 * Op: <Operation> \t Tx: <Transaction> \t [obj: <Object ID>] [wVal: <write value>]
 * After printing, it destroys the transactionOp object to prevent memory leaks.
 */
void printTransactionOp(TransactionOp* transactionOp, FILE *pOut);

// ------------ Default ToString Functions ----------
const char* intToString(const void* val);

const char* floatToString(const void* val);

const char* stringToString(const void* val);
// --------------------------------------------------

#ifdef __cplusplus
}
#endif
