#include "qcvm/common.h"
#ifndef QCVM_AST_H
#define QCVM_AST_H 1

/**
 * @defgroup AstC AST handling
 * @{
 */

#include "amath.h"
#include "string.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum QC_AstNodeKind{
	QC_AST_NODE_BINDING,
	QC_AST_NODE_LITERAL_TYPE,
	QC_AST_NODE_LITERAL_INT,
	QC_AST_NODE_LITERAL_FLOAT,
	QC_AST_NODE_LITERAL_STR,

	QC_AST_NODE_KIND_COUNT
};

QCVM_PREDEF(QC_Ast);

QCVM_PREDEF(QC_AstNode);
QCVM_PREDEF(QC_AstNode_Binding);
QCVM_PREDEF(QC_AstNode_Literal);
QCVM_PREDEF(QC_AstNode_LiteralType);
QCVM_PREDEF(QC_AstNode_LiteralInt);
QCVM_PREDEF(QC_AstNode_LiteralFloat);
QCVM_PREDEF(QC_AstNode_LiteralVector);
QCVM_PREDEF(QC_AstNode_LiteralStr);

QCVM_API QC_Ast *qcCreateAstA(const QC_Allocator *allocator, QC_TypeSet *ts);

static inline QC_Ast *qcCreateAst(QC_TypeSet *ts){
	return qcCreateAstA(QC_DEFAULT_ALLOC, ts);
}

QCVM_API bool qcDestroyAst(QC_Ast *ast);

QCVM_API const QC_TypeSet *qcAstTypes(const QC_Ast *ast);
QCVM_API const QC_StringBuffer *qcAstStringBuffer(const QC_Ast *ast);

QCVM_API const QC_AstNode_Binding *qcAstCreateBinding(QC_Ast *ast, const QC_Type *type, QC_StrView name, const QC_AstNode *value);

QCVM_API const QC_AstNode_LiteralType *qcAstCreateLiteralType(QC_Ast *ast, const QC_Type *type);
QCVM_API const QC_AstNode_LiteralInt *qcAstCreateLiteralInt(QC_Ast *ast, QC_StrView str, int base);
QCVM_API const QC_AstNode_LiteralFloat *qcAstCreateLiteralFloat(QC_Ast *ast, QC_StrView str, int base);
QCVM_API const QC_AstNode_LiteralVector *qcAstCreateLiteralVector(QC_Ast *ast, QC_Uintptr numElems, const QC_AstNode_Literal *const *elements);
QCVM_API const QC_AstNode_LiteralStr *qcAstCreateLiteralStr(QC_Ast *ast, QC_StrView str);

struct QC_AstNode{
	QC_Uint32 kind; //! The kind of node this handle represents
};

struct QC_AstNode_Binding{
	QCVM_DERIVED(QC_AstNode)
	const QC_Type *type;
	QC_StrView name;
	const QC_AstNode *value; //! Default/initial value, may be `NULL`
};

struct QC_AstNode_Literal{
	QCVM_DERIVED(QC_AstNode)
	const QC_Type *type;
};

struct QC_AstNode_LiteralType{
	QCVM_DERIVED(QC_AstNode_Literal);
	const QC_Type *value;
};

struct QC_AstNode_LiteralInt{
	QCVM_DERIVED(QC_AstNode_Literal);
	QC_Aint value; //! \ref qcClearAint will need to be called on this
};

struct QC_AstNode_LiteralFloat{
	QCVM_DERIVED(QC_AstNode_Literal);
	QC_Afloat value; //! \ref qcClearAfloat will need to be called on this
};

struct QC_AstNode_LiteralVector{
	QCVM_DERIVED(QC_AstNode_Literal);
	const QC_Type *elemType;
	const QC_AstNode_Literal *elems;
	QC_Uint64 numElems;
};

struct QC_AstNode_LiteralStr{
	QCVM_DERIVED(QC_AstNode_Literal);
	QC_StrView value; //! Maps to a string in the ast \ref QC_StringBuffer
};

typedef union QC_AstNodeStorage{
	QC_Uint32 kind;
	QC_AstNode_Binding binding;
	QC_AstNode_Literal literal;
	QC_AstNode_LiteralType literalType;
	QC_AstNode_LiteralInt literalInt;
	QC_AstNode_LiteralFloat literalFloat;
	QC_AstNode_LiteralStr literalStr;
} QC_AstNodeStorage;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_AST_H
