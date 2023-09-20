/*
 * Copyright (c) 2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "hpm_qeo_drv.h"

void qeo_wave_get_default_mode_config(QEO_Type *base, qeo_wave_mode_t *config)
{
    config->wave0.above_max_limit = qeo_wave_above_max_limit_max_val;
    config->wave0.high_area0_limit = qeo_wave_high_area_limit_max_val;
    config->wave0.high_area1_limit = qeo_wave_high_area_limit_max_val;
    config->wave0.low_area0_limit = qeo_wave_low_area_limit_zero;
    config->wave0.low_area1_limit = qeo_wave_low_area_limit_zero;
    config->wave0.below_min_limit = qeo_wave_below_min_limit_zero;

    config->wave1.above_max_limit = qeo_wave_above_max_limit_max_val;
    config->wave1.high_area0_limit = qeo_wave_high_area_limit_max_val;
    config->wave1.high_area1_limit = qeo_wave_high_area_limit_max_val;
    config->wave1.low_area0_limit = qeo_wave_low_area_limit_zero;
    config->wave1.low_area1_limit = qeo_wave_low_area_limit_zero;
    config->wave1.below_min_limit = qeo_wave_below_min_limit_zero;

    config->wave2.above_max_limit = qeo_wave_above_max_limit_max_val;
    config->wave2.high_area0_limit = qeo_wave_high_area_limit_max_val;
    config->wave2.high_area1_limit = qeo_wave_high_area_limit_max_val;
    config->wave2.low_area0_limit = qeo_wave_low_area_limit_zero;
    config->wave2.low_area1_limit = qeo_wave_low_area_limit_zero;
    config->wave2.below_min_limit = qeo_wave_below_min_limit_zero;

    config->saddle_type = 0;
    config->wave_type = qeo_wave_cosine;
}

void qeo_wave_config_mode(QEO_Type *base, qeo_wave_mode_t *config)
{
    /* clear other bit except EN_WAVEx_VD_VQ_INJECT in MODE register */
    base->WAVE.MODE &= QEO_WAVE_MODE_EN_WAVE2_VD_VQ_INJECT_MASK
                    | QEO_WAVE_MODE_EN_WAVE1_VD_VQ_INJECT_MASK
                    | QEO_WAVE_MODE_EN_WAVE0_VD_VQ_INJECT_MASK;

    base->WAVE.MODE |= QEO_WAVE_MODE_WAVE2_ABOVE_MAX_LIMIT_SET(config->wave2.above_max_limit)
                    | QEO_WAVE_MODE_WAVE2_HIGH_AREA1_LIMIT_SET(config->wave2.high_area1_limit)
                    | QEO_WAVE_MODE_WAVE2_HIGH_AREA0_LIMIT_SET(config->wave2.high_area0_limit)
                    | QEO_WAVE_MODE_WAVE2_LOW_AREA1_LIMIT_SET(config->wave2.low_area1_limit)
                    | QEO_WAVE_MODE_WAVE2_LOW_AREA0_LIMIT_SET(config->wave2.low_area0_limit)
                    | QEO_WAVE_MODE_WAVE2_BELOW_MIN_LIMIT_SET(config->wave2.below_min_limit)

                    | QEO_WAVE_MODE_WAVE1_ABOVE_MAX_LIMIT_SET(config->wave1.above_max_limit)
                    | QEO_WAVE_MODE_WAVE1_HIGH_AREA1_LIMIT_SET(config->wave1.high_area1_limit)
                    | QEO_WAVE_MODE_WAVE1_HIGH_AREA0_LIMIT_SET(config->wave1.high_area0_limit)
                    | QEO_WAVE_MODE_WAVE1_LOW_AREA1_LIMIT_SET(config->wave1.low_area1_limit)
                    | QEO_WAVE_MODE_WAVE1_LOW_AREA0_LIMIT_SET(config->wave1.low_area0_limit)
                    | QEO_WAVE_MODE_WAVE1_BELOW_MIN_LIMIT_SET(config->wave1.below_min_limit)

                    | QEO_WAVE_MODE_WAVE0_ABOVE_MAX_LIMIT_SET(config->wave0.above_max_limit)
                    | QEO_WAVE_MODE_WAVE0_HIGH_AREA1_LIMIT_SET(config->wave0.high_area1_limit)
                    | QEO_WAVE_MODE_WAVE0_HIGH_AREA0_LIMIT_SET(config->wave0.high_area0_limit)
                    | QEO_WAVE_MODE_WAVE0_LOW_AREA1_LIMIT_SET(config->wave0.low_area1_limit)
                    | QEO_WAVE_MODE_WAVE0_LOW_AREA0_LIMIT_SET(config->wave0.low_area0_limit)
                    | QEO_WAVE_MODE_WAVE0_BELOW_MIN_LIMIT_SET(config->wave0.below_min_limit)
                    | QEO_WAVE_MODE_SADDLE_TYPE_SET(config->saddle_type)
                    | QEO_WAVE_MODE_WAVES_OUTPUT_TYPE_SET(config->wave_type);
}

void qeo_abz_get_default_mode_config(QEO_Type *base, qeo_abz_mode_t *config)
{
    config->a_inv_pol = false;
    config->b_inv_pol = false;
    config->z_inv_pol = false;
    config->output_type = qeo_abz_output_abz;
    config->z_pulse_period = qeo_z_pulse_100_percent;
}

