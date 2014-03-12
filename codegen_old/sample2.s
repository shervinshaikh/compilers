
sample:     file format elf32-i386


Disassembly of section .init:

080482b4 <_init>:
 80482b4:	53                   	push   %ebx
 80482b5:	83 ec 08             	sub    $0x8,%esp
 80482b8:	e8 93 00 00 00       	call   8048350 <__x86.get_pc_thunk.bx>
 80482bd:	81 c3 43 1d 00 00    	add    $0x1d43,%ebx
 80482c3:	8b 83 fc ff ff ff    	mov    -0x4(%ebx),%eax
 80482c9:	85 c0                	test   %eax,%eax
 80482cb:	74 05                	je     80482d2 <_init+0x1e>
 80482cd:	e8 2e 00 00 00       	call   8048300 <__gmon_start__@plt>
 80482d2:	83 c4 08             	add    $0x8,%esp
 80482d5:	5b                   	pop    %ebx
 80482d6:	c3                   	ret    

Disassembly of section .plt:

080482e0 <printf@plt-0x10>:
 80482e0:	ff 35 04 a0 04 08    	pushl  0x804a004
 80482e6:	ff 25 08 a0 04 08    	jmp    *0x804a008
 80482ec:	00 00                	add    %al,(%eax)
	...

080482f0 <printf@plt>:
 80482f0:	ff 25 0c a0 04 08    	jmp    *0x804a00c
 80482f6:	68 00 00 00 00       	push   $0x0
 80482fb:	e9 e0 ff ff ff       	jmp    80482e0 <_init+0x2c>

08048300 <__gmon_start__@plt>:
 8048300:	ff 25 10 a0 04 08    	jmp    *0x804a010
 8048306:	68 08 00 00 00       	push   $0x8
 804830b:	e9 d0 ff ff ff       	jmp    80482e0 <_init+0x2c>

08048310 <__libc_start_main@plt>:
 8048310:	ff 25 14 a0 04 08    	jmp    *0x804a014
 8048316:	68 10 00 00 00       	push   $0x10
 804831b:	e9 c0 ff ff ff       	jmp    80482e0 <_init+0x2c>

Disassembly of section .text:

08048320 <_start>:
 8048320:	31 ed                	xor    %ebp,%ebp
 8048322:	5e                   	pop    %esi
 8048323:	89 e1                	mov    %esp,%ecx
 8048325:	83 e4 f0             	and    $0xfffffff0,%esp
 8048328:	50                   	push   %eax
 8048329:	54                   	push   %esp
 804832a:	52                   	push   %edx
 804832b:	68 d0 84 04 08       	push   $0x80484d0
 8048330:	68 60 84 04 08       	push   $0x8048460
 8048335:	51                   	push   %ecx
 8048336:	56                   	push   %esi
 8048337:	68 1c 84 04 08       	push   $0x804841c
 804833c:	e8 cf ff ff ff       	call   8048310 <__libc_start_main@plt>
 8048341:	f4                   	hlt    
 8048342:	66 90                	xchg   %ax,%ax
 8048344:	66 90                	xchg   %ax,%ax
 8048346:	66 90                	xchg   %ax,%ax
 8048348:	66 90                	xchg   %ax,%ax
 804834a:	66 90                	xchg   %ax,%ax
 804834c:	66 90                	xchg   %ax,%ax
 804834e:	66 90                	xchg   %ax,%ax

08048350 <__x86.get_pc_thunk.bx>:
 8048350:	8b 1c 24             	mov    (%esp),%ebx
 8048353:	c3                   	ret    
 8048354:	66 90                	xchg   %ax,%ax
 8048356:	66 90                	xchg   %ax,%ax
 8048358:	66 90                	xchg   %ax,%ax
 804835a:	66 90                	xchg   %ax,%ax
 804835c:	66 90                	xchg   %ax,%ax
 804835e:	66 90                	xchg   %ax,%ax

08048360 <deregister_tm_clones>:
 8048360:	b8 1f a0 04 08       	mov    $0x804a01f,%eax
 8048365:	2d 1c a0 04 08       	sub    $0x804a01c,%eax
 804836a:	83 f8 06             	cmp    $0x6,%eax
 804836d:	77 02                	ja     8048371 <deregister_tm_clones+0x11>
 804836f:	f3 c3                	repz ret 
 8048371:	b8 00 00 00 00       	mov    $0x0,%eax
 8048376:	85 c0                	test   %eax,%eax
 8048378:	74 f5                	je     804836f <deregister_tm_clones+0xf>
 804837a:	55                   	push   %ebp
 804837b:	89 e5                	mov    %esp,%ebp
 804837d:	83 ec 18             	sub    $0x18,%esp
 8048380:	c7 04 24 1c a0 04 08 	movl   $0x804a01c,(%esp)
 8048387:	ff d0                	call   *%eax
 8048389:	c9                   	leave  
 804838a:	c3                   	ret    
 804838b:	90                   	nop
 804838c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

