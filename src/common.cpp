#include "qcvm/common.h"

#include <mutex>

static void QCVM_PRINTF_LIKE(3, 0) qcvm_defaultLogHandler(void*, QC_LogLevel level, const char *fmtStr, va_list args){
	static char msgBuf[1024] = {'\0'};
	vsnprintf(msgBuf, 1023, fmtStr, args);

	std::FILE *outFile = stdout;
	const char *label = "INFO";

	switch(level){
		case QC_LOG_INFO: break;

		case QC_LOG_WARN:{
			outFile = stderr;
			label = "WARN";
			break;
		}

		case QC_LOG_ERROR:{
			outFile = stderr;
			label = "ERROR";
			break;
		}

		case QC_LOG_FATAL:{
			outFile = stderr;
			label = "FATAL";
			break;
		}

		default:{
			label = "UNKNOWN";
			break;
		}
	}

	fprintf(outFile, "[%s] %s", label, msgBuf);
}

namespace {
	std::mutex qcvm_logMut;
	QC_LogHandler qcvm_logHandler = qcvm_defaultLogHandler;
	void *qcvm_logUser = nullptr;
}

extern "C" {

QC_LogHandler qcGetLogHandler(void **userRet){
	std::scoped_lock lock(qcvm_logMut);
	if (userRet) *userRet = qcvm_logUser;
	return qcvm_logHandler;
}

void qcSetLogHandler(QC_LogHandler handler, void *user){
	std::scoped_lock lock(qcvm_logMut);
	qcvm_logHandler = handler;
	qcvm_logUser = user;
}

void qcLogV(QC_LogLevel level, const char *fmtStr, va_list args){
	std::scoped_lock lock(qcvm_logMut);
	qcvm_logHandler(qcvm_logUser, level, fmtStr, args);
}

}
