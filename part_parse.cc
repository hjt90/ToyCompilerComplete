$Start ::= <N> <声明串>

<A> ::= $Empty
<N> ::= $Empty
<M> ::= $Empty

<声明串> ::= <声明>
<声明串> ::= <声明串> <声明>

<声明> ::= $Int $ID <声明类型>
<声明> ::= $Void $ID <M> <A> <函数声明>
<声明> ::= $Int $ID <M> <A> <函数声明>

<声明类型> ::=  $Semi

<函数声明> ::= $LeftBracket <形参> $RightBracket <语句块>

<形参> ::= <参数列表>
<形参> ::= $Void

<参数列表> ::= <参数>
<参数列表> ::= <参数> $Comma <参数列表>

<参数> ::= $Int $ID

<语句块> ::= $LeftBrace <内部声明> <语句串> $RightBrace

<内部声明> ::= $Empty
<内部声明> ::= <内部变量声明> <内部声明>

<内部变量声明> ::= $Int $ID $Semi

<语句串> ::= <语句>
<语句串> ::= <语句> <M> <语句串>

<语句> ::= <if语句>
<语句> ::= <while语句>
<语句> ::= <return语句>
<语句> ::= <assign语句>

<assign语句> ::= $ID $Equal <表达式> $Semi

<return语句> ::= $Return $Semi
<return语句> ::= $Return <表达式> $Semi

<while语句> ::= $While <M> $LeftBracket <表达式> $RightBracket <A> <语句块>

<if语句> ::= $If $LeftBracket <表达式> $RightBracket <A> <语句块>
<if语句> ::= $If $LeftBracket <表达式> $RightBracket <A> <语句块> <N> $Else <M> <A> <语句块>

<表达式> ::= <加法表达式>
<表达式> ::= <表达式> <比较运算符> <加法表达式>

<比较运算符> ::= $Smaller
<比较运算符> ::= $SmallerEqual
<比较运算符> ::= $Bigger
<比较运算符> ::= $BiggerEqual
<比较运算符> ::= $Equal2
<比较运算符> ::= $NotEqual

<加法表达式> ::= <项>
<加法表达式> ::= <项> $Plus <加法表达式>
<加法表达式> ::= <项> $Minus <加法表达式>

<项> ::= <因子>
<项> ::= <因子> $Multiply <项>
<项> ::= <因子> $Divide <项>

<因子> ::= $Number
<因子> ::= $LeftBracket <表达式> $RightBracket
<因子> ::= $ID $LeftBracket <实参列表> $RightBracket
<因子> ::= $ID

<实参列表> ::= $Empty
<实参列表> ::= <表达式>
<实参列表> ::= <表达式> $Comma <实参列表>

<因子> ::= <数组>
<数组> ::= $ID $LeftArray <表达式> $RightArray
<数组> ::= <数组> $LeftArray <表达式> $RightArray
<assign语句> ::= <数组> $Equal <表达式> $Semi
<声明> ::= $Int $ID <数组声明> $Semi
<数组声明> ::= $LeftArray $Number $RightArray 
<数组声明> ::= <数组声明> $LeftArray $Number $RightArray
<内部变量声明> ::= $Int $ID <数组声明> $Semi