#include "app_adc.h"
#include "board.h"
#include "hpm_adc16_drv.h"
#include "hpm_pwm_drv.h"
#ifndef ADC_SOC_NO_HW_TRIG_SRC
#include "hpm_trgm_drv.h"
#include "hpm_trgmmux_src.h"
#endif

#ifndef APP_ADC16_CORE
#define APP_ADC16_CORE BOARD_RUNNING_CORE
#endif

#define APP_ADC16_CH_SAMPLE_CYCLE            (20U)
#define APP_ADC16_CH_WDOG_EVENT              (1 << BOARD_APP_ADC16_CH_1)

#define APP_ADC16_SEQ_START_POS              (0U)
#define APP_ADC16_SEQ_DMA_BUFF_LEN_IN_4BYTES (1024U)
#define APP_ADC16_SEQ_IRQ_EVENT              adc16_event_seq_single_complete

#ifndef ADC_SOC_NO_HW_TRIG_SRC
#define APP_ADC16_HW_TRIG_SRC_PWM_REFCH_A    (8U)
#define APP_ADC16_HW_TRIG_SRC                BOARD_APP_ADC16_HW_TRIG_SRC
#define APP_ADC16_HW_TRGM                    BOARD_APP_ADC16_HW_TRGM
#define APP_ADC16_HW_TRGM_IN                 BOARD_APP_ADC16_HW_TRGM_IN
#define APP_ADC16_HW_TRGM_OUT_SEQ            BOARD_APP_ADC16_HW_TRGM_OUT_SEQ
#define APP_ADC16_HW_TRGM_OUT_PMT            BOARD_APP_ADC16_HW_TRGM_OUT_PMT
#endif

#define APP_ADC16_PMT_TRIG_CH                BOARD_APP_ADC16_PMT_TRIG_CH
#define APP_ADC16_PMT_DMA_BUFF_LEN_IN_4BYTES ADC_SOC_PMT_MAX_DMA_BUFF_LEN_IN_4BYTES
#define APP_ADC16_PMT_IRQ_EVENT              adc16_event_trig_complete

ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(ADC_SOC_DMA_ADDR_ALIGNMENT) uint32_t seq_buff[APP_ADC16_SEQ_DMA_BUFF_LEN_IN_4BYTES];
ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(ADC_SOC_DMA_ADDR_ALIGNMENT) uint32_t pmt_buff[APP_ADC16_PMT_DMA_BUFF_LEN_IN_4BYTES];

uint8_t seq_adc_channel[] = {BOARD_APP_ADC16_CH_1};
uint8_t trig_adc_channel[] = {BOARD_APP_ADC16_CH_1};

__IO uint8_t seq_complete_flag;
__IO uint8_t trig_complete_flag;
__IO uint32_t res_out_of_thr_flag;

/*
typedef enum {
    adc16_conv_mode_oneshot = 0,    // 读取转换模式
    adc16_conv_mode_period,         // 周期转换模式
    adc16_conv_mode_sequence,       // 序列转换模式
    adc16_conv_mode_preemption      // 抢占转换模式
} adc16_conversion_mode_t;
*/
adc16_conversion_mode_t conv_mode = adc16_conv_mode_sequence;

void isr_adc16(void)
{
    uint32_t status;

    status = adc16_get_status_flags(BOARD_APP_ADC16_BASE);

    /* Clear status */
    adc16_clear_status_flags(BOARD_APP_ADC16_BASE, status);

    if(ADC16_INT_STS_SEQ_CVC_GET(status))
    {
        /* Set flag to read memory data */
        seq_complete_flag = 1;
    }

    if(ADC16_INT_STS_TRIG_CMPT_GET(status))
    {
        /* Set flag to read memory data */
        trig_complete_flag = 1;
    }

    if(ADC16_INT_STS_WDOG_GET(status) & APP_ADC16_CH_WDOG_EVENT)
    {
        adc16_disable_interrupts(BOARD_APP_ADC16_BASE, APP_ADC16_CH_WDOG_EVENT);
        res_out_of_thr_flag = ADC16_INT_STS_WDOG_GET(status) & APP_ADC16_CH_WDOG_EVENT;
    }
}
SDK_DECLARE_EXT_ISR_M(BOARD_APP_ADC16_IRQn, isr_adc16);

