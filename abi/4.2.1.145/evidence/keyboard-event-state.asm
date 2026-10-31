000000000009e868 <t_keyboardManager::GetKeyboardState()@@Base>:
   9e868:	push   %rbp
   9e869:	mov    %rsp,%rbp
   9e86c:	mov    %rdi,-0x8(%rbp)
   9e870:	mov    -0x8(%rbp),%rax
   9e874:	add    $0x400,%rax
   9e87a:	pop    %rbp
   9e87b:	ret
   9e87c:	push   %rbp
   9e87d:	mov    %rsp,%rbp
   9e880:	mov    %rdi,-0x8(%rbp)
   9e884:	mov    %esi,%eax
   9e886:	mov    %al,-0xc(%rbp)
   9e889:	movzbl -0xc(%rbp),%eax
   9e88d:	mov    -0x8(%rbp),%rdx
   9e891:	cltq
   9e893:	movzbl 0x400(%rdx,%rax,1),%eax
   9e89b:	shr    $0x7,%al
   9e89e:	pop    %rbp
   9e89f:	ret
