#define NO_IMPORT_ARRAY

#include <assert.h>
#include <robin/frontends/python/facade.h>
#include <robin/frontends/frontend.h>
#include <Numeric/arrayobject.h>

struct RegData { const char *name; const char *type; RegData *i; void *sym; };
struct PascalString { unsigned long size; const char *chars; char buffer[1]; };

#define F (void*)&

void **PyArray_API;



namespace {
	Handle<Robin::Class> robin_PyArrayObject;
}

/**
 * A passive translator for detecting Numeric.array objects.
 */
class NumericTranslator : public Robin::UserDefinedTranslator
{
public:
	Handle<Robin::TypeOfArgument> detectType(Robin::scripting_element element)
	{
		if (PyArray_Check((PyObject*)element))
			return robin_PyArrayObject->getRefArg();
		else
			return Handle<Robin::TypeOfArgument>();
	}
};

/**
 * An adapter for passing Numeric.array objects to functions and receiving
 * such objects from them.
 */
class NumericAdapter : public Robin::Adapter
{
public:
	Robin::scripting_element get(Robin::basic_block data)
	{
		return reinterpret_cast<Robin::scripting_element&>(data);
	}

	void put(Robin::ArgumentsBuffer& argsbuf, Robin::scripting_element value)
	{
		argsbuf.pushPointer((PyArrayObject*)value);
	}
};


/**
 * Initializes the numpy module.
 */
void numpy_engage()
{
	PyInterpreterState *interp = PyInterpreterState_Head();
	PyThreadState *thread = PyInterpreterState_ThreadHead(interp);

	PyEval_RestoreThread(thread);

	import_array();

	robin_PyArrayObject = Robin::Python::Facade::asClass("PyArrayObject");
	assert(robin_PyArrayObject);

	Handle<Robin::Adapter> adapter(new NumericAdapter);

	robin_PyArrayObject->getPtrArg()->assignAdapter(adapter);
	robin_PyArrayObject->getRefArg()->assignAdapter(adapter);

	// Register a numeric translator
	Handle<Robin::UserDefinedTranslator> udt(new NumericTranslator);
	Robin::Python::Facade::userDefined(udt);

	PyEval_SaveThread();
}


/**
 * A simple test case.
 */
PyArrayObject *pytest(PyArrayObject *array)
{
	fprintf(stderr, "Here I am!\n");
	fprintf(stderr, "PyArray_API = %p\n", PyArray_API);

	PyObject *arrayobj = (PyObject*)array;
	char *data;
	int len;

	if (PyArray_As1D(&arrayobj, &data, &len, PyArray_INT) == 0) {
		fprintf(stderr, "// Array length = %d\n", len);
		for (int i = 0; i < len; ++i) {
			fprintf(stderr, "// [%d] = %d\n", i, ((int*)data)[i]);
		}
	}
	else {
		fprintf(stderr, "// OUCH! Numeric array error.\n");
	}

	// Now create an array
	int dims[] = { 6 };
	return (PyArrayObject*)PyArray_FromDims(1, dims, PyArray_INT); 
}

RegData test_proto[] = {
	{ "array", "*PyArrayObject", 0, 0 },
	{ 0 }
};


// - an empty class
RegData object_proto[] = {
	{ 0 }
};


#ifdef _WIN32
extern "C"
__declspec(dllexport)
#endif
RegData entry[] = {
	{ "engage", "void", 0, F numpy_engage },
	{ "PyArrayObject", "class", object_proto, 0 },
	{ "test", "*PyArrayObject", test_proto, F pytest },
	{ 0 }
};
