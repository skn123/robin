// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * pattern/singleton.h
 *
 * @par TITLE
 * Singleton Template
 *
 * @par PACKAGE
 * Pattern
 */

#ifndef PATTERN_SINGLETON_H
#define PATTERN_SINGLETON_H


/**
 * @brief Implementations of useful design patterns.
 */
namespace Pattern {


/**
 * @class Singleton
 * @nosubgrouping
 */
template < class T, int id = 0x1 >
class Singleton
{
public:
	static inline T * getInstance()
	{
		if (m_instance == 0)
			m_instance = new T;
		return m_instance;
	}

	static void setInstance(T * yourInstance)
	{
		if (m_instance) delete m_instance;
		m_instance = yourInstance;
	}

protected:
	static T * m_instance;
};


template < class T, int id >
T * Singleton<T,id>::m_instance = 0;

} // end of namespace Pattern

#endif
