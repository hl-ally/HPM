/*
 * Copyright (c) 2021 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include "hpm_soc_feature.h"
#include "hpm_trgm_soc_drv.h"
#include "hpm_trgm_drv.h"
#include "hpm_pwm_drv.h"
#include "hpm_adc16_drv.h"
#include "hpm_qeiv2_drv.h"

#ifndef BOARD_APP_QEI_BASE
#define BOARD_APP_QEI_BASE BOARD_BLDC_QEI_BASE
#endif
#ifndef BOARD_APP_QEI_IRQ
#define BOARD_APP_QEI_IRQ  BOARD_BLDC_QEI_IRQ
#endif
#ifndef BOARD_APP_MOTOR_CLK
#define BOARD_APP_MOTOR_CLK BOARD_BLDC_QEI_CLOCK_SOURCE
#endif

#define BOARD_ADC_COS_BASE  HPM_ADC0
#define BOARD_ADC_COS_CHN     (4U)
#define BOARD_ADC_SIN_BASE  HPM_ADC1
#define BOARD_ADC_SIN_CHN     (5U)
#define PWM_FREQ              30000

static volatile bool s_qei_data_ready;
static volatile uint32_t z, ph, pos, ang;
static uint32_t s_record_data[5][500];
static uint32_t s_cos_adc_data[ADC_SOC_PMT_MAX_DMA_BUFF_LEN_IN_4BYTES];
static uint32_t s_sin_adc_data[ADC_SOC_PMT_MAX_DMA_BUFF_LEN_IN_4BYTES];


/* Static function declaration */
static void pwm_init(void);
static void trigger_mux_init(void);
static void adc_init(void);
static void qeiv2_init(void);


/* Function definition */
int main(void)
{
    board_init();
    board_init_adc16_pins();

    pwm_init();
    trigger_mux_init();
    adc_init();
    qeiv2_init();

    printf("qeiv2 sincos encoder example\n");

    for (uint16_t i = 0; i < 500; i++) {
        s_record_data[0][i] = s_cos_adc_data[0] & 0xFFFF;
        s_record_data[1][i] = s_sin_adc_data[0] & 0xFFFF;
        s_record_data[2][i] = qeiv2_get_postion(BOARD_APP_QEI_BASE);
        s_record_data[3][i] = qeiv2_get_angle(BOARD_APP_QEI_BASE);
        s_record_data[4][i] = qeiv2_get_phase_cnt(BOARD_APP_QEI_BASE);
    }

    for (uint16_t i = 0; i < 500; i++) {
        printf("ch0:%#x, ch1:%#x, pos:%#x, ang:%#x, ph:%#x\n", s_record_data[0][i], s_record_data[1][i], s_record_data[2][i], s_record_data[3][i], s_record_data[4][i]);
    }

    while (1) {
        if (s_qei_data_ready) {
            s_qei_data_ready = false;
            printf("z: 0x%x\n", z);
            printf("phase: 0x%x\n", ph);
            printf("position: 0x%x\n", pos);
            printf("ang: 0x%x\n", ang);
        }
    }
    return 0;
}

void isr_qei(void)
{
    uint32_t status = qeiv2_get_status(BOARD_APP_QEI_BASE);

    qeiv2_clear_status(BOARD_APP_QEI_BASE, status);
    if ((status & QEIV2_EVENT_POSITION_COMPARE_FLAG_MASK) != 0) {
        if (!s_qei_data_ready) {
            s_qei_data_ready = true;
            z = qeiv2_get_count_on_read_event(BOARD_APP_QEI_BASE, qeiv2_counter_type_z);
            ph = qeiv2_get_count_on_read_event(BOARD_APP_QEI_BASE, qeiv2_counter_type_phase);
            pos = qeiv2_get_count_on_read_event(BOARD_APP_QEI_BASE, qeiv2_counter_type_speed);
            ang = qeiv2_get_count_on_read_event(BOARD_APP_QEI_BASE, qeiv2_counter_type_timer);
        }
    }
}
SDK_DECLARE_EXT_ISR_M(BOARD_APP_QEI_IRQ, isr_qei)

static void pwm_init(void)
{
    pwm_cmp_config_t pwm_cmp_cfg;
    pwm_output_channel_t pwm_output_ch_cfg;

    pwm_set_reload(HPM_PWM0, 0, clock_get_frequency(BOARD_APP_MOTOR_CLK) / PWM_FREQ - 1);

    /* Set a comparator */
    memset(&pwm_cmp_cfg, 0x00, sizeof(pwm_cmp_config_t));
    pwm_cmp_cfg.enable_ex_cmp  = false;
    pwm_cmp_cfg.mode           = pwm_cmp_mode_output_compare;
    pwm_cmp_cfg.update_trigger = pwm_shadow_register_update_on_shlk;

    /* Select comp8 and trigger at the middle of a pwm cycle */
    pwm_cmp_cfg.cmp = clock_get_frequency(BOARD_APP_MOTOR_CLK) / (PWM_FREQ * 2) - 1;
    pwm_config_cmp(HPM_PWM0, 8, &pwm_cmp_cfg);

    /* Issue a shadow lock */
    pwm_issue_shadow_register_lock_event(HPM_PWM0);

    /* Set comparator channel to generate a trigger signal */
    pwm_output_ch_cfg.cmp_start_index = 8;   /* start channel */
    pwm_output_ch_cfg.cmp_end_index   = 8;   /* end channel */
    pwm_output_ch_cfg.invert_output   = false;
    pwm_config_output_channel(HPM_PWM0, 8, &pwm_output_ch_cfg);

    /* Start the comparator counter */
    pwm_start_counter(HPM_PWM0);
}

