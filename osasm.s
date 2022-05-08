;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 4 starter file
; March 25, 2016

;


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler
        EXPORT  PendSV_Handler

SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
    ; same as Lab 3
    PUSH 	{R4-R11}
	LDR		R0, =RunPt
	LDR		R1, [R0]
	STR		SP, [R1]
	PUSH	{R0,LR}
	BL		Scheduler
	POP		{R0,LR}
	LDR		R1, [R0]
	LDR		SP, [R1]
	POP		{R4-R11}
	CPSIE	I
	BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

StartOS
    LDR     R0, =RunPt         ; currently running thread
    ; same as Lab 3
    LDR		R1, [R0]
	LDR		SP, [R1]
	POP		{R4-R11}
	POP		{R0-R3}
	POP		{R12}
	ADD		SP,SP,#4
	POP		{LR}
	ADD		SP,SP,#4
	CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

PendSV_Handler
    LDR     R0, =RunPt         ; run this thread next
    LDR     R2, [R0]           ; R2 = value of RunPt
    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11
    LDR     LR,=0xFFFFFFF9
    BX      LR                 ; start next thread
	
    ALIGN
    END
