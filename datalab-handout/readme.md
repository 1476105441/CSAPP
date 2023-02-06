# 实验说明

在bit.c中有若干个题目，每个题目上方都会有详细的要求。大部分题目都要求的是：只能使用位运算符，并且限制操作符个数，才能够获得题目的分数。



# BitXor

题目：只能使用~和&运算符实现异或操作，*Legal ops: ~ &*

~~~c
int bitXor(int x, int y) {
  return ~((~x) & (~y)) & ~(x & y);
}
~~~

想法：异或就是找出两个数中相同的位，(~x) & (~y)就是找出两个数中都为0的位，然后再取反，就是去掉这些位，剩下的就是除掉都为0的位之外的其他位，这其中还包含了另一种不符合异或要求的位，也就是都为1的位，所以后面一个表达式就是去掉都为1的位，然后两个再取交集。



# tmin

题目：找出二进制中最小的数，*Legal ops: ! ~ & ^ | + << >>*

~~~c
int tmin(void) {
  return 1 << 31;
}
~~~



# isTmax

题目：如果x是二进制中的最大值，则return 1，否则return 0，*Legal ops: ! ~ & ^ | +*

思考，int类型中最大的数是2147483647，二进制表示是011111111111111111111111111111111，十六进制表示是0x7fffffff，可以发现，在所有正数中，只有最大值加一之后符号位为1，可以通过这个特点区分正数的情况，然后再通过符号位区分负数即可。

1、区分正数和负数的情况，这一个解法是判断加一后符号位为1，然后全部取反后符号位也为1

~~~c
int isTmax(int x) {
  return ((((x+1) & (1 << 31)) & (~x) ) >> 31) & 1;
}
~~~

我这个解法独立测试（./btest）时是没有问题的，但是使用./driver.pl的时候就会错误。



2、复习时想到的另一种解法，但是也和上面的情况一样：

~~~c
int isTmax(int x) {
  int flag = !(x & (1 << 31));
  return flag & ((x+1) >> 31);
}
~~~



3、另一种解法，是可以通过的，主要思想是：只有（并不是）最大值取反后和加一后的值相同，相当于互为逆操作，所以只有最大值执行完第一个括号中的内容后值为0，但其实二进制全为1时，也是可以满足这个式子的，所以要再加上后面的判断。

~~~C
int isTmax(int x) {9
  return !(~(x+1) ^ x) & (!!(~x));
}
~~~

主要思想就是最大值的二进制表示为 0111,1111,1111,1111

可以发现，只有两个数字加一再取反 后和原值异或得到的值为0，即最大值和-1



# allOddBits

题目：如果所有的奇数位都为1，则return 1，否则return 0，*Legal ops: ! ~ & ^ | + << >>*

想法：设置一个mask，只有奇数位为1，然后使用这个mask和x进行异或，如果x的奇数位全为1的话，异或结果为0，否则异或结果非0，再将这个异或结果取反就可以得到题目答案了。

~~~c
int allOddBits(int x) {
  int y = (10 << 4) + 10,res = 0;
  y = (y << 8) + y;
  y = (y << 16) + y;
  res = x & y;
  return !(res ^ y);
}
~~~



# negate

题目：return -x

~~~c
int negate(int x) {
  return ~x + 1;
}
~~~



# isAsciiDigit

题目：如果是ASCII字符'0'-'9'，return 1，否则 return 0。

想法：观察满足条件的字符，不难发现，他们的二进制表示低8位中的前四位是0x30，而后四位除了8和9，其余的都是低于8的，所以可以先判断出来低于8的数，然后再特判出来8和9。

我这个解法也是，btest时可以通过，但driver时不能通过

~~~c
int isAsciiDigit(int x) {
  //如果x是小于8的，那么x&8得到的结果就是0，再取反就得到了1，后面的第一个条件是判断是否小于8。再接着的一个判断是过滤掉除了8和9的情况，因为只有8和9在中间两位上是为0的。
  return (!((x & (-1 << 4)) ^ (3 << 4))) & (!(x & 8) | (!(x & 6)));
}
~~~

改成这样就可以了：

~~~c
int isAsciiDigit(int x) {
  return (!((x >> 4) ^ 3)) & (!(x & 8) | (!(x & 6)));
}
~~~



# conditional

题目：如果x是不等于0的，返回y，反之如果x等于0，返回z

想法：对x取反，如果x为0，那么取反后结果为1，反之，取反后结果为0；使用这个取反结果做掩码，如果取反后为1，则返回z，否则返回y。

这个解法刚好16个运算符:

~~~c
int conditional(int x, int y, int z) {
  int a = !!x;
  a = (a << 1) + a;
  a = (a << 2) + a;
  a = (a << 4) + a;
  a = (a << 8) + a;
  a = (a << 16) + a;
  return (y & a) | (z & (~a));
}
~~~

15个运算符：

~~~c
int conditional(int x, int y, int z) {
  int a = !x;
  a = (a << 1) + a;
  a = (a << 2) + a;
  a = (a << 4) + a;
  a = (a << 8) + a;
  a = (a << 16) + a;
  return (y & (~a)) | (z & a);
}
~~~

7个运算符：

~~~c
int conditional(int x, int y, int z) {
  int a = !x;
  a = a << 31;
  a = a >> 31;
  return (y & (~a)) | (z & a);
}
~~~



# isLessOrEqual

题目：判断x和y的大小，如果x <= y，返回1，否则返回0

要点：补码减法

思想：判断是否相等，x和y是否同号，如果异号的话判断哪个数为负数，如果y为负数则返回0，同号的情况下再使用补码减法，然后判断结果的符号位是否为1。

