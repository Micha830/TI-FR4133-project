/*
 * proj_spi.h
 *
 *  Created on: 09.09.2016
 *      Author: mjakob
 */

//#include <msp430f5529.h>
//#include <stdio.h>
//#include <string.h>
#include "main.h"
//#include "NRF24L01.h"

#ifndef PROJ_SPI_UART_H_
#define PROJ_SPI_UART_H_



/*********Funktionsprototypen, um den printf-Ausgaben auf den UART zu schieben********/
#define UART_PRINTF      		//Definiere Symbol UART_PRINTF

#ifdef UART_PRINTF				//überprüfung, ob das Symbol definiert wurde
int fputc(int ch, register FILE *pFile);
int fputs(const char *pStr, register FILE *pFile);
#endif					//schließt ifdef ab


unsigned char spi_transfer(unsigned char);  		// SPI xfer 1 byte
unsigned int spi_transfer16(unsigned int); 	// SPI xfer 2 bytes


void spi_transmit_sync (unsigned char *, unsigned char);


#endif /* PROJ_SPI_UART_H_ */
