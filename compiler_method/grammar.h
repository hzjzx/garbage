#pragma once
#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "token.h"
#include <cstdio>
#include <vector>
#include <stack>

class Grammar
{
private:
	struct Var
	{
		enum class Kind
		{
			Variable = 0,
			Parameter = 1
		};

		enum class Type
		{
			Integer = 0
		};

		std::string name;
		std::string proc;
		Kind kind;
		Type type;
		int	lev;
		int addr;
	};

	struct Proc
	{
		enum class Type
		{
			Integer = 0
		};

		std::string name;
		int lev;
		int fadr;
		int ladr;
	};

private:
	FILE* m_out;
	FILE* m_err;
	size_t m_line;
	const std::vector<Token>& m_in;
	std::vector<Token>::const_iterator m_iter;
	std::vector<Var> m_vtable;
	std::vector<Proc> m_ptable;
	std::stack<Proc> m_proc;

private:
	void perr(const std::string& msg);
	void perr(const std::string& msg, const Token& token);
	void next();
	void next_semi();
	void program();
	void sub_program();
	void statememt_table1();
	void statement_table2();
	void statement();
	void stat_func();
	void stat_var();
	void func_body();
	void exec_table1();
	void exec_table2();
	void exec();
	void rw();
	void cond();
	void cond_expression();
	void assign();
	void expression1();
	void expression2();
	void item1();
	void item2();
	void factor();
	void func_call();

public:
	Grammar(const std::vector<Token>& in, FILE* out, FILE* err):
		m_in(in),
		m_iter(in.begin())
	{
		m_line = 1;
		m_out = out;
		m_err = err;

		program();
	}
};

#endif