/*
 * Copyright (c) 2021 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef LWIP_H
#define LWIP_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "hpm_enet_drv.h"

/* Exported Macros------------------------------------------------------------*/
#if RGMII
#define ENET_INF_TYPE       enet_inf_rgmii
#define ENET                BOARD_ENET_RGMII
#else
#define ENET_INF_TYPE       enet_inf_rmii
#define ENET                BOARD_ENET_RMII
#endif

#define ENET_TX_BUFF_COUNT  (50U)
#define ENET_RX_BUFF_COUNT  (60U)
#define ENET_RX_BUFF_SIZE   ENET_MAX_FRAME_SIZE
#define ENET_TX_BUFF_SIZE   ENET_MAX_FRAME_SIZE

/* Exported Variables ------------------------------------------------------*/
extern enet_desc_t desc;

#endif /* LWIP_H */
