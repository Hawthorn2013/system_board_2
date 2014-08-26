/*
 * Base.h
 *
 *  Created on: Aug 26, 2014
 *      Author: littlecat
 */

#ifndef BASE_H_
#define BASE_H_


/* 定义外部宏 */
/* 定义板载LED */
#if 1
#define BD0 (SIU.GPDO[12].B.PDO)
#define BD1 (SIU.GPDO[13].B.PDO)
#define BD2 (SIU.GPDO[14].B.PDO)
#define BD3 (SIU.GPDO[15].B.PDO)
#endif


/* 定义外部函数 */
extern void Disable_Watchdog(void);
extern void Enable_IRQ(void);
extern void UART0_TX(UBYTE data);
extern void Init_UART0(void);
extern void Init_ModesAndClock(void);
extern void Init_LQBoard_LED(void);


#endif /* BASE_H_ */
