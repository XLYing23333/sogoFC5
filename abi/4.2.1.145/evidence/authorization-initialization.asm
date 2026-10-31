000000000037acf2 <n_sgAuth::t_stAuthEnv::Init()@@Base>:
  37acf2:	push   %rbp
  37acf3:	mov    %rsp,%rbp
  37acf6:	push   %rbx
  37acf7:	sub    $0x18,%rsp
  37acfb:	mov    %rdi,-0x18(%rbp)
  37acff:	call   2e9f20 <PathManager_GetUserProfilePath@plt>
  37ad04:	mov    %rax,%rbx
  37ad07:	call   2be190 <PathManager_GetWorkspace@plt>
  37ad0c:	mov    %rax,%rcx
  37ad0f:	mov    -0x18(%rbp),%rax
  37ad13:	mov    %rbx,%rdx
  37ad16:	mov    %rcx,%rsi
  37ad19:	mov    %rax,%rdi
  37ad1c:	call   2c9820 <n_sgAuth::t_stAuthEnv::Init(char const*, char const*)@plt>
  37ad21:	add    $0x18,%rsp
  37ad25:	pop    %rbx
  37ad26:	pop    %rbp
  37ad27:	ret
