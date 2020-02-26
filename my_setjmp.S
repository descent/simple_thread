// our buffer looks like: eax,ebx,ecx,edx,esi,edi,esp,ebp,pc

	.globl	my_setjmp
my_setjmp:
	//pushl	%esp

#if 1
	pushl	%ebx
	movl	8(%esp), %ebx
	movl	%eax, (%ebx)
	popl	%eax


	pushl	%eax

	movl	%eax, 4(%ebx)
	movl	%ecx, 8(%ebx)
	movl	%edx, 12(%ebx)
	movl	%esi, 16(%ebx)
	movl	%edi, 20(%ebx)
	movl	%esp, 28(%ebx)
	movl	%ebp, 24(%ebx)


	movl	4(%esp), %eax
	movl	%eax, 32(%ebx)
#endif
        popl %ebx
	xorl	%eax, %eax
	//popl	%esp
	ret

	.globl	my_longjmp
my_longjmp:
	movl	4(%esp), %ebx	// address of buf
	movl	8(%esp), %eax	// store return value

	movl	28(%ebx), %esp	// restore stack
	movl	32(%ebx), %edi
// Next line sets up return address.
	movl	%edi, 0(%esp)	
	movl	8(%ebx), %ecx
	movl	12(%ebx), %edx
	movl	16(%ebx), %esi
	movl	20(%ebx), %edi
	movl	24(%ebx), %ebp
	movl	4(%ebx), %ebx
	testl	%eax,%eax
	jne	bye
	incl	%eax		// eax hold 0 if we are here
bye:
	ret
