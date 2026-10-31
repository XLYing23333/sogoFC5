0000000000422af6 <GetSogouVoiceOperation@@Base+0x347641>:
  422af6:	push   %rbp
  422af7:	mov    %rsp,%rbp
  422afa:	push   %r12
  422afc:	push   %rbx
  422afd:	sub    $0x10,%rsp
  422b01:	call   422d6d <GetSogouVoiceOperation@@Base+0x3478b8>
  422b06:	mov    %rax,-0x18(%rbp)
  422b0a:	cmpq   $0x0,-0x18(%rbp)
  422b0f:	jne    422b18 <GetSogouVoiceOperation@@Base+0x347663>
  422b11:	mov    $0x0,%eax
  422b16:	jmp    422b62 <GetSogouVoiceOperation@@Base+0x3476ad>
  422b18:	mov    -0x18(%rbp),%rax
  422b1c:	mov    0x10(%rax),%rax
  422b20:	test   %rax,%rax
  422b23:	jne    422b42 <GetSogouVoiceOperation@@Base+0x34768d>
  422b25:	mov    $0x50,%edi
  422b2a:	call   78a70 <operator new(unsigned long)@plt>
  422b2f:	mov    %rax,%rbx
  422b32:	mov    %rbx,%rdi
  422b35:	call   77520 <t_env::t_env()@plt>
  422b3a:	mov    -0x18(%rbp),%rax
  422b3e:	mov    %rbx,0x10(%rax)
  422b42:	mov    -0x18(%rbp),%rax
  422b46:	mov    0x10(%rax),%rax
  422b4a:	jmp    422b62 <GetSogouVoiceOperation@@Base+0x3476ad>
  422b4c:	mov    %rax,%r12
  422b4f:	mov    %rbx,%rdi
  422b52:	call   77a20 <operator delete(void*)@plt>
  422b57:	mov    %r12,%rax
  422b5a:	mov    %rax,%rdi
  422b5d:	call   77d00 <_Unwind_Resume@plt>
  422b62:	add    $0x10,%rsp
  422b66:	pop    %rbx
  422b67:	pop    %r12
  422b69:	pop    %rbp
  422b6a:	ret
