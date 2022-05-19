#ifndef TOKEN_H
#define TOKEN_H
#pragma execution_character_set("utf-8")

enum class Token
{
	Id = 0,
	Int, Void, If, Else, While, Return,
	Plus, Minus, Multiply, Divide, Equal, Equal2, Bigger, BiggerEqual, Smaller, SmallerEqual, NotEqual,
	LeftArray, RightArray,Semi, Comma, LeftAnno, RightAnno, Anno, LeftBracket, RightBracket, LeftBrace, RightBrace,
	Number,
	End,
	Unknown
};

#endif
