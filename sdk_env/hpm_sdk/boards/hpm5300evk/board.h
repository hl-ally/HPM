/*
 * Copyright (c) 2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _HPM_BOARD_H
#define _HPM_BOARD_H
#include <stdio.h>
#include <stdarg.h>
#include "hpm_common.h"
#include "hpm_clock_drv.h"
#include "hpm_soc.h"
#include "hpm_soc_feature.h"
#include "pinmux.h"
#if !defined(CONFIG_NDEBUG_CONSOLE) || !CONFIG_NDEBUG_CONSOLE
#include "hpm_debug_console.h"
#endif

#define BOARD_NAME "hpm5300evk"
#define BOARD_UF2_SIGNATURE (0x0A4D5048UL)

/* ACMP desction */
#define BOARD_ACMP HPM_ACMP
#define BOARD_ACMP_CHANNEL ACMP_CHANNEL_CHN1
#define BOARD_ACMP_IRQ IRQn_ACMP_1
#define BOARD_ACMP_PLUS_INPUT ACMP_INPUT_DAC_OUT /* use internal DAC */
#define BOARD_ACMP_MINUS_INPUT ACMP_INPUT_ANALOG_5 /* align with used pin */

/* dma section */
#define BOARD_APP_HDMA HPM_HDMA
#define BOARD_APP_HDMA_IRQ IRQn_HDMA
#define BOARD_APP_DMAMUX HPM_DMAMUX
#define TEST_DMA_CONTROLLER HPM_HDMA
#define TEST_DMA_IRQ IRQn_HDMA

/* uart section */
#ifndef BOARD_RUNNING_CORE
#define BOARD_RUNNING_CORE HPM_CORE0
#endif
#ifndef BOARD_APP_UART_BASE
#define BOARD_APP_UART_BASE HPM_UART0
#define BOARD_APP_UART_IRQ  IRQn_UART0
#else
#ifndef BOARD_APP_UART_IRQ
#warning no IRQ specified for applicaiton uart
#endif
#endif

#define BOARD_APP_UART_BAUDRATE (115200UL)
#define BOARD_APP_UART_CLK_NAME clock_uart0
#define BOARD_APP_UART_RX_DMA_REQ HPM_DMA_SRC_UART0_RX
#define BOARD_APP_UART_TX_DMA_REQ HPM_DMA_SRC_UART0_TX

#ifndef BOARD_CONSOLE_TYPE
#define BOARD_CONSOLE_TYPE CONSOLE_TYPE_UART
#endif

#if BOARD_CONSOLE_TYPE == CONSOLE_TYPE_UART
#ifndef BOARD_CONSOLE_BASE
#define BOARD_CONSOLE_BASE HPM_UART0
#define BOARD_CONSOLE_CLK_NAME clock_uart0
#endif
#define BOARD_CONSOLE_BAUDRATE (115200UL)
#endif

#define BOARD_FREEMASTER_UART_BASE HPM_UART2
#define BOARD_FREEMASTER_UART_IRQ IRQn_UART2
#define BOARD_FREEMASTER_UART_CLK_NAME clock_uart2

/* nor flash section */
#define BOARD_FLASH_BASE_ADDRESS (0x80000000UL) /* Check */
#define BOARD_FLASH_SIZE (SIZE_1MB)

/* i2c section */
#define BOARD_APP_I2C_BASE HPM_I2C0
#define BOARD_APP_I2C_IRQ IRQn_I2C0
#define BOARD_APP_I2C_CLK_NAME clock_i2c0
#define BOARD_APP_I2C_DMA HPM_HDMA
#define BOARD_APP_I2C_DMAMUX HPM_DMAMUX
#define BOARD_APP_I2C_DMA_SRC HPM_DMA_SRC_I2C0

/* gptmr section */
#define BOARD_GPTMR                   HPM_GPTMR2
#define BOARD_GPTMR_IRQ               IRQn_GPTMR2
#define BOARD_GPTMR_CHANNEL           0
#define BOARD_GPTMR_DMA_SRC           HPM_DMA_SRC_GPTMR2_0
#define BOARD_GPTMR_CLK_NAME          clock_gptmr2
#define BOARD_GPTMR_PWM               HPM_GPTMR2
#define BOARD_GPTMR_PWM_CHANNEL       0
#define BOARD_GPTMR_PWM_DMA_SRC       HPM_DMA_SRC_GPTMR2_0
#define BOARD_GPTMR_PWM_CLK_NAME      clock_gptmr2
#define BOARD_GPTMR_PWM_IRQ           IRQn_GPTMR2
#define BOARD_GPTMR_PWM_SYNC          HPM_GPTMR2
#define BOARD_GPTMR_PWM_SYNC_CHANNEL  1
#define BOARD_GPTMR_PWM_SYNC_CLK_NAME clock_gptmr2

