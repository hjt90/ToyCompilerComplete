$Start ::= <������>

<������> ::= <����>
<������> ::= <������> <����>

<����> ::= $Int $ID <��������>
<����> ::= $Void $ID <��������>

<��������> ::= <��������>
<��������> ::= <��������>

<��������> ::=  $Semi

<��������> ::= $LeftBracket <�β�> $RightBracket <����>

<�β�> ::= <�����б�>
<�β�> ::= $Void

<�����б�> ::= <����>
<�����б�> ::= <�����б�> $Comma <����>

<����> ::= $Int $ID

<����> ::= $LeftBrace <�ڲ�����> <��䴮> $RightBrace
<����> ::= $LeftBrace <��䴮> $RightBrace

<�ڲ�����> ::= <�ڲ���������>
<�ڲ�����> ::= <�ڲ�����> <�ڲ���������>

<�ڲ���������> ::= $Int $ID $Semi

<��䴮> ::= <���>
<��䴮> ::= <��䴮> <���>

<���> ::= <if���>
<���> ::= <while���>
<���> ::= <return���>
<���> ::= <��ֵ���>

<��ֵ���> ::= $ID $Equal <���ʽ> $Semi

<return���> ::= $Return $Semi
<return���> ::= $Return <���ʽ> $Semi

<while���> ::= $While $LeftBracket <���ʽ> $RightBracket <����>

<if���> ::= $If $LeftBracket <���ʽ> $RightBracket <����>
<if���> ::= $If $LeftBracket <���ʽ> $RightBracket <����> $Else <����>

<���ʽ> ::= <�ӷ����ʽ>
<���ʽ> ::= <���ʽ> <�Ƚ������> <�ӷ����ʽ>

<�Ƚ������> ::= $Smaller
<�Ƚ������> ::= $SmallerEqual
<�Ƚ������> ::= $Bigger
<�Ƚ������> ::= $BiggerEqual
<�Ƚ������> ::= $Equal2
<�Ƚ������> ::= $NotEqual

<�ӷ����ʽ> ::= <��>
<�ӷ����ʽ> ::= <�ӷ����ʽ> $Plus <��>
<�ӷ����ʽ> ::= <�ӷ����ʽ> $Minus <��>

<��> ::= <����>
<��> ::= <��> $Multiply <����>
<��> ::= <��> $Divide <����>

<����> ::= $Number
<����> ::= $LeftBracket <���ʽ> $RightBracket
<����> ::= $ID <FTYPE>
<����> ::= $ID

<FTYPE> ::= <call>

<call> ::= $LeftBracket <ʵ��> $RightBracket
<call> ::= $LeftBracket $RightBracket

<ʵ��> ::= <ʵ���б�>

<ʵ���б�> ::= <���ʽ>
<ʵ���б�> ::= <ʵ���б�> $Comma <���ʽ>