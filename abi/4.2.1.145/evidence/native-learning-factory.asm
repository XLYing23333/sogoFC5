0000000000237d60 <GetSogouVoiceOperation@@Base+0x15c8ab>:
  237d60:	mov    0x7ee979(%rip),%rax        # a266e0 <_fini@@Base+0x3fcf10>
  237d67:	test   %rax,%rax
  237d6a:	je     237d70 <GetSogouVoiceOperation@@Base+0x15c8bb>
  237d6c:	ret
  237d6d:	nopl   (%rax)
  237d70:	push   %rbp
  237d71:	push   %rbx
  237d72:	mov    $0x14f28,%edi
  237d77:	sub    $0x8,%rsp
  237d7b:	call   78a70 <operator new(unsigned long)@plt>
  237d80:	mov    %rax,%rdi
  237d83:	mov    %rax,%rbx
  237d86:	call   237b70 <GetSogouVoiceOperation@@Base+0x15c6bb>
  237d8b:	mov    %rbx,0x7ee94e(%rip)        # a266e0 <_fini@@Base+0x3fcf10>
  237d92:	add    $0x8,%rsp
  237d96:	mov    %rbx,%rax
  237d99:	pop    %rbx
  237d9a:	pop    %rbp
  237d9b:	ret
