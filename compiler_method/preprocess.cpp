#include "preprocess.h"
#include "file.h"
#include "token.h"
#include <string>
#include <cstdio>

static size_t line;
FILE* out;
FILE* err;

void ptoken(std::string str)
{
		Token token(str);
		fprintf(out, "%16s %2d\n", token.str().c_str(), token.type());
		if (token.type() == Token::Type::Error)
		{
			fprintf(err, "***LINE:%2d  %s '%s'\n", line, "invalid identifier", str.c_str());
		}
}

void preprocess(FILE* in, FILE* _out, FILE* _err)
{
	std::string str;
	line = 1;
	out = _out;
	err = _err;

	while (!feof(in))
	{
		char ch = fgetc(in);
		switch (ch)
		{
		case ' ':
		case '\r':
			if (str.size() > 0)
			{
				ptoken(str);
				str.clear();
			}
			break;
		case '\n':
			if (str.size() > 0)
			{
				ptoken(str);
				str.clear();
			}
			ptoken("EOLN");
			line++;
			break;
		case ';':
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
		case ')':
		case '=':
			if (str.size() > 0)
			{
				ptoken(str);
				str.clear();
			}
			str.push_back(ch);
			ptoken(str);
			str.clear();
			break;
		case '<':
			if (str.size() > 0)
			{
				ptoken(str);
				str.clear();
			}
			str.push_back(ch);
			ch = fgetc(in);
			if (ch == '=')
			{
				str.push_back(ch);
				ptoken(str);
				str.clear();
			}
			else if (ch == '>')
			{
				str.push_back(ch);
				ptoken(str);
				str.clear();
			}
			else
			{
				ptoken(str);
				str.clear();
				str.push_back(ch);
			}
			break;
		case '>':
			if (str.size() > 0)
			{
				ptoken(str);
				str.clear();
			}
			str.push_back(ch);
			ch = fgetc(in);
			if (ch == '=')
			{
				str.push_back(ch);
				ptoken(str);
				str.clear();
			}
			else
			{
				ptoken(str);
				str.clear();
				str.push_back(ch);
			}
			break;
		case ':':
			if (str.size() > 0)
			{
				ptoken(str);
				str.clear();
			}
			str.push_back(ch);
			ch = fgetc(in);
			if (ch == '=')
			{
				str.push_back(ch);
				ptoken(str);
				str.clear();
			}
			else
			{
				fprintf(err, "***LINE:%2d  missing '='\n", line);
			}
			break;
		default:
			str.push_back(ch);
			break;
		}
	}
	ptoken("EOF");
}