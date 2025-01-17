#ifndef __OV5640_H__
#define __OV5640_H__

#include "xil_types.h"
#include "sleep.h"
#include "xiicps.h"

#define P1080 1 // 0 for 720p, 1 for 1080p

struct reginfo
{
    u16 reg;
    u8 val;
};

#define SEQUENCE_INIT        0x00
#define SEQUENCE_NORMAL      0x01

#define SEQUENCE_PROPERTY    0xFFFD
#define SEQUENCE_WAIT_MS     0xFFFE
#define SEQUENCE_END	     0xFFFF

#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID

int sensor_init(XIicPs *IicInstance);

#endif
