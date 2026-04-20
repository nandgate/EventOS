/* File: startup_ARMCM3_STM32F1.S
 * Purpose: startup file for Cortex-M3 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V2.0
 * Date: 16 August 2013
 */
/* Copyright (c) 2011 - 2013 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
	.syntax	unified
	.arch	armv7-m

	.section .stack
	.align	3
#ifdef __STACK_SIZE
	.equ	Stack_Size, __STACK_SIZE
#else
	.equ	Stack_Size, 0xc00
#endif
	.globl	__StackTop
	.globl	__StackLimit
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, . - __StackTop

	.section .heap
	.align	3
#ifdef __HEAP_SIZE
	.equ	Heap_Size, __HEAP_SIZE
#else
	.equ	Heap_Size, 0
#endif
	.globl	__HeapBase
	.globl	__HeapLimit
__HeapBase:
	.if	Heap_Size
	.space	Heap_Size
	.endif
	.size	__HeapBase, . - __HeapBase
__HeapLimit:
	.size	__HeapLimit, . - __HeapLimit

	.section .isr_vector
	.align	2
	.globl	__isr_vector
__isr_vector:
	.long	__StackTop            /* Top of Stack */
	.long	Reset_Handler         /* Reset Handler */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	MemManage_Handler     /* MPU Fault Handler */
	.long	BusFault_Handler      /* Bus Fault Handler */
	.long	UsageFault_Handler    /* Usage Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_Handler           /* SVCall Handler */
	.long	DebugMon_Handler      /* Debug Monitor Handler */
	.long	0                     /* Reserved */
	.long	PendSV_Handler        /* PendSV Handler */
	.long	SysTick_Handler       /* SysTick Handler */

	/* External interrupts */
	.long	WWDG_Handler     	  /* 0: WWDG */
	.long	PVD_Handler      	  /* 1: PVD */
	.long	TAMPER_Handler   	  /* 2: Tamper */
	.long	RTC_Handler 	 	  /* 3: RTC */
	.long	FLASH_Handler 	 	  /* 4: Flash */
	.long	RCC_Handler 	 	  /* 5: RCC */
	.long	EXTI0_Handler 	 	  /* 6: EXTI Line 0 */
	.long	EXTI1_Handler 	 	  /* 7: EXTI Line 1 */
	.long	EXTI2_Handler 	 	  /* 8: EXTI Line 2 */
	.long	EXTI3_Handler 	 	  /* 9: EXTI Line 3 */
	.long	EXTI4_Handler 	 	  /* 10: EXTI Line 4 */
	.long	DMA11_Handler 	 	  /* 11: DMA1 Channel 1 */
	.long	DMA12_Handler 	 	  /* 12: DMA1 Channel 2 */
	.long	DMA13_Handler 	 	  /* 13: DMA1 Channel 3 */
	.long	DMA14_Handler 	 	  /* 14: DMA1 Channel 4 */
	.long	DMA15_Handler 	 	  /* 15: DMA1 Channel 5 */
	.long	DMA16_Handler 	 	  /* 16: DMA1 Channel 6 */
	.long	DMA17_Handler 	 	  /* 17: DMA1 Channel 7 */
	.long	ACD12_Handler 	 	  /* 18: ADC1 and ADC2 */
	.long	USBH_Handler 	 	  /* 19: USB High Priority or CAN TX interrupts */
	.long	USBL_Handler 	 	  /* 20: USB Low Priority or CAN RX0 interrupts */
	.long	CANRX_Handler 	 	  /* 21: CAN RX1 interrupt */
	.long	CANSCE_Handler 	 	  /* 22: CAN SCE interrupt */
	.long	EXTI9_Handler 	 	  /* 23: EXTI Line[9:5] interrupts */
	.long	TIM1BRK_Handler  	  /* 24: TIM1 Break interrupt */
	.long	TIM1_Handler 	 	  /* 25: TIM1 Update interrupt */
	.long	TIM1TRIG_Handler 	  /* 26: TIM1 Trigger and Commutation interrupts */
	.long	TIM1CAP_Handler  	  /* 27: TIM1 Capture Compare interrupt */
	.long	TIM2_Handler 	 	  /* 28: TIM2 global interrupt */
    .long	TIM3_Handler 	 	  /* 29: TIM3 global interrupt */
	.long	TIM4_Handler 	 	  /* 30: TIM4 global interrupt */
	.long	I2C1EVT_Handler  	  /* 31: I2C1 event interrupt */
	.long	I2C1ERR_Handler  	  /* 32: I2C1 error interrupt */
	.long	I2C2EVT_Handler  	  /* 33: I2C2 event interrupt */
	.long	I2C2ERR_Handler  	  /* 34: I2C2 error interrupt */
	.long	SPI1_Handler 	 	  /* 35: SPI1 global interrupt */
	.long	SPI2_Handler 	 	  /* 36: SPI2 global interrupt */
	.long	USART1_Handler 	 	  /* 37: USART1 global interrupt */
	.long	USART2_Handler 	 	  /* 38: USART2 global interrupt */
	.long	USART3_Handler 	 	  /* 39: USART3 global interrupt */
	.long	EXTI15_Handler 	 	  /* 40: EXTI Line[15:10] interrupts */
	.long	RTCALARM_Handler 	  /* 41: RTC Alarms through EXTI line interrupt */
	.long	Default_Handler  	  /* 42: Reserved */
	.long	TIM8BRK_Handler  	  /* 43: TIM8 Break interrupt */
	.long	TIM8_Handler 	 	  /* 44: TIM8 Update interrupt */
	.long	TIMTRG_Handler 	 	  /* 45: TIM8 Trigger and Commutation interrupts */
	.long	TIM8CAP_Handler  	  /* 46: TIM8 Capture Compare interrupt */
	.long	ADC3_Handler 	 	  /* 47: ADC3 global interrupt */
	.long	FSMC_Handler 	 	  /* 48: FSMC global interrupt */
	.long	DSIO_Handler 	 	  /* 49: SDIO global interrupt */
	.long	TIM5_Handler 	 	  /* 50: TIM5 global interrupt */
	.long	SPI3_Handler 	 	  /* 51: SPI3 global interrupt */
	.long	UART4_Handler 	 	  /* 52: UART4 global interrupt */
	.long	UART5_Handler 	 	  /* 53: UART5 global interrupt */
	.long	TIM6_Handler 	 	  /* 54: TIM6 global interrupt */
	.long	TIM7_Handler 	 	  /* 55: TIM7 global interrupt */
	.long	DMA21_Handler 	 	  /* 56: DMA2 Channel1 global interrupt */
	.long	DMA22_Handler 	 	  /* 57: DMA2 Channel2 global interrupt */
	.long	DMA23_Handler 	 	  /* 58: DMA2 Channel3 global interrupt */
	.long	DMA24_Handler 	 	  /* 59: DMA2 Channel4 and Channel 5 global interrupt */
	.size	__isr_vector, . - __isr_vector

	.text
	.thumb
	.thumb_func
	.align	2
	.globl	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r4, =__copy_table_start__
	ldr	r5, =__copy_table_end__

