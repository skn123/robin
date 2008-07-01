// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * adapter.h
 *
 * @par TITLE
 * Adapter Interface
 *
 * Defines the framework for active translations, used by
 * various frontends.
 */

#ifndef ROBIN_FRONTENDS_FRAMEWORK_ADAPTER_H
#define ROBIN_FRONTENDS_FRAMEWORK_ADAPTER_H

#include "../reflection/callable.h"
#include "../reflection/argumentsbuffer.h"


namespace Robin {

/**
 * @class Adapter
 * @nosubgrouping
 *
 * @par REFER-TO
 * HLDD 6.5.1, 6.6.2
 *
 * The Active Translation interface; a frontend should
 * implement this interface for every basic type used. The implemented
 * objects are then placed in the approperiate 
 * <classref>TypeOfArgument</classref>s.
 */
class Adapter
{
public:
    /**
     * Implement how scripting elements should be
     * reduced to C values and put on the stack when calling
     * functions or methods.
     */
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value) = 0;

    /**
     * Implement how return values should be
     * interpreted in order to produce scripting elements when
     * a function or method call returns.
     */
    virtual scripting_element get(basic_block data) = 0;
};


}

#endif
