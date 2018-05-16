project(rtsv)

cmake_minimum_required(VERSION 2.8)

include_directories(.)

add_executable(rtsv rtsv.c lib_getopt.c lib_heap.c lib_list.c lib_logs.c lib_memset.c lib_rt.c lib_rt_fs.c lib_string.c lib_util.c lib_endian.c bits.c fs.c)

install (TARGETS rtsv DESTINATION bin)

add_subdirectory(examples)
