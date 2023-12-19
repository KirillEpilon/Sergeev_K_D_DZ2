/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 9.3.0   2023-06-23

The MIT License (MIT)
Copyright (c) 2019 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include "stm32f0xx.h"

/* Private macro */
#define DEFAULT_PRESCALER 799
#define ARR_FOR_OPTIONS 10000

/* Private variables */
uint8_t buffer, fl=0;
uint8_t Data[1023];
uint8_t en=0;
uint8_t ready=0;
uint16_t arr=1 ;

/* Private function prototypes */
void PINS_Init();
void Reseve();
void Save();
void RCC_Init();
void TIM6_Init();
void Request();
void Transmit();

/* Private functions */
void PINS_Init(){

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOA->MODER &= 0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR2_1 | GPIO_PUPDR_PUPDR3_1 | GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1 | GPIO_PUPDR_PUPDR6_1 | GPIO_PUPDR_PUPDR7_1 | GPIO_PUPDR_PUPDR8_1 | GPIO_PUPDR_PUPDR9_1 | GPIO_PUPDR_PUPDR10_1 | GPIO_PUPDR_PUPDR11_1 | GPIO_PUPDR_PUPDR12_0;
	GPIOB->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0;
	GPIOB->OTYPER &= 0;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR2_1;

}
void Reseve(){

	while(!en);
	en=0;
	buffer = (uint8_t)(GPIOA->IDR & 255);

}
void Save(){

	uint16_t ind=0;
	TIM6_Init();
	while (ind<1024){

		GPIOB->BSRR |= 	GPIO_BSRR_BR_2; //ready =0
		Reseve();
		Data[ind]=buffer;
		ind ++;
		while (!((TIM6->SR & TIM_SR_UIF) == TIM_SR_UIF));
		GPIOB->BSRR |= GPIO_BSRR_BS_2; //ready =1
		GPIOB->BSRR &= 0;
		en=1;

	}

}
void RCC_Init(){

	RCC->CFGR &= ~RCC_CFGR_SW_HSE & ~RCC_CFGR_SW_PLL;
	RCC->CR |= RCC_CR_HSION;
	while(RCC->CR & RCC_CR_HSIRDY);
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
}
void TIM6_Init(){

	TIM6->CR1 &= ~TIM_CR1_CEN;
	TIM6->CR1 &= ~TIM_CR1_UDIS;
	TIM6->PSC = DEFAULT_PRESCALER;
	if (fl)
		TIM6->ARR = ARR_FOR_OPTIONS;
	else
		TIM6->ARR = arr;
	TIM6->CR1 |= TIM_CR1_CEN;
}
void Request(){


	en |= (GPIOA->IDR & 4096) << 12;
	fl=1;
			TIM6_Init();
	while ((GPIOA->IDR & 256)) //êíîïêà çàæàòà
	{
		if ((TIM6->SR & TIM_SR_UIF) == TIM_SR_UIF){

			buffer=(uint8_t)(((7<<9) & GPIOA->IDR)>>9);

			if (buffer == 0)
				arr=100;
			else if (buffer == 1)
				arr=50;
			else if (buffer == 2)
				arr=20;
			else if (buffer == 3)
				arr=10;
			else if (buffer == 4)
				arr=5;
			else if (buffer == 5)
				arr=4;
			else if (buffer == 6)
				arr=2;
			else if (buffer == 7)
				arr=1;

		}

	}
	fl=0;
	if (!(GPIOA->IDR & 256)) //êíîïêó íàæàëè è îòïóñòèëè
		Transmit();

}
void Transmit(){

	uint8_t i = 0;
	uint8_t j = 8;
	TIM6_Init();
	while (i<1024){

		while (!((TIM6->SR & TIM_SR_UIF) == TIM_SR_UIF));
		GPIOB->BSRR |= GPIO_BSRR_BR_1;

		while (j){

			GPIOB->ODR |= (Data[i]<<j) & 1;
			j--;
			while (!((TIM6->SR & TIM_SR_UIF) == TIM_SR_UIF));

		}

		GPIOB->BSRR |= GPIO_BSRR_BS_1;
		GPIOB->BSRR &= 0;
		i++;
		j = 8;
	}

}

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  /* TODO - Add your application code here */

  RCC_Init();
  PINS_Init();

  /* Infinite loop */
  while (1)
  {

	  Request();
	  if(en)
		  Save();
	  en=0;

  }
}
