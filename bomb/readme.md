# 工具的使用

## GDB

是一个gnu的调试器，可以用来一行一行的追踪程序、检查内存和寄存器、既可以看源码也可以看汇编代码、设置断点、设置内存查看点、写脚本。



## objdump -t

可以输出代码的符号表



## objdump -d

可以将程序反汇编，虽然这个指令可以提供很多信息，但是它不会告诉你故事的全部内容，有关于系统调用的反汇编展示出来的信息是神秘的。





# 解题思路

## bomb1

1、使用gdb反汇编phase_1，可以看到关键信息

![image-20230111222252901](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111222252901.png)

首先，将栈指针减8字节，开辟出一段栈空间（另作他用？），然后将立即数 0x402400 存入到寄存器 %esi，这个寄存器是用于存放第二个参数的，也就是说立即数 0x402400是接下来调用函数的第二个参数，再接着，调用strings_not_equal函数，顾名思义，此函数是用来判断两个字符串是否相等的，那么在这里，我们可以猜测输入的是一个字符串。

接下来，我们将进入strings_not_equal函数，寻找其他线索。



2、strings_not_equal函数反汇编代码：

![image-20230111224323115](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111224323115.png)

![image-20230111224344954](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111224344954.png)

可以看到，这个函数的汇编代码很多，但是先别慌，我们慢慢来分析。首先，上来先push了一系列寄存器，这是为了保存调用此函数之前寄存器的状态，因为这些寄存器有可能被修改。然后，将两个传参使用的寄存器%rdi和%rsi也保存到两个专用的寄存器中，再接着进入了string_length函数中。



3、string_length函数反汇编代码：

![image-20230111224511021](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111224511021.png)

进入此函数时，%rdi寄存器的值没有被修改，所以此处存放的还是一开始phase_1调用strings_not_equal传入的第一个参数。那么开始执行这个函数，首先，比较0和***内存地址%rdi处一个字节的值***，如果相等则跳转到倒数第二行代码，否则继续执行。由此可以看出，%rdi存储的是一个地址值，并且是一个字符串（字符数组）的起始地址，这个函数所做的行为就是计算字符串的长度，比较每一个字符是否为0，8-14为循环代码，14-16为比较判断代码。



解析完了string_length函数，我们回到strings_not_equal函数继续执行：

![image-20230111234321334](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111234321334.png)

可以看到，接下来做的操作是将结果保存到%r12d寄存器中，然后将%rbp（即%rsi）的值赋值给%rdi，接着再次调用string_length函数。这说明了什么？显然，说明了第二个参数也是一个内存地址，且是字符串的起始地址，而追溯其由来，是phase1中传递给%rsi寄存器的0x402400。

![image-20230111234648990](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111234648990.png)

说明了在这个内存地址处，存放着一个字符串，这个字符串就是第一个炸弹的钥匙。



4、分析到此处，我们已经知道了最关键的信息，答案就在内存地址0x402400处，那么我们怎么要得到这个答案呢？当然做法有很多种，这里我使用了gdb 指令 x/s：

![image-20230111235049433](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230111235049433.png)

这条指令的作用是将内存地址0x402400处的内容以字符串形式打印出来。于是我们就得到了第一个模块的答案。





## bomb2

1、将phase_2反汇编

![Snipaste_2023-01-28_20-52-06](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/Snipaste_2023-01-28_20-52-06.png)

​	上来进行的一系列操作是将寄存器的值保存起来，然后将栈指针下移40字节，然后将当前的栈顶寄存器的值赋给%rsi寄存器再传递到read_six_numbers函数中，所以接下来进入read_six_numbers一探究竟。



2、将read_six_numbers函数反汇编：

![image-20230129150506361](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230129150506361-16750473334408.png)

​	通过这个函数名称我们可以猜测这个函数就是解析用户传入参数的，接下来我们一步一步进行验证。首先，一进来就将当前的栈寄存器减了24（为了防止栈缓冲区溢出），然后将%rsi寄存器的值传入%rdx中，%rsi中的值是在phase_2函数中设置的，我们先将它设为y，再接着将%rsi寄存器的值+4赋给%rcx，也就是将y+4存入%rcx中。然后将y+20存入%rax，再将%rax的值存入到当前栈指针地址+8的位置。类似的操作再执行一次，将y+16存入%rax，然后再存到当前栈顶位置。再接下来两个操作分别是将y+12、y+8存入到寄存器%r9、%r8中。可能到这里会让人有些懵，我们先整理一下程序执行到此处时栈内存和寄存器的情况：

![image-20230129151030498](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230129151030498-167504734086611.png)

​	可以看到，这些寄存器和栈内存中存放的都是地址值，并且是相隔4字节的地址值，一共有6个地址值，并且每个相隔是4字节，由此可以猜测，每个地址存放一个int类型的数字，有没有可能，是要将用户输入的6个数字分别解析然后存入到这6个地址处？接下来证实这个猜想：

![image-20230129151517063](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230129151517063-167504734787314.png)

