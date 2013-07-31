#include <linux/linkage.h>

ENTRY(atmel_smc)
	stmfd	sp!, {r4-r11, lr}
	dsb
	.arch_extension	sec
	smc	#0
	ldmfd	sp!, {r4-r11, pc}
ENDPROC(atmel_smc)
