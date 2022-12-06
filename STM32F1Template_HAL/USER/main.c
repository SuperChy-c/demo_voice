#include "sys.h"
#include "delay.h"
#include "usart.h"
u8 Run_flag = 0;
TIM_HandleTypeDef TIM3_Handler;
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler=psc;                     //分频系数
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM3_Handler.Init.Period=arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
}
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断   
	}
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&TIM3_Handler))
    {
				Run_flag = 1;
    }
}
void dat_init(void)
{
		GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOA_CLK_ENABLE();           	//开启GPIOA时钟
    GPIO_Initure.Pin=GPIO_PIN_8; 				//PA8
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  	//推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          	//上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;    //高速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
}
void busy_init(void)
{
		GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOC时钟
    GPIO_Initure.Pin=GPIO_PIN_5; 				//PC5
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLUP;          	//上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;    //高速
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
}
void Oneline_send(unsigned int dat)
{
	unsigned char i;
	unsigned int Temp_high;
	unsigned int Temp_low;

	Temp_high = dat & 0xff00;
	Temp_high = Temp_high >> 8;
	Temp_low = dat & 0x00ff;

	__HAL_RCC_TIM3_CLK_DISABLE();
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);	//PA8置0
	delay_ms(5);
	for (i = 0; i < 8; i++)
	{
		if ((Temp_high & 0x0001) == 1)
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//PA8置1
			delay_us(600);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);	//PA8置0
			delay_us(200);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//PA8置1
			delay_us(200);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);	//PA8置0
			delay_us(600);
		}
		Temp_high = Temp_high >> 1;
	}
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//PA8置1
	delay_ms(2);


	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);	//PA8置0
	delay_ms(5);
	for (i = 0; i < 8; i++)
	{
		if ((Temp_low & 0x0001) == 1)
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//PA8置1
			delay_us(600);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);	//PA8置0
			delay_us(200);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//PA8置1
			delay_us(200);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);	//PA8置0
			delay_us(600);
		}
		Temp_low = Temp_low >> 1;
	}
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//PA8置1
	delay_ms(100);
	__HAL_RCC_TIM3_CLK_ENABLE();
}
int main(void)
{
    HAL_Init();                    	 			//初始化HAL库    
    Stm32_Clock_Init(RCC_PLL_MUL9);   			//设置时钟,72M
		busy_init();
		dat_init();
		TIM3_Init(10000-1,7200-1);       	//定时器3 1秒中断一次
	while(1)
	{
		if(Run_flag)
			{
				Oneline_send(0x01);//send 0x01 address voice in per Second 
			}
	}
}
