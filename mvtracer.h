#ifndef MVTRACER_H
#define MVTRACER_H

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
* Prints a transaction and then destroy the transactionOp object.
*/
void printTransactionOp(TransactionOp* transactionOp);

// ------------ Default ToString Functions ----------
const char* intToString(const void* val);

const char* floatToString(const void* val);

const char* stringToString(const void* val);
// --------------------------------------------------

#ifdef __cplusplus
}
#endif
