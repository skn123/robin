#ifndef _ROBIN_DEBUG_ASSERT_
#define _ROBIN_DEBUG_ASSERT_
#include <exception>
#include <string>

namespace Robin {


class AssertException : public std::exception {
public:
	inline AssertException(const std::string &exception) 
		: message(exception)
	{
	
	}
	const char *what() const throw()
	{
		return message.c_str();
	}

	~AssertException() throw() 
	{
	
	}

private:
	std::string message;
};
} //end of namespace Robin

#ifdef NDEBUG
#define assert_true(X) do{}while(0)

#else

#define assert_true(X) do { if(!X) { throw Robin::AssertException(#X); } } while(0)

#endif




#endif