08048390 <register_tm_clones>:
 8048390:	b8 1c a0 04 08       	mov    $0x804a01c,%eax
 8048395:	2d 1c a0 04 08       	sub    $0x804a01c,%eax
 804839a:	c1 f8 02             	sar    $0x2,%eax
 804839d:	89 c2                	mov    %eax,%edx
 804839f:	c1 ea 1f             	shr    $0x1f,%edx
 80483a2:	01 d0                	add    %edx,%eax
 80483a4:	d1 f8                	sar    %eax
 80483a6:	75 02                	jne    80483aa <register_tm_clones+0x1a>
 80483a8:	f3 c3                	repz ret 
 80483aa:	ba 00 00 00 00       	mov    $0x0,%edx
 80483af:	85 d2                	test   %edx,%edx
 80483b1:	74 f5                	je     80483a8 <register_tm_clones+0x18>
 80483b3:	55                   	push   %ebp
 80483b4:	89 e5                	mov    %esp,%ebp
 80483b6:	83 ec 18             	sub    $0x18,%esp
 80483b9:	89 44 24 04          	mov    %eax,0x4(%esp)
 80483bd:	c7 04 24 1c a0 04 08 	movl   $0x804a01c,(%esp)
 80483c4:	ff d2                	call   *%edx
 80483c6:	c9                   	leave  
 80483c7:	c3                   	ret    
 80483c8:	90                   	nop
 80483c9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

080483d0 <__do_global_dtors_aux>:
 80483d0:	80 3d 1c a0 04 08 00 	cmpb   $0x0,0x804a01c
 80483d7:	75 13                	jne    80483ec <__do_global_dtors_aux+0x1c>
 80483d9:	55                   	push   %ebp
 80483da:	89 e5                	mov    %esp,%ebp
 80483dc:	83 ec 08             	sub    $0x8,%esp
 80483df:	e8 7c ff ff ff       	call   8048360 <deregister_tm_clones>
 80483e4:	c6 05 1c a0 04 08 01 	movb   $0x1,0x804a01c
 80483eb:	c9                   	leave  
 80483ec:	f3 c3                	repz ret 
 80483ee:	66 90                	xchg   %ax,%ax

080483f0 <frame_dummy>:
 80483f0:	a1 10 9f 04 08       	mov    0x8049f10,%eax
 80483f5:	85 c0                	test   %eax,%eax
 80483f7:	74 1e                	je     8048417 <frame_dummy+0x27>
 80483f9:	b8 00 00 00 00       	mov    $0x0,%eax
 80483fe:	85 c0                	test   %eax,%eax
 8048400:	74 15                	je     8048417 <frame_dummy+0x27>
 8048402:	55                   	push   %ebp
 8048403:	89 e5                	mov    %esp,%ebp
 8048405:	83 ec 18             	sub    $0x18,%esp
 8048408:	c7 04 24 10 9f 04 08 	movl   $0x8049f10,(%esp)
 804840f:	ff d0                	call   *%eax
 8048411:	c9                   	leave  
 8048412:	e9 79 ff ff ff       	jmp    8048390 <register_tm_clones>
 8048417:	e9 74 ff ff ff       	jmp    8048390 <register_tm_clones>

0804841c <main>:
 804841c:	55                   	push   %ebp
 804841d:	89 e5                	mov    %esp,%ebp
 804841f:	83 e4 f0             	and    $0xfffffff0,%esp
 8048422:	83 ec 10             	sub    $0x10,%esp
 8048425:	e8 1a 00 00 00       	call   8048444 <Main>
 804842a:	89 44 24 04          	mov    %eax,0x4(%esp)
 804842e:	c7 04 24 f4 84 04 08 	movl   $0x80484f4,(%esp)
 8048435:	e8 b6 fe ff ff       	call   80482f0 <printf@plt>
 804843a:	b8 00 00 00 00       	mov    $0x0,%eax
 804843f:	c9                   	leave  
 8048440:	c3                   	ret    
 8048441:	66 90                	xchg   %ax,%ax
 8048443:	90                   	nop

