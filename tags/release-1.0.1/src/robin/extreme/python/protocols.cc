#include "protocols.h"

bool operator^(const Times& first, const Times& second)
{
	return int(first) * int(second) < 0;
}
