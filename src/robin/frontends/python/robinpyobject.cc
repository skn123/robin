/*
 * robinpyobject.cc
 *
 *  Created on: Oct 29, 2009
 *      Author: Marcelo Taube
 */

#include "robinpyobject.h"


namespace Robin {
namespace Python {

void RobinPyObject::__dealloc__(PyObject *self)
{
	//will call the delete defined in this class
	delete (RobinPyObject *)self;
}


};
};
