/*
 * Base.c
 *
 *  Created on: Aug 26, 2014
 *      Author: littlecat
 */


#include "MPC5604B.h"
#include "IntcInterrupts.h"
#include "Base.h"


static void INTC_InterruptHandler_UART0_RX(void);


/*
 * 禁用看门狗
 */
void Disable_Watchdog(void)
{
	SWT.SR.R = 0x0000c520;	/* rite keys to clear soft lock bit */
	SWT.SR.R = 0x0000d928;
	SWT.CR.R = 0x8000010A;	/* Clear watchdog enable (WEN) */
}


/*
 * 初始化单片机
 */
void Init_ModesAndClock(void) 
{
    ME.MER.R = 0x0000001D;	/* Enable DRUN, RUN0, SAFE, RESET modes */
	/* 设置sysclk */
    //CGM.FMPLL_CR.R = 0x02400100;	/* 8 MHz xtal: Set PLL0 to 64 MHz */
    CGM.FMPLL_CR.R = 0x01280000;	/* 8 MHz xtal: Set PLL0 to 80 MHz */
    //CGM.FMPLL_CR.R = 0x013C0000;	/* 8 MHz xtal: Set PLL0 to 120 MHz */ 
    ME.RUN[0].R = 0x001F0064;	/* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL     sysclk选择锁相环时钟 */
    ME.RUNPC[0].R = 0x00000010;	/* Peri. Cfg. 1 settings: only run in RUN0 mode      选择RUN0模式 */
  
	/* PCTL[?] 选择需要时钟模块(默认即可，不用设置) */
	//ME.PCTL[32].R = 0x00;	/* MPC56xxB/P/S ADC 0: select ME.RUNPC[0] */
    ME.PCTL[32].B.DBG_F = 0;
	/* Mode Transition to enter RUN0 mode */
    ME.MCTL.R = 0x40005AF0;	/* Enter RUN0 Mode & Key */
    ME.MCTL.R = 0x4000A50F;	/* Enter RUN0 Mode & Inverted Key */
    
    while (ME.GS.B.S_MTRANS) {}	/* Wait for mode transition to complete 等待模式转换完成 */
    while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode 等待选择RUN0模式 */
  
	/* 开peri0、1、2 */
	/* 外设时钟总线 可用于分频 */
	CGM.SC_DC[0].R = 0x84;	/* LIN */
	CGM.SC_DC[1].R = 0x80;	/* FLEXCAN,DSPI */
    CGM.SC_DC[2].R = 0x80;	/* eMIOS,CTU,ADC */
}


/*
 * 使能外部中断
 */
void Enable_IRQ(void)
{
	INTC.CPR.B.PRI = 0;	/* Single Core: Lower INTC's current priority */
	asm(" wrteei 1");	/* Enable external interrupts */
}


/*
 * 初始化串口0
 */
void Init_UART0(void)
{
	LINFLEX_0.LINCR1.B.INIT=1;  //进入初始化模式
	LINFLEX_0.LINCR1.R=0x00000015; 
	LINFLEX_0.LINIER.B.DRIE=1; //允许接收中断
	/*波特率算法baud=Fperiph_clk/(16*LFDIV)
	DIV_M=LFDIV整数部分
	DIV_F=LFDIV小数部分*16  */ 	
#ifndef PERIPH_SET_1_CLK_16M	//80M
	LINFLEX_0.LINIBRR.B.DIV_M= 43;  	//波特率设置38400:80M-130+3 57600:80M-86+13 115200:80M-43+6  9600:80M-520+83
    LINFLEX_0.LINFBRR.B.DIV_F = 6;		//38400:64M-104+3
#else
	LINFLEX_0.LINIBRR.B.DIV_M= 17;	//波特率设置 2400:416+11, 9600:104+3, 10417:96+0, 19200:52+1, 57600:17+6
    LINFLEX_0.LINFBRR.B.DIV_F = 6;	//波特率设置 115200:8+11, 230400:4+5, 460800:2+3, 921600:1+1
#endif
    LINFLEX_0.UARTCR.B.UART=1;
	LINFLEX_0.UARTCR.R=0x00000033;//8-bit data、UART mode
	LINFLEX_0.LINCR1.B.INIT=0; //退出初始化模式
	
	SIU.PCR[18].R = 0x0400;    /* MPC56xxB: Configure port B2 as LIN0TX */
    SIU.PCR[19].R = 0x0103;    /* MPC56xxB: Configure port B3 as LIN0RX */
  	INTC_InstallINTCInterruptHandler((INTCInterruptFn)INTC_InterruptHandler_UART0_RX, 79, 02); 
}


/*
 * 串口0发送函数
 */
void UART0_TX(UBYTE data)
{
	int i = 0;
	
	LINFLEX_0.BDRL.B.DATA0 = data;	//发送语句
	while(!LINFLEX_0.UARTSR.B.DTF)
	{
		if (i++ >= 1000)
		{
			break;	/* 防止DTF置位失败 */
		}
	}
	LINFLEX_0.UARTSR.B.DTF=1;
}


/*
 * 串口0接收中断服务例程
 */
static void INTC_InterruptHandler_UART0_RX(void)
{
	UBYTE rev_ch;
	
	while(!LINFLEX_0.UARTSR.B.DRF){}
	rev_ch = (UBYTE)LINFLEX_0.BDRM.B.DATA4;
	LINFLEX_0.UARTSR.B.DRF=1;	/* 清空标志位 */
}


/*
 * 初始化龙丘板载LED
 */
void Init_LQBoard_LED(void)
{
 	SIU.PCR[12].R = 0x0203;	/* PA12 */
  	SIU.PCR[13].R = 0x0203;
 	SIU.PCR[14].R = 0x0203; 
	SIU.PCR[15].R = 0x0203;	/* PA15 */
 	
	SIU.GPDO[12].R = 1;	/* 1=熄灭 */
	SIU.GPDO[13].R = 1;
	SIU.GPDO[14].R = 1;
	SIU.GPDO[15].R = 1;
}
