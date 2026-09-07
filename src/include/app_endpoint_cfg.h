#ifndef SRC_INCLUDE_APP_ENDPOINT_CFG_H_
#define SRC_INCLUDE_APP_ENDPOINT_CFG_H_

#define APP_ENDPOINT1 0x01

/**
 *  @brief Defined for basic cluster attributes
 */
typedef struct{
    u8  zclVersion;
    u8  appVersion;
    u8  stackVersion;
    u8  hwVersion;
    u8  manuName[ZCL_BASIC_MAX_LENGTH];
    u8  modelId[ZCL_BASIC_MAX_LENGTH];
    u8  dateCode[ZCL_BASIC_MAX_LENGTH];
    u8  powerSource;
    u8  deviceEnable;
    u8  swBuildId[ZCL_BASIC_MAX_LENGTH];    //attr 4000
} zcl_basicAttr_t;

/**
 *  @brief Defined for identify cluster attributes
 */
typedef struct{
    u16 identifyTime;
}zcl_identifyAttr_t;

extern u8 APP_EP1_CB_CLUSTER_NUM;

/* Attributes */
extern zcl_basicAttr_t g_zcl_basicAttrs;
extern zcl_identifyAttr_t g_zcl_identifyAttrs;

#endif /* SRC_INCLUDE_APP_ENDPOINT_CFG_H_ */