void qeo_abz_config_mode(QEO_Type *base, qeo_abz_mode_t *config)
{
    base->ABZ.MODE &= ~(QEO_ABZ_MODE_Z_POLARITY_MASK
                        | QEO_ABZ_MODE_B_POLARITY_MASK
                        | QEO_ABZ_MODE_A_POLARITY_MASK
                        | QEO_ABZ_MODE_Z_TYPE_MASK
                        | QEO_ABZ_MODE_B_TYPE_MASK
                        | QEO_ABZ_MODE_A_TYPE_MASK);

    base->ABZ.MODE = QEO_ABZ_MODE_Z_POLARITY_SET(config->z_inv_pol)
                    | QEO_ABZ_MODE_B_POLARITY_SET(config->b_inv_pol)
                    | QEO_ABZ_MODE_A_POLARITY_SET(config->a_inv_pol);

    if ((config->output_type == qeo_abz_output_pulse_revise) || (config->output_type == qeo_abz_output_up_down)) {
        base->ABZ.MODE |= QEO_ABZ_MODE_B_TYPE_SET(config->output_type)
                        | QEO_ABZ_MODE_A_TYPE_SET(config->output_type);
    } else if (config->output_type == qeo_abz_output_three_phase) {
        base->ABZ.MODE |= QEO_ABZ_MODE_Z_TYPE_SET(config->output_type)
                        | QEO_ABZ_MODE_B_TYPE_SET(config->output_type)
                        | QEO_ABZ_MODE_A_TYPE_SET(config->output_type);
    } else {
        base->ABZ.MODE |= QEO_ABZ_MODE_Z_TYPE_SET(config->z_pulse_period)
                        | QEO_ABZ_MODE_B_TYPE_SET(config->output_type)
                        | QEO_ABZ_MODE_A_TYPE_SET(config->output_type);
    }
}

void qeo_pwm_get_default_safety_table_config(QEO_Type *base, qeo_pwm_safety_output_table_t *table)
{
    table->pwm7_output = qeo_pwm_safety_output_highz;
    table->pwm6_output = qeo_pwm_safety_output_highz;
    table->pwm5_output = qeo_pwm_safety_output_highz;
    table->pwm4_output = qeo_pwm_safety_output_highz;
    table->pwm3_output = qeo_pwm_safety_output_highz;
    table->pwm2_output = qeo_pwm_safety_output_highz;
    table->pwm1_output = qeo_pwm_safety_output_highz;
    table->pwm0_output = qeo_pwm_safety_output_highz;
}

void qeo_pwm_get_default_phase_table_config(QEO_Type *base, qeo_pwm_phase_output_table_t *table)
{
    table->pwm7_output = qeo_pwm_output_force_0;
    table->pwm6_output = qeo_pwm_output_force_0;
    table->pwm5_output = qeo_pwm_output_force_0;
    table->pwm4_output = qeo_pwm_output_force_0;
    table->pwm3_output = qeo_pwm_output_force_0;
    table->pwm2_output = qeo_pwm_output_force_0;
    table->pwm1_output = qeo_pwm_output_force_0;
    table->pwm0_output = qeo_pwm_output_force_0;
}

void qeo_pwm_get_default_mode_config(QEO_Type *base, qeo_pwm_mode_t *config)
{
    config->phase_num = 4;
    config->shield_hardware_trig_safety = false;
    config->revise_pairs_output = false;
}

void qeo_pwm_config_mode(QEO_Type *base, qeo_pwm_mode_t *config)
{
    base->PWM.MODE &= ~(QEO_PWM_MODE_PWM_SAFETY_BYPASS_MASK
                        | QEO_PWM_MODE_REVISE_UP_DN_MASK
                        | QEO_PWM_MODE_PHASE_NUM_MASK);
    base->PWM.MODE |= QEO_PWM_MODE_PWM_SAFETY_BYPASS_SET(config->shield_hardware_trig_safety)
                    | QEO_PWM_MODE_REVISE_UP_DN_SET(config->revise_pairs_output)
                    | QEO_PWM_MODE_PHASE_NUM_SET(config->phase_num);
}

void qeo_pwm_config_phase_table(QEO_Type *base, uint8_t index, qeo_pwm_phase_output_table_t *table)
{
    base->PWM.PHASE_TABLE[index] = QEO_PWM_PHASE_TABLE_PWM7_SET(table->pwm7_output)
                                | QEO_PWM_PHASE_TABLE_PWM6_SET(table->pwm6_output)
                                | QEO_PWM_PHASE_TABLE_PWM5_SET(table->pwm5_output)
                                | QEO_PWM_PHASE_TABLE_PWM4_SET(table->pwm4_output)
                                | QEO_PWM_PHASE_TABLE_PWM3_SET(table->pwm3_output)
                                | QEO_PWM_PHASE_TABLE_PWM2_SET(table->pwm2_output)
                                | QEO_PWM_PHASE_TABLE_PWM1_SET(table->pwm1_output)
                                | QEO_PWM_PHASE_TABLE_PWM0_SET(table->pwm0_output);
}

void qeo_pwm_config_safety_table(QEO_Type *base, qeo_pwm_safety_output_table_t *table)
{
    base->PWM.MODE &= 0xffffU; /*< clear safety table */
    base->PWM.MODE |= QEO_PWM_MODE_PWM7_SAFETY_SET(table->pwm7_output)
                    | QEO_PWM_MODE_PWM6_SAFETY_SET(table->pwm6_output)
                    | QEO_PWM_MODE_PWM5_SAFETY_SET(table->pwm5_output)
                    | QEO_PWM_MODE_PWM4_SAFETY_SET(table->pwm4_output)
                    | QEO_PWM_MODE_PWM3_SAFETY_SET(table->pwm3_output)
                    | QEO_PWM_MODE_PWM2_SAFETY_SET(table->pwm2_output)
                    | QEO_PWM_MODE_PWM1_SAFETY_SET(table->pwm1_output)
                    | QEO_PWM_MODE_PWM0_SAFETY_SET(table->pwm0_output);
}