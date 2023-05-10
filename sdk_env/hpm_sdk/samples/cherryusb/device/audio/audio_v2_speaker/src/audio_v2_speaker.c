/*
 * Copyright (c) 2022 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "usbd_core.h"
#include "usbd_audio.h"
#include "board.h"
#include "hpm_i2s_drv.h"
#include "hpm_dma_drv.h"
#include "hpm_dmamux_drv.h"
#include "audio_v2_speaker.h"

#if defined(USING_CODEC) && USING_CODEC
#if defined(CONFIG_CODEC_WM8960) && CONFIG_CODEC_WM8960
#include "hpm_wm8960.h"
wm8960_config_t wm8960_config = {
    .route = wm8960_route_playback,
    .left_input = wm8960_input_closed,
    .right_input = wm8960_input_closed,
    .play_source = wm8960_play_source_dac,
    .bus = wm8960_bus_i2s,
    .format = { .mclk_hz = 0U, .sample_rate = 0U, .bit_width = 32U },
};

wm8960_control_t wm8960_control = {
    .ptr = CODEC_I2C,
    .slave_address = WM8960_I2C_ADDR, /* I2C address */
};
#elif defined(CONFIG_CODEC_SGTL5000) && CONFIG_CODEC_SGTL5000
#include "hpm_sgtl5000.h"
sgtl_config_t sgtl5000_config = {
    .route = sgtl_route_playback_record, /*!< Audio data route.*/
    .bus = sgtl_bus_left_justified,      /*!< Audio transfer protocol */
    .master = false,                     /*!< Master or slave. True means master, false means slave. */
    .format = { .mclk_hz = 0,
                .sample_rate = 0,
                .bit_width = 32,
                .sclk_edge = sgtl_sclk_valid_edge_rising }, /*!< audio format */
};

sgtl_context_t sgtl5000_context = {
    .ptr = CODEC_I2C,
    .slave_address = SGTL5000_I2C_ADDR, /* I2C address */
};
#else
#error no specified Audio Codec!!!
#endif
#elif defined(USING_DAO) && USING_DAO
#include "hpm_dao_drv.h"
dao_config_t dao_config;
#else
#error define USING_CODEC or USING_DAO
#endif

#define USBD_VID           0xffff
#define USBD_PID           0xffff
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#ifdef CONFIG_USB_HS
#define EP_INTERVAL 0x04
#else
#define EP_INTERVAL 0x01
#endif

#define AUDIO_OUT_EP 0x01

#define SPEAKER_SAMPLE_FREQ    96000
#define SPEAKER_SLOT_BYTE_SIZE 4
#define SPEAKER_AUDIO_DEPTH    24

#define OUT_CHANNEL_NUM 2

#if OUT_CHANNEL_NUM == 1
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x00000000
#elif OUT_CHANNEL_NUM == 2
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x00000003
#elif OUT_CHANNEL_NUM == 3
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x00000007
#elif OUT_CHANNEL_NUM == 4
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000000f
#elif OUT_CHANNEL_NUM == 5
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000001f
#elif OUT_CHANNEL_NUM == 6
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000003F
#elif OUT_CHANNEL_NUM == 7
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000007f
#elif OUT_CHANNEL_NUM == 8
#define OUTPUT_CTRL DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x000000ff
#endif

#define DMA_MUX_CHANNEL_SPEAKER 1U
#define AUDIO_BUFFER_COUNT      32
#define AUDIO_OUT_PACKET        ((uint32_t)((SPEAKER_SAMPLE_FREQ * SPEAKER_SLOT_BYTE_SIZE * OUT_CHANNEL_NUM) / 1000))

#define BMCONTROL (AUDIO_V2_FU_CONTROL_MUTE | AUDIO_V2_FU_CONTROL_VOLUME)

#define USB_AUDIO_CONFIG_DESC_SIZ (9 +                                                     \
                                   AUDIO_V2_AC_DESCRIPTOR_INIT_LEN +                       \
                                   AUDIO_V2_SIZEOF_AC_CLOCK_SOURCE_DESC +                  \
                                   AUDIO_V2_SIZEOF_AC_INPUT_TERMINAL_DESC +                \
                                   AUDIO_V2_SIZEOF_AC_FEATURE_UNIT_DESC(OUT_CHANNEL_NUM) + \
                                   AUDIO_V2_SIZEOF_AC_OUTPUT_TERMINAL_DESC +               \
                                   AUDIO_V2_AS_DESCRIPTOR_INIT_LEN)