hpm_stat_t process_seq_data(uint32_t *buff, int32_t start_pos, uint32_t len)
{
    adc16_seq_dma_data_t *dma_data = (adc16_seq_dma_data_t *)buff;

    if(ADC16_IS_SEQ_DMA_BUFF_LEN_INVLAID(len))
    {
        return status_invalid_argument;
    }

    for(int i = start_pos; i < start_pos + len; i++)
    {
        printf("Sequence Mode - %s - ", BOARD_APP_ADC16_NAME);
        printf("Cycle Bit: %02d - ",   dma_data[i].cycle_bit);
        printf("Sequence Number:%02d - ", dma_data[i].seq_num);
        printf("ADC Channel: %02d - ",  dma_data[i].adc_ch);
        printf("Result: 0x%04x\n", dma_data[i].result);
    }

    return status_success;
}

hpm_stat_t process_pmt_data(uint32_t *buff, int32_t start_pos, uint32_t len)
{
    adc16_pmt_dma_data_t *dma_data = (adc16_pmt_dma_data_t *)buff;

    if(ADC16_IS_PMT_DMA_BUFF_LEN_INVLAID(len))
    {
        return status_invalid_argument;
    }

    for(int i = start_pos; i < start_pos + len; i++)
    {
        if(dma_data[i].cycle_bit)
        {
            printf("Preemption Mode - %s - ", BOARD_APP_ADC16_NAME);
            printf("Trigger Channel: %02d - ", dma_data[i].trig_ch);
            printf("Cycle Bit: %02d - ", dma_data[i].cycle_bit);
            printf("Sequence Number: %02d - ", dma_data[i].seq_num);
            printf("ADC Channel: %02d - ", dma_data[i].adc_ch);
            printf("Result: 0x%04x\n", dma_data[i].result);
            dma_data[i].cycle_bit = 0;
        }
        else
        {
            printf("invalid data\n");
        }
    }

    return status_success;
}

#ifndef ADC_SOC_NO_HW_TRIG_SRC
void init_trigger_source(PWM_Type *ptr)
{
    pwm_cmp_config_t pwm_cmp_cfg;
    pwm_output_channel_t pwm_output_ch_cfg;

    /* TODO: Set PWM Clock Source and divider */

    /* 33.33KHz reload at 200MHz */
    pwm_set_reload(ptr, 0, 5999);

    /* Set a comparator */
    memset(&pwm_cmp_cfg, 0x00, sizeof(pwm_cmp_config_t));
    pwm_cmp_cfg.enable_ex_cmp  = false;
    pwm_cmp_cfg.mode           = pwm_cmp_mode_output_compare;
    pwm_cmp_cfg.update_trigger = pwm_shadow_register_update_on_shlk;

    /* Select comp8 and trigger at the middle of a pwm cycle */
    pwm_cmp_cfg.cmp = 2999;
    pwm_config_cmp(ptr, APP_ADC16_HW_TRIG_SRC_PWM_REFCH_A, &pwm_cmp_cfg);

    /* Issue a shadow lock */
    pwm_issue_shadow_register_lock_event(APP_ADC16_HW_TRIG_SRC);

    /* Set comparator channel to generate a trigger signal */
    pwm_output_ch_cfg.cmp_start_index = APP_ADC16_HW_TRIG_SRC_PWM_REFCH_A;   /* start channel */
    pwm_output_ch_cfg.cmp_end_index   = APP_ADC16_HW_TRIG_SRC_PWM_REFCH_A;   /* end channel */
    pwm_output_ch_cfg.invert_output   = false;
    pwm_config_output_channel(ptr, APP_ADC16_HW_TRIG_SRC_PWM_REFCH_A, &pwm_output_ch_cfg);

    /* Start the comparator counter */
    pwm_start_counter(ptr);
}

