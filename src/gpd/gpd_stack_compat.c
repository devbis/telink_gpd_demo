/*
 * gpd_stack_compat.c - compatibility hooks for the end-device library.
 *
 * The prebuilt end-device library keeps the BDB attribute initializer in its
 * common startup path. This standalone GPD intentionally does not include
 * BDB commissioning or its attribute database, so the hook is a no-op.
 */
#include "zb_common.h"
#include "bdb.h"

bdb_ctx_t g_bdbCtx = {0};

void tl_bdbAttrInit(void)
{
    /* BDB is not part of a transmit-only GPD. */
}

void bdb_retrieveTcLinkKeyDone(u8 status)
{
    (void)status;
}
