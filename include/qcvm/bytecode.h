#ifndef QCVM_BYTECODE_H
#define QCVM_BYTECODE_H 1

/**
 * @defgroup Bytecode Bytecode
 * @{
 */

#include "common.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Bytecode data types
*/
enum QC_ByteCodeType{
	// Vanilla types
	QC_BYTECODE_TYPE_VOID		= 0x0,
	QC_BYTECODE_TYPE_STRING		= 0x1,
	QC_BYTECODE_TYPE_FLOAT32	= 0x2,
	QC_BYTECODE_TYPE_FLOAT		= QC_BYTECODE_TYPE_FLOAT32,
	QC_BYTECODE_TYPE_VEC3		= 0x3,
	QC_BYTECODE_TYPE_VECTOR		= QC_BYTECODE_TYPE_VEC3,
	QC_BYTECODE_TYPE_ENTITY		= 0x4,
	QC_BYTECODE_TYPE_FIELD		= 0x5,
	QC_BYTECODE_TYPE_FUNC		= 0x6,

	// Extended types
	QC_BYTECODE_TYPE_INT32		= 0x7,
	QC_BYTECODE_TYPE_UINT32		= 0x8,
	QC_BYTECODE_TYPE_INT64		= 0x9,
	QC_BYTECODE_TYPE_UINT64		= 0xA,
	QC_BYTECODE_TYPE_FLOAT64	= 0xB,
	QC_BYTECODE_TYPE_DOUBLE		= QC_BYTECODE_TYPE_FLOAT64,

	// FTEQCC-only types
	QC_BYTECODE_TYPE_VARIANT	= 0xC,
	QC_BYTECODE_TYPE_STRUCT		= 0xD,
	QC_BYTECODE_TYPE_UNION		= 0xE,
	QC_BYTECODE_TYPE_ACCESSOR	= 0xF,
	QC_BYTECODE_TYPE_ENUM		= 0x10,
	QC_BYTECODE_TYPE_BOOL		= 0x11,

	QC_BYTECODE_TYPE_COUNT,
};

/**
 * @brief Extended bytecode data types
 * @note These types are masked out by `(type & 0xffff)` when `(type & 0xffff0000)`
 */
enum QC_ByteCodeTypeExt{
	// fundamental types
	QC_BYTECODE_TYPE_EXT_INT8 = 0x0,
	QC_BYTECODE_TYPE_EXT_UINT8 = 0x1,
	QC_BYTECODE_TYPE_EXT_INT16 = 0x2,
	QC_BYTECODE_TYPE_EXT_UINT16 = 0x3,

	// composite types
	QC_BYTECODE_TYPE_EXT_VEC2 = 0x4,
	QC_BYTECODE_TYPE_EXT_VEC4 = 0x5,

	QC_BYTECODE_TYPE_EXT_COUNT
};

static inline bool qcByteCodeTypeIsExt(QC_Uint32 type){ return (type & 0xffff0000); }

static inline QC_ByteCodeType qcByteCodeType(QC_Uint32 type){ return (QC_ByteCodeType)(type); }
static inline QC_ByteCodeTypeExt qcByteCodeTypeExt(QC_Uint32 type){ return (QC_ByteCodeTypeExt)(type & 0xffff); }

/**
 * @brief Get the size (in number of array elements) of a bytecode type
 * @param type Type code
 * @returns Size of type referred to by \p type
 */
QCVM_API QC_Uint32 qcByteCodeTypeSize(QC_Uint32 type);

/**
 * @brief Opaque handle to loaded bytecode
 */
QCVM_PREDEF(QC_ByteCode);

/**
 * @brief Opaque handle to a bytecode builder
 */
QCVM_PREDEF(QC_ByteCodeBuilder);

/**
 * @brief Bytecode header
 */
QCVM_PREDEF(QC_ByteCodeHeader);