/* User LED */
#define BOARD_LED_GPIO_CTRL HPM_GPIO0
#define BOARD_LED_GPIO_INDEX GPIO_DI_GPIOA
#define BOARD_LED_GPIO_PIN 23

#define BOARD_LED_OFF_LEVEL 1
#define BOARD_LED_ON_LEVEL 0

/* 12V Power Enable*/
#define BOARD_12V_EN_GPIO_CTRL HPM_GPIO0
#define BOARD_12V_EN_GPIO_INDEX GPIO_DI_GPIOA
#define BOARD_12V_EN_GPIO_PIN 24

/* gpiom section */
#define BOARD_APP_GPIOM_BASE            HPM_GPIOM
#define BOARD_APP_GPIOM_USING_CTRL      HPM_FGPIO
#define BOARD_APP_GPIOM_USING_CTRL_NAME gpiom_core0_fast

/* User button */
#define BOARD_APP_GPIO_CTRL HPM_GPIO0
#define BOARD_APP_GPIO_INDEX GPIO_DI_GPIOA
#define BOARD_APP_GPIO_PIN 9
#define BOARD_APP_GPIO_IRQ IRQn_GPIO0_A

/* spi section */
#define BOARD_APP_SPI_BASE HPM_SPI1
#define BOARD_APP_SPI_CLK_NAME          clock_spi1
#define BOARD_APP_SPI_IRQ               IRQn_SPI1
#define BOARD_APP_SPI_SCLK_FREQ         (20000000UL)
#define BOARD_APP_SPI_ADDR_LEN_IN_BYTES (1U)
#define BOARD_APP_SPI_DATA_LEN_IN_BITS  (8U)
#define BOARD_APP_SPI_RX_DMA HPM_DMA_SRC_SPI1_RX
#define BOARD_APP_SPI_TX_DMA HPM_DMA_SRC_SPI1_TX
#define BOARD_SPI_CS_GPIO_CTRL           HPM_GPIO0
#define BOARD_SPI_CS_PIN                 IOC_PAD_PA26
#define BOARD_SPI_CS_ACTIVE_LEVEL        (0U)

/* ADC section */
#define BOARD_APP_ADC16_NAME      "ADC0"
#define BOARD_APP_ADC16_BASE      HPM_ADC0
#define BOARD_APP_ADC16_IRQn      IRQn_ADC0
#define BOARD_APP_ADC16_CH_1      (11U)
#define BOARD_APP_ADC16_CLK_NAME  (clock_adc0)

#ifndef ADC_SOC_PMT_NO_TRIGSOURCE
#define BOARD_APP_ADC16_PMT_PWM      HPM_PWM0
#define BOARD_APP_ADC16_PMT_TRGM     HPM_TRGM0
#define BOARD_APP_ADC16_PMT_TRGM_IN  HPM_TRGM0_INPUT_SRC_PWM0_CH8REF
#define BOARD_APP_ADC16_PMT_TRGM_OUT TRGM_TRGOCFG_ADCX_PTRGI0A
#endif

#define BOARD_APP_ADC16_PMT_TRIG_CH        ADC16_CONFIG_TRG0A

/* DAC section */
#define BOARD_DAC_BASE            HPM_DAC0
#define BOARD_DAC_IRQn            IRQn_DAC0
#define BOARD_APP_DAC_CLOCK_NAME  clock_dac0

/* Flash section */
#define BOARD_APP_XPI_NOR_XPI_BASE            (HPM_XPI0)
#define BOARD_APP_XPI_NOR_CFG_OPT_HDR         (0xfcf90002U)
#define BOARD_APP_XPI_NOR_CFG_OPT_OPT0        (0x00000006U)
#define BOARD_APP_XPI_NOR_CFG_OPT_OPT1        (0x00001000U)

/* SDXC section */
#define BOARD_APP_SDCARD_SDXC_BASE            (HPM_SDXC0)
#define BOARD_APP_SDCARD_SUPPORT_1V8          (0)

/* MCAN section */
#define BOARD_APP_CAN_BASE         HPM_MCAN3
#define BOARD_APP_CAN_IRQn         IRQn_CAN3

/* CALLBACK TIMER section */
#define BOARD_CALLBACK_TIMER (HPM_GPTMR3)
#define BOARD_CALLBACK_TIMER_CH 1
#define BOARD_CALLBACK_TIMER_IRQ IRQn_GPTMR3
#define BOARD_CALLBACK_TIMER_CLK_NAME (clock_gptmr3)