08048444 <Main>:
 8048444:	55                   	push   %ebp
 8048445:	89 e5                	mov    %esp,%ebp
 8048447:	83 ec 0c             	sub    $0xc,%esp
 804844a:	6a 0a                	push   $0xa
 804844c:	6a 05                	push   $0x5
 804844e:	5b                   	pop    %ebx
 804844f:	5a                   	pop    %edx
 8048450:	f7 fb                	idiv   %ebx
 8048452:	50                   	push   %eax
 8048453:	58                   	pop    %eax
 8048454:	89 45 fc             	mov    %eax,-0x4(%ebp)
 8048457:	ff 75 fc             	pushl  -0x4(%ebp)
 804845a:	58                   	pop    %eax
 804845b:	89 ec                	mov    %ebp,%esp
 804845d:	5d                   	pop    %ebp
 804845e:	c3                   	ret    
 804845f:	90                   	nop

08048460 <__libc_csu_init>:
 8048460:	55                   	push   %ebp
 8048461:	57                   	push   %edi
 8048462:	31 ff                	xor    %edi,%edi
 8048464:	56                   	push   %esi
 8048465:	53                   	push   %ebx
 8048466:	e8 e5 fe ff ff       	call   8048350 <__x86.get_pc_thunk.bx>
 804846b:	81 c3 95 1b 00 00    	add    $0x1b95,%ebx
 8048471:	83 ec 1c             	sub    $0x1c,%esp
 8048474:	8b 6c 24 30          	mov    0x30(%esp),%ebp
 8048478:	8d b3 0c ff ff ff    	lea    -0xf4(%ebx),%esi
 804847e:	e8 31 fe ff ff       	call   80482b4 <_init>
 8048483:	8d 83 08 ff ff ff    	lea    -0xf8(%ebx),%eax
 8048489:	29 c6                	sub    %eax,%esi
 804848b:	c1 fe 02             	sar    $0x2,%esi
 804848e:	85 f6                	test   %esi,%esi
 8048490:	74 27                	je     80484b9 <__libc_csu_init+0x59>
 8048492:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
 8048498:	8b 44 24 38          	mov    0x38(%esp),%eax
 804849c:	89 2c 24             	mov    %ebp,(%esp)
 804849f:	89 44 24 08          	mov    %eax,0x8(%esp)
 80484a3:	8b 44 24 34          	mov    0x34(%esp),%eax
 80484a7:	89 44 24 04          	mov    %eax,0x4(%esp)
 80484ab:	ff 94 bb 08 ff ff ff 	call   *-0xf8(%ebx,%edi,4)
 80484b2:	83 c7 01             	add    $0x1,%edi
 80484b5:	39 f7                	cmp    %esi,%edi
 80484b7:	75 df                	jne    8048498 <__libc_csu_init+0x38>
 80484b9:	83 c4 1c             	add    $0x1c,%esp
 80484bc:	5b                   	pop    %ebx
 80484bd:	5e                   	pop    %esi
 80484be:	5f                   	pop    %edi
 80484bf:	5d                   	pop    %ebp
 80484c0:	c3                   	ret    
 80484c1:	eb 0d                	jmp    80484d0 <__libc_csu_fini>
 80484c3:	90                   	nop
 80484c4:	90                   	nop
 80484c5:	90                   	nop
 80484c6:	90                   	nop
 80484c7:	90                   	nop
 80484c8:	90                   	nop
 80484c9:	90                   	nop
 80484ca:	90                   	nop
 80484cb:	90                   	nop
 80484cc:	90                   	nop
 80484cd:	90                   	nop
 80484ce:	90                   	nop
 80484cf:	90                   	nop

080484d0 <__libc_csu_fini>:
 80484d0:	f3 c3                	repz ret 
 80484d2:	66 90                	xchg   %ax,%ax

Disassembly of section .fini:

080484d4 <_fini>:
 80484d4:	53                   	push   %ebx
 80484d5:	83 ec 08             	sub    $0x8,%esp
 80484d8:	e8 73 fe ff ff       	call   8048350 <__x86.get_pc_thunk.bx>
 80484dd:	81 c3 23 1b 00 00    	add    $0x1b23,%ebx
 80484e3:	83 c4 08             	add    $0x8,%esp
 80484e6:	5b                   	pop    %ebx
 80484e7:	c3                   	ret    