void init_trigger_mux(TRGM_Type *ptr, uint8_t output)
{
    trgm_output_t trgm_output_cfg;

    trgm_output_cfg.invert = false;
    trgm_output_cfg.type = trgm_output_same_as_input;

    trgm_output_cfg.input  = APP_ADC16_HW_TRGM_IN;
    trgm_output_config(ptr, output, &trgm_output_cfg);
}
#endif

void init_trigger_target(ADC16_Type *ptr, uint8_t trig_ch)
{
    adc16_pmt_config_t pmt_cfg;

    pmt_cfg.trig_len = sizeof(trig_adc_channel);
    pmt_cfg.trig_ch = trig_ch;

    for(int i = 0; i < pmt_cfg.trig_len; i++)
    {
        pmt_cfg.adc_ch[i] = trig_adc_channel[i];
        pmt_cfg.inten[i] = false;
    }

    pmt_cfg.inten[pmt_cfg.trig_len - 1] = true;

    adc16_set_pmt_config(ptr, &pmt_cfg);
    adc16_set_pmt_queue_enable(ptr, trig_ch, true);
}

hpm_stat_t init_adc_config(adc16_conversion_mode_t conv_mode)
{
    adc16_config_t cfg;

    /* initialize an ADC instance */
    adc16_get_default_config(&cfg);

    cfg.res            = adc16_res_16_bits;
    cfg.conv_mode      = conv_mode;
    cfg.adc_clk_div    = adc16_clock_divider_4;
    cfg.sel_sync_ahb   = (clk_adc_src_ahb0 == clock_get_source(BOARD_APP_ADC16_CLK_NAME)) ? true : false;

    if(cfg.conv_mode == adc16_conv_mode_sequence ||
        cfg.conv_mode == adc16_conv_mode_preemption)
    {
        cfg.adc_ahb_en = true;
    }

    /* adc16 initialization */
    if(adc16_init(BOARD_APP_ADC16_BASE, &cfg) == status_success)
    {
        /* enable irq */
        intc_m_enable_irq_with_priority(BOARD_APP_ADC16_IRQn, 1);
        return status_success;
    }
    else
    {
        printf("%s initialization failed!\n", BOARD_APP_ADC16_NAME);
        return status_fail;
    }
}

void channel_result_out_of_threshold_handler(void)
{
    adc16_channel_threshold_t threshold;
    uint32_t i = 31;

    if(res_out_of_thr_flag)
    {
        while (i--)
        {
            if((res_out_of_thr_flag >> i) & 0x01)
            {
                adc16_get_channel_threshold(BOARD_APP_ADC16_BASE, i, &threshold);
                printf("Warning - %s [channel %02d] - Sample voltage is out of the thresholds between 0x%04x and 0x%04x !\n", BOARD_APP_ADC16_NAME, i, threshold.thshdl, threshold.thshdh);
            }
        }

        res_out_of_thr_flag = 0;
        adc16_enable_interrupts(BOARD_APP_ADC16_BASE, APP_ADC16_CH_WDOG_EVENT);
    }
}

void init_oneshot_config(void)
{
    adc16_channel_config_t ch_cfg;

    /* get a default channel config */
    adc16_get_channel_default_config(&ch_cfg);

    /* initialize an ADC channel */
    ch_cfg.ch           = BOARD_APP_ADC16_CH_1;
    ch_cfg.sample_cycle = 20;

    adc16_init_channel(BOARD_APP_ADC16_BASE, &ch_cfg);

    adc16_set_nonblocking_read(BOARD_APP_ADC16_BASE);

#if defined(ADC_SOC_BUSMODE_ENABLE_CTRL_SUPPORT) && ADC_SOC_BUSMODE_ENABLE_CTRL_SUPPORT
    /* enable oneshot mode */
    adc16_enable_oneshot_mode(BOARD_APP_ADC16_BASE);
#endif
}

void oneshot_handler(void)
{
    uint16_t result;

    if(adc16_get_oneshot_result(BOARD_APP_ADC16_BASE, BOARD_APP_ADC16_CH_1, &result) == status_success)
    {
        if(adc16_is_nonblocking_mode(BOARD_APP_ADC16_BASE))
        {
            adc16_get_oneshot_result(BOARD_APP_ADC16_BASE, BOARD_APP_ADC16_CH_1, &result);
        }
        printf("Oneshot Mode - %s [channel %02d] - Result: 0x%04x\n", BOARD_APP_ADC16_NAME, BOARD_APP_ADC16_CH_1, result);
    }
}

