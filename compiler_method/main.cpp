#include "preprocess.h"
#include "file.h"
#include "grammar.h"
#include <fstream>

void exp1()
{
	FILE* in = file_open("test.pas", READONLY);
	FILE* out = file_open("test.dyd", RWCLS);
	FILE* err = file_open("test.err", RWCLS);
	preprocess(in, out, err);
	fclose(in);
	fclose(out);
	fclose(err);
}

void exp2()
{
	std::ifstream fin("test.dyd");
	std::vector<Token> in;
	while (!fin.eof())
	{
		std::string str;
		int type = 0;
		fin >> str;
		fin >> type;
		in.push_back(Token(str));
	}

	FILE* err = file_open("test.err", RWCLS);
	
	Grammar grammar(in, stdout, err);

	fclose(err);
}

int main()
{
	exp1();
	exp2();

	return 0;
}