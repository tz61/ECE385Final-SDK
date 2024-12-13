#include "helper.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include <stdio.h>
extern uint16_t Bullet_sprite[128 * 256];
XScuTimer_Config *ConfigPtr;
XScuTimer Timer;
XScuTimer *TimerInstance = &Timer;
void setup_timer() {
    ConfigPtr = XScuTimer_LookupConfig(XPAR_XSCUTIMER_0_DEVICE_ID);
    XScuTimer_CfgInitialize(TimerInstance, ConfigPtr, ConfigPtr->BaseAddr);
    XScuTimer_LoadTimer(TimerInstance, 0xFFFFFFFF);
    XScuTimer_EnableAutoReload(TimerInstance);
    XScuTimer_Start(TimerInstance);
}
uint32_t get_time_tick() { return XScuTimer_GetCounterValue(TimerInstance); }
void copy_bullet_sprite_to_dest() {
    // 0x 0080 0000
    xil_printf("Copying bullet sprite to dest\r\n");
    volatile uint16_t *dest = (uint16_t *)BULLET_SPRITE_BASE;
    for (int i = 0; i < 128 * 256; i++) {
        *dest = Bullet_sprite[i];
        dest++;
    }
    Xil_DCacheFlushRange(BULLET_SPRITE_BASE, 128 * 256 * 2);
    xil_printf("Copy done\r\n");
    xil_printf("color at (8,8):%04x\r\n", Bullet_sprite[256 * 8 + 8]);
    xil_printf("color at (8,8) in mem:%04x\r\n", *((volatile uint16_t *)(BULLET_SPRITE_BASE + 2 * (256 * 8 + 8))));
}
void setup_AUDIO(uint32_t is_BGM, uint32_t audio_type) {
    GPIO2_OUT |= (((is_BGM << 4) + audio_type) << 4); // [8:4], bit [8] BGM =1
    GPIO2_OUT |= 0x8;                                 // toggle bit 3
    GPIO2_OUT &= ~0x8;
    GPIO2_OUT &= ~(0x1F << 4); // clear BGM bit and type bit for next set
}
void set_audio_volume(uint8_t volume_shift) {
    GPIO2_OUT &= ~(0x7); // [2:0]
    GPIO2_OUT |= (volume_shift & 0x7);
}
void toggle_fb0_alt() {
    // default 0(not alt)
    static uint8_t fb0_alt = 1;
    // bit 9
    if (fb0_alt) {
        GPIO2_OUT |= 0x1 << 9;
        fb0_alt = 0;
    } else {
        GPIO2_OUT &= ~(0x1 << 9);
        fb0_alt = 1;
    }
}
void toggle_fb1_alt() {
    // default 0(not alt)
    static uint8_t fb1_alt = 1;
    // bit 10
    if (fb1_alt) {
        GPIO2_OUT |= 0x1 << 10;
        fb1_alt = 0;
    } else {
        GPIO2_OUT &= ~(0x1 << 10);
        fb1_alt = 1;
    }
}

void toggle_render() {
    // bit 11
    GPIO2_OUT |= 0x1 << 11;
    GPIO2_OUT &= ~(0x1 << 11);
}
void clear_fb(void *fb_ptr) {
    volatile uint32_t *ptr;
    ptr = fb_ptr;
    for (int i = 0; i < 480; i++) {
        for (int j = 0; j < 640; j++) {
            *ptr = 0x00000000;
            //  xil_printf("%x",board[i*640+j]);
            ptr++;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}
void clear_board(void *fb_ptr) {
    volatile uint32_t *ptr;
    ptr = fb_ptr;
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
        ptr = i * LINE_STRIDE_BYTE + fb_ptr + FB1_START_X * 4;
        for (int j = FB1_START_X; j < FB1_END_X; j++) {
            *ptr = 0x00FF0000;
            ptr++;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}
void set_die_buzzer() {
    // bit 12
    GPIO2_OUT |= 0x1 << 12;
}
void clear_die_buzzer() {
    // bit 12
    GPIO2_OUT &= ~(0x1 << 12);
}
int SDIO_Read(uint32_t dest_mem_addr, uint32_t start_sector, uint32_t sector_count) {
    static XSdPs SdInstance;
    XSdPs_Config *SdConfig;
    uint32_t sector_read = 0;
    int Status;
    /*
     * Initialize the host controller
     */
#ifndef SDT
    SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
#else
    SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_BASEADDR);
#endif
    if (NULL == SdConfig) {
        return XST_FAILURE;
    }

    Status = XSdPs_CfgInitialize(&SdInstance, SdConfig, SdConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = XSdPs_CardInitialize(&SdInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    if (!(SdInstance.HCS)) {
        start_sector *= XSDPS_BLK_SIZE_512_MASK;
        sector_read *= XSDPS_BLK_SIZE_512_MASK;
    }

    /*
     * Read data from SD/eMMC.
     */
#define MAX_SINGLE_READ_SECTOR_COUNT (2 * 1024 * 1024 / 512)
    xil_printf("\r\n SDIO Started reading!\r\n");
    while (sector_read < sector_count) {
        xil_printf("\rReading %d/%d", sector_read, sector_count);
        Status = XSdPs_ReadPolled(&SdInstance, start_sector + sector_read, MAX_SINGLE_READ_SECTOR_COUNT,
                                  dest_mem_addr + sector_read * 512);
        if (Status == XST_SUCCESS) {
            // Max 2MiB in a single poll, 2*1024*1024/512=4096 sectors max
            // only 4096 sector. within a poll
            sector_read += MAX_SINGLE_READ_SECTOR_COUNT;
        }
    }
    xil_printf("\r\n SDIO Read done!\r\n");

    return XST_SUCCESS;
}
