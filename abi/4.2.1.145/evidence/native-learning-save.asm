000000000010d880 <GetSogouVoiceOperation@@Base+0x323cb>:
  10d880:	push   %r12
  10d882:	push   %rbp
  10d883:	mov    %rdi,%r12
  10d886:	push   %rbx
  10d887:	mov    %rdx,%rbp
  10d88a:	sub    $0x10,%rsp
  10d88e:	cmpq   $0x0,(%rdi)
  10d892:	movl   $0x0,0xc(%rsp)
  10d89a:	je     10d91b <GetSogouVoiceOperation@@Base+0x32466>
  10d89c:	test   %rsi,%rsi
  10d89f:	mov    %rsi,%rbx
  10d8a2:	je     10d930 <GetSogouVoiceOperation@@Base+0x3247b>
  10d8a8:	call   e6280 <GetSogouVoiceOperation@@Base+0xadcb>
  10d8ad:	lea    0x520b81(%rip),%rsi        # 62e435 <_fini@@Base+0x4c65>
  10d8b4:	mov    %rax,%rdi
  10d8b7:	mov    %rbx,%rdx
  10d8ba:	xor    %eax,%eax
  10d8bc:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d8c1:	call   e6290 <GetSogouVoiceOperation@@Base+0xaddb>
  10d8c6:	lea    0x520b68(%rip),%rsi        # 62e435 <_fini@@Base+0x4c65>
  10d8cd:	mov    %rax,%rdi
  10d8d0:	mov    %rbx,%rdx
  10d8d3:	xor    %eax,%eax
  10d8d5:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d8da:	mov    (%r12),%rdi
  10d8de:	lea    0xc(%rsp),%rdx
  10d8e3:	xor    %esi,%esi
  10d8e5:	call   264bd0 <GetSogouVoiceOperation@@Base+0x18971b>
  10d8ea:	call   e6280 <GetSogouVoiceOperation@@Base+0xadcb>
  10d8ef:	lea    0x520b5d(%rip),%rsi        # 62e453 <_fini@@Base+0x4c83>
  10d8f6:	mov    %rax,%rdi
  10d8f9:	xor    %eax,%eax
  10d8fb:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d900:	call   e6290 <GetSogouVoiceOperation@@Base+0xaddb>
  10d905:	lea    0x520b47(%rip),%rsi        # 62e453 <_fini@@Base+0x4c83>
  10d90c:	mov    %rax,%rdi
  10d90f:	xor    %eax,%eax
  10d911:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d916:	call   e6340 <GetSogouVoiceOperation@@Base+0xae8b>
  10d91b:	test   %rbp,%rbp
  10d91e:	je     10d927 <GetSogouVoiceOperation@@Base+0x32472>
  10d920:	mov    0xc(%rsp),%eax
  10d924:	mov    %eax,0x0(%rbp)
  10d927:	add    $0x10,%rsp
  10d92b:	pop    %rbx
  10d92c:	pop    %rbp
  10d92d:	pop    %r12
  10d92f:	ret
  10d930:	call   e6280 <GetSogouVoiceOperation@@Base+0xadcb>
  10d935:	lea    0x51f718(%rip),%rdx        # 62d054 <_fini@@Base+0x3884>
  10d93c:	lea    0x520af2(%rip),%rsi        # 62e435 <_fini@@Base+0x4c65>
  10d943:	mov    %rax,%rdi
  10d946:	xor    %eax,%eax
  10d948:	lea    0x51f705(%rip),%rbx        # 62d054 <_fini@@Base+0x3884>
  10d94f:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d954:	jmp    10d8c1 <GetSogouVoiceOperation@@Base+0x3240c>
  10d959:	nop
  10d95a:	nopw   0x0(%rax,%rax,1)
  10d960:	push   %r12
  10d962:	push   %rbp
  10d963:	mov    %rdi,%r12
  10d966:	push   %rbx
  10d967:	mov    %rdx,%rbp
  10d96a:	sub    $0x10,%rsp
  10d96e:	cmpq   $0x0,(%rdi)
  10d972:	movl   $0x0,0xc(%rsp)
  10d97a:	je     10da02 <GetSogouVoiceOperation@@Base+0x3254d>
  10d980:	test   %rsi,%rsi
  10d983:	mov    %rsi,%rbx
  10d986:	je     10da20 <GetSogouVoiceOperation@@Base+0x3256b>
  10d98c:	call   e6280 <GetSogouVoiceOperation@@Base+0xadcb>
  10d991:	lea    0x520e08(%rip),%rsi        # 62e7a0 <_fini@@Base+0x4fd0>
  10d998:	mov    %rax,%rdi
  10d99b:	mov    %rbx,%rdx
  10d99e:	xor    %eax,%eax
  10d9a0:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d9a5:	call   e6290 <GetSogouVoiceOperation@@Base+0xaddb>
  10d9aa:	lea    0x520def(%rip),%rsi        # 62e7a0 <_fini@@Base+0x4fd0>
  10d9b1:	mov    %rax,%rdi
  10d9b4:	mov    %rbx,%rdx
  10d9b7:	xor    %eax,%eax
  10d9b9:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d9be:	mov    (%r12),%rdi
  10d9c2:	lea    0xc(%rsp),%rdx
  10d9c7:	mov    $0x1,%esi
  10d9cc:	call   264bd0 <GetSogouVoiceOperation@@Base+0x18971b>
  10d9d1:	call   e6280 <GetSogouVoiceOperation@@Base+0xadcb>
  10d9d6:	lea    0x520a8f(%rip),%rsi        # 62e46c <_fini@@Base+0x4c9c>
  10d9dd:	mov    %rax,%rdi
  10d9e0:	xor    %eax,%eax
  10d9e2:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d9e7:	call   e6290 <GetSogouVoiceOperation@@Base+0xaddb>
  10d9ec:	lea    0x520a79(%rip),%rsi        # 62e46c <_fini@@Base+0x4c9c>
  10d9f3:	mov    %rax,%rdi
  10d9f6:	xor    %eax,%eax
  10d9f8:	call   e5ef0 <GetSogouVoiceOperation@@Base+0xaa3b>
  10d9fd:	call   e6340 <GetSogouVoiceOperation@@Base+0xae8b>
  10da02:	test   %rbp,%rbp
  10da05:	je     10da0e <GetSogouVoiceOperation@@Base+0x32559>
  10da07:	mov    0xc(%rsp),%eax
  10da0b:	mov    %eax,0x0(%rbp)
  10da0e:	add    $0x10,%rsp
  10da12:	pop    %rbx
  10da13:	pop    %rbp
  10da14:	pop    %r12
  10da16:	ret
