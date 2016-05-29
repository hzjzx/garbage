#include "grammar.h"

void Grammar::perr(const std::string& msg)
{
	fprintf(m_err, "***LINE:%2d  %s\n", m_line, msg.c_str());
}

void Grammar::perr(const std::string& msg, const Token & token)
{
	fprintf(m_err, "***LINE:%2d  %s '%s'\n", m_line, msg.c_str(), token.str().c_str());
}

void Grammar::next()
{
	m_iter++;
	while (m_iter->type() == Token::Type::EOLN)
	{
		m_line++;
		m_iter++;
	}
}

void Grammar::next_semi()
{
	while (m_iter->type() != Token::Type::Semi)
	{
		next();
	}
}

void Grammar::program()
{
	Proc proc;
	proc.lev = 0;
	m_ptable.push_back(proc);
	m_proc.push(proc);

	sub_program();

	m_proc.pop();
}

void Grammar::sub_program()
{
	if (m_iter->type() == Token::Type::Begin)
	{
		next();
	}
	else
	{
		perr("missing 'begin'");
		next();
	}

	statememt_table1();

	/*
	if (m_iter->type() == Token::Type::Semi)
	{
		next();
	}
	else
	{
		perr("missing ';'");
	}*/

	exec_table1();

	if (m_iter->type() == Token::Type::End)
	{
		next();
	}
	else
	{
		perr("missing 'end'");
	}
}

void Grammar::statememt_table1()
{
	statement();
	statement_table2();
}

void Grammar::statement_table2()
{
	if (m_iter->type() == Token::Type::Semi)
	{
		next();
	}
	else if(m_iter->type() != Token::Type::Integer)
	{
		return;
	}
	else
	{
		perr("missing ';'");
	}
	statement();
	statement_table2();
}

void Grammar::statement()
{
	if (m_iter->type() == Token::Type::Integer)
	{
		next();
	}
	else
	{
		return;
	}

	if (m_iter->type() == Token::Type::Function)
	{
		stat_func();
	}
	else if(m_iter->type() == Token::Type::Identifier)
	{
		stat_var();
	}
	else
	{

	}
}

void Grammar::stat_func()
{
	Proc proc;
	proc.lev = m_ptable.size();

	if (m_iter->type() == Token::Type::Function)
	{
		next();
	}
	
	if (m_iter->type() == Token::Type::Identifier)
	{
		proc.name = m_iter->str();
		next();
	}
	
	m_proc.push(proc);
	m_ptable.push_back(proc);

	if (m_iter->type() == Token::Type::BracketLeft)
	{
		next();
	}

	if (m_iter->type() == Token::Type::Identifier)
	{
		Var var;
		var.name = m_iter->str();
		var.kind = Var::Kind::Parameter;
		var.proc = m_proc.top().name;
		var.type = Var::Type::Integer;
		var.addr = m_vtable.size();
		m_vtable.push_back(var);
		m_proc.top().ladr = var.addr;
		m_proc.top().fadr = var.addr;
		next();
	}

	if (m_iter->type() == Token::Type::BracketRight)
	{
		next();
	}
	
	if (m_iter->type() == Token::Type::Semi)
	{
		next();
	}

	func_body();

	m_proc.pop();
}

void Grammar::stat_var()
{
	if (m_iter->type() == Token::Type::Identifier)
	{
		for (const auto& var : m_vtable)
		{
			if (m_proc.top().name == var.proc && var.name == m_iter->str())
			{
				perr("variable re-define", m_iter->str());
				break;
			}
		}

		Var var;
		var.name = m_iter->str();
		var.kind = Var::Kind::Variable;
		var.proc = m_proc.top().name;
		var.type = Var::Type::Integer;
		m_vtable.push_back(var);
		next();
	}
}

void Grammar::func_body()
{
	if (m_iter->type() == Token::Type::Begin)
	{
		next();
	}
	else
	{
		perr("missing 'begin'");
	}

	statememt_table1();

	/*
	if (m_iter->type() == Token::Type::Semi)
	{
		next();
	}
	else
	{
		perr("missing ';'");
	}*/

	exec_table1();

	if (m_iter->type() == Token::Type::End)
	{
		next();
	}
}

void Grammar::exec_table1()
{
	exec();
	exec_table2();
}