void init_period_config(void)
{
    adc16_channel_config_t ch_cfg;
    adc16_prd_config_t prd_cfg;

    /* get a default channel config */
    adc16_get_channel_default_config(&ch_cfg);

    /* initialize an ADC channel */
    ch_cfg.ch           = BOARD_APP_ADC16_CH_1;
    ch_cfg.sample_cycle = 20;

    adc16_init_channel(BOARD_APP_ADC16_BASE, &ch_cfg);

    prd_cfg.ch           = BOARD_APP_ADC16_CH_1;
    prd_cfg.prescale     = 22;    /* Set divider: 2^22 clocks */
    prd_cfg.period_count = 5;     /* 104.86ms when AHB clock at 200MHz is ADC clock source */

    adc16_set_prd_config(BOARD_APP_ADC16_BASE, &prd_cfg);
}

void period_handler(void)
{
    uint16_t result;

    adc16_get_prd_result(BOARD_APP_ADC16_BASE, BOARD_APP_ADC16_CH_1, &result);
    printf("Period Mode - %s [channel %02d] - Result: 0x%04x\n", BOARD_APP_ADC16_NAME, BOARD_APP_ADC16_CH_1, result);
}

void init_sequence_config(void)
{
    adc16_seq_config_t seq_cfg;
    adc16_dma_config_t dma_cfg;
    adc16_channel_config_t ch_cfg;

    /* get a default channel config */
    adc16_get_channel_default_config(&ch_cfg);

    /* initialize an ADC channel */
    ch_cfg.sample_cycle = 20;

    for(int i = 0; i < sizeof(seq_adc_channel); i++)
    {
        ch_cfg.ch           = seq_adc_channel[i];
        adc16_init_channel(BOARD_APP_ADC16_BASE, &ch_cfg);
    }

    /* Set a sequence config */
    seq_cfg.seq_len    = sizeof(seq_adc_channel);
    seq_cfg.restart_en = false;
    seq_cfg.cont_en    = true;
#ifndef ADC_SOC_NO_HW_TRIG_SRC
    #ifndef __ADC16_USE_SW_TRIG
    seq_cfg.hw_trig_en = true;
    seq_cfg.sw_trig_en = false;
    #else
    seq_cfg.hw_trig_en = false;
    seq_cfg.sw_trig_en = true;
    #endif
#else
    seq_cfg.hw_trig_en = false;
    seq_cfg.sw_trig_en = true;
#endif

    for(int i = APP_ADC16_SEQ_START_POS; i < seq_cfg.seq_len; i++)
    {
        seq_cfg.queue[i].seq_int_en = false;
        seq_cfg.queue[i].ch = seq_adc_channel[i];
    }

    /* Enable the single complete interrupt for the last conversion */
    seq_cfg.queue[seq_cfg.seq_len - 1].seq_int_en = true;

    /* Initialize a sequence */
    adc16_set_seq_config(BOARD_APP_ADC16_BASE, &seq_cfg);

    /* Set a DMA config */
    dma_cfg.start_addr         = (uint32_t *)core_local_mem_to_sys_address(APP_ADC16_CORE, (uint32_t)seq_buff);
    dma_cfg.buff_len_in_4bytes = sizeof(seq_adc_channel);
    dma_cfg.stop_en            = false;
    dma_cfg.stop_pos           = 0;

#if !defined(ADC_SOC_NO_HW_TRIG_SRC) && !defined(__ADC16_USE_SW_TRIG)
    /* Trigger source initialization */
    init_trigger_source(APP_ADC16_HW_TRIG_SRC);

    /* Trigger mux initialization */
    init_trigger_mux(APP_ADC16_HW_TRGM, APP_ADC16_HW_TRGM_OUT_SEQ);
#endif

    /* Initialize DMA for the sequence mode */
    adc16_init_seq_dma(BOARD_APP_ADC16_BASE, &dma_cfg);

    /* Enable sequence complete interrupt */
    adc16_enable_interrupts(BOARD_APP_ADC16_BASE, APP_ADC16_SEQ_IRQ_EVENT);
}

