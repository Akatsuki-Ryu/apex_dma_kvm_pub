cmake_minimum_required(VERSION 3.23)
project(apex_dma_kvm_pub)

set(CMAKE_CXX_STANDARD 14)

include_directories(apex_dma)
include_directories(apex_dma/memflow_lib/memflow-ffi)
include_directories(apex_dma/memflow_lib/memflow-win32-ffi)

add_executable(apex_dma_kvm_pub
        apex_dma/memflow_lib/memflow-ffi/examples/phys_mem.c
        apex_dma/memflow_lib/memflow-ffi/binddestr.h
        apex_dma/memflow_lib/memflow-ffi/memflow.h
        apex_dma/memflow_lib/memflow-ffi/memflow_cpp.h
        apex_dma/memflow_lib/memflow-win32-ffi/examples/dump_header.c
        apex_dma/memflow_lib/memflow-win32-ffi/examples/process_list.c
        apex_dma/memflow_lib/memflow-win32-ffi/memflow_win32.h
        apex_dma/memflow_lib/memflow-win32-ffi/memflow_win32_cpp.h
        apex_dma/apex_dma.cpp
        apex_dma/build.sh
        apex_dma/Game.cpp
        apex_dma/Game.h
        apex_dma/Makefile
        apex_dma/Math.cpp
        apex_dma/Math.h
        apex_dma/memory.cpp
        apex_dma/memory.h
        apex_dma/offsets.h
        apex_dma/prediction.h
        apex_dma/vector.h)
