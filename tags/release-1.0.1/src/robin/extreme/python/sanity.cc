#include <stdio.h>

struct RegData { const char *name; const char *type; RegData *i; void *sym; };

#define F (void*)&

long wrap_power(long base, long expo)
{
	if (expo == 0)
		return 1;
	else if (expo == 1)
		return base;
	else {
		long d = wrap_power(base, expo / 2);
		return d * d * wrap_power(base, expo % 2);
	}
}

unsigned long *wrap_mword(unsigned long val)
{
	return new unsigned long(val);
}

unsigned long wrap_mword_get(unsigned long *self)
{
	return *self;
}

void wrap_mword_release(unsigned long *self)
{
	printf("Killing mword %ld\n", *self);
	delete self;
}

RegData power_proto[] = {
	{"base", "long", 0, 0 },
	{"expo", "long", 0, 0 },
	{ 0 }
};

RegData to_mword_proto[] = {
	{"val", "unsigned long", 0, 0 },
	{ 0 }
};

RegData mword_proto[] = {
	{ "^", "constructor", to_mword_proto, F wrap_mword },
	{ "as", "unsigned long", 0, F wrap_mword_get },
	//	{ ".delete", "void", 0, F wrap_mword_release },
	{ 0 }
};

RegData entry[] = {
	{"mword", "class", mword_proto, 0 },
	{"to_mword", "*mword", to_mword_proto, F wrap_mword },
	{"power", "long", power_proto, F wrap_power },
	{ 0 }
};