void Grammar::exec_table2()
{
	if (m_iter->type() == Token::Type::Semi)
	{
		next();
	}
	else if (m_iter->type() == Token::Type::End)
	{
		return;
	}
	else
	{
		perr("missing ';' or 'end'");
	}

	exec();
	exec_table2();
}

void Grammar::exec()
{
	if (m_iter->type() == Token::Type::Read || m_iter->type() == Token::Type::Write)
	{
		rw();
	}
	else if (m_iter->type() == Token::Type::If)
	{
		cond();
	}
	else if (m_iter->type() == Token::Type::Identifier)
	{
		assign();
	}
	else if (m_iter->type() == Token::Type::End)
	{
		m_line--;
		perr("extra ';'");
		m_line++;
	}
	else
	{
		perr("illegal executable line");
	}
}

void Grammar::rw()
{
	if (m_iter->type() == Token::Type::Read || m_iter->type() == Token::Type::Write)
	{
		next();
	}

	if (m_iter->type() == Token::Type::BracketLeft)
	{
		next();
	}
	else
	{
		perr("missing '('");
	}

	if (m_iter->type() == Token::Type::Identifier)
	{
		bool acc = false;
		for (const Var& var : m_vtable)
		{
			if (var.name == m_iter->str() && var.proc == m_proc.top().name)
			{
				acc = true;
				break;
			}
		}
		if (!acc)
		{
			perr("undefined indentifier", m_iter->str());
		}
		next();
	}

	if (m_iter->type() == Token::Type::BracketRight)
	{
		next();
	}
	else
	{
		perr("missing ')'");
	}
}

void Grammar::cond()
{
	if (m_iter->type() == Token::Type::If)
	{
		next();
	}

	cond_expression();

	if (m_iter->type() == Token::Type::Then)
	{
		next();
	}

	exec();

	if (m_iter->type() == Token::Type::Else)
	{
		next();
	}

	exec();
}

void Grammar::cond_expression()
{
	expression1();

	switch (m_iter->type())
	{
	case Token::Type::OperatorEqual:
	case Token::Type::OperatorLarge:
	case Token::Type::OperatorLargeEqual:
	case Token::Type::OperatorNotEqual:
	case Token::Type::OperatorSmall:
	case Token::Type::OperatorSmallEqual:
		next();
		break;
	default:
		perr("missing logic operator");
		break;
	}

	expression1();
}

void Grammar::assign()
{
	if (m_iter->type() == Token::Type::Identifier)
	{
		bool acc = false;
		for (const Var& var : m_vtable)
		{
			if (var.proc == m_proc.top().name && var.name == m_iter->str())
			{
				acc = true;
				break;
			}
			if (m_iter->str() == m_proc.top().name)
			{
				acc = true;
				break;
			}
		}
		if (!acc)
		{
			perr("undefined identifier", m_iter->str());
		}
		next();
	}

	if (m_iter->type() == Token::Type::OperatorAssignment)
	{
		next();
	}
	else
	{
		perr("missing ':='");
		next_semi();
		return;
	}

	expression1();
}

void Grammar::expression1()
{
	item1();
	expression2();
}

void Grammar::expression2()
{
	if (m_iter->type() == Token::Type::OperatorMinus)
	{
		next();
	}
	else
	{
		return;
	}

	item1();
	expression2();
}

void Grammar::item1()
{
	factor();
	item2();
}

void Grammar::item2()
{
	if (m_iter->type() == Token::Type::OperatorMultiply)
	{
		next();
	}
	else
	{
		return;
	}

	factor();
	item2();
}

void Grammar::factor()
{
	if (m_iter->type() == Token::Type::Identifier)
	{
		for (const Var& var : m_vtable)
		{
			if (var.proc == m_proc.top().name && var.name == m_iter->str())
			{
				next();
				return;
			}
		}
		for (const Proc& proc : m_ptable)
		{
			if (proc.name == m_iter->str())
			{
				next();
				func_call();
				return;
			}
		}
		
		perr("undefined identifier", m_iter->str());
		next();
	}
	else if (m_iter->type() == Token::Type::Number)
	{
		next();
		return;
	}
}

void Grammar::func_call()
{
	if (m_iter->type() == Token::Type::BracketLeft)
	{
		next();
	}
	else
	{
		perr("missing '('");
	}

	expression1();

	if (m_iter->type() == Token::Type::BracketRight)
	{
		next();
	}
	else
	{
		perr("missing ')'");
	}
}


