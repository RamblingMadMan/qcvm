#ifndef QCVM_COMMON_H
#define QCVM_COMMON_H 1

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t QC_Int8;
typedef uint8_t QC_Uint8;

typedef int16_t QC_Int16;
typedef uint16_t QC_Uint16;

typedef int32_t QC_Int32;
typedef uint32_t QC_Uint32;

typedef int64_t QC_Int64;
typedef uint64_t QC_Uint64;

enum {
	QC_TRUE = 1, QC_FALSE = 0
};

typedef QC_Uint32 QC_Bool;

typedef float QC_Float;
typedef double QC_Double;

#ifdef __GNUC__
#define QCVM_PRINTF_LIKE(fmtStrArg, vaArg) __attribute__ ((format (printf, (fmtStrArg), (vaArg))))
#else
#define QCVM_PRINTF_LIKE(...)
#endif

enum QC_LogLevel{
	QC_LOG_INFO,
	QC_LOG_WARN,
	QC_LOG_ERROR,
	QC_LOG_FATAL,

	QC_LOG_LEVEL_COUNT
};

typedef void(*QC_LogHandler)(void *user, QC_LogLevel logLevel, const char *fmtStr, va_list args) QCVM_PRINTF_LIKE(3, 0);

QC_LogHandler qcGetLogHandler(void **userRet);
void qcSetLogHandler(QC_LogHandler handler, void *user);

void qcLogV(QC_LogLevel level, const char *fmtStr, va_list args) QCVM_PRINTF_LIKE(2, 0);

inline void QCVM_PRINTF_LIKE(2, 3) qcLog(QC_LogLevel level, const char *fmtStr, ...){
	va_list va;
	va_start(va, fmtStr);
	qcLogV(level, fmtStr, va);
	va_end(va);
}

#define qcLogInfo(fmtStr, ...) qcLog(QC_LOG_INFO, fmtStr __VA_OPT__(,) __VA_ARGS__)
#define qcLogWarn(fmtStr, ...) qcLog(QC_LOG_WARN, fmtStr __VA_OPT__(,) __VA_ARGS__)
#define qcLogError(fmtStr, ...) qcLog(QC_LOG_ERROR, fmtStr __VA_OPT__(,) __VA_ARGS__)
#define qcLogFatal(fmtStr, ...) qcLog(QC_LOG_FATAL, fmtStr __VA_OPT__(,) __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // !QCVM_COMMON_H
