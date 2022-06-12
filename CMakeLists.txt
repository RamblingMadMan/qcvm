cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
	qcvm
	DESCRIPTION "QuakeC VM"
	VERSION 0.0.1
	LANGUAGES C CXX
)

include(FetchContent)

###
# Configuration
###

option(QCVM_BUILD_SHARED_LIBS "Build the library as a shared library" ON)
option(QCVM_BUILD_TEST "Build the test executable" ${PROJECT_IS_TOP_LEVEL})

###
# Dependencies
###

FetchContent_Declare(
	parallel_hashmap_content
	GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
	GIT_TAG 655dee936b5b65ec0468517dd6728a7ada2b2e02 # latest at time of writing
)

FetchContent_Declare(
	plf_colony_content
	GIT_REPOSITORY https://github.com/mattreecebentley/plf_colony.git
	GIT_TAG 7243b4601b5d7e989550c158a9918ea5a05feaf4 # latest at time of writing
)

FetchContent_MakeAvailable(parallel_hashmap_content plf_colony_content)

add_library(plf_colony INTERFACE)
target_include_directories(plf_colony INTERFACE ${plf_colony_content_SOURCE_DIR})

###
# Project targets
###

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
