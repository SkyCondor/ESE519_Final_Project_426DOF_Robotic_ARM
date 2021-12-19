#include <avr/io.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/signal.h>
#include "uart.h"
#include <math.h>

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)
/************************************************************************************
                             VARIABLE DECLARATIONS
*************************************************************************************/
volatile unsigned char Timer_Count;
unsigned char Servo_Angle_x    = 120;
unsigned char Servo_Angle_y    = 120;
unsigned char Servo_Angle_z    = 120;
unsigned char Servo_Angle_f    = 30;
unsigned char Servo_Value_x   = 0;
unsigned char Servo_Value_y   = 0;
unsigned char Servo_Value_z   = 0;
unsigned char Servo_Value_f   = 0;
unsigned char k              = 0;
int adc_value = 0;
int ReadADC_x = 0;
int ReadADC_y = 0;
int ReadADC_z = 0;
int ReadADC_f = 0;
int x_flag = 0;
int y_flag = 0;
int z_flag = 0;
int f_flag = 0;
char String[25];
unsigned char  Convert_Angle(unsigned char  k);

int  main()
{

    UART_init(BAUD_PRESCALER);
    cli();

    //set up ADC
    PRR &=~(1<<PRADC);

    ADMUX |= (1<<REFS0);
    ADMUX &= ~(1 << REFS1);

    ADCSRA |= (1<<ADPS0);
    ADCSRA |= (1<<ADPS1);
    ADCSRA |= (1<<ADPS2);

    DIDR1 |= (1<<ADC1D);

    ADCSRA |= (1<<ADEN);
    ADCSRA |= (1<<ADIE);

    DDRB |= (1<<DDB0);  //output                               /* PC6 pin set as output for servo     */
    DDRB |= (1<<DDB1);  //output
    DDRB |= (1<<DDB2);  //output
    DDRB |= (1<<DDB3);  //output

    TCCR0B |= (1<<CS01); //timer0 setup Prescale of 010                             /* Prescalar is 8                      */
    TCNT0 = 0XEB;                               /* Timer count for 20micro second      */
    TIMSK0 |= (1 << TOIE0);//Set the ISR OVF vect                          /* Timer0 interrupt enable             */
    sei();
    while(1)
    {
        ADCSRA |= (1<<ADSC);

            if(ReadADC_x > 700){
                Servo_Angle_x -= 1;
                Servo_Value_x = Convert_Angle(Servo_Angle_x);
                _delay_ms(10);
            }else if(ReadADC_x < 300){
                Servo_Angle_x += 1;
                Servo_Value_x = Convert_Angle(Servo_Angle_x);
                _delay_ms(10);
            }


            if(ReadADC_y > 700){
                Servo_Angle_y -= 1;
                Servo_Value_y = Convert_Angle(Servo_Angle_y);
                _delay_ms(10);
            }else if(ReadADC_y < 300){
                Servo_Angle_y += 1;
                Servo_Value_y = Convert_Angle(Servo_Angle_y);
                _delay_ms(10);
            }

            if(ReadADC_z > 700){
                Servo_Angle_z -= 3;
                Servo_Value_z = Convert_Angle(Servo_Angle_z);
                _delay_ms(10);
            }else if(ReadADC_z < 300){
                Servo_Angle_z += 3;
                Servo_Value_z = Convert_Angle(Servo_Angle_z);
                _delay_ms(10);
            }

            if(ReadADC_f > 700){
                Servo_Angle_f -= 3;
                Servo_Value_f = Convert_Angle(Servo_Angle_f);
                _delay_ms(10);
            }else if(ReadADC_f < 300){
                Servo_Angle_f += 3;
                Servo_Value_f = Convert_Angle(Servo_Angle_f);
                _delay_ms(10);
            }



    }
}

/************************************************************************************
                           INTERRUPT SERVICE ROUTINE
*************************************************************************************/
ISR(TIMER0_OVF_vect)
{
    Timer_Count++;                              /* Variable for count overflow         */

    if( Timer_Count < 125 )                     /* Count for 180 degree i.e. 2.5ms     */
        TCNT0=0XEB;                                /* Load timer value for 20us pulse     */

    if( Timer_Count == Servo_Value_x && x_flag)
        PORTB &= ~(1 << PINB0);                        /* Clearing servo pin                  */
    if( Timer_Count == Servo_Value_y && y_flag)
        PORTB &= ~(1 << PINB1);
    if( Timer_Count == Servo_Value_z && z_flag)
        PORTB &= ~(1 << PINB2);
    if( Timer_Count == Servo_Value_f && f_flag)
        PORTB &= ~(1 << PINB3);

    if( Timer_Count >= 125 )                    /* Angle greater than 180              */
    {
        TCNT0=0X97;                                /* 3.5 ms  delay                       */
        PORTB |= (1 << PINB0);                                /* Set pin connected to servo          */
        PORTB |= (1 << PINB1);
        PORTB |= (1 << PINB2);
        PORTB |= (1 << PINB3);
    }
    if( Timer_Count == 130 )                    /* 17.5ms(5*3.5) ie:2.5+17.5=20mS      */
    {
        PORTB |= (1 << PINB0);                              /* Set pin connected to servo          */
        PORTB |= (1 << PINB1);
        PORTB |= (1 << PINB2);
        PORTB |= (1 << PINB3);
        TCCR0B |= (1<<CS01); //timer0 setup Prescale of 010
        TCNT0 = 0XEB;                              /* Load 20us pulse                     */
        Timer_Count = 0;                           /* Clear timer count                   */
    }
}

ISR(ADC_vect)
{
    adc_value = ADCL + (ADCH<<8);
    if (ADMUX==64)
    {
        ReadADC_x = adc_value;
        x_flag = 1;
        y_flag = 0;
        z_flag = 0;
        f_flag = 0;
        //switch to channel 1 ADMUX==65
        ADMUX |= (1 << MUX0);
        ADMUX &= ~(1 << MUX1);
        ADMUX &= ~(1 << MUX2);
        ADMUX &= ~(1 << MUX3);

    }
    else if(ADMUX==65)
    {
        ReadADC_y = adc_value;
        y_flag = 1;
        x_flag = 0;
        z_flag = 0;
        f_flag = 0;
        ADMUX &= ~(1 << MUX0);
        ADMUX |=  (1 << MUX1);
        ADMUX &= ~(1 << MUX2);
        ADMUX &= ~(1 << MUX3);
    }
    else if(ADMUX==66)
    {
        ReadADC_z = adc_value;
        y_flag = 0;
        x_flag = 0;
        z_flag = 1;
        f_flag = 0;
        ADMUX |= (1 << MUX0);
        ADMUX |= (1 << MUX1);
        ADMUX &= ~(1 << MUX2);
        ADMUX &= ~(1 << MUX3);
    }
    else if(ADMUX==67)
    {
        ReadADC_f = adc_value;
        y_flag = 0;
        x_flag = 0;
        z_flag = 0;
        f_flag = 1;
        ADMUX &= ~(1 << MUX0);
        ADMUX &= ~(1 << MUX1);
        ADMUX &= ~(1 << MUX2);
        ADMUX &= ~(1 << MUX3);
    }

    _delay_ms(10);
}
unsigned char  Convert_Angle(unsigned char  k)
{
    unsigned char timer_value;
    int temp;
    temp = k*5;
    timer_value = temp/9;                       /* Timer value=(100/180)+25i.e(5/9)+25 */
    timer_value = timer_value+25;
    _delay_ms(3);
    return timer_value;                         /* Return timer value                  */
}
