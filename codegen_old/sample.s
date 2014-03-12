.text

####INSIDE FUNCTION
_test: 
push %ebp
mov %esp, %ebp
sub $4,%esp
#### Param Visited
mov 8(%ebp), %eax
mov %eax, -4(%ebp)
#### visitFuncBlock
#### Visit ID
pushl -4(%ebp)
#### RETURN
popl %eax
mov %ebp, %esp 
pop %ebp
	ret

.globl Main
Main:
push %ebp
mov %esp, %ebp
sub $68,%esp
#### visitFuncBlock
#### Visit INT
pushl $3
#### ASSIGN
popl %eax
mov %eax, -4(%ebp)
#### IF WITH ELSE
####VISIT ARRAY CALL
#### Visit INT
pushl $50
call _test
movl %eax, -60(%ebp)
#### Visit ArrayAccess Children
#### Visit ID
pushl -4(%ebp)
#### Visit ArrayAccess 
pop %eax
movl %eax, -44(%ebp)
pushl -44(%ebp)
#### RETURN
popl %eax
mov %ebp, %esp 
pop %ebp
	ret
