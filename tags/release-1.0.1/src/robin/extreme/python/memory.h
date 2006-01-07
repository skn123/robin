// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef ROBIN_PYTHON_MEMORY_MANAGEMENT_TEST_H
#define ROBIN_PYTHON_MEMORY_MANAGEMENT_TEST_H


class ConsumptionUnit
{
public:
	ConsumptionUnit();
	~ConsumptionUnit();

	static unsigned long counter;

	/**
	 * @par .robin
	 * returns borrowed
	 */
	ConsumptionUnit *meAgain() { return this; }

private:
	long m_space[4];
};


unsigned long ConsumptionUnit::counter = 0;


ConsumptionUnit::ConsumptionUnit()
{
	++counter;
}

ConsumptionUnit::~ConsumptionUnit()
{
	--counter;
}

unsigned long getCounter() { return ConsumptionUnit::counter; }

unsigned long getCounter(int units) { return ConsumptionUnit::counter / units; }

#endif