static void trigger_mux_init(void)
{
    trgm_output_t trgm_output_cfg;

    trgm_output_cfg.invert = false;
    trgm_output_cfg.type   = trgm_output_pulse_at_input_falling_edge;
    trgm_output_cfg.input  = HPM_TRGM0_INPUT_SRC_PWM0_CH8REF;
    trgm_output_config(HPM_TRGM0, TRGM_TRGOCFG_ADCX_PTRGI0A, &trgm_output_cfg);
}

static void adc_init(void)
{
    adc16_config_t cfg;
    adc16_channel_config_t ch_cfg;
    adc16_pmt_config_t pmt_cfg;

    /* initialize ADC instances */
    adc16_get_default_config(&cfg);
    cfg.res            = adc16_res_16_bits;
    cfg.conv_mode      = adc16_conv_mode_preemption;
    cfg.adc_clk_div    = adc16_clock_divider_4;
    cfg.sel_sync_ahb   = true;
    cfg.adc_ahb_en     = true;
    adc16_init(BOARD_ADC_COS_BASE, &cfg);
    adc16_init(BOARD_ADC_SIN_BASE, &cfg);

    /* initialize ADC channels */
    adc16_get_channel_default_config(&ch_cfg);
    ch_cfg.sample_cycle  = 20;
    ch_cfg.ch            = BOARD_ADC_COS_CHN;
    adc16_init_channel(BOARD_ADC_COS_BASE, &ch_cfg);
    ch_cfg.sample_cycle  = 20;
    ch_cfg.ch            = BOARD_ADC_SIN_CHN;
    adc16_init_channel(BOARD_ADC_SIN_BASE, &ch_cfg);

    /* initialize preempt config */
    pmt_cfg.adc_ch[0] = BOARD_ADC_COS_CHN;
    pmt_cfg.inten[0] = false;
    pmt_cfg.trig_ch = ADC16_CONFIG_TRG0A;
    pmt_cfg.trig_len = 1;
    adc16_set_pmt_config(BOARD_ADC_COS_BASE, &pmt_cfg);
    adc16_set_pmt_queue_enable(BOARD_ADC_COS_BASE, pmt_cfg.trig_ch, true);
    pmt_cfg.adc_ch[0] = BOARD_ADC_SIN_CHN;
    pmt_cfg.inten[0] = false;
    pmt_cfg.trig_ch = ADC16_CONFIG_TRG0A;
    pmt_cfg.trig_len = 1;
    adc16_set_pmt_config(BOARD_ADC_SIN_BASE, &pmt_cfg);
    adc16_set_pmt_queue_enable(BOARD_ADC_SIN_BASE, pmt_cfg.trig_ch, true);

    /* Set DMA start address for preemption mode */
    adc16_init_pmt_dma(BOARD_ADC_COS_BASE, (uint32_t)&s_cos_adc_data[0]);
    adc16_init_pmt_dma(BOARD_ADC_SIN_BASE, (uint32_t)&s_sin_adc_data[0]);

    adc16_enable_motor(BOARD_ADC_COS_BASE);
    adc16_enable_motor(BOARD_ADC_SIN_BASE);
}

static void qeiv2_init(void)
{
    qeiv2_adc_config_t adc_config;

    trgm_adc_matrix_config(BOARD_BLDC_QEI_TRGM, BOARD_BLDC_QEI_ADC_MATRIX_ADCX, trgm_adc_matrix_in_from_adc0, false);
    trgm_adc_matrix_config(BOARD_BLDC_QEI_TRGM, BOARD_BLDC_QEI_ADC_MATRIX_ADCY, trgm_adc_matrix_in_from_adc1, false);

    adc_config.adc_select  = 0;
    adc_config.adc_channel = BOARD_ADC_COS_CHN;
    adc_config.offset      = 0x80000000;
    adc_config.param0      = 0x4000;
    adc_config.param1      = 0;
    qeiv2_config_adcx(BOARD_APP_QEI_BASE, &adc_config, true);
    adc_config.adc_select  = 1;
    adc_config.adc_channel = BOARD_ADC_SIN_CHN;
    adc_config.offset      = 0x80000000;
    adc_config.param0      = 0;
    adc_config.param1      = 0x4000;
    qeiv2_config_adcy(BOARD_APP_QEI_BASE, &adc_config, true);
    qeiv2_set_adc_xy_delay(BOARD_APP_QEI_BASE, 0xFFFFFF);

    qeiv2_reset_counter(BOARD_APP_QEI_BASE);

    qeiv2_set_work_mode(BOARD_APP_QEI_BASE, qeiv2_work_mode_sincos);
    qeiv2_select_spd_tmr_register_content(BOARD_APP_QEI_BASE, qeiv2_spd_tmr_as_pos_angle);
    qeiv2_config_z_phase_counter_mode(BOARD_APP_QEI_BASE, qeiv2_z_count_inc_on_phase_count_max);
    qeiv2_config_phmax_phparam(BOARD_APP_QEI_BASE, 128);
    qeiv2_pause_pos_counter_on_fault(BOARD_APP_QEI_BASE, true);

    intc_m_enable_irq_with_priority(BOARD_APP_QEI_IRQ, 1);

    qeiv2_set_spd_pos_cmp_value(BOARD_APP_QEI_BASE, 0x80000000);
    qeiv2_config_cmp_match_option(BOARD_APP_QEI_BASE, true, true, false, true, true, false, false);
    qeiv2_enable_load_read_trigger_event(BOARD_APP_QEI_BASE, QEIV2_EVENT_POSITION_COMPARE_FLAG_MASK);
    qeiv2_enable_irq(BOARD_APP_QEI_BASE, QEIV2_EVENT_POSITION_COMPARE_FLAG_MASK);

    qeiv2_release_counter(BOARD_APP_QEI_BASE);
}


