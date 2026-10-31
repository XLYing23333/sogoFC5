00000000002f9264 <t_sogouIme::GetCursor(int&)@@Base+0x26>:
  2f9264:	mov    0x5c18fd(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f926b:	mov    (%rax),%rax
  2f926e:	mov    (%rax),%rax
  2f9271:	add    $0xd0,%rax
  2f9277:	mov    (%rax),%rax
  2f927a:	mov    0x5c18e7(%rip),%rdx        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f9281:	mov    (%rdx),%rdx
  2f9284:	mov    -0x20(%rbp),%rcx
  2f9288:	mov    %rcx,%rsi
  2f928b:	mov    %rdx,%rdi
  2f928e:	call   *%rax
  2f9290:	mov    %al,-0x1(%rbp)
  2f9293:	movzbl -0x1(%rbp),%eax
  2f9297:	leave
