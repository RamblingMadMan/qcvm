#ifndef QCVM_QUAKEC_H
#define QCVM_QUAKEC_H 1

#include "ast.h"
#include "lex.h"

#ifdef __cplusplus
extern "C" {
#endif

enum QC_QuakeCStandard{
	QC_QUAKEC_VANILLA,
	QC_QUAKEC_EXTENDED,

	QC_QUAKEC_STANDARD_COUNT
};

QCVM_API bool qcParseQuakeC(QC_Ast *ast, QC_Uint32 standard, const QC_Token *toks, QC_Uint64 numToks);

#ifdef __cplusplus
}
#endif

#endif // !QCVM_QUAKEC_H
