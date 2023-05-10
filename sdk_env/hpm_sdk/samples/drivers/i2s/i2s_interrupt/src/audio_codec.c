/*
 * Copyright (c) 2022 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "hpm_clock_drv.h"
#include "hpm_i2s_drv.h"
#include "hpm_dma_drv.h"
#include "hpm_dmamux_drv.h"
#include "hpm_l1c_drv.h"

#define CODEC_I2C            BOARD_APP_I2C_BASE
#define CODEC_I2S            BOARD_APP_I2S_BASE
#define CODEC_I2S_CLK_NAME   BOARD_APP_I2S_CLK_NAME
#define CODEC_I2S_DATA_LINE  BOARD_APP_I2S_DATA_LINE

#define CODEC_I2C_ADDRESS    SGTL5000_I2C_ADDR
#define CODEC_SAMPLE_RATE_HZ 48000U
#define CODEC_BIT_WIDTH      32U

#if CONFIG_CODEC_WM8960
    #include "hpm_wm8960.h"
    wm8960_config_t wm8960_config = {
        .route       = wm8960_route_playback,
        .left_input  = wm8960_input_closed,
        .right_input = wm8960_input_closed,
        .play_source = wm8960_play_source_dac,
        .bus         = wm8960_bus_i2s,
        .format = {.mclk_hz = 0U, .sample_rate = CODEC_SAMPLE_RATE_HZ, .bit_width = 32},
    };

    wm8960_control_t wm8960_control = {
        .ptr = CODEC_I2C,
        .slave_address = WM8960_I2C_ADDR, /* I2C address */
    };
#elif CONFIG_CODEC_SGTL5000
    #include "hpm_sgtl5000.h"
    sgtl_config_t sgtl5000_config = {
        .route = sgtl_route_playback,  /*!< Audio data route.*/
        .bus = sgtl_bus_left_justified,       /*!< Audio transfer protocol */
        .master = false,                      /*!< Master or slave. True means master, false means slave. */
        .format = {.mclk_hz = 0,
                .sample_rate = CODEC_SAMPLE_RATE_HZ,
                .bit_width = CODEC_BIT_WIDTH,
                .sclk_edge = sgtl_sclk_valid_edge_rising}, /*!< audio format */
    };

    sgtl_context_t sgtl5000_context = {
        .ptr = CODEC_I2C,
        .slave_address = SGTL5000_I2C_ADDR, /* I2C address */
    };
#else
    #error no specified Audio Codec!!!
#endif

uint32_t * data_tx;
uint32_t t_count = 0;
uint32_t sin_1khz_48khz[] = {
  0x00000000, 0x0D5DAA00, 0x1A80C900, 0x272FD100, 0x33333300, 0x3E565100, 0x48686100, 0x513D4800,
  0x58AE5600, 0x5E9AF300, 0x62E92A00, 0x65862200, 0x66666600, 0x65862200, 0x62E92B00, 0x5E9AF300,
  0x58AE5600, 0x513D4800, 0x48686100, 0x3E565100, 0x33333300, 0x272FD100, 0x1A80CA00, 0x0D5DAB00,
  0x00000000, 0xF2A25600, 0xE57F3700, 0xD8D02F00, 0xCCCCCD00, 0xC1A9B000, 0xB7979F00, 0xAEC2B800,
  0xA751AA00, 0xA1650E00, 0x9D16D600, 0x9A79DE00, 0x99999A00, 0x9A79DE00, 0x9D16D500, 0xA1650D00,
  0xA751AA00, 0xAEC2B700, 0xB7979E00, 0xC1A9AF00, 0xCCCCCC00, 0xD8D02E00, 0xE57F3600, 0xF2A25500,
};
void isr_i2s(void)
{
    volatile uint32_t s;
    s = CODEC_I2S->STA;
    CODEC_I2S->STA = s;
    if ((s & I2S_CTRL_TX_EN_SET(4))) {
        i2s_send_data(CODEC_I2S, CODEC_I2S_DATA_LINE, sin_1khz_48khz[t_count]);
        i2s_send_data(CODEC_I2S, CODEC_I2S_DATA_LINE, sin_1khz_48khz[t_count]);
        t_count++;
        t_count = t_count % ARRAY_SIZE(sin_1khz_48khz);
    }
}
SDK_DECLARE_EXT_ISR_M(IRQn_I2S0, isr_i2s)


void test_interrupt(void)
{
    i2s_config_t i2s_config;
    i2s_transfer_config_t transfer;
    uint32_t i2s_mclk_hz;

    /* Config I2S interface to CODEC */ 
    i2s_get_default_config(CODEC_I2S, &i2s_config);
    i2s_config.fifo_threshold = 2;
    i2s_config.enable_mclk_out = true;
    i2s_init(CODEC_I2S, &i2s_config);

    i2s_get_default_transfer_config(&transfer);
    transfer.data_line = I2S_DATA_LINE_2;
    transfer.sample_rate = CODEC_SAMPLE_RATE_HZ;
    transfer.master_mode = true;
#if CONFIG_CODEC_WM8960
    transfer.protocol = I2S_PROTOCOL_I2S_PHILIPS;
#endif
    i2s_mclk_hz = clock_get_frequency(CODEC_I2S_CLK_NAME);
    if (status_success != i2s_config_transfer(CODEC_I2S, i2s_mclk_hz, &transfer)) {
        printf("I2S config failed for CODEC\n");
        while(1);
    }

#if CONFIG_CODEC_WM8960
    wm8960_config.format.mclk_hz = i2s_mclk_hz;
    if (wm8960_init(&wm8960_control, &wm8960_config) != status_success) {
        printf("Init Audio Codec failed\n");
    }
#elif CONFIG_CODEC_SGTL5000
    sgtl5000_config.format.mclk_hz = i2s_mclk_hz;
    if (sgtl_init(&sgtl5000_context, &sgtl5000_config) != status_success) {
        printf("Init Audio Codec failed\n");
    }
#endif

    printf("Test Codec playback\n");
    i2s_enable_irq(CODEC_I2S, I2S_IRQ_TX_FIFO_EMPTY);
    intc_m_enable_irq_with_priority(IRQn_I2S0, 1);
    while(1) {
    }
}

int main(void)
{
    board_init();
    board_init_i2c(CODEC_I2C);

    init_i2s_pins(CODEC_I2S);
    board_init_i2s_clock(CODEC_I2S);

    printf("I2S Interrupt example\n");
    test_interrupt();

    while(1);
    return 0;
}
