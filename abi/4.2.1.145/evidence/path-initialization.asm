0000000000481cc5 <SogouShellWrapper::Init(char const*, char const*, char const*, char const*, char const*, char const*)@@Base+0xd9>:
  481cc5:	mov    0x438e9c(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  481ccc:	mov    (%rax),%rax
  481ccf:	mov    (%rax),%rax
  481cd2:	add    $0x10,%rax
  481cd6:	mov    (%rax),%rbx
  481cd9:	call   2e0690 <GetSGAuthEnv@plt>
  481cde:	mov    %rax,%rdx
  481ce1:	mov    0x438e80(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  481ce8:	mov    (%rax),%rax
  481ceb:	mov    %rdx,%rsi
  481cee:	mov    %rax,%rdi
  481cf1:	call   *%rbx
  481cf3:	mov    0x438e6e(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  481cfa:	mov    (%rax),%rax
  481cfd:	mov    (%rax),%rax
  481d00:	add    $0x20,%rax
  481d04:	mov    (%rax),%rax
  481d07:	mov    0x438e5a(%rip),%rdx        # 8bab68 <g_pShell@@Base-0x25bb8>
  481d0e:	mov    (%rdx),%rdi
  481d11:	mov    -0x50(%rbp),%r8
  481d15:	mov    -0x48(%rbp),%rcx
  481d19:	mov    -0x40(%rbp),%rdx
  481d1d:	mov    -0x38(%rbp),%rsi
  481d21:	mov    0x10(%rbp),%r9
  481d25:	call   *%rax
