00000000002fa045 <t_sogouIme::UpdateUiInfo()@@Base+0xb1d>:
  2fa045:	mov    0x5c0b1c(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  2fa04c:	mov    (%rax),%rax
  2fa04f:	mov    (%rax),%rax
  2fa052:	add    $0x88,%rax
  2fa058:	mov    (%rax),%rax
  2fa05b:	mov    0x5c0b06(%rip),%rdx        # 8bab68 <g_pShell@@Base-0x25bb8>
  2fa062:	mov    (%rdx),%rdx
  2fa065:	mov    %rdx,%rdi
  2fa068:	call   *%rax
  2fa06a:	mov    %rax,-0xa0(%rbp)
  2fa071:	mov    -0xa0(%rbp),%rax
  2fa078:	mov    (%rax),%rax
  2fa07b:	mov    (%rax),%rax
  2fa07e:	mov    -0xa0(%rbp),%rdx
  2fa085:	mov    %rdx,%rdi
  2fa088:	call   *%rax
  2fa08a:	mov    %rax,%rdx
  2fa08d:	mov    -0x40(%rbp),%rax
  2fa091:	add    $0x38,%rax
  2fa095:	mov    %rdx,%rsi
  2fa098:	mov    %rax,%rdi
