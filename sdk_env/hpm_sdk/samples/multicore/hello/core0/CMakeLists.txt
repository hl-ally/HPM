# Copyright (c) 2022 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

set(CUSTOM_GCC_LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../linker/gcc/core0.ld)
set(CMAKE_BUILD_TYPE debug)
set(INCLUDE_BOOTHEADER 1)

find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})

project(multcore_core0)

sdk_compile_definitions("-DFLASH_XIP=1")
if(DEFINED SEC_CORE_IMG_START)
    sdk_compile_definitions(-DSEC_CORE_IMG_START=${SEC_CORE_IMG_START})
else()
    sdk_compile_definitions(-DSEC_CORE_IMG_START=0x0)
endif()

sdk_app_inc(src)
sdk_app_src(src/demo.c src/sec_core_img.c)
generate_ses_project()