QCVM_PREDEF(QC_ByteCodeStatement16);
QCVM_PREDEF(QC_ByteCodeStatement32);

/**
 * @brief Bytecode statement
 */
typedef QC_ByteCodeStatement32 QC_ByteCodeStatement;

QCVM_PREDEF(QC_ByteCodeDef16);
QCVM_PREDEF(QC_ByteCodeDef32);

/**
 * @brief Bytecode definition
 */
typedef QC_ByteCodeDef32 QC_ByteCodeDef;

QCVM_PREDEF(QC_ByteCodeField16);
QCVM_PREDEF(QC_ByteCodeField32);

/**
 * @brief Bytecode field
 */
typedef QC_ByteCodeField32 QC_ByteCodeField;

/**
 * @brief Bytecode function
 */
QCVM_PREDEF(QC_ByteCodeFunction);

/**
 * @brief Try to create bytecode from in-memory representation
 * @param allocator Allocator to use for all allocations
 * @param bytes Pointer to the bytecode
 * @param len Size in bytes \p bytes points to
 * @returns Loaded bytecode or `NULL` on error
 * @see qcDestroyByteCode
 */
QCVM_API QC_ByteCode *qcCreateByteCodeA(const QC_Allocator *allocator, const char *bytes, size_t len);

//! @see qcCreateByteCodeA
static inline QC_ByteCode *qcCreateByteCode(const char *bytes, size_t len){
	return qcCreateByteCodeA(QC_DEFAULT_ALLOC, bytes, len);
}

/**
 * @brief Free previously loaded bytecode
 * @param bc Bytecode to free
 * @returns Whether the bytecode was successfully freed
 * @see qcCreateByteCode
 */
QCVM_API bool qcDestroyByteCode(QC_ByteCode *bc);

QCVM_API QC_Uintptr qcByteCodeStringsSize(const QC_ByteCode *bc);
QCVM_API const char *qcByteCodeStrings(const QC_ByteCode *bc);

QCVM_API QC_Uintptr qcByteCodeNumStatements(const QC_ByteCode *bc);
QCVM_API const QC_ByteCodeStatement *qcByteCodeStatements(const QC_ByteCode *bc);

QCVM_API QC_Uintptr qcByteCodeNumDefs(const QC_ByteCode *bc);
QCVM_API const QC_ByteCodeDef *qcByteCodeDefs(const QC_ByteCode *bc);

QCVM_API QC_Uintptr qcByteCodeNumFields(const QC_ByteCode *bc);
QCVM_API const QC_ByteCodeField *qcByteCodeFields(const QC_ByteCode *bc);

QCVM_API QC_Uintptr qcByteCodeNumFunctions(const QC_ByteCode *bc);
QCVM_API const QC_ByteCodeFunction *qcByteCodeFunctions(const QC_ByteCode *bc);

QCVM_API QC_Uintptr qcByteCodeNumGlobals(const QC_ByteCode *bc);
QCVM_API const QC_Value *qcByteCodeGlobals(const QC_ByteCode *bc);

/**
 * @brief Create a new bytecode builder
 * @returns A newly created bytecode builder or `NULL` on error
 * @see qcDestroyBuilder
 */
QCVM_API QC_ByteCodeBuilder *qcCreateBuilder();

/**
 * @brief Free a previously created bytecode builder
 * @note This does not free any bytecode created with \ref qcBuilderEmit
 * @param builder The builder to destroy
 * @returns Whether \p builder was successfully destroyed
 * @see qcCreateBuilder
 */
QCVM_API bool qcDestroyBuilder(QC_ByteCodeBuilder *builder);

/**
 * @brief Emit the current state of the builder as bytecode
 * @note The return value should be freed with \ref qcDestroyByteCode
 * @param builder Builder to emit from
 * @returns Newly created bytecode or `NULL` on error
 */
QCVM_API QC_ByteCode *qcBuilderEmit(QC_ByteCodeBuilder *builder);

