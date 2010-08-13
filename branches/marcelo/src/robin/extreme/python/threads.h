#ifndef ROBIN_THREADS_TEST_H
#define ROBIN_THREADS_TEST_H

#include <sstream>
#include <unistd.h>

class ThreadList {
public:

	ThreadList() {

	}

	void writeChar(char c, long times) {
		for(long i = 0; i < times; ++i) {
			usleep(10);
			m_stream << c;
		}
	}

	std::string getString() {
		return m_stream.str();
	}

private:
	
	std::stringstream m_stream;
};

#endif
