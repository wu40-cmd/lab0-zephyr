# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE src/main.c)

if(CONFIG_SUM_PRINT)
    target_sources(app PRIVATE sum_printk/sum_printk.c)
    target_include_directories(app PRIVATE sum_printk)
elseif(CONFIG_SUM_LOG)
    target_sources(app PRIVATE sum_log/sum_log.c)
    target_include_directories(app PRIVATE sum_log)
endif()