/**
 * @brief Add a single statement to the builder
 * @param builder Builder to add the statement to
 * @param stmt Statement to add to \p builder
 * @returns The index of the newly created statement or `UINT32_MAX` on error
 */
QCVM_API QC_Uintptr qcBuilderAddStatement(QC_ByteCodeBuilder *builder, const QC_ByteCodeStatement *stmt);

/**
 * @brief Add a single definition to the builder
 * @param builder Builder to add the definition to
 * @param def Definition to add to \p builder
 * @returns The index of the newly created definition or `UINT32_MAX` on error
 */
QCVM_API QC_Uintptr qcBuilderAddDef(QC_ByteCodeBuilder *builder, const QC_ByteCodeDef *def);

/**
 * @brief Add a single field to the builder
 * @param builder Builder to add the field to
 * @param field Field to add to \p builder
 * @returns The index of the newly created field or `UINT32_MAX` on error
 */
QCVM_API QC_Uintptr qcBuilderAddField(QC_ByteCodeBuilder *builder, const QC_ByteCodeField *field);

/**
 * @brief Add a function definition to a bytecode builder
 * @param builder Builder to add the function to
 * @param fn Function to add to \p builder
 * @returns The index of the newly created function or `UINT32_MAX` on error
 */
QCVM_API QC_Uintptr qcBuilderAddFunction(QC_ByteCodeBuilder *builder, const QC_ByteCodeFunction *fn);

/**
 * @brief Add a global definition index to a bytecode builder
 * @param builder Builder to add the global index to
 * @param value Global value to add to \p builder
 * @returns The index of the newly created global or `UINT32_MAX` on error
 */
QCVM_API QC_Uintptr qcBuilderAddGlobal(QC_ByteCodeBuilder *builder, QC_Value value);

/**
 * @brief Add a string to a bytecode builder
 * @param builder Builder to add the string to
 * @param str Pointer to the string
 * @param len Length of the string
 * @returns The offset in the string buffer of the added string or `UINT32_MAX` on error
 */
QCVM_API QC_Uintptr qcBuilderAddString(QC_ByteCodeBuilder *builder, const char *str, size_t len);

/**
 * @brief Bytecode sections
 */
enum QC_Section{
	QC_SECTION_STATEMENTS	= 0x0,
	QC_SECTION_DEFS			= 0x1,
	QC_SECTION_FIELDS		= 0x2,
	QC_SECTION_FUNCTIONS	= 0x3,
	QC_SECTION_STRINGS		= 0x4,
	QC_SECTION_GLOBALS		= 0x5,

	QC_SECTION_COUNT
};

struct QC_ByteCodeHeader{
	QC_Uint32 ver; // must be 0x6
	QC_Uint16 crc;
	QC_Uint16 skip; // should be 0x0
	QC_Uint32 sectionData[QC_SECTION_COUNT * 2];
	QC_Uint32 entityFields;
};

struct QC_ByteCodeStatement16{
	QC_Uint16 op;
	QC_Uint16 a, b, c;
};

struct QC_ByteCodeStatement32{
	QC_Uint32 op;
	QC_Uint32 a, b, c;
};

static_assert(sizeof(QC_ByteCodeStatement32) == 16, "misaligned QC_ByteCodeStatement32");

struct QC_ByteCodeDef16{
	QC_Uint16 type;
	QC_Uint16 globalIdx;
	QC_Uint32 nameIdx;
};

static_assert(sizeof(QC_ByteCodeDef16) == 8, "misaligned QC_ByteCodeDef16");

struct QC_ByteCodeDef32{
	QC_Uint32 type;
	QC_Uint32 globalIdx;
	QC_Uint32 nameIdx;
};

struct QC_ByteCodeField16{
	QC_Uint16 type;
	QC_Uint16 offset;
	QC_Uint32 nameIdx;
};

static_assert(sizeof(QC_ByteCodeField16) == 8, "misaligned QC_ByteCodeField16");

