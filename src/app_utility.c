#include "tl_common.h"
#include "zcl_include.h"

#include "app_main.h"

//uint8_t mcuBootAddrGet(void);
u32 mcuBootAddrGet(void);

static u8 append_decimal_nibble(u8 *dst, u8 value) {
    value &= 0xf;

    if (value >= 10) {
        dst[0] = '1';
        dst[1] = '0' + value - 10;
        return 2;
    }

    dst[0] = '0' + value;
    return 1;
}

static u8 build_month(void) {
    const char *date = __DATE__;

    switch (date[0]) {
        case 'J':
            if (date[1] == 'a') {
                return 1;
            }
            return date[2] == 'n' ? 6 : 7;
        case 'F':
            return 2;
        case 'M':
            return date[2] == 'r' ? 3 : 5;
        case 'A':
            return date[1] == 'p' ? 4 : 8;
        case 'S':
            return 9;
        case 'O':
            return 10;
        case 'N':
            return 11;
        case 'D':
            return 12;
        default:
            return 0;
    }
}

void populate_sw_build(void) {
    u8 *version = g_zcl_basicAttrs.swBuildId + 1;
    u8 length = 0;

    /* APP_RELEASE is packed as major/minor; APP_BUILD low nibble is patch. */
    length += append_decimal_nibble(version + length, APP_RELEASE >> 4);
    version[length++] = '.';
    length += append_decimal_nibble(version + length, APP_RELEASE & 0xf);
    version[length++] = '.';
    length += append_decimal_nibble(version + length, APP_BUILD & 0xf);
    version[length] = 0;
    g_zcl_basicAttrs.swBuildId[0] = length;
}

void populate_date_code(void) {
    u8 month = build_month();

    if (month == 0) {
        return;
    }

    /* __DATE__ is "Mmm dd yyyy"; ZCL date code is "YYYYMMDD". */
    g_zcl_basicAttrs.dateCode[0] = 8;
    g_zcl_basicAttrs.dateCode[1] = __DATE__[7];
    g_zcl_basicAttrs.dateCode[2] = __DATE__[8];
    g_zcl_basicAttrs.dateCode[3] = __DATE__[9];
    g_zcl_basicAttrs.dateCode[4] = __DATE__[10];
    g_zcl_basicAttrs.dateCode[5] = '0' + month / 10;
    g_zcl_basicAttrs.dateCode[6] = '0' + month % 10;
    g_zcl_basicAttrs.dateCode[7] = __DATE__[4] == ' ' ? '0' : __DATE__[4];
    g_zcl_basicAttrs.dateCode[8] = __DATE__[5];
}

void print_boot_message() {

#ifdef ZCL_OTA
#if UART_PRINTF_MODE
        printf("OTA mode enabled. MCU boot from address: 0x%x\r\n", mcuBootAddrGet());
#endif /* UART_PRINTF_MODE */
#else
#if UART_PRINTF_MODE
    printf("OTA mode disabled. MCU boot from address: 0x%x\r\n", mcuBootAddrGet());
#endif /* UART_PRINTF_MODE */
#endif

#if UART_PRINTF_MODE
    printf("Firmware version: %s\r\n", g_zcl_basicAttrs.swBuildId + 1);
    printf("Build date: %s\r\n", g_zcl_basicAttrs.dateCode + 1);
#endif
}


s32 poll_rateAppCb(void *arg) {

    u32 poll_rate = zb_getPollRate();

    if (poll_rate == g_appCtx.long_poll) {
        zb_setPollRate(g_appCtx.short_poll);
        return TIMEOUT_30SEC;
    }

    zb_setPollRate(g_appCtx.long_poll);

    return g_appCtx.long_poll;
}

s32 delayedMcuResetCb(void *arg) {

    //printf("mcu reset\r\n");
    zb_resetDevice();
    return -1;
}

s32 delayedFactoryResetCb(void *arg) {

    //printf("factory reset\r\n");
    zb_resetDevice2FN();
//    sleep_ms(500);
//    rf_setTxPower(MY_RF_POWER_INDEX);
//    zb_factoryReset();
//    sleep_ms(100);

//    TL_ZB_TIMER_SCHEDULE(delayedMcuResetCb, NULL, TIMEOUT_3SEC);

//    zb_resetDevice();
    return -1;
}

s32 delayedFullResetCb(void *arg) {

    //printf("full reset\r\n");
    return -1;
}

s32 fakeTimer500msCb(void *arg) {

    return 0;
}

u32 reverse32(u32 in) {
    u32 out;
    uint8_t *source = (uint8_t*)&in;
    uint8_t *destination = (uint8_t*)&out;

    destination[3] = source[0];
    destination[2] = source[1];
    destination[1] = source[2];
    destination[0] = source[3];

    return out;
}

uint16_t reverse16(uint16_t in) {
    uint16_t out;
    uint8_t *source = (uint8_t*)&in;
    uint8_t *destination = (uint8_t*)&out;

    destination[1] = source[0];
    destination[0] = source[1];

    return out;
}

s32 int32_from_str(uint8_t *data) {

    s32 val = 0;

    val = data[0] << 24;
    val |= data[1] << 16;
    val |= data[2] << 8;
    val |= data[3];

    return val;
}

int16_t int16_from_str(uint8_t *data) {

    int16_t val = 0;

    val |= data[0] << 8;
    val |= data[1];

    return val;
}

uint8_t set_zcl_str(uint8_t *str_in, uint8_t *str_out, uint8_t len) {
    uint8_t *data = str_out;
    uint8_t *str_len = data;
    uint8_t *str_data = data+1;

    for (uint8_t i = 0; *(str_in+i) != 0 && i < (len-1); i++) {
        *(str_data+i) = *(str_in+i);
        (*str_len)++;
    }

    return *str_len;
}

//char * mystrstr(char * mainStr, char * subStr) {
//    char *s1, *s2;
//
//    // Iterate over the main string
//    while( *mainStr != NULL)
//    {
//        // search for the first character in substring
//        if(*mainStr == *subStr)
//        {
//            // The first character of substring is matched.
//            // Check if all characters of substring.
//            s1 = mainStr;
//            s2 = subStr;
//
//            while (*s1 && *s2 )
//            {
//                if(*s1 != *s2)
//                    break;      // not matched
//
//                s1++;
//                s2++;
//            }
//
//            // we reached end of subStr
//            if(*s2 == NULL)
//                return mainStr;
//        }
//
//        // go to next element
//        mainStr++;
//    }
//
//    return NULL;
//}
