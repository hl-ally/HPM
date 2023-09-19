/*
 * Copyright (c) 2021 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include "board.h"
#include "hpm_clock_drv.h"
#include "hpm_dmav2_drv.h"
#include "hpm_dmamux_drv.h"
#include "hpm_uart_drv.h"
#include "hpm_sysctl_drv.h"
#include "hpm_mchtmr_drv.h"
#include "hpm_interrupt.h"

/* Define */
#define TEST_UART BOARD_APP_UART_BASE
#define TEST_UART_CLK_NAME BOARD_APP_UART_CLK_NAME
#define TEST_UART_DMA_CONTROLLER BOARD_APP_HDMA
#define TEST_UART_DMAMUX_CONTROLLER BOARD_APP_DMAMUX

#define TEST_BUFFER_SIZE 64

/* dma buffer should be 4-byte aligned */
ATTR_PLACE_AT_NONCACHEABLE_BSS_WITH_ALIGNMENT(4) uint8_t uart_tx_buf[TEST_BUFFER_SIZE];
ATTR_PLACE_AT_NONCACHEABLE_BSS_WITH_ALIGNMENT(4) uint8_t uart_rx_buf[TEST_BUFFER_SIZE];
static uint32_t rx_front_index;
static uint32_t rx_rear_index;


void init_board_app_uart(void)
{
    hpm_stat_t stat;
    uart_config_t config = { 0 };

    uart_default_config(TEST_UART, &config);
    config.fifo_enable = true;
    config.dma_enable = true;
    config.src_freq_in_hz = clock_get_frequency(TEST_UART_CLK_NAME);
    config.tx_fifo_level = uart_tx_fifo_trg_not_full;
    config.rx_fifo_level = uart_rx_fifo_trg_not_empty;
    board_init_uart(TEST_UART);
    stat = uart_init(TEST_UART, &config);
    if (stat != status_success) {
        while (1) {
        }
    }
}

void init_board_app_dma(void)
{
    dma_handshake_config_t ch_config = { 0 };

/* 1. config uart circle rx dma */
    dmamux_config(TEST_UART_DMAMUX_CONTROLLER, DMAMUX_MUXCFG_HDMA_MUX30, HPM_DMA_SRC_UART0_RX, true);
    ch_config.ch_index = 30;
    ch_config.dst = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)uart_rx_buf);
    ch_config.dst_fixed = false;
    ch_config.src = (uint32_t)&TEST_UART->RBR;
    ch_config.src_fixed = true;
    ch_config.data_width = DMA_TRANSFER_WIDTH_BYTE;
    ch_config.size_in_byte = TEST_BUFFER_SIZE;
    ch_config.en_infiniteloop = true;
    ch_config.interrupt_mask = DMA_INTERRUPT_MASK_ALL;
    dma_setup_handshake(TEST_UART_DMA_CONTROLLER, &ch_config, true);

/* 2. config uart normal tx dma */
    dmamux_config(TEST_UART_DMAMUX_CONTROLLER, DMAMUX_MUXCFG_HDMA_MUX31, HPM_DMA_SRC_UART0_TX, true);
    ch_config.ch_index = 31;
    ch_config.dst = (uint32_t)&TEST_UART->THR;
    ch_config.dst_fixed = true;
    ch_config.src = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)uart_tx_buf);
    ch_config.src_fixed = false;
    ch_config.data_width = DMA_TRANSFER_WIDTH_BYTE;
    ch_config.size_in_byte = TEST_BUFFER_SIZE;
    ch_config.en_infiniteloop = false;
    ch_config.interrupt_mask = DMA_INTERRUPT_MASK_ALL;
    dma_setup_handshake(TEST_UART_DMA_CONTROLLER, &ch_config, false);
}

void task_entry_5ms(void)
{
    uint32_t rx_data_size;
    uint32_t dma_residue_size;

    mchtmr_delay(HPM_MCHTMR, (clock_get_frequency(clock_mchtmr0) / 1000) * 5);

    dma_residue_size = dma_get_residue_transfer_size(TEST_UART_DMA_CONTROLLER, 30);

    rx_rear_index = TEST_BUFFER_SIZE - dma_residue_size;
    if (rx_front_index > rx_rear_index) {
        rx_data_size = TEST_BUFFER_SIZE + rx_rear_index - rx_front_index;
    } else {
        rx_data_size = rx_rear_index - rx_front_index;
    }

    for (uint32_t i = 0; i < rx_data_size; i++) {
        if ((rx_front_index + i) < TEST_BUFFER_SIZE) {
            uart_tx_buf[i] = uart_rx_buf[rx_front_index + i];
        } else {
            uart_tx_buf[i] = uart_rx_buf[rx_front_index + i - TEST_BUFFER_SIZE];
        }
    }
    rx_front_index = rx_rear_index;

    if (rx_data_size > 0) {
        if (!dma_channel_is_enable(TEST_UART_DMA_CONTROLLER, 31)) {
            dma_set_transfer_size(TEST_UART_DMA_CONTROLLER, 31, rx_data_size);
            dma_set_source_address(TEST_UART_DMA_CONTROLLER, 31, core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)uart_tx_buf));
            dma_enable_channel(TEST_UART_DMA_CONTROLLER, 31);
        }
    }
}
SDK_DECLARE_MCHTMR_ISR(task_entry_5ms)

int main(void)
{
    board_init();
    init_board_app_uart();
    init_board_app_dma();

    printf("\nDmav2 circle transmission example");
    printf("\nPlease input some characters, the serial port will echo. Please input characters:\n");

    mchtmr_delay(HPM_MCHTMR, (clock_get_frequency(clock_mchtmr0) / 1000) * 5);
    enable_mchtmr_irq();

    while (1) {
        ;
    }
    return 0;
}