​	可以看到，这些寄存器都是用来存放参数的，而前两个寄存器肯定是要用来存放其他参数的，所以还有两个参数就放到栈内存中了。

​	继续往下走：

![Snipaste_2023-01-29_15-21-26](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/Snipaste_2023-01-29_15-21-26-167504735468817.png)

​	接下来程序将一个地址存入了%esi 也就是%rsi寄存器的低32位，作为第二个参数，然后清空%eax寄存器，调用sscanf函数，这个函数相信很多人都不陌生，我们可以看一下0x4025c3处的内容：

![image-20230129152509813](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230129152509813-167504736102820.png)

​	很明显，这是一个字符格式，表示6个数字，注意，用于传递第一个参数的寄存器%rdi没有更改过，所以是进入phase_2时传入的参数，也就是用户输入的内容，到这里，调用sscanf的所有参数都齐了。

​	分析到这里，我们可以得出一些关键信息，将有助于后续解开这道题目：用户应该在第二个模块中输入6个数字，然后phase_2调用read_six_numbers函数将输入解析为6个数字，然后写到栈内存中y的位置（还记得前面那张图吗？），具体是以y，y+4，y+8，y+12，y+16，y+20分别为起始位置存放6个数字。接下来的代码就是判断用户输入的数字是否大于5个，然后返回。



3、从read_six_numbers函数中返回，回到phase_2中继续执行

![Snipaste_2023-01-30_14-49-20](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/Snipaste_2023-01-30_14-49-20.png)

​	一回来的第一个操作就是一条cmpl指令，判断当前栈顶的值是否为1，如果为1，则跳转到箭头所指位置执行。那么当前栈顶的值是在什么时候设置的呢？其实当前栈顶的地址值就是y，也就是在read_six_numbers函数中调用sscanf的时候传递的6个地址参数中的第一个，也就是说，当前栈顶存放的是用户输入的第一个数字（没搞懂的话可以看看上一个步骤），其实也就是判断用户输入的第一个参数是否为1，那么我们就可以确定第一个数字要输入1。

​	接着往下走，将%rsp + 4 存入到%rbx中，将%rsp + 24 存入到%rbp中，然后又跳转到上面的代码中，从(%rbx-4)内存地址处取出数据存入%eax中，(%rbx-4) 也就是 %rsp，也就是取出栈顶位置存储的值，此时为1，即将1存入%eax中。然后再将%eax的值翻倍，再和%rbx内存地址处的值比较，如果不相等，炸弹爆炸，说明第二个参数的值为2，是前一个参数值的2倍。再接着往下执行，将%rbx+4，和%rbp进行比较，如果不相等，跳转到上面执行，可见这是一个循环，而%rbp存储的值就是循环结束的地址值，往后的参数也和第二个参数计算方式一样，为前一个参数的2倍。至此，我们得到答案为：1 2 4 8 16 32

![image-20230130174217221](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230130174217221.png)





## bomb3

1、反汇编phase_3：

![Snipaste_2023-01-30_17-47-21](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/Snipaste_2023-01-30_17-47-21.png)

​	一开始，设当前栈顶指针为y，将 y+12 和 y+8 分别存入到 %rcx 和 %rdx中，肯定也是作为参数使用的，再接着往下看，又将0x4025cf存入 %esi 中，再将 %eax 寄存器清空，最后调用了 sscanf函数，这样的话，我们就明白了，和bomb2中一样，%rcx 和 %rdx中的值是sscanf解析字符串后的结果的存放位置。我们可以看看0x4025cf中存放的内容：

![image-20230130201910221](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230130201910221.png)

​	说明我们应该输入两个数字。接着往下执行，从ssanf调用回来后，先是校验入参的个数是否合法，然后再比较当前 y（栈顶指针） + 8 位置存储的值和7，如果大于7的话，炸弹就爆炸，所以我们可以知道，输入的第一个参数应该是小于7的，并且由于比较指令使用的是无符号数的比较指令，所以第一个参数的值也要大于等于0。

​	再接着往下：

![image-20230130203825831](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230130203825831.png)

​	这里将第一个参数值存入到 %eax 中，然后跳转到另一个地址去执行，通过这个跳转方式可以推测 0x402470 是一个函数表的首地址，根据第一个参数传递的值的不同，跳转到不同的位置执行。那么现在我们怎么知道会跳转到哪里呢？我的方法是设置断点，然后单步执行程序：

![image-20230130204846492](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230130204846492.png)

​	最终会来到phase_3函数中的这里执行，可以看到这里会将 0x137 和 y + 12 进行比较，如果不相等，炸弹爆炸。所以我们就得到了bomb3的其中一个答案 ：1 311

![image-20230130205410669](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230130205410669.png)

​	以此类推，这个炸弹共有8个解决方案，感兴趣的可以按照这个方法找出其他参数对应的答案。



## bomb4

1、反汇编phase_4：

![image-20230201205628532](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230201205628532.png)

