# Copyright (c) 2021-2022 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    SET(CMAKE_BUILD_TYPE flash_xip)
endif()

set(HPM_SDK_NO_NANO_SPECS 1)
set(CONFIG_LWIP 1)
#set(CONFIG_ENET_PHY_DP83867 1)
#set(CONFIG_ENET_PHY_RTL8211 1)
set(CONFIG_ENET_PHY_DP83848 1)
#set(CONFIG_ENET_PHY_RTL8201 1)

set(CONFIG_LWIP_IPERF 1)

find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})


if("${CMAKE_BUILD_TYPE}" STREQUAL "flash_xip" OR "${CMAKE_BUILD_TYPE}" STREQUAL "flash_xip_release")
    sdk_compile_definitions(-DLWIP_MEM_SECTION=)
endif()

if(${CONFIG_ENET_PHY_DP83867})
    sdk_compile_definitions(-DRGMII=1)
    sdk_compile_definitions(-D__USE_DP83867=1)
endif()

if(${CONFIG_ENET_PHY_RTL8211})
    sdk_compile_definitions(-DRGMII=1)
    sdk_compile_definitions(-D__USE_RTL8211=1)
endif()

if(${CONFIG_ENET_PHY_DP83848})
    sdk_compile_definitions(-DRGMII=0)
    sdk_compile_definitions(-D__USE_DP83848=1)
endif()

if(${CONFIG_ENET_PHY_RTL8201})
    sdk_compile_definitions(-DRGMII=0)
    sdk_compile_definitions(-D__USE_RTL8201=1)
endif()

project(lwip_iperf_example)

sdk_inc(../common)
sdk_inc(../common/arch)
sdk_inc(../common/feature)
sdk_inc(inc)
sdk_inc(inc/eth)
sdk_inc(inc/app)

sdk_app_src(../common/arch/sys_arch.c)
sdk_app_src(../common/arch/ethernetif.c)
sdk_app_src(../common/feature/common.c)
sdk_app_src(src/eth/netconf.c)
sdk_app_src(src/lwip.c)
generate_ses_project()
