.text            // 放在代码段
.global  add     // 实现全局函数add
.thumb           // 使用thumb指令, main.c默认使用thumb指令, 所以这里也使用thumb指令

add:
	add r0, r0, r1
	bx lr
	
