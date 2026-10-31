00000000000a175e <t_keyboardManager::GetKeyboardState()@@Base+0x2ef6>:
   a175e:	push   %rbp
   a175f:	mov    %rsp,%rbp
   a1762:	push   %rbx
   a1763:	sub    $0x48,%rsp
   a1767:	mov    %rdi,-0x48(%rbp)
   a176b:	mov    %esi,-0x4c(%rbp)
   a176e:	mov    -0x4c(%rbp),%eax
   a1771:	shr    $0x1f,%eax
   a1774:	mov    %al,-0x19(%rbp)
   a1777:	mov    -0x4c(%rbp),%eax
   a177a:	mov    %al,-0x1a(%rbp)
   a177d:	movzbl -0x1a(%rbp),%ebx
   a1781:	call   78180 <t_keyboardManager::GetInstance()@plt>
   a1786:	mov    %ebx,%esi
   a1788:	mov    %rax,%rdi
   a178b:	call   9e87c <t_keyboardManager::GetKeyboardState()@@Base+0x14>
   a1790:	cmp    -0x19(%rbp),%al
   a1793:	setne  %al
   a1796:	test   %al,%al
   a1798:	je     a17c9 <t_keyboardManager::GetKeyboardState()@@Base+0x2f61>
   a179a:	mov    -0x48(%rbp),%rax
   a179e:	mov    (%rax),%rax
   a17a1:	add    $0xd8,%rax
   a17a7:	mov    (%rax),%rax
   a17aa:	mov    -0x4c(%rbp),%ecx
   a17ad:	mov    -0x48(%rbp),%rdx
   a17b1:	mov    %ecx,%esi
   a17b3:	mov    %rdx,%rdi
   a17b6:	call   *%rax
   a17b8:	xor    $0x1,%eax
   a17bb:	test   %al,%al
   a17bd:	je     a17c9 <t_keyboardManager::GetKeyboardState()@@Base+0x2f61>
   a17bf:	mov    $0x0,%eax
   a17c4:	jmp    a1941 <t_keyboardManager::GetKeyboardState()@@Base+0x30d9>
   a17c9:	movzbl -0x1a(%rbp),%ebx
   a17cd:	call   78180 <t_keyboardManager::GetInstance()@plt>
   a17d2:	mov    %rbx,%rsi
   a17d5:	mov    %rax,%rdi
   a17d8:	call   43b614 <t_keyboardManager::GetKeyboardState(unsigned char*) const@@Base+0x8a2>
   a17dd:	movzwl %ax,%eax
   a17e0:	mov    %rax,-0x28(%rbp)
   a17e4:	movw   $0x0,-0x3a(%rbp)
   a17ea:	call   78180 <t_keyboardManager::GetInstance()@plt>
   a17ef:	mov    %rax,%rdi
   a17f2:	call   78920 <t_keyboardManager::GetKeyboardState()@plt>
   a17f7:	mov    %rax,%rdi
   a17fa:	mov    -0x28(%rbp),%rax
   a17fe:	mov    %eax,%esi
   a1800:	movzbl -0x1a(%rbp),%eax
   a1804:	lea    -0x3a(%rbp),%rdx
   a1808:	mov    $0x0,%r8d
   a180e:	mov    %rdx,%rcx
   a1811:	mov    %rdi,%rdx
   a1814:	mov    %eax,%edi
   a1816:	call   79660 <ToAscii(unsigned int, unsigned int, unsigned char const*, unsigned short*, unsigned int)@plt>
   a181b:	mov    %eax,-0x2c(%rbp)
