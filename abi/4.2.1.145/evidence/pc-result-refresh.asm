00000000000a2ff8 <t_keyboardManager::GetKeyboardState()@@Base+0x4790>:
   a2ff8:	push   %rbp
   a2ff9:	mov    %rsp,%rbp
   a2ffc:	push   %rbx
   a2ffd:	sub    $0x1a8,%rsp
   a3004:	mov    %rdi,-0x1a8(%rbp)
   a300b:	mov    -0x1a8(%rbp),%rax
   a3012:	add    $0x8,%rax
   a3016:	mov    %rax,%rdi
   a3019:	call   7cb70 <shm_unlink@plt+0x2b50>
   a301e:	mov    -0x1a8(%rbp),%rax
   a3025:	add    $0x18,%rax
   a3029:	mov    %rax,%rdi
   a302c:	call   7cb70 <shm_unlink@plt+0x2b50>
   a3031:	mov    -0x1a8(%rbp),%rax
   a3038:	add    $0x38,%rax
   a303c:	mov    %rax,%rdi
   a303f:	call   7cb70 <shm_unlink@plt+0x2b50>
   a3044:	mov    -0x1a8(%rbp),%rax
   a304b:	add    $0xf8,%rax
   a3051:	mov    %rax,%rdi
   a3054:	call   7cb70 <shm_unlink@plt+0x2b50>
   a3059:	mov    -0x1a8(%rbp),%rax
   a3060:	add    $0x48,%rax
   a3064:	mov    %rax,%rdi
   a3067:	call   7cb70 <shm_unlink@plt+0x2b50>
