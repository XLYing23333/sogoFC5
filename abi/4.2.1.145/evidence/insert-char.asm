00000000002f7a3e <t_sogouIme::InsertChar(int)@@Base+0x84>:
  2f7a3e:	mov    0x5c3123(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f7a45:	mov    (%rax),%rax
  2f7a48:	mov    (%rax),%rax
  2f7a4b:	add    $0x30,%rax
  2f7a4f:	mov    (%rax),%rax
  2f7a52:	mov    0x5c310f(%rip),%rdx        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f7a59:	mov    (%rdx),%rdx
  2f7a5c:	mov    -0x1c(%rbp),%ecx
  2f7a5f:	mov    %ecx,%esi
  2f7a61:	mov    %rdx,%rdi
  2f7a64:	call   *%rax
  2f7a66:	mov    %al,-0x1(%rbp)
