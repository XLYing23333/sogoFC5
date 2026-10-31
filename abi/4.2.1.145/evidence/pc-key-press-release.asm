00000000000a181e <t_keyboardManager::GetKeyboardState()@@Base+0x2fb6>:
   a181e:	movq   $0x0,-0x18(%rbp)
   a1826:	cmpb   $0x0,-0x19(%rbp)
   a182a:	je     a183e <t_keyboardManager::GetKeyboardState()@@Base+0x2fd6>
   a182c:	mov    -0x28(%rbp),%rax
   a1830:	shl    $0x10,%rax
   a1834:	or     $0x1,%rax
   a1838:	mov    %rax,-0x18(%rbp)
   a183c:	jmp    a1855 <t_keyboardManager::GetKeyboardState()@@Base+0x2fed>
   a183e:	mov    -0x28(%rbp),%rax
   a1842:	shl    $0x10,%rax
   a1846:	mov    %rax,%rdx
   a1849:	mov    $0xc0000001,%eax
   a184e:	or     %rdx,%rax
   a1851:	mov    %rax,-0x18(%rbp)
   a1855:	movzbl -0x1a(%rbp),%edx
   a1859:	movzwl -0x3a(%rbp),%eax
   a185d:	movzwl %ax,%eax
   a1860:	mov    %edx,%esi
   a1862:	mov    %eax,%edi
   a1864:	call   a0ad0 <t_keyboardManager::GetKeyboardState()@@Base+0x2268>
   a1869:	mov    %rax,-0x38(%rbp)
   a186d:	cmpb   $0x0,-0x19(%rbp)
   a1871:	je     a18c6 <t_keyboardManager::GetKeyboardState()@@Base+0x305e>
   a1873:	mov    -0x48(%rbp),%rax
   a1877:	mov    0x128(%rax),%rax
   a187e:	mov    (%rax),%rax
   a1881:	add    $0x28,%rax
   a1885:	mov    (%rax),%rbx
   a1888:	call   78180 <t_keyboardManager::GetInstance()@plt>
   a188d:	mov    %rax,%rdi
   a1890:	call   78920 <t_keyboardManager::GetKeyboardState()@plt>
   a1895:	mov    %rax,%rcx
   a1898:	mov    -0x18(%rbp),%rax
   a189c:	shr    $0x10,%rax
   a18a0:	mov    %eax,%edx
   a18a2:	mov    -0x38(%rbp),%rax
   a18a6:	mov    %eax,%esi
   a18a8:	mov    -0x48(%rbp),%rax
   a18ac:	mov    0x128(%rax),%rax
   a18b3:	mov    %rax,%rdi
   a18b6:	call   *%rbx
   a18b8:	mov    %eax,%edx
   a18ba:	mov    -0x48(%rbp),%rax
   a18be:	mov    %edx,0x154(%rax)
   a18c4:	jmp    a1926 <t_keyboardManager::GetKeyboardState()@@Base+0x30be>
   a18c6:	mov    -0x48(%rbp),%rax
   a18ca:	mov    0x154(%rax),%eax
   a18d0:	cmp    $0x3,%eax
   a18d3:	je     a1926 <t_keyboardManager::GetKeyboardState()@@Base+0x30be>
   a18d5:	mov    -0x48(%rbp),%rax
   a18d9:	mov    0x128(%rax),%rax
   a18e0:	mov    (%rax),%rax
   a18e3:	add    $0x28,%rax
   a18e7:	mov    (%rax),%rbx
   a18ea:	call   78180 <t_keyboardManager::GetInstance()@plt>
   a18ef:	mov    %rax,%rdi
   a18f2:	call   78920 <t_keyboardManager::GetKeyboardState()@plt>
   a18f7:	mov    %rax,%rcx
   a18fa:	mov    -0x18(%rbp),%rax
   a18fe:	shr    $0x10,%rax
   a1902:	mov    %eax,%edx
   a1904:	mov    -0x38(%rbp),%rax
   a1908:	mov    %eax,%esi
   a190a:	mov    -0x48(%rbp),%rax
   a190e:	mov    0x128(%rax),%rax
   a1915:	mov    %rax,%rdi
   a1918:	call   *%rbx
   a191a:	mov    %eax,%edx
   a191c:	mov    -0x48(%rbp),%rax
   a1920:	mov    %edx,0x154(%rax)
   a1926:	mov    -0x48(%rbp),%rax
   a192a:	mov    %rax,%rdi
   a192d:	call   a1b9a <t_keyboardManager::GetKeyboardState()@@Base+0x3332>
   a1932:	mov    -0x48(%rbp),%rax
   a1936:	mov    0x154(%rax),%eax
   a193c:	test   %eax,%eax
   a193e:	setne  %al
   a1941:	add    $0x48,%rsp
   a1945:	pop    %rbx
   a1946:	pop    %rbp
   a1947:	ret
