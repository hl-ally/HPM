# Copyright (c) 2021 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

# include littlevgl
set(CONFIG_LVGL 1)
if(NOT DEFINED CONFIG_TOUCH)
set(CONFIG_TOUCH "gt911")
endif()
set(CONFIG_HPM_TOUCH 1)
set(CONFIG_LV_DEMO lv_demo_stress)
set(CUSTOM_GCC_LINKER_FILE $ENV{HPM_SDK_BASE}/samples/littlevgl/linkers/gcc/lvgl.ld)

find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})

project(lv_demo_stress)
sdk_compile_definitions(-DLV_USE_DEMO_STRESS=1)
sdk_compile_definitions(-DINIT_EXT_RAM_FOR_DATA=1)
sdk_inc(src)
sdk_app_src(src/lvgl.c)

generate_ses_project()
