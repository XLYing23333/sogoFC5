00000000001ad098 <t_env::SetValueBool(t_envEntry<bool> const*, bool)@@Base>:
  1ad098:	push   %rbp
  1ad099:	mov    %rsp,%rbp
  1ad09c:	sub    $0x20,%rsp
  1ad0a0:	mov    %rdi,-0x8(%rbp)
  1ad0a4:	mov    %rsi,-0x10(%rbp)
  1ad0a8:	mov    %edx,%eax
  1ad0aa:	mov    %al,-0x14(%rbp)
  1ad0ad:	mov    -0x8(%rbp),%rax
  1ad0b1:	movb   $0x1,0x48(%rax)
  1ad0b5:	movzbl -0x14(%rbp),%edx
  1ad0b9:	mov    -0x10(%rbp),%rcx
  1ad0bd:	mov    -0x8(%rbp),%rax
  1ad0c1:	mov    %rcx,%rsi
  1ad0c4:	mov    %rax,%rdi
  1ad0c7:	call   159890 <void t_env::LoSetValue<bool>(t_envEntry<bool> const*, bool)@plt>
  1ad0cc:	leave
  1ad0cd:	ret
