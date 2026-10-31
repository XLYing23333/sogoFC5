0000000000410d3e <GetSogouVoiceOperation@@Base+0x335889>:
  410d3e:	push   %rbp
  410d3f:	mov    %rsp,%rbp
  410d42:	sub    $0x20,%rsp
  410d46:	mov    %rdi,-0x18(%rbp)
  410d4a:	mov    %rsi,-0x20(%rbp)
  410d4e:	call   439d36 <GetSogouVoiceOperation@@Base+0x35e881>
  410d53:	mov    %rax,-0x8(%rbp)
  410d57:	mov    -0x8(%rbp),%rax
  410d5b:	mov    %rax,%rdi
  410d5e:	call   412e8a <GetSogouVoiceOperation@@Base+0x3379d5>
  410d63:	mov    %rax,-0x10(%rbp)
  410d67:	mov    -0x10(%rbp),%rax
  410d6b:	mov    (%rax),%rax
  410d6e:	add    $0x38,%rax
  410d72:	mov    (%rax),%rax
  410d75:	mov    -0x10(%rbp),%rdx
  410d79:	mov    %rdx,%rdi
  410d7c:	call   *%rax
  410d7e:	mov    %rax,%rdx
  410d81:	mov    -0x20(%rbp),%rax
  410d85:	mov    %rdx,(%rax)
  410d88:	mov    $0x1,%eax
  410d8d:	leave
  410d8e:	ret
  410d8f:	nop
