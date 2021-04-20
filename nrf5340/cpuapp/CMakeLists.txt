#
# Copyright (c) 2020 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(empty_app_core)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c src/bitarray.c src/bitcnt_1.c src/bitcnt_2.c src/bitcnt_3.c src/bitcnt_4.c
			   src/bitfiles.c src/bitstrng.c src/bstr_i.c)
# NORDIC SDK APP END