#define AUDIO_AC_SIZ (AUDIO_V2_SIZEOF_AC_HEADER_DESC +                        \
                      AUDIO_V2_SIZEOF_AC_CLOCK_SOURCE_DESC +                  \
                      AUDIO_V2_SIZEOF_AC_INPUT_TERMINAL_DESC +                \
                      AUDIO_V2_SIZEOF_AC_FEATURE_UNIT_DESC(OUT_CHANNEL_NUM) + \
                      AUDIO_V2_SIZEOF_AC_OUTPUT_TERMINAL_DESC)

const uint8_t audio_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0001, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_AUDIO_CONFIG_DESC_SIZ, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    AUDIO_V2_AC_DESCRIPTOR_INIT(0x00, 0x02, AUDIO_AC_SIZ, AUDIO_CATEGORY_SPEAKER, 0x00, 0x00),
    AUDIO_V2_AC_CLOCK_SOURCE_DESCRIPTOR_INIT(0x01, 0x03, 0x07),
    AUDIO_V2_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x02, AUDIO_TERMINAL_STREAMING, 0x01, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, 0x0000),
    AUDIO_V2_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x03, 0x02, OUTPUT_CTRL),
    AUDIO_V2_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x04, AUDIO_OUTTERM_SPEAKER, 0x03, 0x01, 0x0000),
    AUDIO_V2_AS_DESCRIPTOR_INIT(0x01, 0x02, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, SPEAKER_SLOT_BYTE_SIZE, SPEAKER_AUDIO_DEPTH, AUDIO_OUT_EP, AUDIO_OUT_PACKET, EP_INTERVAL),
    /*
     * string0 descriptor
     */
    USB_LANGID_INIT(USBD_LANGID_STRING),
    /*
     * string1 descriptor
     */
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    /*
     * string2 descriptor
     */
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'U', 0x00,                  /* wcChar10 */
    'A', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    /*
     * string3 descriptor
     */
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '1', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '3', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '0', 0x00,                  /* wcChar7 */
    '0', 0x00,                  /* wcChar8 */
    '3', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    /*
     * device qualifier descriptor
     */
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};

