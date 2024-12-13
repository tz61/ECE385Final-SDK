#ifndef INTR_H
#define INTR_H
#include "xil_exception.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xscugic.h"
#include "xil_util.h"
extern volatile u32 InitNewFrameCond;
extern volatile u32 Render2DDoneCond;
#define XSCUGIC_SW_TIMEOUT_VAL 10000000U /* Wait for 10 sec */
void setup_irq();
#endif