struct QC_ByteCodeField32{
	QC_Uint32 type;
	QC_Uint32 offset;
	QC_Uint32 nameIdx;
};

struct QC_ByteCodeFunction{
	QC_Int32 entryPoint;
	QC_Int32 localIdx;
	QC_Uint32 numLocals;
	QC_Int32 profile; // must be 0 when loaded
	QC_Int32 nameIdx;
	QC_Int32 fileIdx;
	QC_Int32 numArgs;
	int8_t argSizes[8];
};

static_assert(sizeof(QC_ByteCodeFunction) == 36, "misaligned QC_ByteCodeFunction");

/**
 * @brief Bytecode ops
 */
enum QC_Op{
	// -- Vanilla opcodes --

	// Misc
	QC_OP_DONE = 0x00,
	QC_OP_STATE = 0x3C,
	QC_OP_GOTO = 0x3D,
	QC_OP_ADDRESS = 0X1E,
	QC_OP_RETURN = 0X2B,

	// Arithmetic
	QC_OP_MUL_F = 0X01,
	QC_OP_MUL_V = 0X02,
	QC_OP_MUL_FV = 0X03,
	QC_OP_MUL_VF = 0X04,
	QC_OP_DIV_F = 0X05,
	QC_OP_ADD_F = 0X06,
	QC_OP_ADD_V = 0X07,
	QC_OP_SUB_F = 0X08,
	QC_OP_SUB_V = 0X09,

	// Comparison
	QC_OP_EQ_F = 0X0A,
	QC_OP_EQ_V = 0X0B,
	QC_OP_EQ_S = 0X0C,
	QC_OP_EQ_E = 0X0D,
	QC_OP_EQ_FNC = 0X0E,
	QC_OP_NE_F = 0X0F,
	QC_OP_NE_V = 0X10,
	QC_OP_NE_S = 0X11,
	QC_OP_NE_E = 0X12,
	QC_OP_NE_FNC = 0X13,
	QC_OP_LE = 0X14,
	QC_OP_GE = 0X15,
	QC_OP_LT = 0X16,
	QC_OP_GT = 0X17,

	// Loading/Storing
	QC_OP_LOAD_F = 0x18,
	QC_OP_LOAD_V = 0x19,
	QC_OP_LOAD_S = 0x1A,
	QC_OP_LOAD_ENT = 0x1B,
	QC_OP_LOAD_FLD = 0x1C,
	QC_OP_LOAD_FNC = 0x1D,
	QC_OP_STORE_F = 0x1F,
	QC_OP_STORE_V = 0x20,
	QC_OP_STORE_S = 0x21,
	QC_OP_STORE_ENT = 0x22,
	QC_OP_STORE_FLD = 0x23,
	QC_OP_STORE_FNC = 0x24,
	QC_OP_STOREP_F = 0x25,
	QC_OP_STOREP_V = 0x26,
	QC_OP_STOREP_S = 0x27,
	QC_OP_STOREP_ENT = 0x28,
	QC_OP_STOREP_FLD = 0x29,
	QC_OP_STOREP_FNC = 0x2A,

	// If, Not
	QC_OP_NOT_F = 0X2C,
	QC_OP_NOT_V = 0X2D,
	QC_OP_NOT_S = 0X2E,
	QC_OP_NOT_ENT = 0X2F,
	QC_OP_NOT_FNC = 0X30,
	QC_OP_IF = 0X31,
	QC_OP_IFNOT = 0X32,

	// Function Calls
	QC_OP_CALL0 = 0X33,
	QC_OP_CALL1 = 0X34,
	QC_OP_CALL2 = 0X35,
	QC_OP_CALL3 = 0X36,
	QC_OP_CALL4 = 0X37,
	QC_OP_CALL5 = 0X38,
	QC_OP_CALL6 = 0X39,
	QC_OP_CALL7 = 0X3A,
	QC_OP_CALL8 = 0X3B,