((a ^ b) & a)：a是x的符号，b是y的符号，如果a和b相同的话，结果为0，也就是结果不从这条语句返回；如果不同的话，异或了之后和a相与，是为了排除x为正数，y为负数的情况，这种情况下也应该返回0。

~~~C
int isLessOrEqual(int x, int y) {
  int a = (x >> 31) & 1,b = (y >> 31) & 1,c = y + (~x + 1);
  return !(x ^ y) | ((a ^ b) & a) | (!(a ^ b) & !((c >> 31) & 1));
}
~~~



# logicalNeg

题目：实现非运算符（!），不能使用!运算符。

要点：0和非0的数字有什么不同？

思想：非0的数都有一个相反数，相反数的符号和原数符号不同，但有一个例外，那就是-2147483648，它取相反数之后符号还是相同的，不过符号却是1，利用这点，找出0和非0数。

~~~C
int logicalNeg(int x) {
  int a = 1 << 31,b = (x & a) >> 31,c = ((~x + 1) & a) >> 31,d = b ^ c;
  //这一步要用d&1是为了去掉前面多余的1
  return ((d & 1) ^ 1) & (b ^ 1);
}
~~~



# howManyBits

题目：返回能够代表x的最少的位数量，比如能代表5的位的数量为4，因为5的二进制表示为0101，真值其实只有3位，还有一位是符号位。

想法：

​	首先，分成几种情况，为0、正数、负数，其中，如果是负数的话，我们需要统一转换成正数来处理（位转换，统计的信息是一样的）。然后，类似于分治的思想，首先考虑前16位有没有数字，然后根据结果选择是舍弃后16位还是前16位。然后再考虑前8位，以此类推。

~~~c
int howManyBits(int x) {
  int isZero = !x,bit_16,bit_8,bit_4,bit_2,bit_1,bit_0,flag = x >> 31,mask = ((!isZero) << 31) >> 31;
  x = ((~flag) & x) | (flag & (~x));
  bit_16 = !((!!(x >> 16)) ^ 0x1) << 4;
  x = x >> bit_16;
  bit_8 = !((!!(x >> 8)) ^ 0x1) << 3;
  x = x >> bit_8;
  bit_4 = !((!!(x >> 4)) ^ 0x1) << 2;
  x = x >> bit_4;
  bit_2 = !((!!(x >> 2)) ^ 0x1) << 1;
  x = x >> bit_2;
  bit_1 = !((!!(x >> 1)) ^ 0x1);
  x = x >> bit_1;
  bit_0 = x;
  return isZero | (mask & (bit_16+bit_8+bit_4+bit_2+bit_1+bit_0+1));
}
~~~

重新写一遍，其实并不需要特判出0，因为0的计算也是符合公式的：

~~~C
int howManyBits(int x) {
    //其实并不需要特别判断出来是否为0
    int bit16,bit8,bit4,bit2,bit1,bit0;
    bit16 = (!!(x >> 16)) << 4;
    x = x >> bit16;
    bit8 = (!!(x >> 8)) << 3;
    x = x >> bit8;
    bit4 = (!!(x >> 4)) << 2;
    x = x >> bit4;
    bit2 = (!!(x >> 2)) << 1;
    x = x >> bit2;
    bit1 = !!(x >> 1);
    x = x >> bit1;
    bit0 = x;
    //还需要加上一个符号位，0也是需要加这个符号位，所以这个式子其实是通用的
    return bit16+bit8+bit4+bit2+bit1+bit0+1;
}
~~~



# floatScale2

题目：返回符合和表达式2*f一样的二进制表示，乘法需要按照float类型进行乘法。

想法：此题目需要根据浮点数的格式，分情况进行返回值。如果是无穷大或者NaN，直接返回即可；如果是非规格化的浮点数，则进行右移；如果是规格化的浮点数，则将浮点数的阶码加一即可。

~~~C
unsigned floatScale2(unsigned uf) {
  int mask = 255 << 23,val = ((mask & uf) >> 23);
  if(val == 0xff)
    return uf;
  if(val == 0)
    return (uf << 1) | (uf & 0x80000000u);
  
  val = (val+1) << 23;
  uf = (uf & (~mask)) | val;
  return uf;
}
~~~



# floatFloat2Int

题目：将二进制表示f看作是浮点数，将浮点数转换成对应格式表示的int类型，其中NaN和无穷值返回*0x80000000u*。

想法：通过阶码判断是否是NaN或无穷大、数值是否过小/过大，然后循环把数值部分的数取出来（有多少阶码就循环几次），最后还要对符号进行特殊处理。

~~~c
int floatFloat2Int(unsigned uf) {
  int mask = 255 << 23,val = ((mask & uf) >> 23),res = 1,pos = 1 << 22,flag;
  if(val == 0xff)
    return 0x80000000u;
  
  val -= 127;
  if(val < 0)
    return 0;
  if(val > 31)
    return 0x80000000u;
  
  flag = !!(uf >> 31);
  while(val > 0){
    res = (res << 1) + (uf & pos);
    pos = pos >> 1;
    val--;
  }
  if(flag == 1)
    return (~res)+1;
  return res;
}
~~~





# floatPower2

题目：返回2的x次方的二进制表示，如果数值过小则返回0，数值过大则返回+INF。

思想：考虑到浮点数的格式（IEEE754标准），只需要对阶码进行处理即可。

~~~C
unsigned floatPower2(int x) {
    unsigned res;
    if(x > 128)
      return 0x7f800000;
    if(x < -127)
      return 0;
    res = (x + 127) << 23;
    return res;
}
~~~



# 通关

![image-20221211220234171](https://gitee.com/wang-junshen/csapp/raw/master/datalab-handout/题解.assets/image-20221211220234171.png)

