// -*- mode: c++; c-basic-offset: 4; tab-width: 4 -*-
#include <valgrind/callgrind.h>
#include <iostream>

struct RegData { const char *name; const char *type; RegData *i; void *sym; };

#define F (void*)&

static void toggleCollect() {
	CALLGRIND_TOGGLE_COLLECT();
}

static void startInstrumentation() {
	std::cout << "start instrumentation\n" << std::endl;
	CALLGRIND_START_INSTRUMENTATION();
}

static void stopInstrumentation() {
	std::cout << "stop instrumentation\n" << std::endl;
	CALLGRIND_STOP_INSTRUMENTATION();
}

#ifdef _WIN32
extern "C"
__declspec(dllexport)
#endif
RegData entry[] = {
	{ "toggleCollect", "void", 0, F toggleCollect },
	{ "startInstrumentation", "void", 0, F startInstrumentation },
	{ "stopInstrumentation", "void", 0, F stopInstrumentation },
	{ 0 }
};
