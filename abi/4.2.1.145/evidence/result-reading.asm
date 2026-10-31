00000000002f997a <t_sogouIme::UpdateUiInfo()@@Base+0x452>:
  2f997a:	mov    0x5c11e7(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f9981:	mov    (%rax),%rax
  2f9984:	mov    (%rax),%rax
  2f9987:	add    $0x98,%rax
  2f998d:	mov    (%rax),%rax
  2f9990:	mov    0x5c11d1(%rip),%rdx        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f9997:	mov    (%rdx),%rdx
  2f999a:	mov    %rdx,%rdi
  2f999d:	call   *%rax
  2f999f:	mov    %rax,-0x60(%rbp)
  2f99a3:	mov    -0x60(%rbp),%rax
  2f99a7:	mov    (%rax),%rax
  2f99aa:	mov    (%rax),%rax
  2f99ad:	mov    -0x60(%rbp),%rdx
  2f99b1:	mov    %rdx,%rdi
  2f99b4:	call   *%rax
  2f99b6:	mov    %rax,%rdx
  2f99b9:	mov    -0x40(%rbp),%rax
  2f99bd:	add    $0x58,%rax
  2f99c1:	mov    %rdx,%rsi
  2f99c4:	mov    %rax,%rdi
  2f99c7:	call   2cba10 <std::string::operator=(char const*)@plt>
