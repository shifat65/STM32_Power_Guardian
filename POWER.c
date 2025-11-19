#include"stm32f10x.h"
uint8_t Cnt_on=0;
uint8_t Cnt_on2=0;
uint8_t Emr_En=0;
void EnClock(void){ // Enableing port A & B and AFIO
RCC->APB2ENR |= (1<<2) | (1<<3) | (1<<0);
}
void EnPin(void){
GPIOB->CRL &= ~(15UL<<0);//B0
GPIOB->CRL |= 3UL<<0;//Output FOR POWER ON s1
GPIOB->CRL &= ~(15UL<<4);//B1
GPIOB->CRL |= 3UL<<4;//Output FOR MESSAGE s1
	
GPIOA->CRH &= ~(15UL<<4);//A9
GPIOA->CRH |= 3UL<<4;//Output FOR POWER ON s2
GPIOA->CRH &= ~(15UL<<8);//A10
GPIOA->CRH |= 3UL<<8;//Output FOR MESSAGE s2

GPIOA->CRH &= ~(15UL<<0);//A8
GPIOA->CRH |= 3UL<<0;//Output FOR Emargency
	
GPIOA->CRL &= ~(15UL<<0);//A0
GPIOA->CRL |= 4UL<<0;//INPUT  for overloads s1
GPIOA->CRL &= ~(15UL<<4);//A1
GPIOA->CRL |= 4UL<<4;//INPUT  for overloads s2

GPIOB->CRH &= ~(15UL<<16);//B12
GPIOB->CRH |= 4UL<<16;//Input for emargency
}

void Timer_Config(void){
RCC->APB1ENR |= 1UL<<0 | 1UL<<1;
TIM2->CNT = 0;
TIM2->PSC = 36000-1;
TIM2->ARR =8000-1;//8s config
TIM2->CR1 |= 1UL<<3;
	
TIM3->CNT = 0;
TIM3->PSC = 36000-1;
TIM3->ARR =8000-1;//8s config
TIM3->CR1 |= 1UL<<3;
}

void SysTick_En(){// Systic Configuration to get 1ms delay. 
SysTick->LOAD = 72000-1;
SysTick->VAL = 0; 
SysTick->CTRL = 5;
}

void delay_ms(){// Function that will provide 1ms delay
while (!(SysTick->CTRL & 1UL<<16)){}
}

void delay(uint32_t t){// Function that will provide t ms delay
	while(t--){delay_ms();}
}

void Em_delay(){
 uint32_t t= 5000;
		while ( t-- && Emr_En){
			delay_ms();			
				if((GPIOA->IDR & 1UL<<0) == 0)//handling load led 
				{	Cnt_on =0;
				TIM2->CR1 &=~(1<<0);//reset timer
				GPIOB->ODR &= ~(1<<1);
				}
		}
		Emr_En = 0;
	}
void EXTI_Config(){// Configuring EXTI12	
AFIO->EXTICR[3] &= ~(15UL<<0);//config b12
AFIO->EXTICR[3] |= 1<<0;

AFIO->EXTICR[0] &= ~(15UL<<0);//config A0
AFIO->EXTICR[0] |= 0<<0;
	
AFIO->EXTICR[0] &= ~(15UL<<4);//config A1
AFIO->EXTICR[0] |= 0<<4;	
	
EXTI->IMR  |= 1<<12 | 1<<0 | 1<<1;
EXTI->RTSR |= 1<<12 | 1<<0 | 1<<1;
}
void NVIC_Config(){// Enabling NVIC
NVIC_EnableIRQ(EXTI15_10_IRQn);
NVIC_EnableIRQ(EXTI0_IRQn);
NVIC_EnableIRQ(EXTI1_IRQn);
}
void EXTI15_10_IRQHandler(){// Interrupt Handler B0//emergency
if(EXTI->PR & 1UL<<12)
	EXTI->PR |= 1UL<<12;
Emr_En = ~Emr_En;
}
void EXTI0_IRQHandler(){// Interrupt Handler A0 //segment 1 over load
if(EXTI->PR & 1UL<<0)
	EXTI->PR |= 1UL<<0;
	TIM2->CR1 |=1;//start timer
	Cnt_on = 1;
	GPIOB->ODR |= 1<<1;//send message	
}
void EXTI1_IRQHandler(){// Interrupt Handler A0 //segment 1 over load
if(EXTI->PR & 1UL<<1)
	EXTI->PR |= 1UL<<1;

	TIM3->CR1 |=1;//start timer
	Cnt_on2 = 1;
	GPIOA->ODR |= 1<<10;//send message a10 s-2	
}
void Emergency(void){
	GPIOB->ODR |= 1<<0;// puting power on s1
	GPIOA->ODR |= 1<<9;// puting power on s2
	GPIOA->ODR |= (1<<8);// emargency led on
	Em_delay();// emargency delay	
	if(!Emr_En)// if emargency off, emargency led off
	GPIOA->ODR &= ~(1<<8);
}  
void Seg_01(void){ // segment 1 all control 	
if(Emr_En){// if emargency, then power on and go for 3s emargency  delay
	Emergency();}
else if(Cnt_on && !(TIM2->CR1 & 1<<0)){// if overload is on and count complete , power off
					GPIOB->ODR &= ~(1<<0);
			}
else if(Cnt_on && (TIM2->CR1 & 1<<0)){// if overload is on and count not complete , send message
				GPIOB->ODR |= 1<<1;//send message
			}
else if(!Cnt_on){// if no overload, power on and message off
			GPIOB->ODR |= 1<<0;
			GPIOB->ODR &= ~(1<<1);
			}

if((GPIOA->IDR & 1UL<<0) == 0)// if load is off, count off, message off
			{Cnt_on =0;
			TIM2->CR1 &=~(1<<0);//reset timer
			GPIOB->ODR &= ~(1<<1);
			}
if(!Emr_En)// if emargency off, emargency led off
	GPIOA->ODR &= ~(1<<8);	
}
void Seg_02(void){ // segment 2 all control 	
if(Emr_En){// if emargency, then power on and go for 3s emargency  delay
	Emergency();
}
else if(Cnt_on2 && !(TIM3->CR1 & 1<<0)){// if overload is on and count complete , power off
					GPIOA->ODR &= ~(1<<9);
			}
else if(Cnt_on2 && (TIM3->CR1 & 1<<0)){// if overload is on and count not complete , send message
				GPIOA->ODR |= 1<<10;//send message
			}
else if(!Cnt_on2){// if no overload, power on and message off
			GPIOA->ODR |= 1<<9;
			GPIOA->ODR &= ~(1<<10);
			}
if((GPIOA->IDR & 1UL<<1) == 0)// if load is off, count off, message off
			{Cnt_on2 =0;
			TIM3->CR1 &=~(1<<0);//reset timer
			GPIOA->ODR &= ~(1<<10);
			}
if(!Emr_En)// if emargency off, emargency led off
	GPIOA->ODR &= ~(1<<8);	
}
int main(){
EnClock();
EnPin();
Timer_Config();
SysTick_En();
EXTI_Config();
NVIC_Config();

while(1){
	if(Emr_En){// if emargency, then power on and go for 3s emargency  delay
	Emergency();}
	Seg_01();
	Seg_02();
}	
return 0;}