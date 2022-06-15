#ifndef QCVM_LEX_H
#define QCVM_LEX_H 1

/**
 * @defgroup LexC Lexing
 * @{
 */

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QC_SourceLocation{
	QC_Uint32 col, line;
} QC_SourceLocation;

typedef enum QC_TokenKind{
	QC_TOKEN_SPACE,
	QC_TOKEN_NEWLINE,
	QC_TOKEN_ID,
	QC_TOKEN_INT,
	QC_TOKEN_FLOAT,
	QC_TOKEN_STR,
	QC_TOKEN_OP,
	QC_TOKEN_BRACKET,

	QC_TOKEN_KIND_COUNT
} QC_TokenKind;

typedef struct QC_Token{
	QC_TokenKind kind;
	QC_SourceLocation loc;
	QC_StrView str;
} QC_Token;

/**
 * @brief Lex a token of source code
 * @param src Source to lex
 * @param loc Location of the lex start point
 * @param ret Pointer to return token into (Required)
 * @param nextLoc Pointer to return the next location into
 * @returns The source remaining to be lexed or `QC_EMPTY_STRVIEW` on error (\p ret and \p nextLoc unchanged on error)
 */
QCVM_API QC_StrView qcLex(QC_StrView src, QC_SourceLocation loc, QC_Token *ret, QC_SourceLocation *nextLoc);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_LEX_H
