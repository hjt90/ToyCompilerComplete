$Start ::= <N> <������>

<A> ::= $Empty
<N> ::= $Empty
<M> ::= $Empty

<������> ::= <����>
<������> ::= <������> <����>

<����> ::= $Int $ID <��������>
<����> ::= $Void $ID <M> <A> <��������>
<����> ::= $Int $ID <M> <A> <��������>

<��������> ::=  $Semi

<��������> ::= $LeftBracket <�β�> $RightBracket <����>

<�β�> ::= <�����б�>
<�β�> ::= $Void

<�����б�> ::= <����>
<�����б�> ::= <����> $Comma <�����б�>

<����> ::= $Int $ID

<����> ::= $LeftBrace <�ڲ�����> <��䴮> $RightBrace

<�ڲ�����> ::= $Empty
<�ڲ�����> ::= <�ڲ���������> <�ڲ�����>

<�ڲ���������> ::= $Int $ID $Semi

<��䴮> ::= <���>
<��䴮> ::= <���> <M> <��䴮>

<���> ::= <if���>
<���> ::= <while���>
<���> ::= <return���>
<���> ::= <assign���>

<assign���> ::= $ID $Equal <���ʽ> $Semi

<return���> ::= $Return $Semi
<return���> ::= $Return <���ʽ> $Semi

<while���> ::= $While <M> $LeftBracket <���ʽ> $RightBracket <A> <����>

<if���> ::= $If $LeftBracket <���ʽ> <A> $RightBracket <����>
<if���> ::= $If $LeftBracket <���ʽ> <A> $RightBracket <����> <N> $Else <M> <A> <����>

<���ʽ> ::= <�ӷ����ʽ>
<���ʽ> ::= <���ʽ> <�Ƚ������> <�ӷ����ʽ>

<�Ƚ������> ::= $Smaller
<�Ƚ������> ::= $SmallerEqual
<�Ƚ������> ::= $Bigger
<�Ƚ������> ::= $BiggerEqual
<�Ƚ������> ::= $Equal2
<�Ƚ������> ::= $NotEqual

<�ӷ����ʽ> ::= <��>
<�ӷ����ʽ> ::= <��> $Plus <�ӷ����ʽ>
<�ӷ����ʽ> ::= <��> $Minus <�ӷ����ʽ>

<��> ::= <����>
<��> ::= <����> $Multiply <��>
<��> ::= <����> $Divide <��>

<����> ::= $Number
<����> ::= $LeftBracket <���ʽ> $RightBracket
<����> ::= $ID $LeftBracket <ʵ���б�> $RightBracket
<����> ::= $ID

<ʵ���б�> ::= $Empty
<ʵ���б�> ::= <���ʽ>
<ʵ���б�> ::= <���ʽ> $Comma <ʵ���б�>
