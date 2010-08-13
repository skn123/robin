/*
 * pyrobinobject.h
 *
 *  Created on: Oct 29, 2009
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_FRONTENDS_PYTHON_PYROBINOBJECT_H_
#define ROBIN_FRONTENDS_PYTHON_PYROBINOBJECT_H_

#include <Python.h>

namespace Robin {
namespace Python {
	/**
	 *  RobinPyObject is an improvement/wrapping of PyObject
	 *  in a C++ fashion (more secure). It inherits from PyObject
	 *  and adds some features.
	 *  It is recommended that nothing in Robin inherits or extends
	 *  PyObject directly but uses RobinPyObject.
	 *
	 *  Currently it includes the following features:
	 *  	- It ensures that PyObject_Init is called when the
	 *  		object is created.
	 *      - It ensures that delete is never called and it is only
	 *          freed from memory when the dealloc function is called.
	 *      - It ensures that new[] and delete[] are not used!
	 *        (this generally wont limit because python objects will
	 *        be inserted in PyLists. Or one of the classes PyReferenceSteal,
	 *        PyReferenceBorrow, PyReferenceCreate will be used instead of
	 *        directly creating a vector of objects.
	 *        Notice that in general objects wont have a default constructor
	 *        so there is no point in having arrays for them.
	 *      - It ensures that the class has a virtual destructor avoiding
	 *         potential memory leaks.
	 *
	 *  Warning: RobinPyObjects are not C++-POD (plain old data), they include
	 *  		a virtual function table and other information. That means that
	 *  		it is not possible to cast them to void* and then cast them back
	 *  		to a different type like type PyObject*. The cast has to be done
	 *  		directly. The following code is undefined and will result in errors:
     *              void *pointer = static_cast<PyObject*>(&myrobinpyobject);
	 *  			PyObject* obj= static_cast<PyObject*>(pointer);
	 *  		However the following is valid:
	 *  			PyObject * obj = static_cast<PyObject*>(&myrobinpyobject);
	 *
	 */
	struct RobinPyObject : public PyObject
	{
		public:
			inline RobinPyObject(PyTypeObject *type) {
				PyObject_Init(this, type);
			}
			static void __dealloc__(PyObject *self);

/*			template <typename T>

			PyNewReference<FunctionObject> do_new() {
				return new
			}*/


		protected:
			inline virtual ~RobinPyObject() =0;

			/*
			 * The pyObjects are destructed through the python reference count mechanism
			 * and not through a call to delete. Thus the operator delete should be made private.
			 * (it is protected because C++ does not allow to call new if delete is private,
			 * now at least subclasses of RobinPyObject can define their own wrappers for new).
			 * It is used only by dealloc.
			 */
			inline void operator delete(void *memory) {
				::operator delete(memory);
			}

		private:


			/*
			 * No array operations on python objects. They will be not implemented
			 */
			void *operator new[](size_t );
			void operator delete[](void *);

	};


	RobinPyObject::~RobinPyObject() {

	}
} // namespace Robin
} // namespace Python

#endif /* ROBIN_FRONTENDS_PYTHON_PYROBINOBJECT_H_ */
