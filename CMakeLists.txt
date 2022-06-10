cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
	qcvm
	DESCRIPTION "QuakeC VM"
	VERSION 0.0.1
	LANGUAGES C CXX
)

option(QCVM_BUILD_SHARED_LIBS "Build the library as a shared library" ON)
option(QCVM_BUILD_TEST "Build the test executable" ${PROJECT_IS_TOP_LEVEL})

set(QCVM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(
	QCVM_C_HEADERS
	${QCVM_INCLUDE_DIR}/qcvm/common.h
	${QCVM_INCLUDE_DIR}/qcvm/bytecode.h
	${QCVM_INCLUDE_DIR}/qcvm/vm.h
)

add_subdirectory(src)

if(QCVM_BUILD_TEST)
	add_subdirectory(test)
endif()
