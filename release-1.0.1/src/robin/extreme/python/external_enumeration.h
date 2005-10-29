
#ifndef ROBIN_TEST_ENUMERATION_H
#define ROBIN_TEST_ENUMERATION_H


template < class T >
class Enumeration
{
 public:
	virtual Enumeration<T> *clone() const { return 0; }
};


#endif
