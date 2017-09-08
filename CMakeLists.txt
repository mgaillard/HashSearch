cmake_minimum_required(VERSION 3.5)

project(HashGenerator)
set(CMAKE_CXX_STANDARD 11)
add_executable(HashGenerator hash_generator.cpp)

project(HashSearch)

find_package(CUDA)

set(CMAKE_CXX_STANDARD 11)
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} --std=c++11")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -fopenmp")
	set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} --optimize 3 --gpu-architecture=sm_50 --compiler-options '-march=native'")
endif()

include_directories("${PROJECT_SOURCE_DIR}/mihasher/include")
set(SOURCE_FILES HashSearch.cu mihasher/src/array32.cpp mihasher/src/mihasher.cpp mihasher/src/sparse_hashtable.cpp mihasher/src/bucket_group.cpp mihasher/src/reorder.cpp)
cuda_add_executable(HashSearch ${SOURCE_FILES})