	// Boolean Operations
	QC_OP_AND = 0X3E,
	QC_OP_OR = 0X3F,
	QC_OP_BITAND = 0X40,
	QC_OP_BITOR = 0X41,

	// -- Hexen 2 Extensions --

	QC_OP_MULSTORE_F = 0x42,
	QC_OP_MULSTORE_VF,
	QC_OP_MULSTOREP_F,
	QC_OP_MULSTOREP_VF,

	QC_OP_DIVSTORE_F = 0x46,
	QC_OP_DIVSTOREP_F,

	QC_OP_ADDSTORE_F = 0x48,
	QC_OP_ADDSTORE_V,
	QC_OP_ADDSTOREP_F,
	QC_OP_ADDSTOREP_V,

	QC_OP_SUBSTORE_F = 0x4C,
	QC_OP_SUBSTORE_V,
	QC_OP_SUBSTOREP_F,
	QC_OP_SUBSTOREP_V,

	QC_OP_FETCH_GBL_F = 0x50,
	QC_OP_FETCH_GBL_V,
	QC_OP_FETCH_GBL_S,
	QC_OP_FETCH_GBL_E,
	QC_OP_FETCH_GBL_FNC,

	QC_OP_CSTATE = 0x55,
	QC_OP_CWSTATE,

	QC_OP_THINKTIME = 0x57, // shortcut for OPA.nextthink=time+OPB

	QC_OP_BITSETSTORE_F = 0x58,
	QC_OP_BITSETSTOREP_F,
	QC_OP_BITCLRSTORE_F,
	QC_OP_BITCLRSTOREP_F,

	QC_OP_RAND0 = 0x5C,	// OPC = random()
	QC_OP_RAND1,			// OPC = random()*OPA
	QC_OP_RAND2,			// OPC = random()*(OPB-OPA)+OPA
	QC_OP_RANDV0,			// 3d/box versions of the above.
	QC_OP_RANDV1,
	QC_OP_RANDV2,

	QC_OP_SWITCH_F = 0x62, // switchref=OPA; PC += OPB   --- the jump allows the jump table (such as it is) to be inserted after the block.
	QC_OP_SWITCH_V,
	QC_OP_SWITCH_S,
	QC_OP_SWITCH_E,
	QC_OP_SWITCH_FNC,

	QC_OP_CASE = 0x67,		// if (OPA===switchref) PC += OPB
	QC_OP_CASERANGE,		// if (OPA<=switchref&&switchref<=OPB) PC += OPC

	// Hexen 2 calls
	QC_OP_CALL1H = 0x69,	// OFS_PARM0=OPB
	QC_OP_CALL2H,			// OFS_PARM0,1=OPB,OPC
	QC_OP_CALL3H,			// no extra args
	QC_OP_CALL4H,
	QC_OP_CALL5H,
	QC_OP_CALL6H,
	QC_OP_CALL7H,
	QC_OP_CALL8H,

	QC_OP_STORE_I = 0x71,
	QC_OP_STORE_IF,		// OPB.f = (float)OPA.i (makes more sense when written as a->b)
	QC_OP_STORE_FI,		// OPB.i = (int)OPA.f

	QC_OP_ADD_I = 0x74,
	QC_OP_ADD_FI,				// OPC.f = OPA.f + OPB.i
	QC_OP_ADD_IF,				// OPC.f = OPA.i + OPB.f	-- redundant...

	QC_OP_SUB_I = 0x77,			// OPC.i = OPA.i - OPB.i
	QC_OP_SUB_FI,				// OPC.f = OPA.f - OPB.i
	QC_OP_SUB_IF,				// OPC.f = OPA.i - OPB.f

