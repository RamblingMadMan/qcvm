#ifndef QCVM_AST_H
#define QCVM_AST_H 1

/**
 * @defgroup AstC AST Handling
 * @{
 */

#include "amath.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

enum QC_AstNodeKind{
	QC_AST_NODE_BINDING,
	QC_AST_NODE_LITERAL_TYPE,
	QC_AST_NODE_LITERAL_INT,
	QC_AST_NODE_LITERAL_FLOAT,
	QC_AST_NODE_LITERAL_STR,
};

QCVM_PREDEF(QC_Ast);
QCVM_PREDEF(QC_AstType);
QCVM_PREDEF(QC_AstTypeSet);
QCVM_PREDEF(QC_AstNode);
QCVM_PREDEF(QC_AstNode_Binding);
QCVM_PREDEF(QC_AstNode_Literal);
QCVM_PREDEF(QC_AstNode_LiteralType);
QCVM_PREDEF(QC_AstNode_LiteralInt);
QCVM_PREDEF(QC_AstNode_LiteralFloat);
QCVM_PREDEF(QC_AstNode_LiteralStr);

QCVM_API QC_AstTypeSet *qcCreateAstTypeSetA(const QC_Allocator *allocator);

static inline QC_AstTypeSet *qcCreateAstTypeSet(){
	return qcCreateAstTypeSetA(QC_DEFAULT_ALLOC);
}

QCVM_API bool qcDestroyAstTypeSet(QC_AstTypeSet *types);

QCVM_API QC_Ast *qcCreateAstA(const QC_Allocator *allocator, QC_AstTypeSet *types);

static inline QC_Ast *qcCreateAst(QC_AstTypeSet *types){
	return qcCreateAstA(QC_DEFAULT_ALLOC, types);
}

QCVM_API bool qcDestroyAst(QC_Ast *ast);

QCVM_API const QC_AstTypeSet *qcAstTypes(const QC_Ast *ast);
QCVM_API const QC_StringBuffer *qcAstStringBuffer(const QC_Ast *ast);

struct QC_AstType{
	QC_Uint32 id;
};

struct QC_AstNode{
	QC_Uint32 kind; //! The kind of node this handle represents
};

struct QC_AstNode_Binding{
	QCVM_DERIVED(QC_AstNode)
	const QC_AstType *type;
	QC_StrView name;
	const QC_AstNode *value; //! Default/initial value, may be `NULL`
};

struct QC_AstNode_Literal{
	QCVM_DERIVED(QC_AstNode)
	const QC_AstNode *type;
};

struct QC_AstNode_LiteralType{
	QCVM_DERIVED(QC_AstNode_Literal);
	const QC_AstType *value;
};

struct QC_AstNode_LiteralInt{
	QCVM_DERIVED(QC_AstNode_Literal);
	QC_Aint value; //! \ref qcClearAint will need to be called on this
};

struct QC_AstNode_LiteralFloat{
	QCVM_DERIVED(QC_AstNode_Literal);
	QC_Afloat value; //! \ref qcClearAfloat will need to be called on this
};

struct QC_AstNode_LiteralStr{
	QCVM_DERIVED(QC_AstNode_Literal);

	//! maps to a string in the ast \ref QC_StringBuffer
	QC_String value;
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
