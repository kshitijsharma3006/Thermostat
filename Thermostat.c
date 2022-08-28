#define F_CPU 1000000UL 
#include "defines.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd.h"
#include "hd44780.h"
#include <stdbool.h>
// prototypes of the functions used
void furnace();
void input();
void print();
void mode();
FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE); // global variable for the LCD stream
float set_temp=25.0; //temperature set by the user
float T1;            //global variable representing the internal temperature
float T2;            //global variable representing the external temperature
float ext_sensor(void);
float int_sensor(void);
int fur_check;
int mode_check;
int main(void) 
{
    DDRB |= (1 << DDB0); //OUTPUT(LED)
    DDRC &= ~(1 << DDC3); //INPUT(SWITCH 1)
    DDRC &= ~(1 << DDC4); //INPUT(SWITCH 2)
    DDRC &= ~(1 << DDC5); //INPUT(SWITCH 3)
    PORTC |= (1<<PORTC3);
    PORTC |= (1<<PORTC4);
    PORTC |= (1<<PORTC5);
    
    ADCSRA |= (1<< ADPS1) | (1<< ADPS0);
    ADCSRA |= (1 << ADEN);
    DIDR0 |= (1<<ADC0D);
    lcd_init();
    while (1) 
    {
        T1 = int_sensor();  //calls function for the interior sensor calculations and puts the value returned into T1
        T2 = ext_sensor();  //calls function for the interior sensor calculations and puts the value returned into T2
        furnace();          //calls function furnace
        print();            //calls print function
        input();            // calls input function
        
    }
}
float ext_sensor()    //gives the external temperature 
{

    float vref = 5.0;      //reference voltage used to calculate the external temperature
    float Temp_ext;          
    ADMUX=0;
    ADMUX |= (1 << REFS0) ;    //setting REFSO to 1
    ADMUX &= ~(1 << REFS1);    //setting REFS1 and MUX3 to 0
    ADMUX &= ~(1<<MUX3);
    ADCSRA |= (1 << ADSC);      //setting ADSC to 1
    while (!(ADCSRA &(1 << ADIF)))  // this while loop 
    {
    }
    ADCSRA |= (1<<ADIF);         //setting ADRF to one 
    Temp_ext = (ADC * vref * 100) / 1024;  //calculation for the external temperature 
    return (Temp_ext);   //returning external temperature to main
}

float int_sensor() 
{
    int slope=1;                      //the calculated slope using (y2-y1)/(x2-x1)
    float intercept = 327.9;          //calculated intercept from y=mx+c
    float vref1 = 1.1;                //reference voltage for the internal voltage calculations 
    float Temp_int=0;                 

    ADMUX = 0;
    ADMUX |= (1 << REFS1);
    ADMUX |= (1 << REFS0);
    ADMUX |= (1 << MUX3);
    ADCSRA |= (1 << ADSC);

    while (!(ADCSRA &(1 << ADIF)))
    {
    }
    ADCSRA |= (1<<ADIF);
    
    Temp_int = (slope*ADC)-intercept;     //this is basically the calibrated value of the internal temperature 
                                          //this is done by using y=mx+c where m is the slope, x is the ADC value,
                                          //c is the intercept and y is the internal temperature value
    return (Temp_int);                    //returns internal temperature to main
}
void furnace()                                //function responsible for furncce functioning
{
    //I have used a deadband of 2 (1 on either side)
    if ((mode_check%2==0)&&(set_temp>(T2+1)))   //if mode check is even we are in exterior mode
    {
        fur_check=1;                       
        PORTB |= (1 << PORTB0);         //lights LED
    }
    if ((mode_check%2!=0)&&(set_temp>(T1+1)))     //if mode check is even we are in interior mode
    {
        fur_check=1;
        PORTB |= (1 << PORTB0);
    }
    if ( (mode_check%2==0) && (set_temp<(T2-1)))
    {
        fur_check=0;
        PORTB &= ~(1 << PORTB0);                  //turns LED off
    }
    if ((mode_check%2!=0) && (set_temp<(T1-1)))
    {
        fur_check=0;
        PORTB &= ~(1 << PORTB0);
    }
}


void input()
{
 if(!(PINC & (1 << PINC3)))            //if switch 1 is pressed
 {
     while ((!(PINC & (1 << PINC3))));   //waits for the user's input
     if (set_temp < 35)                     //doesn't allow the user to set temperature above 35
      {
        set_temp = set_temp + 1;                   //increments set temperature by one
     }
     
 }
 if(!(PINC & (1 << PINC4)))
 {
     while ((!(PINC & (1 << PINC4))));
     if (set_temp > 10){
     set_temp = set_temp - 1;
     }
 }
 if(!(PINC & (1 << PINC5)))
    {
        while ((!(PINC & (1 << PINC5))));
        mode_check++;
        //even = external
        //odd = internal
        
    }
}
void print()                      //used to print stuff to the LED
{   if (mode_check%2==0)
    {
    fprintf(&lcd_str, "%3.1f\xDF", T1);
    fprintf(&lcd_str, "C");
    fprintf(&lcd_str, "-->%3.1f\xDF", T2);
    fprintf(&lcd_str, "C\x1b\xC0");
    fprintf(&lcd_str, "   %3.1f\xDF", set_temp);
    
    }
   else if (mode_check%2 != 0)
   {
    fprintf(&lcd_str, "%3.1f\xDF", T1);
    fprintf(&lcd_str, "C");
    fprintf(&lcd_str, "<--%3.1f\xDF", T2);
    fprintf(&lcd_str, "C\x1b\xC0");
    fprintf(&lcd_str, "   %3.1f\xDF", set_temp);
   }
    if (fur_check ==1)
    {
        fprintf(&lcd_str, "C   ON");
        fprintf(&lcd_str, "\x1b\x80");
    }
    else if (fur_check ==0)
    {
        fprintf(&lcd_str, "C  OFF");
        fprintf(&lcd_str, "\x1b\x80");
    }
}


//END OF CODE//






