static const uint8_t default_sampling_freq_table[] = {
    AUDIO_SAMPLE_FREQ_NUM(5),
    AUDIO_SAMPLE_FREQ_4B(8000),
    AUDIO_SAMPLE_FREQ_4B(8000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
    AUDIO_SAMPLE_FREQ_4B(16000),
    AUDIO_SAMPLE_FREQ_4B(16000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
    AUDIO_SAMPLE_FREQ_4B(32000),
    AUDIO_SAMPLE_FREQ_4B(32000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
    AUDIO_SAMPLE_FREQ_4B(48000),
    AUDIO_SAMPLE_FREQ_4B(48000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
    AUDIO_SAMPLE_FREQ_4B(96000),
    AUDIO_SAMPLE_FREQ_4B(96000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
};

/* Static Functions Declaration */
static void usbd_audio_iso_out_callback(uint8_t ep, uint32_t nbytes);
static hpm_stat_t speaker_init_i2s_playback(uint32_t sample_rate, uint8_t audio_depth, uint8_t channel_num);
static void speaker_i2s_dma_start_transfer(uint32_t addr, uint32_t size);
static bool speaker_out_buff_is_empty(void);

/* Static Variables */
static struct usbd_interface intf0;
static struct usbd_interface intf1;
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t s_speaker_out_buffer[AUDIO_BUFFER_COUNT][AUDIO_OUT_PACKET];
static uint32_t s_speaker_out_buffer_size[AUDIO_BUFFER_COUNT];
static volatile uint32_t s_speaker_i2s_mclk_hz;
static volatile bool s_speaker_rx_flag;
static volatile uint8_t s_speaker_out_buffer_front;
static volatile uint8_t s_speaker_out_buffer_rear;
static volatile bool s_speaker_dma_transfer_req;
static volatile bool s_speaker_dma_transfer_done;
static volatile bool s_speaker_mute;
static volatile bool s_set_speaker_mute_req;
static volatile float s_speaker_volume_percent;
static volatile bool s_set_speaker_volume_req;
static volatile uint32_t s_speaker_sampling_freq;
static volatile bool s_set_speaker_sampling_freq_req;
static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_iso_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

/* Extern Functions Definition */
void cherryusb_audio_init(void)
{
    usbd_desc_register(audio_descriptor);
    usbd_add_interface(usbd_audio_init_intf(&intf0));
    usbd_add_interface(usbd_audio_init_intf(&intf1));
    usbd_add_endpoint(&audio_out_ep);

    usbd_audio_add_entity(0x01, AUDIO_CONTROL_CLOCK_SOURCE);
    usbd_audio_add_entity(0x03, AUDIO_CONTROL_FEATURE_UNIT);

    usbd_initialize();
}

void speaker_init_i2s_dao_codec(void)
{
    (void)speaker_init_i2s_playback(SPEAKER_SAMPLE_FREQ, SPEAKER_AUDIO_DEPTH, OUT_CHANNEL_NUM);

#if defined(USING_CODEC) && USING_CODEC
#if defined(CONFIG_CODEC_WM8960) && CONFIG_CODEC_WM8960
    wm8960_config.route = wm8960_route_playback;
    wm8960_config.format.sample_rate = SPEAKER_SAMPLE_FREQ;
    wm8960_config.format.bit_width = SPEAKER_AUDIO_DEPTH;
    wm8960_config.format.mclk_hz = s_speaker_i2s_mclk_hz;
    if (wm8960_init(&wm8960_control, &wm8960_config) != status_success) {
        printf("Init Audio Codec failed\n");
    }
#elif defined(CONFIG_CODEC_SGTL5000) && CONFIG_CODEC_SGTL5000
    sgtl5000_config.route = sgtl_route_playback;
    sgtl5000_config.format.sample_rate = SPEAKER_SAMPLE_FREQ;
    sgtl5000_config.format.bit_width = SPEAKER_AUDIO_DEPTH;
    sgtl5000_config.format.mclk_hz = s_speaker_i2s_mclk_hz;
    if (sgtl_init(&sgtl5000_context, &sgtl5000_config) != status_success) {
        printf("Init Audio Codec failed\n");
    }
#endif
#elif defined(USING_DAO) && USING_DAO
    dao_get_default_config(HPM_DAO, &dao_config);
    dao_config.enable_mono_output = true;
    dao_init(HPM_DAO, &dao_config);
#endif
}

void i2s_enable_dma_irq_with_priority(int32_t priority)
{
    i2s_enable_tx_dma_request(TARGET_I2S);
    dmamux_config(BOARD_APP_DMAMUX, DMA_MUX_CHANNEL_SPEAKER, TARGET_I2S_TX_DMAMUX_SRC, true);

    intc_m_enable_irq_with_priority(BOARD_APP_HDMA_IRQ, priority);
}

void isr_dma(void)
{
    volatile uint32_t speaker_status;

    speaker_status = dma_check_transfer_status(BOARD_APP_HDMA, DMA_MUX_CHANNEL_SPEAKER);
    if (0 != (speaker_status & DMA_CHANNEL_STATUS_TC)) {
        s_speaker_dma_transfer_done = true;
    }
}
SDK_DECLARE_EXT_ISR_M(BOARD_APP_HDMA_IRQ, isr_dma)

void cherryusb_audio_main_task(void)
{
    hpm_stat_t state;

    if (s_set_speaker_sampling_freq_req) {
        state = speaker_init_i2s_playback(s_speaker_sampling_freq, SPEAKER_AUDIO_DEPTH, OUT_CHANNEL_NUM);
        if (state == status_success) {
#if defined(USING_CODEC) && USING_CODEC
#if defined(CONFIG_CODEC_WM8960) && CONFIG_CODEC_WM8960
            wm8960_set_data_format(&wm8960_control, s_speaker_i2s_mclk_hz, s_speaker_sampling_freq, SPEAKER_AUDIO_DEPTH);
#elif defined(CONFIG_CODEC_SGTL5000) && CONFIG_CODEC_SGTL5000
            sgtl_config_data_format(&sgtl5000_context, s_speaker_i2s_mclk_hz, s_speaker_sampling_freq, SPEAKER_AUDIO_DEPTH);
#endif
#endif
            USB_LOG_RAW("Init I2S Clock Ok! Sample Rate: %d, speaker_i2s_mclk_hz: %d\r\n", s_speaker_sampling_freq, s_speaker_i2s_mclk_hz);
        } else {
            USB_LOG_RAW("Init I2S Clock Fail!\r\n");
        }
        s_speaker_out_buffer_front = s_speaker_out_buffer_rear;
        s_speaker_dma_transfer_req = true;
        s_set_speaker_sampling_freq_req = false;
    }

    if (s_set_speaker_mute_req) {
#if defined(USING_CODEC) && USING_CODEC
#if defined(CONFIG_CODEC_WM8960) && CONFIG_CODEC_WM8960
        if (s_speaker_mute) {
            wm8960_set_volume(&wm8960_control, wm8960_module_dac, 0);
        } else {
            volume = 256 * s_speaker_volume_percent / 100;
            wm8960_set_volume(&wm8960_control, wm8960_module_dac, volume);
        }
#elif defined(CONFIG_CODEC_SGTL5000) && CONFIG_CODEC_SGTL5000
        if (sgtl_set_mute(&sgtl5000_context, sgtl_module_dac, s_speaker_mute) != status_success) {
            USB_LOG_RAW("set mute Fail!\r\n");
        }
#endif
#elif defined(USING_DAO) && USING_DAO
        if (s_speaker_mute) {
            dao_stop(HPM_DAO);
        } else {
            dao_start(HPM_DAO);
        }
#else
#error define USING_CODEC or USING_DAO
#endif
        s_set_speaker_mute_req = false;
    }

    if (s_set_speaker_volume_req) {
#if defined(USING_CODEC) && USING_CODEC
#if defined(CONFIG_CODEC_WM8960) && CONFIG_CODEC_WM8960
        volume = 256 * s_speaker_volume_percent / 100;
        if (wm8960_set_volume(&wm8960_control, wm8960_module_dac, volume) != status_success) {
            USB_LOG_RAW("set volume Fail!\r\n");
        }
#elif defined(CONFIG_CODEC_SGTL5000) && CONFIG_CODEC_SGTL5000
        volume = SGTL5000_DAC_MAX_VOLUME_VALUE - (SGTL5000_DAC_MAX_VOLUME_VALUE - SGTL5000_DAC_MIN_VOLUME_VALUE) * s_speaker_volume_percent / 100;
        if (sgtl_set_volume(&sgtl5000_context, sgtl_module_dac, volume) != status_success) {
            USB_LOG_RAW("set volume Fail!\r\n");
        }
#endif
#endif
        s_set_speaker_volume_req = false;
    }

    if (s_speaker_rx_flag) {
        if (!speaker_out_buff_is_empty()) {
            if (s_speaker_dma_transfer_req) {
                s_speaker_dma_transfer_req = false;
                speaker_i2s_dma_start_transfer((uint32_t)&s_speaker_out_buffer[s_speaker_out_buffer_front][0],
                                               s_speaker_out_buffer_size[s_speaker_out_buffer_front]);

                s_speaker_out_buffer_front++;
                if (s_speaker_out_buffer_front >= AUDIO_BUFFER_COUNT) {
                    s_speaker_out_buffer_front = 0;
                }
            } else if (s_speaker_dma_transfer_done) {
                s_speaker_dma_transfer_done = false;
                speaker_i2s_dma_start_transfer((uint32_t)&s_speaker_out_buffer[s_speaker_out_buffer_front][0],
                                               s_speaker_out_buffer_size[s_speaker_out_buffer_front]);
                s_speaker_out_buffer_front++;
                if (s_speaker_out_buffer_front >= AUDIO_BUFFER_COUNT) {
                    s_speaker_out_buffer_front = 0;
                }
            } else {
                ;    /* Do Nothing */
            }
        }
    }
}

void usbd_audio_open(uint8_t intf)
{
    s_speaker_rx_flag = 1;
    s_speaker_out_buffer_front = 0;
    s_speaker_out_buffer_rear = 0;
    s_speaker_dma_transfer_req = true;
    /* setup first out ep read transfer */
    usbd_ep_start_read(AUDIO_OUT_EP, (uint8_t *)&s_speaker_out_buffer[s_speaker_out_buffer_rear][0], AUDIO_OUT_PACKET);
#if defined(USING_DAO) && USING_DAO
    dao_start(HPM_DAO);
#endif
    USB_LOG_RAW("OPEN SPEAKER\r\n");
}

void usbd_audio_close(uint8_t intf)
{
    s_speaker_rx_flag = 0;
#if defined(USING_DAO) && USING_DAO
    dao_stop(HPM_DAO);
#endif
    USB_LOG_RAW("CLOSE SPEAKER\r\n");
}

void usbd_configure_done_callback(void)
{
}

void usbd_audio_set_volume(uint8_t entity_id, uint8_t ch, float dB)
{
    if (entity_id == 0x03) {
        s_speaker_volume_percent = dB;
        s_set_speaker_volume_req = true;
    }
}

void usbd_audio_set_mute(uint8_t entity_id, uint8_t ch, uint8_t enable)
{
    if (entity_id == 0x03) {
        s_speaker_mute = enable;
        s_set_speaker_mute_req = true;
    }
}

void usbd_audio_set_sampling_freq(uint8_t entity_id, uint8_t ep_ch, uint32_t sampling_freq)
{
    if (entity_id == 0x01) {
        s_speaker_sampling_freq = sampling_freq;
        s_set_speaker_sampling_freq_req = true;
    }
}

void usbd_audio_get_sampling_freq_table(uint8_t entity_id, uint8_t **sampling_freq_table)
{
    if (entity_id == 0x01) {
        *sampling_freq_table = (uint8_t *)default_sampling_freq_table;
    }
}

/* Static Function Definition */
static void usbd_audio_iso_out_callback(uint8_t ep, uint32_t nbytes)
{
    if (s_speaker_rx_flag) {
        s_speaker_out_buffer_size[s_speaker_out_buffer_rear] = nbytes;
        s_speaker_out_buffer_rear++;
        if (s_speaker_out_buffer_rear >= AUDIO_BUFFER_COUNT) {
            s_speaker_out_buffer_rear = 0;
        }
        usbd_ep_start_read(ep, &s_speaker_out_buffer[s_speaker_out_buffer_rear][0], AUDIO_OUT_PACKET);
    }
}

static hpm_stat_t speaker_init_i2s_playback(uint32_t sample_rate, uint8_t audio_depth, uint8_t channel_num)
{
    i2s_config_t i2s_config;
    i2s_transfer_config_t transfer;

    if (channel_num > 2) {
        return status_invalid_argument; /* Currently not support TDM mode */
    }

    i2s_get_default_config(TARGET_I2S, &i2s_config);
#if defined(USING_CODEC) && USING_CODEC
    i2s_config.enable_mclk_out = true;
#endif
    i2s_init(TARGET_I2S, &i2s_config);

    i2s_get_default_transfer_config_for_dao(&transfer);
    transfer.data_line = TARGET_I2S_DATA_LINE;
    transfer.sample_rate = sample_rate;
    transfer.audio_depth = audio_depth;
    transfer.channel_num_per_frame = 2; /* non TDM mode, channel num fix to 2. */
    transfer.channel_slot_mask = 0x3;   /* data from hpm_wav_decode API is 2 channels */
#if defined(CONFIG_CODEC_WM8960) && CONFIG_CODEC_WM8960
    transfer.protocol = I2S_PROTOCOL_I2S_PHILIPS;
#endif

    clock_set_i2s_source(TARGET_I2S_CLK_NAME, clk_i2s_src_aud0);
    s_speaker_i2s_mclk_hz = clock_get_frequency(TARGET_I2S_CLK_NAME);

    if (status_success != i2s_config_tx(TARGET_I2S, s_speaker_i2s_mclk_hz, &transfer)) {
        return status_fail;
    }

    return status_success;
}

static void speaker_i2s_dma_start_transfer(uint32_t addr, uint32_t size)
{
    dma_channel_config_t ch_config = { 0 };

    dma_default_channel_config(BOARD_APP_HDMA, &ch_config);
    ch_config.src_addr = core_local_mem_to_sys_address(HPM_CORE0, addr);
    ch_config.dst_addr = (uint32_t)&TARGET_I2S->TXD[TARGET_I2S_DATA_LINE];
    ch_config.src_width = DMA_TRANSFER_WIDTH_WORD;
    ch_config.dst_width = DMA_TRANSFER_WIDTH_WORD;
    ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_INCREMENT;
    ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
    ch_config.size_in_byte = size;
    ch_config.dst_mode = DMA_HANDSHAKE_MODE_HANDSHAKE;
    ch_config.src_burst_size = 0;

    if (status_success != dma_setup_channel(BOARD_APP_HDMA, DMA_MUX_CHANNEL_SPEAKER, &ch_config, true)) {
        printf(" dma setup channel failed\n");
        return;
    }
}

static bool speaker_out_buff_is_empty(void)
{
    bool empty = false;

    if (s_speaker_out_buffer_front == s_speaker_out_buffer_rear) {
        empty = true;
    }

    return empty;
}