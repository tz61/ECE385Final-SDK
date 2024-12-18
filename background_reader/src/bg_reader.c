#include "platform.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xscugic.h"
#include "xscutimer.h"
#include "xsdps.h"
#include <stdio.h>
// learned that from https://www.youtube.com/watch?v=scdaa3KaJmo
#define INFORM_READER 0x38000000
#define INFORM_STAGE2 0x38000004
#define INFORM_RESTART 0x38000008
#define FB0_BASE 0x01000000
#define FB0_ALT_BASE 0x01258000
#define GPIO2_OUT_BASE 0xE000A048 // lower 32bits in EMIO
#define GPIO2_OUT *((volatile uint32_t *)GPIO2_OUT_BASE)
uint8_t getMemFlag(volatile uint32_t *memAddr) {
    Xil_DCacheInvalidateRange(memAddr, 1);
    return *memAddr;
}
void setMemFlag(volatile uint32_t *memAddr, uint8_t val) {
    Xil_DCacheInvalidateRange(memAddr, 1);
    *memAddr = val;
}
void ReadAnimation() {
    static XSdPs SdInstance;
    XSdPs_Config *SdConfig;
    static uint32_t sector_read = 0;
    int Status;
    static int is_init = 0;
    static uint8_t fb0_alt = 1;
    int is_stage2 = 0;
    if (!is_init) {
        SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
        XSdPs_CfgInitialize(&SdInstance, SdConfig, SdConfig->BaseAddress);
        XSdPs_CardInitialize(&SdInstance);
        is_init = 1;
    }
    uint32_t time_tick_last, time_tick;
    if (sector_read < 10863 * 2400) { // 10863 frames
                                      // bit 9
        if (fb0_alt) {
            Status = XSdPs_ReadPolled(&SdInstance, 1015808 + sector_read, 2400, (u8 *)FB0_ALT_BASE);
            GPIO2_OUT |= 0x1 << 9;
            fb0_alt = 0;
        } else {
            Status = XSdPs_ReadPolled(&SdInstance, 1015808 + sector_read, 2400, (u8 *)FB0_BASE);
            GPIO2_OUT &= ~(0x1 << 9);
            fb0_alt = 1;
        }
        if (Status == XST_SUCCESS) {
            sector_read += 4800; // skip 1 frame
        }
        if (!is_stage2) {
            if (sector_read > 8394926) {
                is_stage2 = 1;
                setMemFlag(INFORM_STAGE2, 1);
            }
        }
        if(getMemFlag(INFORM_RESTART)){
            sector_read = 0;
            setMemFlag(INFORM_RESTART, 0);
        }
    } else {
        // reset(loop)
        sector_read = 0;
        is_stage2 = 0;
    }
}
int main() {
    init_platform();
    Xil_SetTlbAttributes(0xFFFF0000, 0x14de2);
    xil_printf("\r\n3DBG reader board, Version:" __DATE__ " " __TIME__ "\r\n");
    setMemFlag(INFORM_READER, 0);
    while (1) {
        if (getMemFlag(INFORM_READER)) {
            break;
        }
        usleep(50000); // sleep 50ms
    }
    while (1) {
        ReadAnimation();
    }
    cleanup_platform();
    return 0;
}
