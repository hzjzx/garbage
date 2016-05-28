#include "preprocess.h"
#include "file.h"

void exp1()
{
	FILE* in = file_open("test.pas", READONLY);
	FILE* out = file_open("test.dyd", RWCLS);
	FILE* err = file_open("test.dyd", RWCLS);
	preprocess(in, out, err);
	fclose(in);
	fclose(out);
	fclose(err);
}

void exp2()
{

}

int main()
{
	exp1();
	exp2();

	return 0;
}