/* APP PWM */
#define BOARD_APP_PWM HPM_PWM0
#define BOARD_APP_PWM_CLOCK_NAME clock_mot0
#define BOARD_APP_PWM_OUT1 0
#define BOARD_APP_PWM_OUT2 1
#define BOARD_APP_TRGM HPM_TRGM0
#define BOARD_APP_PWM_IRQ IRQn_PWM0
#define BOARD_APP_TRGM_PWM_OUTPUT TRGM_TRGOCFG_PWM0_SYNCI

/*BLDC pwm*/
/*PWM define*/
#define BOARD_BLDCPWM                     HPM_PWM0
#define BOARD_BLDC_UH_PWM_OUTPIN         (5U)
#define BOARD_BLDC_UL_PWM_OUTPIN         (2U)
#define BOARD_BLDC_VH_PWM_OUTPIN         (6U)
#define BOARD_BLDC_VL_PWM_OUTPIN         (3U)
#define BOARD_BLDC_WH_PWM_OUTPIN         (7U)
#define BOARD_BLDC_WL_PWM_OUTPIN         (4U)
#define BOARD_BLDCPWM_TRGM                HPM_TRGM0
#define BOARD_BLDCAPP_PWM_IRQ             IRQn_PWM0
#define BOARD_BLDCPWM_CMP_INDEX_0         (0U)
#define BOARD_BLDCPWM_CMP_INDEX_1         (1U)
#define BOARD_BLDCPWM_CMP_INDEX_2         (2U)
#define BOARD_BLDCPWM_CMP_INDEX_3         (3U)
#define BOARD_BLDCPWM_CMP_INDEX_4         (4U)
#define BOARD_BLDCPWM_CMP_INDEX_5         (5U)
#define BOARD_BLDCPWM_CMP_TRIG_CMP        (20U)

/*HALL define*/

/*RDC*/
#define BOARD_RDC_TRGM                  HPM_TRGM0
#define BOARD_RDC_TRGIGMUX_IN_NUM       HPM_TRGM0_INPUT_SRC_RDC_TRGO_0
#define BOARD_RDC_TRG_NUM               TRGM_TRGOCFG_MOT_GPIO0
#define BOARD_RDC_TRG_ADC_NUM           TRGM_TRGOCFG_ADCX_PTRGI0A
#define BOARD_RDC_ADC_I_BASE  HPM_ADC0
#define BOARD_RDC_ADC_Q_BASE  HPM_ADC1
#define BOARD_RDC_ADC_I_CHN     (5U)
#define BOARD_RDC_ADC_Q_CHN     (6U)
#define BOARD_RDC_ADC_IRQn   IRQn_ADC0
#define BOARD_RDC_ADC_TRIG_FLAG               adc16_event_trig_complete
#define BOARD_RDC_ADC_TRG                    ADC16_CONFIG_TRG0A

/*QEI*/
#define BOARD_BLDC_QEI_TRGM              HPM_TRGM0
#define BOARD_BLDC_QEI_BASE              HPM_QEI1
#define BOARD_BLDC_QEI_IRQ               IRQn_QEI1
#define BOARD_BLDC_QEI_MOTOR_PHASE_COUNT_PER_REV     (16U)
#define BOARD_BLDC_QEI_CLOCK_SOURCE      clock_mot0
#define BOARD_BLDC_QEI_FOC_PHASE_COUNT_PER_REV       (4000U)
#define BOARD_BLDC_QEI_ADC_MATRIX_ADCX    trgm_adc_matrix_output_to_qei1_adcx
#define BOARD_BLDC_QEI_ADC_MATRIX_ADCY    trgm_adc_matrix_output_to_qei1_adcy

/*Timer define*/
#define BOARD_BLDC_TMR_1MS                       HPM_GPTMR2
#define BOARD_BLDC_TMR_CH                        0
#define BOARD_BLDC_TMR_CMP                       0
#define BOARD_BLDC_TMR_IRQ                       IRQn_GPTMR2
#define BOARD_BLDC_TMR_RELOAD                    (100000U)

/*adc*/
#define BOARD_BLDC_ADC_MODULE                  (ADCX_MODULE_ADC16)
#define BOARD_BLDC_ADC_U_BASE                  HPM_ADC0
#define BOARD_BLDC_ADC_V_BASE                  HPM_ADC0
#define BOARD_BLDC_ADC_W_BASE                  HPM_ADC0
#define BOARD_BLDC_ADC_TRIG_FLAG               adc16_event_trig_complete

