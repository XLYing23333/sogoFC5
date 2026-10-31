0000000000426223 <GetSogouVoiceOperation@@Base+0x34ad6e>:
  426223:	mov    -0x48(%rbp),%rax
  426227:	mov    0x10(%rax),%rax
  42622b:	mov    %rax,%rdi
  42622e:	call   429992 <GetSogouVoiceOperation@@Base+0x34e4dd>
  426233:	not    %eax
  426235:	shr    $0x1f,%eax
  426238:	test   %al,%al
  42623a:	je     426267 <GetSogouVoiceOperation@@Base+0x34adb2>
  42623c:	mov    0x5e776d(%rip),%rax        # a0d9b0 <BOOL_PadNumAsSelect>
  426243:	mov    (%rax),%rdx
  426246:	mov    -0x48(%rbp),%rax
  42624a:	mov    0x20(%rax),%rax
  42624e:	mov    %rdx,%rsi
  426251:	mov    %rax,%rdi
  426254:	call   786b0 <t_env::GetValueBool(t_envEntry<bool> const*) const@plt>
  426259:	xor    $0x1,%eax
  42625c:	test   %al,%al
  42625e:	je     426267 <GetSogouVoiceOperation@@Base+0x34adb2>
  426260:	mov    $0x0,%eax
  426265:	jmp    42626c <GetSogouVoiceOperation@@Base+0x34adb7>
