$Start ::= <声明串> $End

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

<内部声明> ::= $Empty
<内部声明> ::= <内部变量声明>
<内部声明> ::= <内部声明> $Semi <内部变量声明>

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

<FTYPE> ::= <call>
<FTYPE> ::= $Empty

<call> ::= $LeftBracket <实参> $RightBracket

<实参> ::= <实参列表>
<实参> ::= $Empty

<实参列表> ::= <表达式>
<实参列表> ::= <实参列表> $Comma <表达式>