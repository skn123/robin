#ifndef ROBIN_EXTREME_PYTHON_LANGUAGE_STL_H
#define ROBIN_EXTREME_PYTHON_LANGUAGE_STL_H

#include <string>
#include <vector>
#include <map>


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
		 * @param v [output] blaht
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
	};

	struct DerivedFromVector : public std::vector<unsigned short>
	{
	};

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

}


#endif
