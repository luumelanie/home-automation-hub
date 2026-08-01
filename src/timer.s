; LPC24xx Timer0 in ARM assembly (Keil ARMASM)
; Exposes:
;   void     init_timer_polling(void);
;   int      timer_update_seconds(void);
;   unsigned get_seconds(void);
;   uint32_t timer_get_tick(void);

        AREA    |.text|, CODE, READONLY, ALIGN=2
        ARM

        ; --------- Constants (LPC23xx/24xx) ----------
PCONP       EQU     0xE01FC0C4
PCLKSEL0    EQU     0xE01FC1A8

T0_BASE     EQU     0xE0004000
T0IR        EQU     (T0_BASE + 0x00)
T0TCR       EQU     (T0_BASE + 0x04)
T0TC        EQU     (T0_BASE + 0x08)
T0PR        EQU     (T0_BASE + 0x0C)
T0PC        EQU     (T0_BASE + 0x10)
T0MCR       EQU     (T0_BASE + 0x14)

        EXPORT  init_timer_polling
        EXPORT  timer_update_seconds
        EXPORT  get_seconds
        EXPORT  timer_get_tick
        EXPORT  seconds

; --------- Zero-initialized data ----------
        AREA    |.bss|, NOINIT, READWRITE, ALIGN=2
seconds     SPACE   4       ; volatile unsigned int seconds
last_tick   SPACE   4       ; static unsigned int last_tick

; --------- Code ----------
        AREA    |.text|, CODE, READONLY, ALIGN=2

; void init_timer_polling(void)
init_timer_polling
        STMFD   sp!, {lr}

        ; PCONP |= (1<<1)  (PCTIM0)
        LDR     r0, =PCONP
        LDR     r1, [r0]
        ORR     r1, r1, #(1<<1)
        STR     r1, [r0]

        ; PCLKSEL0: bits [3:2] = 01 -> CCLK
        LDR     r0, =PCLKSEL0
        LDR     r1, [r0]
        BIC     r1, r1, #(3<<2)
        ORR     r1, r1, #(1<<2)
        STR     r1, [r0]

        ; T0TCR = 0x02 (reset)
        LDR     r0, =T0TCR
        MOV     r1, #0x02
        STR     r1, [r0]

        ; T0PR = 13000 - 1  (1ms tick @ 13 MHz)
        LDR     r0, =T0PR
        LDR     r1, =12999
        STR     r1, [r0]

        ; T0TC = 0
        LDR     r0, =T0TC
        MOV     r1, #0
        STR     r1, [r0]

        ; T0TCR = 0x01 (enable)
        LDR     r0, =T0TCR
        MOV     r1, #0x01
        STR     r1, [r0]

        ; last_tick = 0; seconds = 0;
        LDR     r0, =last_tick
        MOV     r1, #0
        STR     r1, [r0]
        LDR     r0, =seconds
        STR     r1, [r0]

        LDMFD   sp!, {pc}

; int timer_update_seconds(void)
timer_update_seconds
        STMFD   sp!, {r4, lr}

        ; r0 = current tick (T0TC)
        LDR     r0, =T0TC
        LDR     r0, [r0]

        ; r2 = last_tick
        LDR     r1, =last_tick
        LDR     r2, [r1]

        ; r3 = diff = r0 - r2
        SUBS    r3, r0, r2
        CMP     r3, #1000
        BLT     no_second

        ; seconds++
        LDR     r4, =seconds
        LDR     r2, [r4]
        ADD     r2, r2, #1
        STR     r2, [r4]

        ; last_tick = current tick
        LDR     r1, =last_tick
        STR     r0, [r1]

        ; return 1
        MOV     r0, #1
        B       done

no_second
        MOV     r0, #0

done
        LDMFD   sp!, {r4, pc}

; unsigned int get_seconds(void)
get_seconds
        LDR     r0, =seconds
        LDR     r0, [r0]
        BX      lr

; uint32_t timer_get_tick(void)
timer_get_tick
        LDR     r0, =T0TC
        LDR     r0, [r0]
        BX      lr
		
		END
