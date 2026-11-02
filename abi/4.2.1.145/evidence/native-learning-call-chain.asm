00000000000f1f9e <GetSogouVoiceOperation@@Base+0x16ae9>:
   f1f9e:	push   %rbp
   f1f9f:	mov    %rsp,%rbp
   f1fa2:	sub    $0x20,%rsp
   f1fa6:	mov    %rdi,-0x8(%rbp)
   f1faa:	mov    %rsi,-0x10(%rbp)
   f1fae:	mov    %edx,%eax
   f1fb0:	mov    %al,-0x14(%rbp)
   f1fb3:	cmpb   $0x0,-0x14(%rbp)
   f1fb7:	je     f1fdf <GetSogouVoiceOperation@@Base+0x16b2a>
   f1fb9:	call   237d60 <GetSogouVoiceOperation@@Base+0x15c8ab>
   f1fbe:	mov    %rax,%rdi
   f1fc1:	call   237f40 <GetSogouVoiceOperation@@Base+0x15ca8b>
   f1fc6:	mov    %rax,%rcx
   f1fc9:	mov    -0x10(%rbp),%rax
   f1fcd:	mov    $0x0,%edx
   f1fd2:	mov    %rax,%rsi
   f1fd5:	mov    %rcx,%rdi
   f1fd8:	call   10d960 <GetSogouVoiceOperation@@Base+0x324ab>
   f1fdd:	jmp    f2003 <GetSogouVoiceOperation@@Base+0x16b4e>
   f1fdf:	call   237d60 <GetSogouVoiceOperation@@Base+0x15c8ab>
   f1fe4:	mov    %rax,%rdi
   f1fe7:	call   237f40 <GetSogouVoiceOperation@@Base+0x15ca8b>
   f1fec:	mov    %rax,%rcx
   f1fef:	mov    -0x10(%rbp),%rax
   f1ff3:	mov    $0x0,%edx
   f1ff8:	mov    %rax,%rsi
   f1ffb:	mov    %rcx,%rdi
   f1ffe:	call   10d880 <GetSogouVoiceOperation@@Base+0x323cb>
   f2003:	leave
   f2004:	ret
