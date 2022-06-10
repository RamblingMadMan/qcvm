#include "qcvm/common.h"
#include "qcvm/vm.h"

#include <cstdlib>
#include <vector>

extern "C" {

struct QC_VM{
	std::vector<QC_ByteCode*> loadedBc;
};

QC_VM *qcCreateVM(){
	const auto mem = std::aligned_alloc(alignof(QC_VM), sizeof(QC_VM));
	if(!mem){
		qcLogError("failed to allocate memory for QC_VM");
		return nullptr;
	}

	const auto p = new(mem) QC_VM;

	return p;
}

bool qcDestroyVM(QC_VM *vm){
	if(!vm) return false;
	std::destroy_at(vm);
	std::free(vm);
	return true;
}

}
