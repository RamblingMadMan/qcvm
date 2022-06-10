#include "qcvm/vm.h"

#include <cstdlib>

#define qcFatal(fmtStr, ...) \
	(qcLogFatal((fmtStr) __VA_OPT__(,) __VA_ARGS__), std::exit(EXIT_FAILURE), 0)

int main(int argc, char *argv[]){
	(void)argc; (void)argv;

	const auto vm = qcCreateVM();
	if(!vm){
		qcFatal("failed to create VM");
	}

	if(!qcDestroyVM(vm)){
		qcFatal("error destroying VM");
	}

	return 0;
}
