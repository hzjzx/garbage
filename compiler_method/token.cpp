#include "token.h"
#include <map>

static std::map<std::string, Token::Type> dic =
{
	{"begin",Token::Type::Begin},
	{"end",Token::Type::End},
	{"integer",Token::Type::Integer},
	{"if",Token::Type::If},
	{"then",Token::Type::Then},
	{"else",Token::Type::Else},
	{"function",Token::Type::Function},
	{"read",Token::Type::Read},
	{"write",Token::Type::Write},
	{"=",Token::Type::OperatorEqual},
	{"<>",Token::Type::OperatorNotEqual},
	{"<=",Token::Type::OperatorSmallEqual},
	{"<",Token::Type::OperatorSmall},
	{">=",Token::Type::OperatorLargeEqual},
	{">",Token::Type::OperatorLarge},
	{"-",Token::Type::OperatorMinus},
	{"*",Token::Type::OperatorMultiply},
	{":=",Token::Type::OperatorAssignment},
	{"(",Token::Type::BracketLeft},
	{")",Token::Type::BracketRight},
	{";",Token::Type::Semi},
	{"EOLN",Token::Type::EOLN},
	{"EOF",Token::Type::EOFL}
};

Token::Token(std::string str):
	m_str(str)
{
	if (dic.find(str) == dic.end())
	{
		for (char ch : str)
		{
			if ((ch < '0') || (ch > '9'))
			{
				m_type = Type::Identifier;
				if (str[0] >= '0' && str[0] <= '9')
				{
					m_type = Type::Error;
				}
				return;
			}
		}
		m_type = Type::Number;
	}
	else
	{
		m_type = dic[str];
	}
}