.L_loop0:
	cmp	r4, r5
	bge	.L_loop0_done
	ldr	r1, [r4]
	ldr	r2, [r4, #4]
	ldr	r3, [r4, #8]

.L_loop0_0:
	subs	r3, #4
	ittt	ge
	ldrge	r0, [r1, r3]
	strge	r0, [r2, r3]
	bge	.L_loop0_0

	adds	r4, #12
	b	.L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__etext
	ldr	r2, =__data_start__
	ldr	r3, =__data_end__

.L_loop1:
	cmp	r2, r3
	ittt	lt
	ldrlt	r0, [r1], #4
	strlt	r0, [r2], #4
	blt	.L_loop1
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
	ldr	r3, =__zero_table_start__
	ldr	r4, =__zero_table_end__

.L_loop2:
	cmp	r3, r4
	bge	.L_loop2_done
	ldr	r1, [r3]
	ldr	r2, [r3, #4]
	movs	r0, 0

.L_loop2_0:
	subs	r2, #4
	itt	ge
	strge	r0, [r1, r2]
	bge	.L_loop2_0

	adds	r3, #8
	b	.L_loop2
.L_loop2_done:
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__bss_start__
	ldr	r2, =__bss_end__

	movs	r0, 0
.L_loop3:
	cmp	r1, r2
	itt	lt
	strlt	r0, [r1], #4
	blt	.L_loop3
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
	bl	SystemInit
#endif

#ifndef __START
#define __START _start
#endif
	bl	__START

	.pool
	.size	Reset_Handler, . - Reset_Handler

	.align	1
	.thumb_func
	.weak	Default_Handler
	.type	Default_Handler, %function
Default_Handler:
	b	.
	.size	Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

	def_irq_handler	NMI_Handler
	def_irq_handler	HardFault_Handler
	def_irq_handler	MemManage_Handler
	def_irq_handler	BusFault_Handler
	def_irq_handler	UsageFault_Handler
	def_irq_handler	SVC_Handler
	def_irq_handler	DebugMon_Handler
	def_irq_handler	PendSV_Handler
	def_irq_handler	SysTick_Handler

	def_irq_handler	WWDG_Handler
	def_irq_handler	PVD_Handler
	def_irq_handler	TAMPER_Handler
	def_irq_handler	RTC_Handler 	  /* 3: RTC */
	def_irq_handler	FLASH_Handler 	  /* 4: Flash */
	def_irq_handler	RCC_Handler 	  /* 5: RCC */
	def_irq_handler	EXTI0_Handler 	  /* 6: EXTI Line 0 */
	def_irq_handler	EXTI1_Handler 	  /* 7: EXTI Line 1 */
	def_irq_handler	EXTI2_Handler 	  /* 8: EXTI Line 2 */
	def_irq_handler	EXTI3_Handler 	  /* 9: EXTI Line 3 */
	def_irq_handler	EXTI4_Handler 	  /* 10: EXTI Line 4 */
	def_irq_handler	DMA11_Handler 	  /* 11: DMA1 Channel 1 */
	def_irq_handler	DMA12_Handler 	  /* 12: DMA1 Channel 2 */
	def_irq_handler	DMA13_Handler 	  /* 13: DMA1 Channel 3 */
	def_irq_handler	DMA14_Handler 	  /* 14: DMA1 Channel 4 */
	def_irq_handler	DMA15_Handler 	  /* 15: DMA1 Channel 5 */
	def_irq_handler	DMA16_Handler 	  /* 16: DMA1 Channel 6 */
	def_irq_handler	DMA17_Handler 	  /* 17: DMA1 Channel 7 */
	def_irq_handler	ACD12_Handler 	  /* 18: ADC1 and ADC2 */
	def_irq_handler	USBH_Handler 	  /* 19: USB High Priority or CAN TX interrupts */
	def_irq_handler	USBL_Handler 	  /* 20: USB Low Priority or CAN RX0 interrupts */
	def_irq_handler	CANRX_Handler 	  /* 21: CAN RX1 interrupt */
	def_irq_handler	CANSCE_Handler 	  /* 22: CAN SCE interrupt */
	def_irq_handler	EXTI9_Handler 	  /* 23: EXTI Line[9:5] interrupts */
	def_irq_handler	TIM1BRK_Handler 	  /* 24: TIM1 Break interrupt */
	def_irq_handler	TIM1_Handler 	  /* 25: TIM1 Update interrupt */
	def_irq_handler	TIM1TRIG_Handler 	  /* 26: TIM1 Trigger and Commutation interrupts */
	def_irq_handler	TIM1CAP_Handler 	  /* 27: TIM1 Capture Compare interrupt */
	def_irq_handler	TIM2_Handler 	  /* 28: TIM2 global interrupt */
    def_irq_handler	TIM3_Handler 	  /* 29: TIM3 global interrupt */
	def_irq_handler	TIM4_Handler 	  /* 30: TIM4 global interrupt */
	def_irq_handler	I2C1EVT_Handler 	  /* 31: I2C1 event interrupt */
	def_irq_handler	I2C1ERR_Handler 	  /* 32: I2C1 error interrupt */
	def_irq_handler	I2C2EVT_Handler 	  /* 33: I2C2 event interrupt */
	def_irq_handler	I2C2ERR_Handler 	  /* 34: I2C2 error interrupt */
	def_irq_handler	SPI1_Handler 	  /* 35: SPI1 global interrupt */
	def_irq_handler	SPI2_Handler 	  /* 36: SPI2 global interrupt */
	def_irq_handler	USART1_Handler 	  /* 37: USART1 global interrupt */
	def_irq_handler	USART2_Handler 	  /* 38: USART2 global interrupt */
	def_irq_handler	USART3_Handler 	  /* 39: USART3 global interrupt */
	def_irq_handler	EXTI15_Handler 	  /* 40: EXTI Line[15:10] interrupts */
	def_irq_handler	RTCALARM_Handler 	  /* 41: RTC Alarms through EXTI line interrupt */
	def_irq_handler	TIM8BRK_Handler 	  /* 43: TIM8 Break interrupt */
	def_irq_handler	TIM8_Handler 	  /* 44: TIM8 Update interrupt */
	def_irq_handler	TIMTRG_Handler 	  /* 45: TIM8 Trigger and Commutation interrupts */
	def_irq_handler	TIM8CAP_Handler 	  /* 46: TIM8 Capture Compare interrupt */
	def_irq_handler	ADC3_Handler 	  /* 47: ADC3 global interrupt */
	def_irq_handler	FSMC_Handler 	  /* 48: FSMC global interrupt */
	def_irq_handler	DSIO_Handler 	  /* 49: SDIO global interrupt */
	def_irq_handler	TIM5_Handler 	  /* 50: TIM5 global interrupt */
	def_irq_handler	SPI3_Handler 	  /* 51: SPI3 global interrupt */
	def_irq_handler	UART4_Handler 	  /* 52: UART4 global interrupt */
	def_irq_handler	UART5_Handler 	  /* 53: UART5 global interrupt */
	def_irq_handler	TIM6_Handler 	  /* 54: TIM6 global interrupt */
	def_irq_handler	TIM7_Handler 	  /* 55: TIM7 global interrupt */
	def_irq_handler	DMA21_Handler 	  /* 56: DMA2 Channel1 global interrupt */
	def_irq_handler	DMA22_Handler 	  /* 57: DMA2 Channel2 global interrupt */
	def_irq_handler	DMA23_Handler 	  /* 58: DMA2 Channel3 global interrupt */
	def_irq_handler	DMA24_Handler 	  /* 59: DMA2 Channel4 and Channel 5 global interrupt */

	.end
