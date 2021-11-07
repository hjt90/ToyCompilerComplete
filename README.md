

# ToyCompiler

## 项目简介

本项目是由[pamnso(JackyWu)](https://github.com/pamnso)和[hjt90](https://github.com/hjt90)共同合作完成的同济大学编译原理程的大作业，实现一个语法分析器，包括词法分析、语法分析。

程序可以生成词法分析部分或以json格式给出抽象语法树（AST）同时也支持输出 `dot` 格式的语法树结构文件。



## 演示

使用大作业的示例题目 `test.c` :

```c++
int a;
int b;
int program(int a, int b, int c)
{
	int i;
	int j;
	i = 0;
	if (a > (b + c))
	{
		j = a + (b * c + 1);
	}
	else
	{
		j = a;
	}
	while (i <= 100)
	{
		i = j * 2;
	}
	return i;
}

int demo(int a)
{
	a = a + 2;
	return a * 2;
}

void main(void)
{
	int a;
	int b;
	int c;
	a = 3;
	b = 4;
	c = 2;
	a = program(a, b, demo(c));
	return;
}
```



语法分析器的语法可以自行定义，遵循规范请参考附的报告：

```
$Start ::= <声明串>

<声明串> ::= <声明>
<声明串> ::= <声明串> <声明>

<声明> ::= $Int $ID <声明类型>
<声明> ::= $Void $ID <函数声明>

<声明类型> ::= <变量声明>
<声明类型> ::= <函数声明>

<变量声明> ::=  $Semi

<函数声明> ::= $LeftBracket <形参> $RightBracket <语句块>

<形参> ::= <参数列表>
<形参> ::= $Void

<参数列表> ::= <参数>
<参数列表> ::= <参数列表> $Comma <参数>

<参数> ::= $Int $ID

<语句块> ::= $LeftBrace <内部声明> <语句串> $RightBrace
<语句块> ::= $LeftBrace <语句串> $RightBrace

<内部声明> ::= <内部变量声明>
<内部声明> ::= <内部声明> <内部变量声明>

<内部变量声明> ::= $Int $ID $Semi

<语句串> ::= <语句>
<语句串> ::= <语句串> <语句>

<语句> ::= <if语句>
<语句> ::= <while语句>
<语句> ::= <return语句>
<语句> ::= <赋值语句>

<赋值语句> ::= $ID $Equal <表达式> $Semi

<return语句> ::= $Return $Semi
<return语句> ::= $Return <表达式> $Semi

<while语句> ::= $While $LeftBracket <表达式> $RightBracket <语句块>

<if语句> ::= $If $LeftBracket <表达式> $RightBracket <语句块>
<if语句> ::= $If $LeftBracket <表达式> $RightBracket <语句块> $Else <语句块>

<表达式> ::= <加法表达式>
<表达式> ::= <表达式> <比较运算符> <加法表达式>

<比较运算符> ::= $Smaller
<比较运算符> ::= $SmallerEqual
<比较运算符> ::= $Bigger
<比较运算符> ::= $BiggerEqual
<比较运算符> ::= $Equal2
<比较运算符> ::= $NotEqual

<加法表达式> ::= <项>
<加法表达式> ::= <加法表达式> $Plus <项>
<加法表达式> ::= <加法表达式> $Minus <项>

<项> ::= <因子>
<项> ::= <项> $Multiply <因子>
<项> ::= <项> $Divide <因子>

<因子> ::= $Number
<因子> ::= $LeftBracket <表达式> $RightBracket
<因子> ::= $ID <FTYPE>
<因子> ::= $ID

<FTYPE> ::= <call>

<call> ::= $LeftBracket <实参> $RightBracket
<call> ::= $LeftBracket $RightBracket

<实参> ::= <实参列表>

<实参列表> ::= <表达式>
<实参列表> ::= <实参列表> $Comma <表达式>
```



生成语法分析树：

![image-20211107162040750](D:\workspace\GitHub\ToyCompiler\README.assets\image-20211107162040750.png)



生成的 `json` 代码：

![image-20211107162124718](D:\workspace\GitHub\ToyCompiler\README.assets\image-20211107162124718.png)



根据 dot 生成图片:

![image-20211107162400063](D:\workspace\GitHub\ToyCompiler\README.assets\image-20211107162400063.png)

![test](D:\workspace\GitHub\ToyCompiler\README.assets\test.png)

## 编译

​	可以在 `Linux`、 `MacOS` 、`Windows` 下使用 `Makefile` 编译此程序。



## 注意事项

​	如果在转换图像时出现乱码等情况，请注意字符集的变化，并且由于 `Windows` 和 `Linux` 的文件换行会有差别，输入文件可能会有问题，建议使用 `dos2unix` 来解决该问题。
