/*
 * justfloat.h
 *
 * VOFA+ FireWater telemetry helper.
 * Frame format: little-endian float values followed by 00 00 80 7F.
 */

#ifndef CODE_APP_JUSTFLOAT_H_
#define CODE_APP_JUSTFLOAT_H_

#include "zf_common_headfile.h"

#define JUSTFLOAT_CHANNEL_DEBUG_UART   (0U)
#define JUSTFLOAT_CHANNEL_WIRELESS     (1U)
#define JUSTFLOAT_CHANNEL_WIFI_SPI     (2U)

// Default to the debug UART so VOFA+ can read from the same serial port as printf.
#define JUSTFLOAT_CHANNEL              (JUSTFLOAT_CHANNEL_DEBUG_UART)

// Used only when JUSTFLOAT_CHANNEL is JUSTFLOAT_CHANNEL_WIFI_SPI.
#define JUSTFLOAT_WIFI_SSID            "exin"
#define JUSTFLOAT_WIFI_PASSWORD        "88888888"
#define JUSTFLOAT_UDP_TARGET_IP        "192.168.137.1"
#define JUSTFLOAT_UDP_TARGET_PORT      "1347"
#define JUSTFLOAT_UDP_LOCAL_PORT       "6666"

static inline void Float_to_Byte(float value, unsigned char byte[4])
{
    union
    {
        float fdata;
        unsigned long ldata;
    } converter;

    converter.fdata = value;
    byte[0] = (unsigned char)(converter.ldata);
    byte[1] = (unsigned char)(converter.ldata >> 8);
    byte[2] = (unsigned char)(converter.ldata >> 16);
    byte[3] = (unsigned char)(converter.ldata >> 24);
}

static inline void justfloat_send_raw(const uint8 *buffer, uint32 length)
{
#if JUSTFLOAT_CHANNEL == JUSTFLOAT_CHANNEL_WIFI_SPI
    wifi_spi_send_buffer(buffer, length);
#elif JUSTFLOAT_CHANNEL == JUSTFLOAT_CHANNEL_WIRELESS
    wireless_uart_send_buffer(buffer, length);
#else
    uart_write_buffer(DEBUG_UART_INDEX, buffer, length);
#endif
}

static inline uint8 justfloat_init(void)
{
#if JUSTFLOAT_CHANNEL == JUSTFLOAT_CHANNEL_WIFI_SPI
    if (wifi_spi_init(JUSTFLOAT_WIFI_SSID, JUSTFLOAT_WIFI_PASSWORD) != 0)
    {
        return 1U;
    }

    if (wifi_spi_socket_connect("UDP",
                                JUSTFLOAT_UDP_TARGET_IP,
                                JUSTFLOAT_UDP_TARGET_PORT,
                                JUSTFLOAT_UDP_LOCAL_PORT) != 0)
    {
        return 2U;
    }
#elif JUSTFLOAT_CHANNEL == JUSTFLOAT_CHANNEL_WIRELESS
    if (wireless_uart_init() != 0)
    {
        return 1U;
    }
#endif
    return 0U;
}

static inline void justfloat_send_values(const float *values, uint8 count)
{
    uint8 i;
    uint8 byte[4] = {0U};
    const uint8 tail[4] = {0x00U, 0x00U, 0x80U, 0x7FU};

    for (i = 0U; i < count; i++)
    {
        Float_to_Byte(values[i], byte);
        justfloat_send_raw(byte, 4U);
    }

    justfloat_send_raw(tail, 4U);

#if JUSTFLOAT_CHANNEL == JUSTFLOAT_CHANNEL_WIFI_SPI
    wifi_spi_udp_send_now();
#endif
}

static inline void JustFloat_Test_eight(float a,
                                        float b,
                                        float c,
                                        float d,
                                        float e,
                                        float f,
                                        float g,
                                        float h)
{
    const float values[8] = {a, b, c, d, e, f, g, h};
    justfloat_send_values(values, 8U);
}

#endif /* CODE_APP_JUSTFLOAT_H_ */
