#include "qcvm/lex.h"

#include <cctype>

inline QC_StrView qcvmLexResult(
	QC_TokenKind kind,
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret
){
	const auto strLen = uintptr_t(it) - uintptr_t(beg);
	const auto remLen = uintptr_t(end) - uintptr_t(it);

	*ret = QC_Token{
		.kind = kind,
		.loc = startLoc,
		.str = QC_StrView{ beg, strLen }
	};

	return QC_StrView{ it, remLen };
}

// called after the first alphabetic character, underscore or '$'
inline QC_StrView qcLexId(
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	while(it < end && ((*it == '_') || std::isalnum(*it))){
		++nextLoc->col;
		++it;
	}

	return qcvmLexResult(QC_TOKEN_ID, it, beg, end, startLoc, ret);
}

// called after the decimal point
inline QC_StrView qcLexFloat(
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	while(it < end){
		if(*it == '.'){
			qcLogError("multiple decimal places encountered in floating-point literal");
			return QC_EMPTY_STRVIEW;
		}
		else if(!std::isdigit(*it)){
			break;
		}

		++it;
		++nextLoc->col;
	}

	return qcvmLexResult(QC_TOKEN_FLOAT, it, beg, end, startLoc, ret);
}

// called after the '0x'
inline QC_StrView qcLexIntHex(
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	while(it < end && std::isxdigit(*it)){
		++nextLoc->col;
		++it;
	}

	return qcvmLexResult(QC_TOKEN_INT, it, beg, end, startLoc, ret);
}

// called after the first digit encountered
inline QC_StrView qcLexInt(
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	if(it != end && (*it == 'x' || *it == 'X')){
		++nextLoc->col;
		return qcLexIntHex(it + 1, beg, end, startLoc, ret, nextLoc);
	}

	while(it < end){
		if(*it == '.'){
			++nextLoc->col;
			return qcLexFloat(it + 1, beg, end, startLoc, ret, nextLoc);
		}
		else if(!std::isdigit(*it)){
			break;
		}

		++nextLoc->col;
		++it;
	}

	return qcvmLexResult(QC_TOKEN_INT, it, beg, end, startLoc, ret);
}

inline constexpr bool qcvmIsOpChar(char c){
	switch(c){
		case '+':
		case '-':
		case '&':
		case '|':
		case '=':
		case '!':
		case '~':
		case '%':
		case '^':
		case '*':
		case '/':
		case '<':
		case '>':
		case '?':
		case ':':
		case ',':
		case '#':
		case ';':
			return true;

		default:
			return false;
	}
}

// called after initial operator char or comma
inline QC_StrView qcLexOp(
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	if(*beg == ','){
		return qcvmLexResult(QC_TOKEN_OP, it, beg, end, startLoc, ret);
	}

	char ch;

	while(it < end && qcvmIsOpChar(ch = *it)){
		if(ch == ',' || ch == '#' || ch == ';'){
			break;
		}

		++it;
		++nextLoc->col;
	}

	return qcvmLexResult(QC_TOKEN_OP, it, beg, end, startLoc, ret);
}

inline constexpr bool qcvmIsBracket(char c){
	switch(c){
		case '(':
		case ')':
		case '{':
		case '}':
		case '[':
		case ']':
			return true;

		default: return false;
	}
}

// called after an initial '"'
inline QC_StrView qcLexStr(
	const char *it,
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	bool escaped = false;

	while(1){
		if(it == end){
			qcLogError("unexpected end of source in string literal");
			return QC_EMPTY_STRVIEW;
		}

		bool doBreak = false;
		bool onNewline = false;

		switch(*it){
			case '"':{
				if(escaped){
					escaped = false;
					break;
				}

				doBreak = true;
				break;
			}

			case '\n':{
				onNewline = true;
				break;
			}

			case '\\':{
				if(escaped){
					escaped = false;
					break;
				}

				escaped = true;
				break;
			}

			default:{
				if(escaped){
					switch(*it){
						case 'n':
						case 'r':
						case 'b':
						case 't':
							break;

						default:{
							if(std::isprint(*it)){
								qcLogWarn("unrecognized escape character '%c'", *it);
							}
							else{
								qcLogWarn("unrecognized escape character 0x%ux", QC_Uint32(*it));
							}
							break;
						}
					}

					escaped = false;
					break;
				}

				if(!std::isprint(*it)){
					qcLogWarn("non-printable character 0x%ux encountered", QC_Uint32(*it));
				}

				break;
			}
		}

		++it;

		if(onNewline){
			++nextLoc->line;
			nextLoc->col = 0;
		}
		else{
			++nextLoc->col;
		}

		if(doBreak){
			break;
		}
	}

	return qcvmLexResult(QC_TOKEN_STR, it, beg, end, startLoc, ret);
}

QC_StrView qcLexInner(
	const char *const beg, const char *const end,
	QC_SourceLocation startLoc,
	QC_Token *ret, QC_SourceLocation *nextLoc
){
	const auto ch = *beg;

	if(ch == '\n'){
		++nextLoc->line;
		nextLoc->col = 0;
		return qcvmLexResult(QC_TOKEN_NEWLINE, beg + 1, beg, end, startLoc, ret);
	}
	else if(ch == '"'){
		++nextLoc->col;
		return qcLexStr(beg + 1, beg, end, startLoc, ret, nextLoc);
	}
	else if(qcvmIsBracket(ch)){
		++nextLoc->col;
		return qcvmLexResult(QC_TOKEN_BRACKET, beg + 1, beg, end, startLoc, ret);
	}
	else if(qcvmIsOpChar(ch)){
		++nextLoc->col;
		return qcLexOp(beg + 1, beg, end, startLoc, ret, nextLoc);
	}
	else if(std::isblank(ch)){
		++nextLoc->col;

		auto it = beg + 1;

		while(it < end && std::isblank(*it)){
			++it;
			++nextLoc->col;
		}

		return qcvmLexResult(QC_TOKEN_SPACE, it, beg, end, startLoc, ret);
	}
	else if((ch == '_') || (ch == '$') || std::isalpha(ch)){
		++nextLoc->col;
		return qcLexId(beg + 1, beg, end, startLoc, ret, nextLoc);
	}
	else if(std::isdigit(*beg)){
		++nextLoc->col;
		return qcLexInt(beg + 1, beg, end, startLoc, ret, nextLoc);
	}
	else{
		return QC_EMPTY_STRVIEW;
	}
}

extern "C" {

QC_StrView qcLex(QC_StrView src, QC_SourceLocation loc, QC_Token *ret, QC_SourceLocation *nextLoc){
	if(!ret){
		return QC_EMPTY_STRVIEW;
	}

	const auto beg = src.ptr;
	const auto end = beg + src.len;

	if(!beg || beg == end){
		return QC_EMPTY_STRVIEW;
	}

	static QC_SourceLocation nextLocDiscard;

	if(!nextLoc){
		nextLoc = &nextLocDiscard;
	}
	else{
		*nextLoc = loc;
	}

	return qcLexInner(beg, end, loc, ret, nextLoc);
}

}
