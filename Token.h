#ifndef TOKEN_H
#define TOKEN_H

enum class Token
{
	Id,
	Int, Void, If, Else, While, Return,
	Plus, Minus, Multiply, Divide, Equal, Equal2, Bigger, BiggerEqual, Smaller, SmallerEqual, NotEqual,
	Semi, Comma, LeftAnno, RightAnno, Anno, LeftBracket, RightBracket, LeftBrace, RightBrace,
	Number,
	End,
	Unknown
};

#endif