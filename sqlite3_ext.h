#ifndef SQLITE3_EXT_H
#define SQLITE3_EXT_H

#include "sqlite3.h"

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ifndef UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# else
#  define UINT32_TYPE unsigned int
# endif
#endif
#ifndef UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# else
#  define UINT16_TYPE unsigned short int
# endif
#endif
#ifndef INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# else
#  define INT16_TYPE short int
# endif
#endif
#ifndef UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# else
#  define UINT8_TYPE unsigned char
# endif
#endif
#ifndef INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# else
#  define INT8_TYPE signed char
# endif
#endif
#ifndef LONGDOUBLE_TYPE
# define LONGDOUBLE_TYPE long double
#endif

typedef sqlite_int64 i64;          /* 8-byte signed integer */
typedef sqlite_uint64 u64;         /* 8-byte unsigned integer */
typedef UINT32_TYPE u32;           /* 4-byte unsigned integer */
typedef UINT16_TYPE u16;           /* 2-byte unsigned integer */
typedef INT16_TYPE i16;            /* 2-byte signed integer */
typedef UINT8_TYPE u8;             /* 1-byte unsigned integer */
typedef INT8_TYPE i8;              /* 1-byte signed integer */

struct FuncDef;
struct sqlite3_context;
struct CollSeq;
struct Mem;
struct VTable;
struct KeyInfo;
struct SubProgram;
struct Table;
#ifdef SQLITE_ENABLE_CURSOR_HINTS
struct Expr;
#endif

// Definition of VdbeOp
typedef struct VdbeOp {
    u8 opcode;
    signed char p4type;
    u16 p5;
    int p1;
    int p2;
    int p3;
    union {
        int i;
        void *p;
        char *z;
        i64 *pI64;
        double *pReal;
        struct FuncDef *pFunc;
        struct sqlite3_context *pCtx;
        struct CollSeq *pColl;
        struct Mem *pMem;
        struct VTable *pVtab;
        struct KeyInfo *pKeyInfo;
        u32 *ai;
        struct SubProgram *pProgram;
        struct Table *pTab;
#ifdef SQLITE_ENABLE_CURSOR_HINTS
        struct Expr *pExpr;
#endif
    } p4;
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
    char *zComment;
#endif
#ifdef SQLITE_VDBE_COVERAGE
    u32 iSrcLine;
#endif
#if defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
    u64 nExec;
    u64 nCycle;
#endif
} VdbeOp;

const char *sqlite3OpcodeName(int);

#endif /* SQLITE3_EXT_H */
