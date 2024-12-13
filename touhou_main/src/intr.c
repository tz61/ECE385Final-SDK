#include "intr.h"
// SGI Software Generated Interrupt
// PPI Private Peripheral Interrupt
// SPI Shared Peripheral Interrupt
// IRQ_F2P[2:0] is  [63:61]
#define RENDER2D_DONE_IRQ 61
#define INIT_NEW_FRAME_IRQ 62 // from fb_read module
XScuGic InterruptController;
static XScuGic_Config *GicConfig;
void Render2D_DoneHandler(void *CallbackRef);
void InitNewFrame_Handler(void *CallbackRef);
// condition
volatile u32 InitNewFrameCond = FALSE;
volatile u32 Render2DDoneCond = FALSE;
void setup_irq() {
    // on core 0
    GicConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);

    XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &InterruptController);
    Xil_ExceptionEnable();
    // actual register
    // IRQ_F2P[0] RENDER2D_DONE_IRQ
    XScuGic_Connect(&InterruptController, RENDER2D_DONE_IRQ, (Xil_ExceptionHandler)Render2D_DoneHandler,
                    (void *)&InterruptController);
    // set it to rising edge
    XScuGic_SetPriorityTriggerType(&InterruptController, RENDER2D_DONE_IRQ, 0xA0, 0x3);

    XScuGic_Enable(&InterruptController, RENDER2D_DONE_IRQ);
    // IRQ_F2P[1] INIT_NEW_FRAME_IRQ
    XScuGic_Connect(&InterruptController, INIT_NEW_FRAME_IRQ, (Xil_ExceptionHandler)InitNewFrame_Handler,
                    (void *)&InterruptController);
    // set it to rising edge
    XScuGic_SetPriorityTriggerType(&InterruptController, INIT_NEW_FRAME_IRQ, 0x08, 0x3);

    XScuGic_Enable(&InterruptController, INIT_NEW_FRAME_IRQ);
}
void Render2D_DoneHandler(void *CallbackRef) { Render2DDoneCond = TRUE; }
void InitNewFrame_Handler(void *CallbackRef) { InitNewFrameCond = TRUE; }