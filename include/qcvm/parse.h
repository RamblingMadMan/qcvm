#ifndef QCVM_PARSE_H
#define QCVM_PARSE_H 1

#include "qcvm/common.h"
#include "qcvm/lex.h"
#include "qcvm/ast.h"

#ifdef __cplusplus
extern "C" {
#endif

QCVM_PREDEF(QC_Parser);

typedef const QC_AstNode*(*QC_ParseOuterFn)(
    void *user,
    QC_Ast *ast,
    QC_Uintptr numToks, const QC_Token *toks,
    QC_Uintptr nToksRet, const QC_Token **remRet
);

QCVM_API QC_Parser *qcCreateParserA(const QC_Allocator *allocator, QC_ParseOuterFn outerFn, void *user);

QCVM_API bool qcDestroyParser(QC_Parser *parser);

QCVM_API bool qcParserSetOuterFn(QC_Parser *parser, QC_ParseOuterFn outerFn, void *user);

QCVM_API const QC_AstNode *qcParse(
    QC_Parser *parser,
    QC_Ast *ast,
    QC_Uintptr numToks, const QC_Token *toks,
    QC_Uintptr *nToksRet, const QC_Token **remRet
);

#ifdef __cplusplus
}
#endif

#endif // !QCVM_PARSE_H
