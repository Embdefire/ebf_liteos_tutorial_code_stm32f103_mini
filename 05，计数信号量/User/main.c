/***************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   STM32全系列开发板-LiteOS！
  **************************************************************
  * @attention
  *
  * 实验平台:野火 F103-MINI STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ***************************************************************
  */ 
 /* LiteOS 头文件 */
#include "los_sys.h"
#include "los_task.ph"
#include "los_sem.h"
/* 板级外设头文件 */
#include "bsp_usart.h"
#include "bsp_led.h"
#include "bsp_key.h" 

/********************************** 任务句柄 *************************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

/* 定义任务句柄 */
UINT32 Pend_Task_Handle;
UINT32 Post_Task_Handle;

/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */
/* 定义计数信号量的句柄 */
UINT32 CountSem_Handle;

/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */



/* 函数声明 */
static UINT32 AppTaskCreate(void);
static UINT32 Creat_Pend_Task(void);
static UINT32 Creat_Post_Task(void);

static void Pend_Task(void);
static void Post_Task(void);
static void BSP_Init(void);


/***************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
			第二步：创建APP应用任务
			第三步：启动LiteOS，开始多任务调度，启动失败则输出错误信息
  **************************************************************/
int main(void)
{	
	UINT32 uwRet = LOS_OK;  //定义一个任务创建的返回值，默认为创建成功
	
	/* 板载相关初始化 */
  BSP_Init();
	
	printf("这是一个[野火]-STM32全系列开发板-LiteOS计数信号量实验！\n\n");
  printf("车位默认值为5个，按下KEY1申请车位，按下KEY2释放车位！\n\n");
	
	/* LiteOS 内核初始化 */
	uwRet = LOS_KernelInit();
	
  if (uwRet != LOS_OK)
  {
		printf("LiteOS 核心初始化失败！失败代码0x%X\n",uwRet);
		return LOS_NOK;
  }
	
	 /* 创建APP应用任务，所有的应用任务都可以放在这个函数里面 */
	uwRet = AppTaskCreate();
	if (uwRet != LOS_OK)
  {
		printf("AppTaskCreate创建任务失败！失败代码0x%X\n",uwRet);
		return LOS_NOK;
  }

  /* 开启LiteOS任务调度 */
  LOS_Start();
	
	//正常情况下不会执行到这里
	while(1);
}


/*********************************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 任务创建，为了方便管理，所有的任务创建函数都可以放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  ********************************************************************************/
static UINT32 AppTaskCreate(void)
{
	/* 定义一个返回类型变量，初始化为LOS_OK */
	UINT32 uwRet = LOS_OK;
	
	/* 创建一个计数信号量，初始化计数值为5*/
	uwRet = LOS_SemCreate (5,&CountSem_Handle);
	if (uwRet != LOS_OK)
	{
		printf("CountSem创建失败！失败代码0x%X\n",uwRet);
	}
		
	uwRet = Creat_Pend_Task();
  if (uwRet != LOS_OK)
  {
		printf("Pend_Task任务创建失败！失败代码0x%X\n",uwRet);
		return uwRet;
  }
	
	uwRet = Creat_Post_Task();
  if (uwRet != LOS_OK)
  {
		printf("Post_Task任务创建失败！失败代码0x%X\n",uwRet);
		return uwRet;
  }
	return LOS_OK;
}


/******************************************************************
  * @ 函数名  ： Creat_Pend_Task
  * @ 功能说明： 创建Pend_Task任务
  * @ 参数    ：   
  * @ 返回值  ： 无
  ******************************************************************/
static UINT32 Creat_Pend_Task()
{
	//定义一个创建任务的返回类型，初始化为创建成功的返回值
	UINT32 uwRet = LOS_OK;			
	
	//定义一个用于创建任务的参数结构体
	TSK_INIT_PARAM_S task_init_param;	

	task_init_param.usTaskPrio = 5;	/* 任务优先级，数值越小，优先级越高 */
	task_init_param.pcName = "Pend_Task";/* 任务名 */
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)Pend_Task;/* 任务函数入口 */
	task_init_param.uwStackSize = 1024;		/* 堆栈大小 */

	uwRet = LOS_TaskCreate(&Pend_Task_Handle, &task_init_param);/* 创建任务 */
	return uwRet;
}
/*******************************************************************
  * @ 函数名  ： Creat_Post_Task
  * @ 功能说明： 创建Post_Task任务
  * @ 参数    ：   
  * @ 返回值  ： 无
  ******************************************************************/
static UINT32 Creat_Post_Task()
{
	// 定义一个创建任务的返回类型，初始化为创建成功的返回值
	UINT32 uwRet = LOS_OK;				
	TSK_INIT_PARAM_S task_init_param;

	task_init_param.usTaskPrio = 4;	/* 任务优先级，数值越小，优先级越高 */
	task_init_param.pcName = "Post_Task";	/* 任务名*/
	task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)Post_Task;/* 任务函数入口 */
	task_init_param.uwStackSize = 1024;	/* 堆栈大小 */
	
	uwRet = LOS_TaskCreate(&Post_Task_Handle, &task_init_param);/* 创建任务 */

	return uwRet;
}

/******************************************************************
  * @ 函数名  ： Pend_Task
  * @ 功能说明： Pend_Task任务实现
  * @ 参数    ： NULL 
  * @ 返回值  ： NULL
  *****************************************************************/
static void Pend_Task(void)
{
	UINT32 uwRet = LOS_OK;				
	
  /* 任务都是一个无限循环，不能返回 */
	while(1)
	{
		//如果KEY1被按下
		if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )      
		{
			/* 获取一个计数信号量，等待时间0 */
			uwRet = LOS_SemPend ( CountSem_Handle,0); 

			if (LOS_OK ==  uwRet)                      
				printf ( "\r\nKEY1被按下，成功申请到停车位。\r\n" );
			else
				printf ( "\r\nKEY1被按下，不好意思，现在停车场已满！\r\n" );			
				
		}
		LOS_TaskDelay(20);     //每20ms扫描一次		
	}
}
/******************************************************************
  * @ 函数名  ： Post_Task
  * @ 功能说明： Post_Task任务实现
  * @ 参数    ： NULL 
  * @ 返回值  ： NULL
  *****************************************************************/
static void Post_Task(void)
{
	UINT32 uwRet = LOS_OK;				
   
	while(1)
	{
		//如果KEY2被按下
		if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )       
		{
			/* 释放一个计数信号量，LiteOS的计数信号量允许一直释放，在编程中注意一下即可*/
			uwRet = LOS_SemPost(CountSem_Handle);    
				
			if ( LOS_OK == uwRet ) 
				printf ( "\r\nKEY2被按下，释放1个停车位。\r\n" );	
			else
				printf ( "\r\nKEY2被按下，但已无车位可以释放！\r\n" );					
	
		}
		LOS_TaskDelay(20);     //每20ms扫描一次		
	}		
}


/*******************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  ******************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* LED 初始化 */
	LED_GPIO_Config();

	/* 串口初始化	*/
	USART_Config();

  /* 按键初始化 */
	Key_GPIO_Config();
}


/*********************************************END OF FILE**********************/

