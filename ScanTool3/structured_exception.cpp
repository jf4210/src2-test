#include "structured_exception.h"

using namespace std;

//
//  ::
//
static void my_translator(unsigned code, EXCEPTION_POINTERS *info)
{
	switch (code)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			throw access_violation(*info);
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			throw divide_by_zero(*info);
			break;
		default:
			throw structured_exception(*info);
			break;
	}
}

//
//  structured_exception::
//
structured_exception::structured_exception
(EXCEPTION_POINTERS const &info) throw()
{
	EXCEPTION_RECORD const &exception = *(info.ExceptionRecord);
	address_ = exception.ExceptionAddress;
	//code_ = exception.ExceptionCode;
}


void structured_exception::install() throw()
{
	_set_se_translator(my_translator);
}

char const *structured_exception::what() const throw()
{
	return "unspecified Structured Exception";
}

void const *structured_exception::where() const throw()
{
	return address_;
}

//
//  access_violation::
//
access_violation::access_violation
(EXCEPTION_POINTERS const &info) throw()
	: structured_exception(info)
{
}

char const *access_violation::what() const throw()
{
	return "access violation";
}

//
//  divide_by_zero::
//
divide_by_zero::divide_by_zero
(EXCEPTION_POINTERS const &info) throw()
	: structured_exception(info)
{
}

char const *divide_by_zero::what() const throw()
{
	return "divide by zero";
}