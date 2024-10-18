#include "mvtracer.h"
#include "sqlite3_ext.h"

#ifndef SQLITE3TRACEADAPTER_H
#define SQLITE3TRACEADAPTER_H

#ifdef __cplusplus
extern "C"
{
#endif

// checkVdbeOP
typedef const int (*vdbeOpCheckPredicate)(u8);

int checkVdbeOp(VdbeOp *op, vdbeOpCheckPredicate predicate);

// isCursorMovement - detect if an instruction is a cursor movement.
int isCursorMovement(u8 opCode);

// isColumnOp - check if given instruction is a `Column` (read) operation.
int isColumnOp(u8 opCode);

// isRowId - check if given instruction is a `RowId` operation.
int isRowId(u8 opCode);



/**
 * Main driver
 * -----------
 * Stateful trace logger that catches EVERY read and write operation.
 * In SQLite3, a record read is done by and only by the `column` VDBE
 * operation on the record the specified cursor is pointing at.
 * While `column` does not tell which record is being read, we can
 * maintain the cursor state and know the recordId from other instruction.
 */
void sqlite3TraceInterceptor(VdbeOp *pOp);


/**
* Tracer state management struct. Mainly used for tracking
* read operations since it involves multiple instructions.
*/
typedef struct {
 // Current read operation.
 TransactionOp *readOp;

 // Current write operation.
 TransactionOp *writeOp;

 // Current rowId.
 int rowId;
} TraceState;

TraceState* initTraceState();

// Sets the rowId of the current state
void setRowId(int rowId);

// Intercepts write inside an "Insert" opcode
void interceptWrite(VdbeOp *pOp, int recordId, char* val);

// Enables trace output to stdout.
void enableTraceOutput();

#ifdef __cplusplus
}
#endif

#endif //SQLITE3TRACEADAPTER_H