	QC_OP_CONV_ITOF = 0x7A,		// OPC.f=(float)OPA.i -- useful mostly so decompilers don't do weird stuff.
	QC_OP_CONV_FTOI,			// OPC.i=(int)OPA.f
	QC_OP_LOADP_ITOF,			// OPC.f=(float)(*OPA).i	-- fixme: rename to LOADP_ITOF
	QC_OP_LOADP_FTOI,			// OPC.i=(int)(*OPA).f
	QC_OP_LOAD_I,
	QC_OP_STOREP_I,
	QC_OP_STOREP_IF,
	QC_OP_STOREP_FI,

	QC_OP_BITAND_I = 0x82,
	QC_OP_BITOR_I,

	QC_OP_MUL_I = 0x84,
	QC_OP_DIV_I,
	QC_OP_EQ_I,
	QC_OP_NE_I,

	QC_OP_IFNOT_S = 0x88, // compares string empty, rather than just null.
	QC_OP_IF_S,

	QC_OP_NOT_I = 0x8A,

	QC_OP_DIV_VF = 0x8B,

	QC_OP_BITXOR_I = 0x8C,
	QC_OP_RSHIFT_I,
	QC_OP_LSHIFT_I,

	QC_OP_GLOBALADDRESS = 0x8F,		// C.p = &A + B.i*4
	QC_OP_ADD_PIW,					// C.p = A.p + B.i*4

	QC_OP_LOADA_F = 0x91,
	QC_OP_LOADA_V,
	QC_OP_LOADA_S,
	QC_OP_LOADA_ENT,
	QC_OP_LOADA_FLD,
	QC_OP_LOADA_FNC,
	QC_OP_LOADA_I,

	QC_OP_STORE_P = 0x98,
	QC_OP_LOAD_P,

	QC_OP_LOADP_F = 0x9A,
	QC_OP_LOADP_V,
	QC_OP_LOADP_S,
	QC_OP_LOADP_ENT,
	QC_OP_LOADP_FLD,
	QC_OP_LOADP_FNC,
	QC_OP_LOADP_I,

	QC_OP_LE_I = 0xA1,
	QC_OP_GE_I,
	QC_OP_LT_I,
	QC_OP_GT_I,

	QC_OP_LE_IF = 0xA5,
	QC_OP_GE_IF,
	QC_OP_LT_IF,
	QC_OP_GT_IF,

	QC_OP_LE_FI = 0xA9,
	QC_OP_GE_FI,
	QC_OP_LT_FI,
	QC_OP_GT_FI,

	QC_OP_EQ_IF = 0xAD,
	QC_OP_EQ_FI,

	//string manipulation.
	QC_OP_ADD_SF = 0xAF,	// (char*)c = (char*)a + (float)b    add_fi->i
	QC_OP_SUB_S,			// (float)c = (char*)a - (char*)b    sub_ii->f
	QC_OP_STOREP_C,			// (float)c = *(char*)b = (float)a
	QC_OP_LOADP_C,			// (float)c = *(char*)

	QC_OP_MUL_IF = 0xB3,
	QC_OP_MUL_FI,
	QC_OP_MUL_VI,
	QC_OP_MUL_IV,
	QC_OP_DIV_IF,
	QC_OP_DIV_FI,
	QC_OP_BITAND_IF,
	QC_OP_BITOR_IF,
	QC_OP_BITAND_FI,
	QC_OP_BITOR_FI,
	QC_OP_AND_I,
	QC_OP_OR_I,
	QC_OP_AND_IF,
	QC_OP_OR_IF,
	QC_OP_AND_FI,
	QC_OP_OR_FI,
	QC_OP_NE_IF,
	QC_OP_NE_FI,

	// -- FTE Extensions --

