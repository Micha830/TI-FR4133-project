#include "msp430fr4133.h"
#include <stdio.h>
#include <string.h>
//#include "proj_spi_uart.h"

#include "stdint.h"



int tuart_putchar(int c);
int tuart_puts(const char *str);
uint16_t proc_socket();
uint16_t proc_light();


uint8_t msprf24_rx_pending();
uint8_t getdata, indgetch, ADC_Result;

char getstr[4];



/*
 * main.c
 */
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    /****************************CLOCK****************************/
    FRCTL0 = FRCTLPW | NWAITS_1;
    P4SEL0 |= BIT1 | BIT2;                       // set XT1 pin as second function
     do
     {
         CSCTL7 &= ~(XT1OFFG | DCOFFG);           // Clear XT1 and DCO fault flag
         SFRIFG1 &= ~OFIFG;
     } while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag

    __bis_SR_register(SCG0);                 // disable FLL
    CSCTL3 |= SELREF__XT1CLK;                // Set XT1 as FLL reference source
    CSCTL0 = 0;                              // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);                  // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_5;                     // Set DCO = 8MHz
    CSCTL2 = FLLD_0 + 487;                   // DCODIV = 8MHz


    PM5CTL0 &= ~LOCKLPM5;
    CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
    CSCTL5 |= DIVS_1; // SMCLK 8MHZ für UART

    __delay_cycles(500);
    __bic_SR_register(SCG0);                 // enable FLL

    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked

    /*********************IO-Port**************************/
    P1DIR |= (BIT0 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);       //BIT3 - 7 -> 4x LED Lampen innen & BIT1.3 1xLED Stubenseite r
    P2DIR |= BIT7;                                            // 1x LED stube mitte
    P4DIR |= BIT0;                                            //Test LED
    P5DIR |= (BIT0 | BIT1 | BIT2 | BIT3);                     // 3x LED Fensterseite & BIT5.1 1x lED Stube l

    P8DIR |= (BIT2 | BIT3);       // 2x Steckdosen außen


    P1OUT |= (BIT0 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P2OUT |= (BIT7);
    P5OUT |= (BIT0 | BIT1 | BIT2 | BIT3);
    P8OUT |= (BIT2 | BIT3);

    P5OUT &= ~(BIT0 | BIT3);
    P2OUT &= ~(BIT7);


    //Port 2.5 Interrupt
    P2DIR &= ~(BIT5);
    P2IFG = 0;                          // Clear all P1 interr
    P2IE |= BIT5;

    __delay_cycles(50000);


    /***********************UART* A0**************************/
    // Configure UART pins
    P1SEL0 |= BIT0 | BIT1;
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;
    UCA0BR0 = 4;
    UCA0BR1 = 0x00;
    UCA0MCTLW = 0x5500 | UCOS16 | UCBRF_5;

    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE |= UCRXIE;

    /*****************ADC*********************/
    // Configure ADC
    ADCCTL0 |= ADCSHT_2 | ADCON;                              // ADCON, S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;                                        // ADCCLK = MODOSC; sampling timer
    ADCCTL2 |= ADCRES;                                        // 10-bit conversion results
    ADCIE |= ADCIE0;                                          // Enable ADC conv complete interrupt
    ADCMCTL0 |= ADCINCH_9 | ADCSREF_0;                        // A1 ADC input select; Vref=1.5V

    TA0CCR0 = 1000; //3000 = 1,5min //Timer zum Abschalten des Lasers
    TA0CCTL0 = CCIE;
    TA0CTL =  TASSEL_1 + ID_3 + MC_0 + TACLR; //ACLK , Timer up/down, cleared  ID_3
  //  TA0CTL |= MC_2;     //Timer einschalten

    __delay_cycles(40000);                                      // Delay for reference settling
   __enable_interrupt(); 							// enable all interrupts


    /*******************Initial***********************/

   getstr[3] = '0';
   indgetch= 0;

    while(1)
    {

        if (autolaa > 3){

            TA0CTL &= ~MC_2;    //Timer abschalten
            TA0CTL |= TACLR;
            P4OUT &= ~BIT0;
            autolaa=0;
        }


        /***********Ansteuerung **************/
        else if (getstr[3] == '\n' & getstr[0] == 'L') proc_light();

        else if(AlarmId==1 & automode==1){
            AlarmId=0;
            tuart_puts ("Alarm\n");
        }

        else{
            __delay_cycles(500);
            //do nothing
        }
    }
}



#pragma vector=PORT2_VECTOR
__interrupt void Port_02_ISR(void){

    if(automode==1)ADCCTL0 |= ADCENC | ADCSC;                            // Sampling and conversion start

    AlarmId=1;

  //  indgetch++;
	P2IFG &= ~BIT5; //P1.3 IFG cleared
	P2IES ^= BIT5; 	//toggle the interrupt edge
}


#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void){
    ADC_Result = ADCMEM0;//funktioniert < 100 Aus -> >100 An
    if (ADC_Result < 100) P1OUT &= ~BIT0;                                   // Clear P1.0 LED off
    else P1OUT |= BIT0;                                    // Set P1.0 LED on

    TA0CTL |= MC_2;     //Timer einschalten
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void){

   // indgetch++;
    autolaa++;
    P4OUT |= BIT0;
 //   TA0CTL &= ~MC_2;    //Timer abschalten
 //   TA1CCR0 += 50000;                         // Add Offset to TA1CCR0
}




#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){

    getstr[indgetch] = UCA0RXBUF;
    indgetch++;

}


uint16_t proc_light(){


    if (getstr[1] == 'X'){

        if (getstr[2] == 'X'){

            P1OUT &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
            P2OUT &= ~(BIT7);
            P5OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
            tuart_puts ("OK\n");
        }
    }
    else if (getstr[1] == 'X'){
        if (getstr[2] == 'X'){

            P1OUT |= (BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
            P5OUT |= (BIT1);
            P2OUT |= (BIT7 );

            P5OUT &= ~(BIT0 | BIT2 | BIT3);
            tuart_puts ("OK\n");

        }
    }

    //Einzelanschaltung
   else if(getstr[1] == 'E'){
       if (getstr[2] == '0')P5OUT ^= BIT0;
       else if (getstr[2] == '1')P5OUT ^= BIT2;
   }

    indgetch=0;
    getstr[3] = '0';

    return 0;
}



/************************Transmit Data*****************************/
int tuart_putchar(int c)
{
    /* Wait for the transmit buffer to be ready */
    while(!(UCA0IFG&UCTXIFG));

    /* Transmit data */
    UCA0TXBUF = (char ) c;

    return 0;
}



int tuart_puts(const char *str)
{
    int status = -1;

    if (str != NULL) {
        status = 0;

        while (*str != '\0') {
            /* Wait for the transmit buffer to be ready */
            while(!(UCA0IFG&UCTXIFG));

            /* Transmit data */
            UCA0TXBUF = *str;

            /* If there is a line-feed, add a carriage return */
         //   if (*str == '\n') {
                /* Wait for the transmit buffer to be ready */
          //      while(!(UCA0IFG&UCTXIFG));
          //      UCA0TXBUF = '\r';
           // }

            str++;
        }
    }

    return status;
}
