/*
 * proj_spi.c
 *
 *  Created on: 09.09.2016
 *      Author: mjakob
 */

#include "proj_spi_uart.h"
unsigned int ic;


/******************************Datentransfer SPI*******************************************/
unsigned char spi_transfer(unsigned char inb)
{
	UCB0TXBUF = inb;
	while (!(UCB0IFG & UCRXIFG));
	return UCB0RXBUF;
}



unsigned int spi_transfer16(unsigned int inw)
{
	uint16_t retw;
	uint8_t *retw8 = (uint8_t *)&retw, *inw8 = (uint8_t *)&inw;

	UCB0TXBUF = inw8[1];
	while (!(UCB0IFG & UCRXIFG));
	retw8[1] = UCB0RXBUF;
	UCB0TXBUF = inw8[0];
	while (!(UCB0IFG & UCRXIFG));
	retw8[0] = UCB0RXBUF;
	return retw;
}


void spi_transmit_sync (unsigned char *dataout, unsigned char len)
// Shift full array to target device without receiving any byte
{
	unsigned char i;
    for (i = 0; i < len; i++) {
    	//while ( !(UCA0IFG & UCRXIFG) );
    	__delay_cycles(50);

    	UCA0TXBUF = dataout[i];

    	//while(!(UCA0IFG&UCTXIFG));
    	//UCA0IFG &= ~(UCRXIFG);
    	while (!(UCA0IFG & UCRXIFG));

    	//UCA0IFG &= ~(UCRXIFG);
    	//while (!(UCA0IFG & UCRXIFG));
    //	UCA0IFG &= ~(UCTXIFG);
    //	for(ic=0; ic<50000; ic++);
    	__delay_cycles(50);
    }
}



/**********************Rerouting printf auf UART-Puffer***********************************/
#ifdef UART_PRINTF
int fputc(int ch, register FILE *pFile)
{
  while(!(UCA0IFG&UCTXIFG));				//warten bis der Puffer frei ist
  UCA0TXBUF = (unsigned char) ch;			//Char auf den UART-Puffer

  return((unsigned char)ch);				//return char
}

int fputs(const char *pStr, register FILE *pFile)
{
  unsigned int ind, lstr;					//deklaraktion String länge und Index

  lstr = strlen(pStr);						//Wertzuweisung der Stringlänge mittels Funktion strlen

  for(ind=0 ; ind<lstr ; ind++)				//Schleife um jedes Char-Zeichen eines Strings auf die digitale Schnittstelle zu legen
  {
    while(!(UCA0IFG&UCTXIFG));				//warten bis der Puffer frei ist
    UCA0TXBUF = (unsigned char) pStr[ind];	//Char auf den UART-Puffer
  }

  return lstr;								//return stringlänge
}
#endif