	// fte doesn't really model two separate pointer types. these are thus special-case things for array access only.
	QC_OP_GSTOREP_I = 0xC5,
	QC_OP_GSTOREP_F,
	QC_OP_GSTOREP_ENT,
	QC_OP_GSTOREP_FLD,
	QC_OP_GSTOREP_S,
	QC_OP_GSTOREP_FNC,
	QC_OP_GSTOREP_V,
	QC_OP_GADDRESS, // poorly defined opcode, which makes it too unreliable to actually use.
	QC_OP_GLOAD_I,
	QC_OP_GLOAD_F,
	QC_OP_GLOAD_FLD,
	QC_OP_GLOAD_ENT,
	QC_OP_GLOAD_S,
	QC_OP_GLOAD_FNC,

	QC_OP_BOUNDCHECK = 0xD3,
	QC_OP_UNUSED,	// used to be STOREP_P, which is now emulated with STOREP_I, fteqcc nor fte generated it
	QC_OP_PUSH,		// push 4octets onto the local-stack (which is ALWAYS poped on function return). Returns a pointer.
	QC_OP_POP,		// pop those ones that were pushed (don't over do it). Needs assembler.

	QC_OP_SWITCH_I = 0xD7,
	QC_OP_GLOAD_V,
	QC_OP_IF_F,		// compares as an actual float, instead of treating -0 as positive.
	QC_OP_IFNOT_F,

	// fte r5697+
	QC_OP_STOREF_V = 0xDB,	// 3 elements...
	QC_OP_STOREF_F,			// 1 fpu element...
	QC_OP_STOREF_S,			// 1 string reference
	QC_OP_STOREF_I,			// 1 non-string reference/int

	// fte r5744+
	QC_OP_STOREP_B,	// ((char*)b)[(int)c] = (int)a
	QC_OP_LOADP_B,	// (int)c = *(char*)

	// fte r5768+
	// opcodes for 32bit uints
	QC_OP_LE_U,		// aka GT
	QC_OP_LT_U,		// aka GE
	QC_OP_DIV_U,	// don't need mul+add+sub
	QC_OP_RSHIFT_U,	// lshift is the same for signed+unsigned

	// opcodes for 64bit ints
	QC_OP_ADD_I64,
	QC_OP_SUB_I64,
	QC_OP_MUL_I64,
	QC_OP_DIV_I64,
	QC_OP_BITAND_I64,
	QC_OP_BITOR_I64,
	QC_OP_BITXOR_I64,
	QC_OP_LSHIFT_I64I,
	QC_OP_RSHIFT_I64I,
	QC_OP_LE_I64,		// aka GT
	QC_OP_LT_I64,		// aka GE
	QC_OP_EQ_I64,
	QC_OP_NE_I64,
	// extra opcodes for 64bit uints
	QC_OP_LE_U64,		// aka GT
	QC_OP_LT_U64,		// aka GE
	QC_OP_DIV_U64,
	QC_OP_RSHIFT_U64I,

	// general 64bitness
	QC_OP_STORE_I64,
	QC_OP_STOREP_I64,
	QC_OP_STOREF_I64,
	QC_OP_LOAD_I64,
	QC_OP_LOADA_I64,
	QC_OP_LOADP_I64,
	// various conversions for our 64bit types (yay type promotion)
	QC_OP_CONV_UI64,	// zero extend
	QC_OP_CONV_II64,	// sign extend
	QC_OP_CONV_I64I,	// truncate
	QC_OP_CONV_FD,		// extension
	QC_OP_CONV_DF,		// truncation
	QC_OP_CONV_I64F,	// logically a promotion (always signed)
	QC_OP_CONV_FI64,	// demotion (always signed)
	QC_OP_CONV_I64D,	// 'promotion' (always signed)
	QC_OP_CONV_DI64,	// demotion (always signed)

	// opcodes for doubles.
	QC_OP_ADD_D,
	QC_OP_SUB_D,
	QC_OP_MUL_D,
	QC_OP_DIV_D,
	QC_OP_LE_D,
	QC_OP_LT_D,
	QC_OP_EQ_D,
	QC_OP_NE_D,

	QC_OP_COUNT
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_BYTECODE_H
