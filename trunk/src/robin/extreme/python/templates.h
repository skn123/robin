// -*- c++ -*-

#ifndef ROBIN_PYTHON_CHECK_TEMPLTES_H
#define ROBIN_PYTHON_CHECK_TEMPLTES_H

#include <vector>
#include "external_enumeration.h"


class T { };

template < class E >
class Holder
{
public:
	typedef E held_type;

	void init(E value) { e = value; }
	operator E() const { return e; }

	held_type increment() { return ++e; }

private:
	E e;
};

template < class E >
class Multiplier
{
public:
	Multiplier(E op1, E op2) : result(op1 * op2) { }

	template < class J >
	void join(Holder<J> hold, Multiplier<J> mult) { }

	E result;
};

class Carrier
{
public:
	Carrier() { kints.push_back(666); }

	template < class J >
	void join(Holder<J> hold, Multiplier<J> mult) { }

	std::vector<unsigned short> kints;
	typedef std::vector<unsigned short> Input;
	static const int SIZE = 7;
};

template < int T >
class BarrierBase
{
public:
	std::vector< Holder<int> > vec() const {
		std::vector< Holder<int> > v;
		v.resize(T); return v;
	}
};

/**
 * @par .robin
 * cloneable
 */
template < int K >
class Barrier : public BarrierBase<K>
{
public:
	Barrier<K> bar() const { return *this; }

	typedef Multiplier<int> Block;

	Block squarer() const { return Multiplier<int>(K,K); }

private:
	Holder<int> array[K];
};

class Barrier5Vec : public std::vector< Barrier<5> >
{
public:
	static Barrier5Vec barriers(int length) {
		Barrier5Vec vec;
		Barrier<5> item;
		for (int i = 0; i < length; ++i)
			vec.push_back(item);
		return vec;
	}
};

template < class Element, long Size = Element::SIZE >
class Listen : public Enumeration<Element>
{
public:
	Barrier<Size> barrier() const { return Barrier<Size>(); }
};	

typedef Holder<int> Integer;
typedef Multiplier<int> IntegerMult;
typedef Barrier<4> Barrier4;

typedef Listen<Carrier, 4> ListenCarrier4;
typedef Listen<Carrier> ListenCarrier6;


template < class E, class Traits >
class IteratorInterface
{
public:
	virtual typename Traits::value_type nextValue() = 0;
};
 
template < class E, class Traits >
class IteratorPartialImpl : public IteratorInterface<E,Traits>
{
public:
	IteratorPartialImpl() { value.init(Traits::initial); }

	typename Traits::value_type nextValue() {
		forward(value);
		return value;
	}

protected:
	virtual void forward(typename Traits::value_type& val) = 0;

private:
	typename Traits::value_type value;
};

template < class E, class Traits >
class IteratorInc : public IteratorPartialImpl<E,Traits>
{
protected:
	virtual void forward(typename Traits::value_type& val) {
		val.increment();
	}
};

class IntegerTraits { public: typedef Holder<int> value_type; 
	                          static const int initial = 1; };

typedef IteratorInc<int, IntegerTraits> IntegerIterator;



namespace More {

	typedef Holder<double> Real;
	const int SOUND = 9;

}


namespace Less {

	using More::Real;
	using More::SOUND;

	typedef std::vector<Real> RealSeries;
	class SoundBarrier : public Barrier<SOUND> { };

	template < class Traits >
	class Yets {
	public:
		Barrier<Traits::SIZE> barrier() const {
			Barrier<Traits::SIZE> b; return b;
		}

		void progress(const typename Traits::Input& input) { }
	};

	template < class Freights >
	class Lets : public Barrier<Freights::SIZE> {
	};

	typedef Yets<Carrier> Carry;
	typedef Lets<Carrier> Merry;

	typedef Merry Meriadoc;

	class Pippin : public Meriadoc
	{
	};
}


int conj(int i1, int i2)
{
	return i1 + i2;
}

template < class E >
Holder<E> conj(const Holder<E>& he1, const Holder<E>& he2)
{
	return Holder<E>();
}

Barrier4 barr()
{
	Barrier4 b4;
	return b4;
}


class InnerDummy {{

};

template<typename T>
class TemplatedInner {

public:
    typedef InnerDummy TemplatedInnerDummy;

};

template<typename G>
class TemplatedOuter {

public:
    void method(G::TemplatedInnerDummy param) {
        return 1;
    }

};

typedef Nest<Test<int> > NestedTemplate;

#endif
