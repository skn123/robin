#include <xparam_extend.h>
#include <robin/frontends/python/python_xparam.h>

using namespace xParam;


// ----------------------------------------------------------------------

class El { };

enum Em { EM };

class DataMembers
{
public:
	DataMembers(long factor) : square(factor * factor), m(EM) { }

	int square;
	El e;
	Em m;

	static const int zero;

private:
	int hidden;
};

// ----------------------------------------------------------------------

class print_DataMembers
{
public:
	static ValueList sub_objects(const DataMembers& val) {
		ValueList pi;
		pi << Val(val.square);
		return pi;
	}
};


extern "C"
void initwith_xparam()
{
	static PyMethodDef methods[] = {
		{ 0 }
	};

	// Register xParam types
	param_class<DataMembers>("DataMembers");
	param_ctor(TypeTag<DataMembers>(), ByVal<long>("factor"));
	param_output<DataMembers, print_DataMembers>();

	// Register xParam types with Robin
	xParam::registerPythonType(
		xParam_internal::Handle<xParam::PythonConverter>(
		    new Robin::Python::xParamConverter<DataMembers>("DataMembers", 0,0)
		)
	);

	// Register Python module
	PyObject *module =
		Py_InitModule4("with_xparam", methods,
					   "Robin's xParam Integration Testing Module",
					   NULL, PYTHON_API_VERSION);

}
