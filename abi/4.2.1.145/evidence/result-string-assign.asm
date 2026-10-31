000000000007cd5c <GetSogouShell@@Base-0xcf71>:
   7cd5c:	push   %rbp
   7cd5d:	mov    %rsp,%rbp
   7cd60:	sub    $0x10,%rsp
   7cd64:	mov    %rdi,-0x8(%rbp)
   7cd68:	mov    %rsi,-0x10(%rbp)
   7cd6c:	cmpq   $0x0,-0x10(%rbp)
   7cd71:	je     7cd8a <shm_unlink@plt+0x2d6a>
   7cd73:	mov    -0x8(%rbp),%rax
   7cd77:	lea    0x8(%rax),%rdx
   7cd7b:	mov    -0x10(%rbp),%rax
   7cd7f:	mov    %rax,%rsi
   7cd82:	mov    %rdx,%rdi
   7cd85:	call   77cf0 <std::string::operator=(char const*)@plt>
   7cd8a:	mov    -0x8(%rbp),%rax
   7cd8e:	leave
   7cd8f:	ret
