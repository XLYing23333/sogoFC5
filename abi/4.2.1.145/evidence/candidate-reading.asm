00000000002f98f2 <t_sogouIme::UpdateUiInfo()@@Base+0x3ca>:
  2f98f2:	mov    0x5c126f(%rip),%rax        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f98f9:	mov    (%rax),%rax
  2f98fc:	mov    (%rax),%rax
  2f98ff:	sub    $0xffffffffffffff80,%rax
  2f9903:	mov    (%rax),%rax
  2f9906:	mov    0x5c125b(%rip),%rdx        # 8bab68 <g_pShell@@Base-0x25bb8>
  2f990d:	mov    (%rdx),%rdx
  2f9910:	mov    %rdx,%rdi
  2f9913:	call   *%rax
  2f9915:	mov    %rax,-0x50(%rbp)
  2f9919:	mov    -0x50(%rbp),%rax
  2f991d:	mov    (%rax),%rax
  2f9920:	add    $0x8,%rax
  2f9924:	mov    (%rax),%rax
  2f9927:	mov    -0x50(%rbp),%rdx
  2f992b:	mov    %rdx,%rdi
  2f992e:	call   *%rax
  2f9930:	mov    %eax,-0x54(%rbp)
  2f9933:	lea    0x2857e2(%rip),%rdi        # 57f11c <_fini@@Base+0x11c>
  2f993a:	call   2cddf0 <n_sogouBase::t_logger::GetInstanceW(wchar_t const*)@plt>
  2f993f:	mov    (%rax),%rdx
  2f9942:	add    $0x8,%rdx
  2f9946:	mov    (%rdx),%r9
  2f9949:	mov    -0x54(%rbp),%edx
  2f994c:	mov    %edx,%r8d
  2f994f:	mov    $0x496,%ecx
  2f9954:	lea    0x2860ba(%rip),%rdx        # 57fa15 <_fini@@Base+0xa15>
  2f995b:	lea    0x2859dc(%rip),%rsi        # 57f33e <_fini@@Base+0x33e>
  2f9962:	mov    %rax,%rdi
  2f9965:	mov    $0x0,%eax
  2f996a:	call   *%r9
  2f996d:	mov    -0x40(%rbp),%rax
  2f9971:	mov    -0x54(%rbp),%edx
  2f9974:	mov    %edx,0x284(%rax)
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
  2f99cc:	mov    -0x40(%rbp),%rax
  2f99d0:	add    $0x290,%rax
  2f99d6:	mov    %rax,%rdi
  2f99d9:	call   2cb140 <std::vector<std::string, std::allocator<std::string> >::clear()@plt>
  2f99de:	movl   $0x0,-0x14(%rbp)
  2f99e5:	jmp    2f9a7b <t_sogouIme::UpdateUiInfo()@@Base+0x553>
  2f99ea:	lea    -0xf1(%rbp),%rax
  2f99f1:	mov    %rax,%rdi
  2f99f4:	call   2d29a0 <std::allocator<char>::allocator()@plt>
  2f99f9:	mov    -0x50(%rbp),%rax
  2f99fd:	mov    (%rax),%rax
  2f9a00:	mov    (%rax),%rax
  2f9a03:	mov    -0x14(%rbp),%edx
  2f9a06:	movslq %edx,%rcx
  2f9a09:	mov    -0x50(%rbp),%rdx
  2f9a0d:	mov    %rcx,%rsi
  2f9a10:	mov    %rdx,%rdi
  2f9a13:	call   *%rax
  2f9a15:	mov    (%rax),%rdx
  2f9a18:	mov    (%rdx),%rdx
  2f9a1b:	mov    %rax,%rdi
  2f9a1e:	call   *%rdx
  2f9a20:	mov    %rax,%rcx
  2f9a23:	lea    -0xf1(%rbp),%rdx
  2f9a2a:	lea    -0x100(%rbp),%rax
  2f9a31:	mov    %rcx,%rsi
  2f9a34:	mov    %rax,%rdi
  2f9a37:	call   2d9690 <std::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(char const*, std::allocator<char> const&)@plt>
  2f9a3c:	mov    -0x40(%rbp),%rax
  2f9a40:	lea    0x290(%rax),%rdx
  2f9a47:	lea    -0x100(%rbp),%rax
  2f9a4e:	mov    %rax,%rsi
  2f9a51:	mov    %rdx,%rdi
  2f9a54:	call   2e62c0 <std::vector<std::string, std::allocator<std::string> >::push_back(std::string&&)@plt>
  2f9a59:	lea    -0x100(%rbp),%rax
  2f9a60:	mov    %rax,%rdi
  2f9a63:	call   2f4470 <std::basic_string<char, std::char_traits<char>, std::allocator<char> >::~basic_string()@plt>
  2f9a68:	lea    -0xf1(%rbp),%rax
  2f9a6f:	mov    %rax,%rdi
  2f9a72:	call   2bdd40 <std::allocator<char>::~allocator()@plt>
  2f9a77:	addl   $0x1,-0x14(%rbp)
  2f9a7b:	mov    -0x14(%rbp),%eax
  2f9a7e:	cmp    -0x54(%rbp),%eax
  2f9a81:	jl     2f99ea <t_sogouIme::UpdateUiInfo()@@Base+0x4c2>
