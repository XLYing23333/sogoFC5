000000000008b192 <Release@@Base+0x11c5>:
   8b192:	push   %rbp
   8b193:	mov    %rsp,%rbp
   8b196:	sub    $0x10,%rsp
   8b19a:	mov    %rdi,-0x8(%rbp)
   8b19e:	mov    -0x8(%rbp),%rax
   8b1a2:	mov    0x38(%rax),%rax
   8b1a6:	test   %rax,%rax
   8b1a9:	jne    8b1b4 <Release@@Base+0x11e7>
   8b1ab:	lea    0x99350e(%rip),%rax        # a1e6c0 <_fini@@Base+0x3f4ef0>
   8b1b2:	jmp    8b1d5 <Release@@Base+0x1208>
   8b1b4:	mov    -0x8(%rbp),%rax
   8b1b8:	mov    0x38(%rax),%rax
   8b1bc:	mov    (%rax),%rax
   8b1bf:	add    $0x98,%rax
   8b1c5:	mov    (%rax),%rax
   8b1c8:	mov    -0x8(%rbp),%rdx
   8b1cc:	mov    0x38(%rdx),%rdx
   8b1d0:	mov    %rdx,%rdi
   8b1d3:	call   *%rax
   8b1d5:	leave
   8b1d6:	ret
   8b1d7:	nop