​	和前几题一样，又是熟悉的sscanf，而且调用之前设置了两个内存地址作为参数，盲猜此步骤需要输入两个整数：

![image-20230201212423652](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230201212423652.png)

​	果然是这样，继续往下走，做了校验参数的个数，如果不为2则炸弹爆炸。接着再比较第一个参数和14的大小，如果大于14，则直接引爆炸弹。接着，将14赋值给%edx，0赋值给%esi，第一个参数值赋值给%edi，然后调用func4。



2、反汇编func4：

![image-20230201214529650](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230201214529650.png)

​	整个题目的精髓就在这里，由于描述起来比较复杂，我就直接把我观察出来的结论说出来：这是一个二分查找的汇编代码！%rdx中存放的是二分查找右边界的值，%rsi中存放的是左边界的值，%rdi存放的是第一个参数（设为x）的值，这个二分查找算法就是从右边界r = 14，左边界 l = 0 开始，查找x的值。认真阅读代码就能发现，只有当x = 1、3、7的时候返回值为0。



3、回到phase_4：

![image-20230201215552909](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230201215552909.png)

​	可以看到，从func4返回后，先测试%eax是否为0，不为0则炸弹爆炸，所以可以得到第一个参数的值为1、3、7。接着，将第二个参数和0比较，不相等则炸弹爆炸，所以第二个参数值为0。



## bomb5

1、反汇编phase_5：

![image-20230201223747350](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230201223747350.png)



最终结果：

![image-20230201233450769](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230201233450769.png)





## bomb6

~~~c
Dump of assembler code for function phase_6:
   0x00000000004010f4 <+0>:		push   %r14
   0x00000000004010f6 <+2>:		push   %r13
   0x00000000004010f8 <+4>:		push   %r12
   0x00000000004010fa <+6>:		push   %rbp
   0x00000000004010fb <+7>:		push   %rbx
   0x00000000004010fc <+8>:		sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13
   0x0000000000401103 <+15>:	mov    %rsp,%rsi
   0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
   0x000000000040110b <+23>:	mov    %rsp,%r14
   0x000000000040110e <+26>:	mov    $0x0,%r12d
       
   //外层循环    
   0x0000000000401114 <+32>:	mov    %r13,%rbp
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
   0x000000000040111b <+39>:	sub    $0x1,%eax
   0x000000000040111e <+42>:	cmp    $0x5,%eax
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
   0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
   0x0000000000401128 <+52>:	add    $0x1,%r12d
   0x000000000040112c <+56>:	cmp    $0x6,%r12d
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>
   0x0000000000401132 <+62>:	mov    %r12d,%ebx
       
   //内循环
   //判断后面元素是否小于前面元素
   0x0000000000401135 <+65>:	movslq %ebx,%rax
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
   0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb>
   0x0000000000401145 <+81>:	add    $0x1,%ebx
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>
   //内循环结束
       
   0x000000000040114d <+89>:	add    $0x4,%r13
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
   //外层循环结束
      
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
   0x0000000000401158 <+100>:	mov    %r14,%rax
   0x000000000040115b <+103>:	mov    $0x7,%ecx
   //循环开始    
   0x0000000000401160 <+108>:	mov    %ecx,%edx
   0x0000000000401162 <+110>:	sub    (%rax),%edx
   0x0000000000401164 <+112>:	mov    %edx,(%rax)
   0x0000000000401166 <+114>:	add    $0x4,%rax
   0x000000000040116a <+118>:	cmp    %rsi,%rax
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
   //循环结束
       
   0x000000000040116f <+123>:	mov    $0x0,%esi
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   
   //小循环开始    
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
   0x000000000040117a <+134>:	add    $0x1,%eax
   0x000000000040117d <+137>:	cmp    %ecx,%eax
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
   //小循环结束
       
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)
   0x000000000040118d <+153>:	add    $0x4,%rsi
   0x0000000000401191 <+157>:	cmp    $0x18,%rsi
   0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
   0x000000000040119a <+166>:	cmp    $0x1,%ecx
   0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>
   0x000000000040119f <+171>:	mov    $0x1,%eax
   0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx
   0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
       
   0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx
   0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax
   0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi
   0x00000000004011ba <+198>:	mov    %rbx,%rcx
   0x00000000004011bd <+201>:	mov    (%rax),%rdx
   0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)
   0x00000000004011c4 <+208>:	add    $0x8,%rax
   0x00000000004011c8 <+212>:	cmp    %rsi,%rax
   0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:	mov    %rdx,%rcx
   0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
       
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)
   0x00000000004011da <+230>:	mov    $0x5,%ebp
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:	mov    (%rax),%eax
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:	callq  0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	retq 
~~~

![Snipaste_2023-02-04_00-21-58](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/Snipaste_2023-02-04_00-21-58.png)



答案为：4 3 2 1 6 5

![image-20230206235420600](https://gitee.com/wang-junshen/csapp/raw/master/bomb/readme.assets/image-20230206235420600.png)
