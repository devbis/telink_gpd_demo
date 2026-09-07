/*
 * The TLSR8258 application does not use the external flash security/OTP
 * registers. Some SDK flash-MID adapters still reference the generic OTP
 * entry points, while the SDK's 8258 implementation is disabled with #if 0.
 *
 * Keep the compatibility symbols in the application instead of modifying
 * the shared SDK. These functions must remain unused; they only satisfy the
 * linker for the optional adapters.
 */

#if defined(__GNUC__) || defined(__clang__)
#define APP_WEAK __attribute__((weak))
#else
#define APP_WEAK
#endif

APP_WEAK void flash_read_otp(unsigned long addr, unsigned long len, unsigned char *buf)
{
    (void)addr;
    (void)len;
    (void)buf;
}

APP_WEAK void flash_write_otp(unsigned long addr, unsigned long len, unsigned char *buf)
{
    (void)addr;
    (void)len;
    (void)buf;
}

APP_WEAK void flash_erase_otp(unsigned long addr)
{
    (void)addr;
}
