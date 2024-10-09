#ifndef MVTRACER_H
#define MVTRACER_H

#endif //MVTRACER_H

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
    Value writeVal;
} TransactionOp;

/**
*
*/
TransactionOp *trackRead(int transactionId, unsigned long objectId);

/**
*
*/
TransactionOp *trackWrite(int transactionId, unsigned long objectId, Value* value);

/**
* WARNING: This operation **REMOVES** the object in the input.
* Prints a transaction and then removes
*/
void printTransactionOp(TransactionOp* transactionOp);

// ------------ Default ToString Functions ----------
const char* intToString(const void* val);

const char* floatToString(const void* val);

const char* stringToString(const void* val);
// --------------------------------------------------