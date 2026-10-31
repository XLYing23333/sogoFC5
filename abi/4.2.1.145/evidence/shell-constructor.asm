000000000008a1f2 <Release@@Base+0x225>:
   8a1f2:	push   %rbp
   8a1f3:	mov    %rsp,%rbp
   8a1f6:	push   %rbx
   8a1f7:	sub    $0x18,%rsp
   8a1fb:	mov    %rdi,-0x18(%rbp)
   8a1ff:	mov    -0x18(%rbp),%rax
   8a203:	mov    %rax,%rdi
   8a206:	call   8f25c <Release@@Base+0x528f>
   8a20b:	mov    -0x18(%rbp),%rax
   8a20f:	lea    0x96252a(%rip),%rdx        # 9ec740 <_fini@@Base+0x3c2f70>
   8a216:	lea    0x10(%rdx),%rdx
   8a21a:	mov    %rdx,(%rax)
   8a21d:	mov    -0x18(%rbp),%rax
   8a221:	add    $0x8,%rax
   8a225:	mov    %rax,%rdi
   8a228:	call   8f4b8 <Release@@Base+0x54eb>
