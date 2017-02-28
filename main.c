/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */


/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

/* Board Header file */
#include "Board.h"

#include <string.h>
#include <stdbool.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* SimpleLink Wi-Fi Host Driver Header files */
#include <simplelink.h>


/* Local Platform Specific Header file */
#include "sockets.h"

/* Port number for listening for TCP packets */
#define TCPPORT         1000

#define TCPPACKETSIZE   256
#define TASKSTACKSIZE   1024

extern bool smartConfigFlag;
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];



void gpioButtonFxn(unsigned int index)
{
    /* Begin smart config process */
    smartConfigFlag = true;
}

void sendTCP(int port){
	int         bytesRcvd;
	int         bytesSent;
	int         status;
	int         clientfd;
	int         server;
	sockaddr_in localAddr;
	sockaddr_in clientAddr;
	socklen_t   addrlen = sizeof(clientAddr);
	char        buffer[TCPPACKETSIZE] = "TCP test send\n";

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    if (server == -1) {
	        System_printf("Error: socket not created.\n");
	        goto shutdown;
	    }

	    memset(&localAddr, 0, sizeof(localAddr));
	        localAddr.sin_family = AF_INET;
	        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	        localAddr.sin_port = htons(port);

	        status = bind(server, (const sockaddr *)&localAddr, sizeof(localAddr));
	        if (status == -1) {
	            System_printf("Error: bind failed.\n");
	            goto shutdown;
	        }

	        status = listen(server, 0);
	        if (status == -1){
	            System_printf("Error: listen failed.\n");
	            goto shutdown;
	        }
	        clientfd = accept(server, (sockaddr *)&clientAddr, &addrlen);
	        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
	                    if (bytesSent < 0 || bytesSent != bytesRcvd) {
	                        System_printf("Error: send failed.\n");
	                        break;
	                    }
	        /* addrlen is a value-result param, must reset for next accept call */
	                    addrlen = sizeof(clientAddr);
	                    close(clientfd);
shutdown:
   if (server >= 0) {
	   close(server);
   }

}


/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    // Board_initI2C();
    // Board_initSDSPI();
    // Board_initSPI();
    // Board_initUART();
    // Board_initWatchdog();
    // Board_initWiFi();

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
