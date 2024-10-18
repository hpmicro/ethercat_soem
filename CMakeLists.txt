# Copyright (c) 2021-2022 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_inc(src)
sdk_inc(src/port)
sdk_src(src/ethercatbase.c)
sdk_src(src/ethercatcoe.c)
sdk_src(src/ethercatconfig.c)
sdk_src(src/ethercatdc.c)
sdk_src(src/ethercatfoe.c)
sdk_src(src/ethercatmain.c)
sdk_src(src/ethercatprint.c)
sdk_src(src/ethercatsoe.c)

sdk_src(src/port/nicdrv.c)
sdk_src(src/port/osal.c)
sdk_src(src/port/hpm_enet_port.c)