void sequence_handler(void)
{
#if defined(ADC_SOC_NO_HW_TRIG_SRC) || defined(__ADC16_USE_SW_TRIG)
    /* SW trigger */
    adc16_trigger_seq_by_sw(BOARD_APP_ADC16_BASE);
#endif

    while(seq_complete_flag == 0)
    {

    }

    /* Process data */
    process_seq_data(seq_buff, APP_ADC16_SEQ_START_POS, sizeof(seq_adc_channel));

    /* Clear the flag */
    seq_complete_flag = 0;
}

void init_preemption_config(void)
{
    adc16_channel_config_t ch_cfg;

    /* get a default channel config */
    adc16_get_channel_default_config(&ch_cfg);

    /* initialize an ADC channel */
    ch_cfg.sample_cycle = 20;

    for(int i = 0; i < sizeof(trig_adc_channel); i++)
    {
        ch_cfg.ch = trig_adc_channel[i];
        adc16_init_channel(BOARD_APP_ADC16_BASE, &ch_cfg);
    }

#if !defined(ADC_SOC_NO_HW_TRIG_SRC) && !defined(__ADC16_USE_SW_TRIG)
    /* Trigger source initialization */
    init_trigger_source(APP_ADC16_HW_TRIG_SRC);

    /* Trigger mux initialization */
    init_trigger_mux(APP_ADC16_HW_TRGM, APP_ADC16_HW_TRGM_OUT_PMT);
#endif

    /* Trigger target initialization */
    init_trigger_target(BOARD_APP_ADC16_BASE, APP_ADC16_PMT_TRIG_CH);

    /* Set DMA start address for preemption mode */
    adc16_init_pmt_dma(BOARD_APP_ADC16_BASE, core_local_mem_to_sys_address(APP_ADC16_CORE, (uint32_t)pmt_buff));

    /* Enable trigger complete interrupt */
    adc16_enable_interrupts(BOARD_APP_ADC16_BASE, APP_ADC16_PMT_IRQ_EVENT);
}

void preemption_handler(void)
{
#if defined(ADC_SOC_NO_HW_TRIG_SRC) || defined(__ADC16_USE_SW_TRIG)
    /* SW trigger */
    adc16_trigger_pmt_by_sw(BOARD_APP_ADC16_BASE, APP_ADC16_PMT_TRIG_CH);
#endif

    /* Wait for a complete of conversion */
    while(trig_complete_flag == 0)
    {

    }

    /* Process data */
    process_pmt_data(pmt_buff, APP_ADC16_PMT_TRIG_CH * sizeof(adc16_pmt_dma_data_t), sizeof(trig_adc_channel));

    /* Clear the flag */
    trig_complete_flag = 0;
}


void adc_test_init(void)
{
    uint32_t freq = 0;
    board_init_adc16_pins();    // ADC pin init
    freq = board_init_adc16_clock(BOARD_APP_ADC16_BASE, false); // ADC clock init
    printf("adc clock is %d\n", freq);

    init_adc_config(conv_mode);

    switch(conv_mode)
    {
    case adc16_conv_mode_oneshot:
        init_oneshot_config();
        break;

    case adc16_conv_mode_period:
        init_period_config();
        break;

    case adc16_conv_mode_sequence:
        init_sequence_config();
        break;

    case adc16_conv_mode_preemption:
        init_preemption_config();
        break;

    default:
        break;
    }
}


void adc_test_process(void)
{
    channel_result_out_of_threshold_handler();

    if(conv_mode == adc16_conv_mode_oneshot)
    {
        oneshot_handler();
    }
    else if(conv_mode == adc16_conv_mode_period)
    {
        period_handler();
    }
    else if(conv_mode == adc16_conv_mode_sequence)
    {
        sequence_handler();
    }
    else if(conv_mode == adc16_conv_mode_preemption)
    {
        preemption_handler();
    }
    else
    {
        printf("Conversion mode is not supported!\n");
    }
}
