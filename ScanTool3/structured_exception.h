#if !defined INC_structured_exception_
#define  INC_structured_exception_

#include <eh.h>
#include <windows.h>
#include <exception>

class structured_exception : public std::exception
{
public:

	structured_exception(EXCEPTION_POINTERS const &info) throw();
	static void install() throw();
	virtual char const *what() const throw();
	void const *where() const throw();

private:
	void const *address_;
};

class access_violation : public structured_exception
{
public:
	access_violation(EXCEPTION_POINTERS const &) throw();
	virtual char const *what() const throw();
};

class divide_by_zero : public structured_exception
{
public:
	divide_by_zero(EXCEPTION_POINTERS const &) throw();
	virtual char const *what() const throw();
};

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//							�ڶ���ʹ�÷���
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// �ѽṹ���쳣ת��ΪC++�쳣  
struct SException
{
	EXCEPTION_RECORD er;
	CONTEXT            context;
	SException(PEXCEPTION_POINTERS pep)
	{
		er = *(pep->ExceptionRecord);
		context = *(pep->ContextRecord);
	}

	operator DWORD() { return er.ExceptionCode; }
	static void MapSEtoCE() { _set_se_translator(TranslateSEToCE); }

	static void __cdecl TranslateSEToCE(UINT dwEC, PEXCEPTION_POINTERS pep)
	{
		throw SException(pep);
	}
};

//ʹ�÷���������ɨ���ǵײ��쳣
/*
	SException::MapSEtoCE();
	try
	{
		char * p = dest;
		if (!isConvert)
		{
			memcpy_s(dest, dataByteSize, src, dataByteSize);
			return true;
		}
		if (height <= 0) return true;
		int rowByteSize = dataByteSize / height;
		src = src + dataByteSize - rowByteSize;
		for (int i = 0; i < height; i++)
		{
			memcpy_s(dest, rowByteSize, src, rowByteSize);
			dest += rowByteSize;
			src -= rowByteSize;
		}
	}
	catch (SException& e)
	{
		std::stringstream ss;
		ss << "CopyDataʱ�����쳣��dataByteSize: " << dataByteSize << ", isConvert: " << isConvert << ", height" << height;
		g_pLogger->warning(ss.str());
		return false;
	}
*/
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
#endif // !defined INC_structured_exception_