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

最终结果：

![Snipaste_2023-02-16_21-23-41](D:\Project\project_revelant\CSAPP\target1\readme.assets\Snipaste_2023-02-16_21-23-41.png)
