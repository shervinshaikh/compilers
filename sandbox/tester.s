# CS 160 Simple Testbed
# Winter 2014
# Do not distribute
#
#
# Don't modify any code before the
# comment "Your code starts here."
#

.text

.global _Tester
_Tester:
  pushl %ebp
  movl  %esp, %ebp
  pushl 8(%ebp)
  pushl 12(%ebp)
  
  # 
  # Your code starts here.
  # Parameters 1 and 2 are on the
  # stack. Param 2 is on the top,
  # param 1 is below it.
  # 3
  # 10
  # The code below simply adds the
  # two parameters, and pushes the
  # result. Replace with your code.
  # 

  call doWork


  doWork:
# parameter
####### ADD
####### INT literal
        pushl $5
####### INT literal
        pushl $5
        popl %ebx
        popl %eax
        addl %ebx, %eax
        pushl %eax
# parameter
####### INT literal
        pushl $5
        call Program_haha
## post-call
        addl $8, %ebp

Program_haha:
        pushl %ebp
        movl %esp, %ebp
        subl $4,%esp
#### total subtract from %%esp: 4
#### METHODBODY
## Local: 'z', type: bt_undef, offset: -4
##### ASSIGNMENT
####### INT literal
        pushl $5
        popl %eax
        movl %eax, -4(%ebp)
####### ADD
####### ADD
# variable: a, offset: 8, size: 4
        pushl 8(%ebp)
# variable: b, offset: 12, size: 4
        pushl 12(%ebp)
        popl %ebx
        popl %eax
        addl %ebx, %eax
        pushl %eax
# variable: z, offset: -4, size: 4
        pushl -4(%ebp)
        popl %ebx
        popl %eax
        addl %ebx, %eax
        pushl %eax
        popl %eax
## epilogue
        movl %ebp, %esp 
        popl %ebp
        leave
        ret



 end: 
  # 
  # Your code ends here.
  # Leave the return value on the
  # stack, and it will be printed.
  # 
  # Don't modify anything below this.
  # 
  
  popl  %eax
  leave
  ret
  