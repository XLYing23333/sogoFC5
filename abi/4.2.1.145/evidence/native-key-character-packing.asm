00000000000a0ad0 <t_keyboardManager::GetKeyboardState()@@Base+0x2268>:
   a0ad0:	push   %rbp
   a0ad1:	mov    %rsp,%rbp
   a0ad4:	mov    %edi,%edx
   a0ad6:	mov    %esi,%eax
   a0ad8:	mov    %dx,-0x24(%rbp)
   a0adc:	mov    %al,-0x28(%rbp)
   a0adf:	movq   $0x0,-0x8(%rbp)
   a0ae7:	movzwl -0x24(%rbp),%eax
   a0aeb:	mov    %rax,-0x10(%rbp)
   a0aef:	movzbl -0x28(%rbp),%eax
   a0af3:	mov    %rax,-0x18(%rbp)
   a0af7:	mov    -0x10(%rbp),%rax
   a0afb:	shl    $0x10,%rax
   a0aff:	or     -0x18(%rbp),%rax
   a0b03:	mov    %rax,-0x8(%rbp)
   a0b07:	mov    -0x8(%rbp),%rax
   a0b0b:	pop    %rbp
   a0b0c:	ret
   a0b0d:	nop