#define BOARD_BLDC_ADC_CH_U                    (5U)
#define BOARD_BLDC_ADC_CH_V                    (6U)
#define BOARD_BLDC_ADC_CH_W                    (4U)
#define BOARD_BLDC_ADC_IRQn                    IRQn_ADC0
#define BOARD_BLDC_ADC_SEQ_DMA_SIZE_IN_4BYTES  (40U)
#define BOARD_BLDC_ADC_TRG                    ADC16_CONFIG_TRG0A
#define BOARD_BLDC_ADC_PREEMPT_TRIG_LEN        (1U)
#define BOARD_BLDC_PWM_TRIG_CMP_INDEX          (8U)
#define BOARD_BLDC_TRIGMUX_IN_NUM              HPM_TRGM0_INPUT_SRC_PWM0_CH8REF
#define BOARD_BLDC_TRG_NUM                     TRGM_TRGOCFG_ADCX_PTRGI0A

#define BOARD_PLB_COUNTER   HPM_PLB
#define BOARD_PLB_PWM_BASE  HPM_PWM0
#define BOARD_PLB_PWM_CLOCK_NAME clock_mot0
#define BOARD_PLB_TRGM                HPM_TRGM0
#define BOARD_PLB_PWM_TRG                       (HPM_TRGM0_INPUT_SRC_PWM0_CH8REF)
#define BOARD_PLB_IN_PWM_TRG_NUM                (TRGM_TRGOCFG_PLB_IN_00)
#define BOARD_PLB_IN_PWM_PULSE_TRG_NUM          (TRGM_TRGOCFG_PLB_IN_02)
#define BOARD_PLB_OUT_TRG                       (HPM_TRGM0_INPUT_SRC_PLB_OUT00)
#define BOARD_PLB_IO_TRG_NUM                    (TRGM_TRGOCFG_MOT_GPIO2)
#define BOARD_PLB_IO_TRG_SHIFT                    (2)
#define BOARD_PLB_PWM_CMP                       (8U)
#define BOARD_PLB_PWM_CHN                       (8U)
#define BOARD_PLB_CHN    plb_chn0

/* lin section */
#define BOARD_LIN             HPM_LIN3
#define BOARD_LIN_CLK_NAME    clock_lin3
#define BOARD_LIN_IRQ         IRQn_LIN3
#define BOARD_LIN_BAUDRATE    (19200U)

/* QEO */
#define BOARD_QEO HPM_QEO0
#define BOARD_QEO_TRGM_POS trgm_pos_matrix_output_to_qeo0

/* moto */
#define BOARD_MOTOR_CLK_NAME clock_mot0

/* SEI */
#define BOARD_SEI_CTRL SEI_CTRL_1
#define BOARD_SEI_IRQn IRQn_SEI1

/* USB */
#define BOARD_USB HPM_USB0

#ifndef BOARD_SHOW_CLOCK
#define BOARD_SHOW_CLOCK 1
#endif
#ifndef BOARD_SHOW_BANNER
#define BOARD_SHOW_BANNER 1
#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef void (*board_timer_cb)(void);

void board_init_gpio_pins(void);
void board_init_led_pins(void);
void board_init_usb_pins(void);
void board_led_write(uint8_t state);
void board_led_toggle(void);
void board_init_uart(UART_Type *ptr);
uint32_t board_init_spi_clock(SPI_Type *ptr);
void board_init_spi_pins(SPI_Type *ptr);
void board_usb_vbus_ctrl(uint8_t usb_index, uint8_t level);
uint32_t board_init_adc16_clock(ADC16_Type *ptr, bool clk_src_ahb);
void board_init_adc16_pins(void);
uint32_t board_init_dac_clock(DAC_Type *ptr, bool clk_src_ahb);
void board_init_can(MCAN_Type *ptr);
uint32_t board_init_can_clock(MCAN_Type *ptr);
void board_init_rgb_pwm_pins(void);
void board_disable_output_rgb_led(uint8_t color);
void board_enable_output_rgb_led(uint8_t color);
void board_init_dac_pins(DAC_Type *ptr);
void board_write_spi_cs(uint32_t pin, uint8_t state);
void board_init_spi_pins_with_gpio_as_cs(SPI_Type *ptr);
void board_init_lin_pins(LINV2_Type *ptr);
uint32_t board_init_lin_clock(LINV2_Type *ptr);

void board_init(void);
void board_init_clock(void);
void board_delay_us(uint32_t us);
void board_delay_ms(uint32_t ms);
void board_timer_create(uint32_t ms, board_timer_cb cb);
void board_ungate_mchtmr_at_lp_mode(void);

uint8_t board_get_led_gpio_off_level(void);
uint8_t board_get_led_pwm_off_level(void);

void board_init_pmp(void);

uint32_t board_init_uart_clock(UART_Type *ptr);
void board_init_sei_pins(SEI_Type *ptr, uint8_t sei_ctrl_idx);
void board_init_qeiv2_abz_uvw_pins(QEIV2_Type *ptr);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _HPM_BOARD_H */
