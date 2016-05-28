#pragma once
#ifndef TOKEN_H
#define TOKEN_H

#include <string>

class Token
{
public:
	enum class Type
	{
		Begin = 1,
		End  = 2,
		Integer = 3,
		If = 4,
		Then = 5,
		Else = 6,
		Function = 7,
		Read = 8,
		Write = 9,
		Identifier = 10,
		Number = 11,
		OperatorEqual = 12,
		OperatorNotEqual = 13,
		OperatorSmallEqual = 14,
		OperatorSmall = 15,
		OperatorLargeEqual = 16,
		OperatorLarge = 17,
		OperatorMinus = 18,
		OperatorMultiply = 19,
		OperatorAssignment = 20,
		BracketLeft = 21,
		BracketRight = 22,
		Semi = 23,
		EOLN = 24,
		EOFL = 25
	};

private:
	std::string m_str;
	Type m_type;

public:
	Token(std::string str);
	const std::string& str()
	{
		return m_str;
	}
	Type type()
	{
		return m_type;
	}

};

#endif