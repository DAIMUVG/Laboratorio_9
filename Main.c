//Archivo:	Main_lab_9.s
//dispositivo:	PIC16F887
//Autor:		Dylan Ixcayau
//Compilador:	XC8, MPLABX V5.45

//Programa:	PWM
//Hardware:	2 potenciometros y 2 servomotores

//Creado:	27 abril, 2021
//Ultima modificacion:  2 de mayo, 2021
// ----------------- Laboratorio No.9 ----------------------------------------

// CONFIG1
//-----------------------------------------------------------------------------------------------------------------------------------
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = ON      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)
//-----------------------------------------------------------------------------------------------------------------------------------
// CONFIG2
//-----------------------------------------------------------------------------------------------------------------------------------
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#include <xc.h>
#define _XTAL_FREQ 4000000     // Frecuencia del osilador
//__________
// PROTOTIPOS DE FUNCIONES
void Setup(void);       // Llamamos las configuraciones de los pines 

//----------------------Interrupciones--------------------------------------
void __interrupt() isr(void){
    if(PIR1bits.ADIF){           //Verificamos si la bandera del ADC es 1 
       
        if(ADCON0bits.CHS == 0){        //si el canal es 0
            CCPR1L   = (ADRESH>>1)+124 ;         //le damos vuelta a Adresh y le sumamos 124ton
            CCP1CONbits.DC1B1 = (ADRESL>>6);
            CCP1CONbits.DC1B0 = (ADRESL>>7);
          }
        else{       //Verificamos si el pin esta precionado 
            CCPR2L   = (ADRESH>>1)+124 ;         //Aumentamos el Puerto C cuando se preciona el boton
            CCP2CONbits.DC2B1 = (ADRESL>>6);
            CCP2CONbits.DC2B0 = (ADRESL>>7);
        }
        PIR1bits.ADIF = 0;          //limpiamos la bandera del ADC
      }
}
//---------------------funcion principal----------------------------------------
void main(void){
    Setup();        //llamamos a la funcion de configuracion
     while(1){      //ciclo infinito
       if(ADCON0bits.GO == 0){  //Si GO es 0
            if (ADCON0bits.CHS == 0){   //Si el canal es 0
             ADCON0bits.CHS = 1;        //Se pasa al canal 1
            }
            else{                   //Si no es 0
             ADCON0bits.CHS = 0;    //se pasa a 0
            }
          __delay_us(50);           //delay de 50us
          ADCON0bits.GO = 1;        //Dejamos GO como cero
       }
    }
}

//__________
//---------------- Configuraciones Generales ----------------------------------

void Setup(void){
//-------  Asigancion de Pines Didigitales--------------------------------------
    ANSEL  = 0b00000011;    // Configuracion a pines Digitale 
    ANSELH = 0;              //dejamos ans0 y ans1 como entrada analogica

    TRISC = 0x00;           //Dejamos los los demas puertos Como salidas digitales
    TRISA = 0x03;           //Dejamos A0 y A1 como entradas

    PORTC = 0x00;           //limpiamos los puertos
    PORTA = 0x00;
//---------------------- Configuracion de reloj -------------------------
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.SCS   = 1;       //reloj interno a 8MHz
 //---------------------- Configuracion del ADC ------- -------------------------
    ADCON1bits.ADFM  = 0;    //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;    //Voltage de referencia VDD
    ADCON1bits.VCFG1 = 0;   //Voltage de referencia VSS
    
    ADCON0bits.ADCS  = 0b10;   //Configuracion del reloj del modulo 
    ADCON0bits.CHS   = 0;   ///Empezamos con el canal 0
    __delay_us(100);
    ADCON0bits.ADON  = 1;   //ADC encendido
    __delay_us(50);
//---------------------- Configuracion del PWM ---------------------------------
    TRISCbits.TRISC2 = 1;    // RC2/CCP1 encendido
    TRISCbits.TRISC1 = 1;    // RC1/CCP2 encendido
    
    
    PR2 = 255;               // Configurando el periodo
    CCP1CONbits.P1M = 0;     // Configurar el modo PWM
    
    CCP1CONbits.CCP1M = 0b1100; //zConfiguramos CCP1 y CCP2 para ambos servos
    CCP2CONbits.CCP2M = 0b1100;
    
    CCPR1L = 0x00;
    CCPR2L = 0x00;
    
    CCP1CONbits.DC1B = 0;
    CCP2CONbits.DC2B0 = 0;
    CCP2CONbits.DC2B1 = 0;
    
    
    PIR1bits.TMR2IF  =  0;
    T2CONbits.T2CKPS =  0b11;
    T2CONbits.TMR2ON =  1;
    
    while(PIR1bits.TMR2IF  ==  0);  //Ciclo que se repite constantemente
    PIR1bits.TMR2IF  =  0;
    
    TRISCbits.TRISC2 = 0; //limpiamos las salidas a los servos
    TRISCbits.TRISC1 = 0; 
//--------------------------Configuraciones de TMR0 ----------------------------
    OPTION_REGbits.T0CS = 0;    //Ciclo de reloj interno
    OPTION_REGbits.PSA  = 0;    //Prescaler para el modulo de timer 0
    OPTION_REGbits.PS2  = 1;
    OPTION_REGbits.PS1  = 1;
    OPTION_REGbits.PS0  = 1;    //Prescaler a 256
//---------------------- Configuracion del las interrupciones -----------------------
    INTCONbits.GIE  = 1;    //Habilitamos las interrupciones globales
    INTCONbits.PEIE = 1;    //Dejamos la  bandera levantada
    PIE1bits.ADIE   = 1;    //Encendemos la bandera de interrupcion del ADC
    PIR1bits.ADIF   = 0;    //Bandera de interrupcion del ADC
}
