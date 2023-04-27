/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
#include <sleep.h>
#include <stdio.h>

#include "Si5395-SURF-835-1-Registers.h"

// Use I2C #0
#define IIC_DEVICE_ID XPAR_XIICPS_1_DEVICE_ID
#define IIC_SCLK_RATE 100000

// I hate Xilinx's naming convention
XIicPs i2c_dev;
// Si5395 clock address
const uint8_t clk_addr = 0x69;

typedef struct surf_clk {
	XIicPs *i2c_dev;
	u8 last_page;
	u8 chip_addr;
} surf_clk_t;

surf_clk_t main_clk;

int clk_set_page(surf_clk_t *clk,
			     u8 page) {
	u8 write_buf[2];
	int Status;

	write_buf[0] = (u8) 0x1;
	write_buf[1] = (u8) page;
	Status = XIicPs_MasterSendPolled(clk->i2c_dev,
								     write_buf,
									 2,
									 clk->chip_addr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed writing I2C: %d\n\r", Status);
		return Status;
	}
	while (XIicPs_BusIsBusy(clk->i2c_dev));
	clk->last_page = page;
	return XST_SUCCESS;
}

int clk_write_register(surf_clk_t *clk,
					   u16 address,
					   u8 value) {
	u8 write_buf[2];
	int Status;

	u8 page = (address>>8) & 0xFF;
	u8 reg = (address & 0xFF);
	if (page != clk->last_page) {
		clk_set_page(clk, page);
	}
	write_buf[0] = reg;
	write_buf[1] = value;
	Status = XIicPs_MasterSendPolled(clk->i2c_dev,
								     write_buf,
									 2,
									 clk->chip_addr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed writing I2C: %d\n\r", Status);
		return Status;
	}
	while (XIicPs_BusIsBusy(clk->i2c_dev));
	return XST_SUCCESS;
}

int clk_read_register(surf_clk_t *clk,
					  u16 address,
					  u8 *value_p) {
	u8 write_buf;
	u8 page = (address>>8) & 0xFF;
	u8 reg = (address & 0xFF);
	int Status;

	if (page != clk->last_page) {
		clk_set_page(clk, page);
	}
	write_buf = reg;
	Status = XIicPs_MasterSendPolled(clk->i2c_dev, &write_buf, 1, clk->chip_addr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed sending reg addr: %d\n\r", Status);
		return Status;
	}
	while (XIicPs_BusIsBusy(clk->i2c_dev));
	Status = XIicPs_MasterRecvPolled(clk->i2c_dev, value_p, 1, clk->chip_addr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed reading I2C: %d\n\r", Status);
		return Status;
	}
	return XST_SUCCESS;
}

void clk_config(surf_clk_t *clk) {
	int index;
	for (index=0;index<SI5395_REVA_REG_CONFIG_NUM_REGS;index++) {
		if (!(index % 50)) {
			xil_printf("Programming index %d\n\r", index);
		}
		if (si5395_reva_registers[index].address == 0xFFFF) {
			xil_printf("Delaying 300 ms");
			usleep(0x50000U);
		} else {
			clk_write_register(clk,
				si5395_reva_registers[index].address,
				si5395_reva_registers[index].value);
		}
	}
}

int main()
{
    XIicPs_Config *Config;
    int Status;
    unsigned char ver, ver2;

    init_platform();

    xil_printf("Starting up (with I2C6)!\n\r");

    // OK! Horrible software time.
    Config = XIicPs_LookupConfig(IIC_DEVICE_ID);
    if (NULL == Config) {
    	xil_printf("Couldn't find IIC device ID.\n\r");
    	return 1;
    }
    Status = XIicPs_CfgInitialize(&i2c_dev, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
    	xil_printf("Couldn't configure IIC\n\r");
    	return 1;
    }
    Status = XIicPs_SelfTest(&i2c_dev);
    if (Status != XST_SUCCESS) {
    	xil_printf("Self test for IIC0 failed!\n\r");
    	return 1;
    }

    XIicPs_SetSClk(&i2c_dev, IIC_SCLK_RATE);

    main_clk.i2c_dev = &i2c_dev;
    main_clk.chip_addr = clk_addr;
    clk_set_page(&main_clk, 0);

    clk_read_register(&main_clk, 0x0002, &ver);
    clk_read_register(&main_clk, 0x0003, &ver2);
    if (ver != 0x95 || ver2 != 0x53) {
    	xil_printf("Couldn't detect Si5395!\n\r");
    	xil_printf("Ver: %x\n\r", ver);
    	xil_printf("Ver2: %x\n\r", ver2);
    	return 1;
    }
    xil_printf("Clock version: %x %x\n\r", ver, ver2);

    clk_config(&main_clk);




    xil_printf("Startup complete.\n\r");
    cleanup_platform();
    return 0;
}
