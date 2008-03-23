// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef ROBIN_EXTREME_PYTHON_LANGUAGE_STL_H
#define ROBIN_EXTREME_PYTHON_LANGUAGE_STL_H

#include <string>
#include <vector>
#include <map>
#include <complex>
#include <stdexcept>


namespace StandardLibrary
{

	class UsingStrings
	{
	public:
		UsingStrings(std::string data) : m_data(data) { }

		int size() const { return m_data.size(); }
		char operator[](int index) { return m_data[index]; }

	private:
		std::string m_data;
	};

	class UsingStringConversions
	{
	public:
		UsingStringConversions(std::string data) : m_conversion(1) { }
		UsingStringConversions(const char* data) : m_conversion(2) { }

		int getConversionType() { return m_conversion; }

	private:
		int m_conversion;
	};

	class UsingVectors
	{
	public:
		UsingVectors(std::vector<double> data) : m_data(data), m_id(0) { }
		UsingVectors(std::vector<long> data) : m_id(1) { }
		UsingVectors(std::vector<std::string> data) : m_id(2) { }
		UsingVectors(std::vector<long long> data) : m_id(3) { }
		UsingVectors(std::vector<unsigned long long> data) : m_id(4) { }

		std::vector<double> get() const { return m_data; }
		std::vector<std::vector<double> > getv() const { 
			std::vector<std::vector<double> > vd;
			vd.push_back(m_data);
			return vd;
		};

		void atof(std::vector<char> a) { m_data.push_back(::atof(&a[0])); }
		void atof(std::vector<signed char> a) { }

		int getVectorType() { return m_id; }

		/**
		 * @param v [output] result vector
		 */
		static void modifyVectorInPlace(std::vector<int> &v)
		{
			for(int i = 0; i < v.size(); ++i) {
				v[i] = v[i] * 2;
			}
		}

	private:
		std::vector<double> m_data;
		int m_id;

		struct Unseen { };
		std::vector<Unseen> m_priv_vector; // see ticket #49
	};

	struct DerivedFromVector : public std::vector<unsigned short>
	{
	};

	/* TODO
	struct DerivedFromVectorOfClass : public std::vector<DerivedFromVector>
	{
	};
	*/

	class UsingPairs
	{
	public:
		typedef std::pair<int, std::string> CoPair;
		typedef std::vector<double> CoVector;

		UsingPairs(std::pair<int, std::string> data) : m_data(data) { }

		CoPair get() const { return m_data; }

	private:
		std::pair<int, std::string> m_data;
	};

	void copy(UsingStrings& a, const UsingStrings& b) { }

	typedef std::pair<long,long> LPair;

	class UsingComplex
	{
	public:
		typedef std::complex<double> C;
		typedef std::pair<C,C> Qubit;

		void append(C alpha, C beta) 
		{ m_states.push_back(Qubit(alpha, beta)); }

		std::complex<double> pivot() const { return C(1.0, 1.0); }

		std::vector<C> tensor()
		{
			if (m_states.size() != 2) throw std::domain_error("sz must be 2");

			C p[2] = { m_states[0].first, m_states[0].second };
			C q[2] = { m_states[1].first, m_states[1].second };
			std::vector<C> t;
			t.resize(4);
			for (int i1 = 0; i1 < 2; ++ i1)
				for (int i2 = 0; i2 < 2; ++ i2)
					t[i1*2 + i2] = p[i1] * q[i2];
			return t;
		}

	private:
		std::vector<Qubit> m_states;
	};

	class UsingStreams
	{
	public:
		struct Datum {
			int num; std::string text;
			
			Datum() {} 
			void setText(const std::string& t) { text = t; }
		};
		
		void read_or_write(std::ostream& out, Datum& datum)
		{
			out << datum.text << ", " << datum.num;
		}
		void read_or_write(std::istream& in, Datum& datum)
		{
			in >> datum.text >> datum.num;
		}
		
		void write(std::ostream& out, Datum& datum)
		{
			out << datum.text << ", " << datum.num;
		}
		void read(std::istream& in, Datum& datum)
		{
			in >> datum.text >> datum.num;
		}
	};
}


#endif
