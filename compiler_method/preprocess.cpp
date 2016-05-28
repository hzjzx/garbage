#include "preprocess.h"
#include "file.h"
#include "token.h"
#include <string>
#include <cstdio>

void ptoken(std::string str, FILE* out)
{
	Token token(str);
	fprintf(out, "%16s %2d\n", token.str().c_str(), token.type());
}

void preprocess(FILE* in, FILE* out, FILE* err)
{
	std::string str;
	size_t line = 0;

	while (!feof(in))
	{
		char ch = fgetc(in);
		switch (ch)
		{
		case ' ':
		case '\r':
			if (str.size() > 0)
			{
				ptoken(str, out);
				str.clear();
			}
			break;
		case '\n':
			if (str.size() > 0)
			{
				ptoken(str, out);
				str.clear();
			}
			ptoken("EOLN", out);
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
				ptoken(str, out);
				str.clear();
			}
			str.push_back(ch);
			ptoken(str, out);
			str.clear();
			break;
		case '<':
			if (str.size() > 0)
			{
				ptoken(str, out);
				str.clear();
			}
			str.push_back(ch);
			ch = fgetc(in);
			if (ch == '=')
			{
				str.push_back(ch);
				ptoken(str, out);
				str.clear();
			}
			else if (ch == '>')
			{
				str.push_back(ch);
				ptoken(str, out);
				str.clear();
			}
			else
			{
				ptoken(str, out);
				str.clear();
				str.push_back(ch);
			}
			break;
		case '>':
			if (str.size() > 0)
			{
				ptoken(str, out);
				str.clear();
			}
			str.push_back(ch);
			ch = fgetc(in);
			if (ch == '=')
			{
				str.push_back(ch);
				ptoken(str, out);
				str.clear();
			}
			else
			{
				ptoken(str, out);
				str.clear();
				str.push_back(ch);
			}
			break;
		case ':':
			if (str.size() > 0)
			{
				ptoken(str, out);
				str.clear();
			}
			str.push_back(ch);
			ch = fgetc(in);
			if (ch == '=')
			{
				str.push_back(ch);
				ptoken(str, out);
				str.clear();
			}
			else
			{
				fprintf(err, "***LINE%d:  missing '='\n", line);
			}
			break;
		default:
			str.push_back(ch);
			break;
		}
	}
}