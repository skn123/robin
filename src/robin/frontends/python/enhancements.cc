// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/enhancements.h
 *
 * @par TITLE
 * Python Frontend - Enhancements
 *
 * @par PACKAGE
 * Robin
 */

#include "enhancements.h"

// STL includes
#include <stdexcept>

// Robin includes
#include <robin/reflection/class.h>
#include <robin/reflection/instance.h>
#include "pythonobjects.h"


namespace Robin {

namespace Python {

	namespace {

		PyObject *pyowned(PyObject *ref) {
			Py_XINCREF(ref); return ref;
		}

		bool isFamiliar(PyObject *self) {
			PyObject *type = PyObject_Type(self);
			return ClassObject_Check(type);
		}

		/**
		 * Returns 'self's type as a ClassObject.
		 */
		ClassObject *self_class(PyObject *self) {
			PyObject *type = PyObject_Type(self);
			assert(ClassObject_Check(type));
			return (ClassObject *)type;
		}

		/**
		 * Checks whether a protocol can be invoked on 'self'.
		 */
		bool self_supports(EnhancementsPack::SlotID slot, PyObject *self) {
			// Get the ClassObject
			ClassObject *clas = self_class(self);
			// Get indication
			return clas->getEnhancements().supports(slot);
		}

		/**
		 * Invokes a protocol handler on 'self'.
		 */
		PyObject *self_trigger(EnhancementsPack::SlotID slot,
							   PyObject *self, PyObject *args) {
			// Get the ClassObject
			ClassObject *clas = self_class(self);
			// Trigger enhancement slot
			return
			clas->getEnhancements().trigger(slot, (InstanceObject*)self, args);
		}

		PyObject *safe_self_trigger(EnhancementsPack::SlotID slot,
									PyObject *self, PyObject *args)
		{
			if (self_supports(slot, self)) {
				return self_trigger(slot, self, args);
			}
			else {
				PyErr_SetString(PyExc_AttributeError, 
								"class does not support this protocol");
				return NULL;
			}
		}
		
		/**
		 * A no-op coercion function used as the default nb_coerce for
		 * numeric types.
		 */
		int trivial_coercion(PyObject **v, PyObject **w) {
			Py_XINCREF(*v);
			Py_XINCREF(*w);
			return 0;
		}

		/**
		 * Makes sure a given type is set as a numeric type having an
		 * associated PyNumberMethods structure (initially filled with
		 * nulls).
		 * Also sets a default coercion function which is 
		 * <code>trivial_coercion</code> (a coercion that does nothing).
		 */
		void type_as_number(PyTypeObject *type) {
			if (type->tp_as_number == 0) {
				type->tp_as_number =
					(PyNumberMethods*)malloc(sizeof(PyNumberMethods));
				memset(type->tp_as_number, 0, sizeof(PyNumberMethods));
				type->tp_as_number->nb_coerce = &trivial_coercion;
			}
		}

		/**
		 * Makes sure a given type is set as a sequence type having an
		 * associated PySequenceMethods structure (initially filled with
		 * nulls).
		 */
		void type_as_sequence(PyTypeObject *type) {
			if (type->tp_as_sequence == 0) {
				type->tp_as_sequence = 
					(PySequenceMethods*)malloc(sizeof(PySequenceMethods));
				memset(type->tp_as_sequence, 0, sizeof(PySequenceMethods));
			}
		}

		/**
		 * Makes sure a given type is set as a mapping type having an
		 * associated PyMappingMethods structure (initially filled with
		 * nulls).
		 */
		void type_as_mapping(PyTypeObject *type) {
			if (type->tp_as_mapping == 0) {
				type->tp_as_mapping =
					(PyMappingMethods*)malloc(sizeof(PyMappingMethods));
				memset(type->tp_as_mapping, 0, sizeof(PyMappingMethods));
			}
		}

		struct EnhancementByMethodAssociation
		{
			const char *methodname;
			EnhancementsPack::SlotID slot;
		} implicit_method_enhancements[] = {
			{ ".string", EnhancementsPack::STRING },
			{ ".string", EnhancementsPack::REPR },
			{ "operator()", EnhancementsPack::CALL },
			{ "operator[]", EnhancementsPack::GETITEM },
			{ "operator+", EnhancementsPack::ADD },
			{ "operator-", EnhancementsPack::SUBTRACT },
			{ "operator *", EnhancementsPack::MULTIPLY },
			{ "operator/", EnhancementsPack::DIVIDE },
			{ "operator%", EnhancementsPack::MODULO },
			{ "operator &", EnhancementsPack::BW_AND },
			{ "operator|", EnhancementsPack::BW_OR },
			{ "operator^", EnhancementsPack::BW_XOR },
			{ "operator~", EnhancementsPack::BW_NOT },
			{ "operator<<", EnhancementsPack::LSHIFT },
			{ "operator>>", EnhancementsPack::RSHIFT },
			{ "operator==", EnhancementsPack::EQUALS },
			{ "operator!=", EnhancementsPack::NEQUAL },
			{ "operator<", EnhancementsPack::LESS_THAN },
			{ "operator>", EnhancementsPack::GREATER_THAN },
			{ "operator<=", EnhancementsPack::LESS_OR_EQ },
			{ "operator>=", EnhancementsPack::GREATER_OR_EQ },
			{ "operator int",    EnhancementsPack::TO_INT },
			{ "operator long",   EnhancementsPack::TO_INT },
			{ "operator double", EnhancementsPack::TO_FLOAT },
			{ "length",    EnhancementsPack::LENGTH },
			{ "size",      EnhancementsPack::LENGTH },
			{ 0 }
		};
	}

/**
 * Deploys the PRINT protocol in Python.
 */
class PrintProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type->tp_print = &__print__;
		type->getEnhancements().setSlot(EnhancementsPack::PRINT, handler);
	}

	static int __print__(PyObject *self, FILE *out, int mode) {
		PyObject *args = PyTuple_New(0);
		PyObject *res = self_trigger(EnhancementsPack::PRINT, self, args);
		Py_XDECREF(res);
		return PyErr_Occurred() ? -1 : 0;
	}
};

/**
 * Deploys the STRING protocol in Python.
 */
class StringProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type->tp_str = &__str__;
		type->getEnhancements().setSlot(EnhancementsPack::STRING, handler);
	}

	static PyObject * __str__(PyObject *self) {
		PyObject *args = PyTuple_New(0);
		PyObject *res = self_trigger(EnhancementsPack::STRING, self, args);
		Py_XDECREF(args);
		return res;
	}
};

/**
 * Deploys the REPR protocol in Python.
 */
class ReprProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type->tp_repr = &__repr__;
		type->getEnhancements().setSlot(EnhancementsPack::REPR, handler);
	}

	static PyObject * __repr__(PyObject *self) {
		PyObject *args = PyTuple_New(0);
		PyObject *res = self_trigger(EnhancementsPack::REPR, self, args);
		Py_XDECREF(args);
		return res;
	}
};

/**
 * Deploys the HASH protocol in Python.
 */
class HashProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type->tp_hash = &__hash__;
		type->getEnhancements().setSlot(EnhancementsPack::HASH, handler);
	}

	static long int __hash__(PyObject *self) {
		PyObject *args = PyTuple_New(0);
		PyObject *res = self_trigger(EnhancementsPack::HASH, self, args);
		Py_XDECREF(args);

		// Recursively apply hash() if non-int returned
		long int lhash;
		if (res) {
			lhash = PyInt_Check(res) ? PyInt_AsLong(res) : PyObject_Hash(res);
			Py_XDECREF(res);
		}
		else {
			lhash = -1;
		}	

		return lhash;
	}
};

/**
 * Deploys the CALL protocol in Python.
 */
class CallProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type->tp_call = &__call__;
		type->getEnhancements().setSlot(EnhancementsPack::CALL, handler);
	}

	static PyObject * __call__(PyObject *self, PyObject *args,
							   PyObject *kw) {
		PyObject *res = self_trigger(EnhancementsPack::CALL, self, args);
		return res;
	}
};

/**
 * Deploys the LENGTH protocol in Python.
 */
class LengthProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_length = &__len__;
		type->getEnhancements().setSlot(EnhancementsPack::LENGTH, handler);
	}

	static int __len__(PyObject *self)
	{
		PyObject *args = PyTuple_New(0);
		PyObject *r = self_trigger(EnhancementsPack::LENGTH, self, args);
		Py_DECREF(args);
		if (r == NULL) return -1;
		// r should now be an integer
		if (PyInt_Check(r)) {
			int len = PyInt_AsLong(r);			Py_XDECREF(r);
			return len;
		}
		else {
			PyErr_SetString(PyExc_TypeError, "__len__ implementation returned "
							"non-int.");
			return -1;
		}
	}
};

/**
 * Deploys the GETITEM protocol in Python.
 */
class GetItemProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_item = &__getitem__;
		type->getEnhancements().setSlot(EnhancementsPack::GETITEM, handler);
	}

	static PyObject *__getitem__(PyObject *self, int index)
	{
		// If a LENGTH handler exists, validate index
		if (self_supports(EnhancementsPack::LENGTH, self)) {
			int len = LengthProtocol::__len__(self);
			if (len < 0) return NULL;
			if (index >= len) {
				PyErr_SetString(PyExc_IndexError, "index out of bounds.");
				return NULL;
			}
			else while (index < 0) index += len;
		}
		// Continue invoking the GETITEM handler
		PyObject *args = PyTuple_New(1);
		PyTuple_SET_ITEM(args, 0, PyInt_FromLong(index));
		PyObject *r = self_trigger(EnhancementsPack::GETITEM, self, args);
		Py_DECREF(args);
		return r;
	}
};

/**
 * Deploys the SETITEM protocol in Python.
 */
class SetItemProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_ass_item = &__setdelitem__;
		type->getEnhancements().setSlot(EnhancementsPack::SETITEM, handler);
	}

	static int __setdelitem__(PyObject *self, int index, PyObject *v)
	{
		PyObject *args, *r;
		if (v == NULL) {
			args = PyTuple_New(1);
			PyTuple_SET_ITEM(args, 0, PyInt_FromLong(index));
			r = safe_self_trigger(EnhancementsPack::DELITEM, self, args);
		}
		else {
			args = PyTuple_New(2);
			PyTuple_SET_ITEM(args, 0, PyInt_FromLong(index));
			PyTuple_SET_ITEM(args, 1, pyowned(v));
			r = safe_self_trigger(EnhancementsPack::SETITEM, self, args);
		}
		Py_DECREF(args);
		Py_XDECREF(r);
		return PyErr_Occurred() ? -1 : 0;
	}
};

/**
 * Deploys the DELITEM protocol in Python.
 */
class DelItemProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_ass_item = &SetItemProtocol::__setdelitem__;
		type->getEnhancements().setSlot(EnhancementsPack::DELITEM, handler);
	}
};

/**
 * Deploys the GETSLICE protocol in Python.
 */
class GetSliceProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_slice = &__getslice__;
		type->getEnhancements().setSlot(EnhancementsPack::GETSLICE, handler);
	}

	static inline void normalizeSliceIndex(int len, int& index)
	{
		if (index < 0)
			index = 0;
		else if (index > len)
			index = len;
	}

	static inline void normalizeSlice(int len, int& from, int& to)
	{
		normalizeSliceIndex(len, from);
		normalizeSliceIndex(len, to);
		if (from > to) from = to;
	}

	static PyObject *__getslice__(PyObject *self, int from, int to)
	{
		// If a LENGTH handler exists, validate indices
		if (self_supports(EnhancementsPack::LENGTH, self)) {
			int len = LengthProtocol::__len__(self);
			if (len < 0) return NULL;
			normalizeSlice(len, from, to);
		}
		// Continue invoking the GETITEM handler
		PyObject *args = PyTuple_New(2);
		PyTuple_SET_ITEM(args, 0, PyInt_FromLong(from));
		PyTuple_SET_ITEM(args, 1, PyInt_FromLong(to));
		PyObject *r = self_trigger(EnhancementsPack::GETSLICE, self, args);
		Py_DECREF(args);
		return r;
	}
};

/**
 * Deploys the SETSLICE protocol in Python.
 */
class SetSliceProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_ass_slice = &__setdelslice__;
		type->getEnhancements().setSlot(EnhancementsPack::SETSLICE, handler);
	}

	static int __setdelslice__(PyObject *self, int from, int to, PyObject *v)
	{
		// If a LENGTH handler exists, validate indices
		if (self_supports(EnhancementsPack::LENGTH, self)) {
			int len = LengthProtocol::__len__(self);
			if (len < 0) return -1;
			GetSliceProtocol::normalizeSlice(len, from, to);
		}
		// Continue invoking the SETITEM handler
		PyObject *args, *r;
		if (v == NULL) {
			args = PyTuple_New(2);
			PyTuple_SET_ITEM(args, 0, PyInt_FromLong(from));
			PyTuple_SET_ITEM(args, 1, PyInt_FromLong(to));
			r = self_trigger(EnhancementsPack::DELSLICE, self, args);
		}
		else {
			args = PyTuple_New(3);
			PyTuple_SET_ITEM(args, 0, PyInt_FromLong(from));
			PyTuple_SET_ITEM(args, 1, PyInt_FromLong(to));
			PyTuple_SET_ITEM(args, 2, pyowned(v));
			r = self_trigger(EnhancementsPack::SETSLICE, self, args);
		}
		Py_DECREF(args);
		Py_XDECREF(r);
		return PyErr_Occurred() ? -1 : 0;
	}
};

/**
 * Deploys the DELSLICE protocol in Python.
 */
class DelSliceProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_sequence(type);
		type->tp_as_sequence->sq_ass_slice =
			&SetSliceProtocol::__setdelslice__;
		type->getEnhancements().setSlot(EnhancementsPack::DELSLICE, handler);
	}
};

/**
 * Base class for binary-operation based protocols.
 */
class BinaryOperationProtocol : public Protocol
{
protected:
	static PyObject *binaryOperation(EnhancementsPack::SlotID slot,
									 PyObject *operand1, PyObject *operand2)
	{
		// Reverse arguments if needed
		if (!isFamiliar(operand1)) {
			std::swap(operand1, operand2);
			slot = (SlotID)(slot + 1); // - it is assumed that R_xxx = xxx + 1
		}
		// Invoke appropriate slot
		PyObject *args = PyTuple_New(1);
		PyTuple_SET_ITEM(args, 0, pyowned(operand2));
		PyObject *r = self_trigger(slot, operand1, args);
		Py_DECREF(args);
		return r;
	}
};

/**
 * Base class for unary-operation based protocols.
 */
class UnaryOperationProtocol : public Protocol
{
protected:
	static PyObject *unaryOperation(EnhancementsPack::SlotID slot,
									PyObject *operand)
	{
		PyObject *args = PyTuple_New(0);
		PyObject *r = self_trigger(slot, operand, args);
		Py_DECREF(args);
		return r;
	}
};

/**
 * Deploys the numeric ADD protocol.
 */
class AddProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_add = &__add__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__add__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::ADD, self, other);
	}
};

/**
 * Deploys the numeric SUBTRACT protocol.
 */
class SubtractProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_subtract = &__sub__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__sub__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::SUBTRACT, self, other);
	}
};

/**
 * Deploys the numeric MULTIPLY protocol.
 */
class MultiplyProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_multiply = &__mul__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__mul__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::MULTIPLY, self, other);
	}
};

/**
 * Deploys the numeric DIVIDE protocol.
 */
class DivideProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_divide = &__div__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__div__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::DIVIDE, self, other);
	}
};

/**
 * Deploys the numeric MODULO protocol.
 */
class ModuloProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_remainder = &__mod__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__mod__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::MODULO, self, other);
	}
};

/**
 * Deploys the numeric POWER protocol.
 */
class PowerProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_power = &__pow__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__pow__(PyObject *self, PyObject *other, PyObject *mod)
	{
		return binaryOperation(EnhancementsPack::POWER, self, other);
	}
};

/**
 * Deploys the numeric bitwise AND protocol.
 */
class BitwiseAndProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_and = &__and__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__and__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::BW_AND, self, other);
	}
};

/**
 * Deploys the numeric bitwise OR protocol.
 */
class BitwiseOrProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_or = &__or__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__or__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::BW_OR, self, other);
	}
};

/**
 * Deploys the numeric bitwise XOR protocol.
 */
class BitwiseXorProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_xor = &__xor__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__xor__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::BW_XOR, self, other);
	}
};

/**
 * Deploys the numeric bitwise NOT protocol.
 */
class BitwiseNotProtocol : public UnaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_invert = &__not__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__not__(PyObject *self)
	{
		return unaryOperation(EnhancementsPack::BW_NOT, self);
	}
};

/**
 * Deploys the numeric LSHIFT protocol.
 */
class LShiftProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_lshift = &__lshift__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__lshift__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::LSHIFT, self, other);
	}
};

/**
 * Deploys the numeric RSHIFT protocol.
 */
class RShiftProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_rshift = &__rshift__;
		type->getEnhancements().setSlot(slot, handler);
	}

	static PyObject *__rshift__(PyObject *self, PyObject *other)
	{
		return binaryOperation(EnhancementsPack::RSHIFT, self, other);
	}
};

/**
 * Deploys the numeric OCT protocol in Python.
 */
class OctProtocol : public UnaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_oct = &__oct__;
		type->getEnhancements().setSlot(EnhancementsPack::TO_OCT, handler);
	}

	static PyObject* __oct__(PyObject *self) {
		return unaryOperation(EnhancementsPack::TO_OCT, self);
	}
};

/**
 * Deploys the numeric HEX protocol in Python.
 */
class HexProtocol : public UnaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_hex = &__hex__;
		type->getEnhancements().setSlot(EnhancementsPack::TO_HEX, handler);
	}

	static PyObject* __hex__(PyObject *self) {
		return unaryOperation(EnhancementsPack::TO_HEX, self);
	}
};

/**
 * Deploys the numeric INT protocol.
 */
class IntProtocol : public UnaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_int = &__int__;
		type->getEnhancements().setSlot(EnhancementsPack::TO_INT, handler);
	}

	static PyObject *__int__(PyObject *self)
	{
		return unaryOperation(EnhancementsPack::TO_INT, self);
	}
};

/**
 * Deploys the numeric FLOAT protocol.
 */
class FloatProtocol : public UnaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_number(type);
		type->tp_as_number->nb_float = &__float__;
		type->getEnhancements().setSlot(EnhancementsPack::TO_FLOAT, handler);
	}

	static PyObject *__float__(PyObject *self)
	{
		return unaryOperation(EnhancementsPack::TO_FLOAT, self);
	}
};

/**
 * Deploys the mapping GETSUBSCRIPT protocol
 */
class GetSubscriptProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_mapping(type);
		type->tp_as_mapping->mp_subscript = &__getsubscript__;
		type->getEnhancements().setSlot(EnhancementsPack::GETSUBSCRIPT,
										handler);
	}

	static PyObject *__getsubscript__(PyObject *self, PyObject *subscript)
	{
		return binaryOperation(EnhancementsPack::GETSUBSCRIPT, 
							   self, subscript);
	}
};

/**
 * Deploys the mapping SETSUBSCRIPT protocol.
 */
class SetSubscriptProtocol : public BinaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_mapping(type);
		type->tp_as_mapping->mp_ass_subscript = &__setdelsubscript__;
		type->getEnhancements().setSlot(EnhancementsPack::SETSUBSCRIPT,
										handler);
	}

	static int __setdelsubscript__(PyObject *self, PyObject *subscript,
									  PyObject *value)
	{
		PyObject *r, *args;

		if (value == NULL) {
			args = PyTuple_New(1);
			PyTuple_SET_ITEM(args, 0, pyowned(subscript));
			r = safe_self_trigger(EnhancementsPack::DELSUBSCRIPT, self, args);
		}
		else {
			args = PyTuple_New(2);
			PyTuple_SET_ITEM(args, 0, pyowned(subscript));
			PyTuple_SET_ITEM(args, 1, pyowned(value));
			r = safe_self_trigger(EnhancementsPack::SETSUBSCRIPT, self, args);
		}

		Py_DECREF(args);
		if (r) {
			Py_DECREF(r);
			return 0;
		}
		else {
			return -1;
		}
	}
};

/**
 * Deploys the mapping DELSUBSCRIPT protocol.
 */
class DelSubscriptProtocol : public Protocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_mapping(type);
		type->tp_as_mapping->mp_ass_subscript =
			&SetSubscriptProtocol::__setdelsubscript__;
		type->getEnhancements().setSlot(EnhancementsPack::DELSUBSCRIPT,
										handler);
	}
};

/**
 * Deploys the MAPSIZE protocol for mapping objects.
 */
class MapSizeProtocol : public UnaryOperationProtocol
{
public:
	virtual void deploy(ClassObject *type, SlotID slot, Handler handler) {
		type_as_mapping(type);
		type->tp_as_mapping->mp_length = &__mapsize__;
		type->getEnhancements().setSlot(EnhancementsPack::MAPSIZE, handler);
	}

	static int __mapsize__(PyObject *self)
	{
		PyObject *pysize = unaryOperation(EnhancementsPack::MAPSIZE, self);
		if (pysize) {
			long size = PyInt_AsLong(pysize);
			Py_DECREF(pysize);
			return size;
		}
		else {
			return -1;
		}
	}
};


class RichCompareProtocol : public BinaryOperationProtocol
{
public:
	/**
	 * Choose slot to be one of:
	 * <ul><li>LESS_THAN</li>   <li>GREATER_THAN</li>
	 *     <li>EQUALS</li>      <li>NEQUAL</li>
	 * </ul>
	 */
	RichCompareProtocol(EnhancementsPack::SlotID slot) : m_slotid(slot) { }

	virtual void deploy(ClassObject *type, SlotID slot, Handler handler)
	{
		type->tp_flags |= Py_TPFLAGS_HAVE_RICHCOMPARE;
		type->tp_richcompare = &__richcmp__;
		type->getEnhancements().setSlot(m_slotid, handler);
	}

	static EnhancementsPack::SlotID complement(EnhancementsPack::SlotID slot)
	{
		switch (slot) {
		case EnhancementsPack::EQUALS: return EnhancementsPack::NEQUAL;
		case EnhancementsPack::NEQUAL: return EnhancementsPack::EQUALS;
		case EnhancementsPack::LESS_THAN:
			return EnhancementsPack::GREATER_OR_EQ;
		case EnhancementsPack::GREATER_THAN: 
			return EnhancementsPack::LESS_OR_EQ;
		case EnhancementsPack::LESS_OR_EQ: 
			return EnhancementsPack::GREATER_THAN;
		case EnhancementsPack::GREATER_OR_EQ:
			return EnhancementsPack::LESS_THAN;
		default:
			return slot;
		}
	}

	static EnhancementsPack::SlotID symmetry(EnhancementsPack::SlotID slot)
	{
		switch (slot) {
		case EnhancementsPack::LESS_THAN:
			return EnhancementsPack::GREATER_THAN;
		case EnhancementsPack::GREATER_THAN: 
			return EnhancementsPack::LESS_THAN;
		case EnhancementsPack::LESS_OR_EQ: 
			return EnhancementsPack::GREATER_OR_EQ;
		case EnhancementsPack::GREATER_OR_EQ:
			return EnhancementsPack::LESS_OR_EQ;
		default:
			return slot;
		}
	}

	static PyObject *__richcmp__(PyObject *self, PyObject *other,
								 int opid)
	{
		if (PyObject_Type(self) != PyObject_Type(other))
			return pyowned(Py_NotImplemented);

		EnhancementsPack::SlotID slot;
		// Deduce slot number from opid
		switch (opid) {
		case Py_EQ: slot = EnhancementsPack::EQUALS;        break;
		case Py_NE: slot = EnhancementsPack::NEQUAL;        break;
		case Py_GT: slot = EnhancementsPack::GREATER_THAN;  break;
		case Py_LT: slot = EnhancementsPack::LESS_THAN;     break;
		case Py_GE: slot = EnhancementsPack::GREATER_OR_EQ; break;
		case Py_LE: slot = EnhancementsPack::LESS_OR_EQ;    break;
		default:
			return pyowned(Py_NotImplemented);
		}
		// Invoke binary operation
		PyObject *result = NULL;
		if (self_supports(slot, self))
			result = binaryOperation(slot, self, other);
		else if (self_supports(complement(slot), self))
			result = binaryOperation(complement(slot), self, other);
		else if (isFamiliar(other) && self_supports(symmetry(slot), other))
			result = binaryOperation(symmetry(slot), other, self);
		else
			return pyowned(Py_NotImplemented);

		return result;
	}

private:
	const EnhancementsPack::SlotID m_slotid;
};

/**
 * EnhancementsPack constructor.
 */
EnhancementsPack::EnhancementsPack()
{
}

/**
 * Sets the required action for a specified protocol slot.
 */
void EnhancementsPack::setSlot(SlotID slot, EnhancementsPack::Handler action)
{
	m_slots[slot] = action;
}

/**
 * Checks whether the current pack provides a handler for the requested slot.
 */
bool EnhancementsPack::supports(SlotID slot) const
{
	return (m_slots[slot]);
}

/**
 * Invokes a protocol handler stored in this pack. The slot indicated by
 * 'slot' must have been previously assigned a handler by setSlot.
 */
PyObject *EnhancementsPack::trigger(SlotID slot, InstanceObject *self,
									PyObject *args)
{
	Handler action = m_slots[slot];
	if (action) {
		return action->callUpon(self, args);
	}
	else {
		PyErr_Format(PyExc_TypeError,
					 "protocol handler not defined for '%s'",
					 self->getUnderlying()->getClass()->name().c_str());
		return 0;
	}
}

/**
 * Returns an enumerated slot identifier by analyzing a text literal
 * describing the slot.
 */
EnhancementsPack::SlotID EnhancementsPack::slotByName(const std::string& 
													  slotname)
{
	if      (slotname == "__print__")        return PRINT;
	else if (slotname == "__str__")          return STRING;
	else if (slotname == "__repr__")         return REPR;
	else if (slotname == "__hash__")         return HASH;
	else if (slotname == "__call__")         return CALL;
	else if (slotname == "__len__")          return LENGTH;
	else if (slotname == "__getitem__")      return GETITEM;
	else if (slotname == "__setitem__")      return SETITEM;
	else if (slotname == "__delitem__")      return DELITEM;
	else if (slotname == "__getslice__")     return GETSLICE;
	else if (slotname == "__setslice__")     return SETSLICE;
	else if (slotname == "__delslice__")     return DELSLICE;
	else if (slotname == "__add__")          return ADD;
	else if (slotname == "__sub__")          return SUBTRACT;
	else if (slotname == "__mul__")          return MULTIPLY;
	else if (slotname == "__div__")          return DIVIDE;
	else if (slotname == "__mod__")          return MODULO;
	else if (slotname == "__pow__")          return POWER;
	else if (slotname == "__and__")          return BW_AND;
	else if (slotname == "__or__")           return BW_OR;
	else if (slotname == "__xor__")          return BW_XOR;
	else if (slotname == "__not__")          return BW_NOT;
	else if (slotname == "__lshift__")       return LSHIFT;
	else if (slotname == "__rshift__")       return RSHIFT;
	else if (slotname == "__oct__")          return TO_OCT;
	else if (slotname == "__hex__")          return TO_HEX;
	else if (slotname == "__int__")          return TO_INT;
	else if (slotname == "__float__")        return TO_FLOAT;
	else if (slotname == "__getsubscript__") return GETSUBSCRIPT;
	else if (slotname == "__setsubscript__") return SETSUBSCRIPT;
	else if (slotname == "__delsubscript__") return DELSUBSCRIPT;
	else if (slotname == "__mapsize__")      return MAPSIZE;
	else if (slotname == "__eq__")           return EQUALS;
	else if (slotname == "__ne__")           return NEQUAL;
	else if (slotname == "__lt__")           return LESS_THAN;
	else if (slotname == "__gt__")           return GREATER_THAN;
	else if (slotname == "__le__")           return LESS_OR_EQ;
	else if (slotname == "__ge__")           return GREATER_OR_EQ;
	// - R_xxx
	else if (slotname == "__radd__")         return R_ADD;
	else if (slotname == "__rsub__")         return R_SUBTRACT;
	else if (slotname == "__rmul__")         return R_MULTIPLY;
	else if (slotname == "__rdiv__")         return R_DIVIDE;
	else if (slotname == "__rmod__")         return R_MODULO;
	else if (slotname == "__rpow__")         return R_POWER;
	else if (slotname == "__rand__")         return R_BW_AND;
	else if (slotname == "__ror__")          return R_BW_OR;
	else if (slotname == "__rxor__")         return R_BW_XOR;
	else if (slotname == "__rlshift__")      return R_LSHIFT;
	else if (slotname == "__rrshift__")      return R_RSHIFT;
	else if (slotname == "__req__")          return R_EQUALS;
	else if (slotname == "__rne__")          return R_NEQUAL;
	else if (slotname == "__rlt__")          return R_LESS_THAN;
	else if (slotname == "__rgt__")          return R_GREATER_THAN;
	else if (slotname == "__rle__")          return R_LESS_OR_EQ;
	else if (slotname == "__rge__")          return R_GREATER_OR_EQ;
	else 
		return NSLOTS;
}

/**
 * Build a functor that uses an instance method of the given instance
 * when called.
 * @param methodname the name of the instance method to invoke
 */
InstanceMethodFunctor::InstanceMethodFunctor(const std::string& methodname)
	: m_methodname(methodname)
{
}

/**
 * Invoke an instance method with given arguments.
 */
PyObject *InstanceMethodFunctor::callUpon(InstanceObject *self,
										  PyObject *args)
{
	PyObject *pymeth = 
		PyObject_GetAttrString(self, (char*)m_methodname.c_str());
	if (pymeth == NULL) return NULL;
	PyObject *result = PyObject_Call(pymeth, args, NULL);
	Py_DECREF(pymeth);
	return result;
}

/**
 * Builds a functor using a Python object. This object must be callable, i.e.
 * it must support Python's CALL protocol - a function or a bound method are
 * common cases.
 */
PyObjectNativeFunctor::PyObjectNativeFunctor(PyObject *pycallable)
	: m_pycallable(pycallable)
{
	Py_XINCREF(m_pycallable);
}

PyObjectNativeFunctor::~PyObjectNativeFunctor()
{
	Py_XDECREF(m_pycallable);
}

/**
 * Implements invocation by forwarding call to held PyObject. The instance
 * passed is set as the first arguments and the rest of the arguments are
 * shifted one place to the right.
 */
PyObject *PyObjectNativeFunctor::callUpon(InstanceObject *self, PyObject *args)
{
	PyObject *fwdargs = PyTuple_New(1 + PyTuple_Size(args));
	// Set 'self' instance as first argument in fwdargs
	PyTuple_SET_ITEM(fwdargs, 0, pyowned(self));
	// Copy the rest of the arguments
	for (int argindex = 0; argindex < PyTuple_Size(args); ++argindex) {
		PyObject *arg = PyTuple_GET_ITEM(args, argindex);
		PyTuple_SET_ITEM(fwdargs, argindex + 1, pyowned(arg));
	}

	PyObject *result = PyObject_CallObject(m_pycallable, fwdargs);
	Py_DECREF(fwdargs);
	return result;
}

/**
 * A factory routine that returns a Protocol instance for deploying the
 * given slot.
 */
Protocol& Protocol::deployerBySlot(EnhancementsPack::SlotID slot)
{
	static PrintProtocol        print;
	static StringProtocol       str;
	static ReprProtocol         repr;
	static HashProtocol         hash;
	static CallProtocol         call;
	static LengthProtocol       len;
	static GetItemProtocol      getitem;
	static SetItemProtocol      setitem;
	static DelItemProtocol      delitem;
	static GetSliceProtocol     getslice;
	static SetSliceProtocol     setslice;
	static DelSliceProtocol     delslice;
	static AddProtocol          add;
	static SubtractProtocol     sub;
	static MultiplyProtocol     mul;
	static DivideProtocol       div;
	static ModuloProtocol       mod;
	static PowerProtocol        pow;
	static BitwiseAndProtocol   bwand;
	static BitwiseOrProtocol    bwor;
	static BitwiseXorProtocol   bwxor;
	static BitwiseNotProtocol   bwnot;
	static LShiftProtocol       lshift;
	static RShiftProtocol       rshift;
	static OctProtocol          oct;
	static HexProtocol          hex;
	static IntProtocol          cint;
	static FloatProtocol        cfloat;
	static GetSubscriptProtocol getsubscript;
	static SetSubscriptProtocol setsubscript;
	static DelSubscriptProtocol delsubscript;
	static MapSizeProtocol      mapsize;
	static RichCompareProtocol  eq = (EnhancementsPack::EQUALS);
	static RichCompareProtocol  ne = (EnhancementsPack::NEQUAL);
	static RichCompareProtocol  lt = (EnhancementsPack::LESS_THAN);
	static RichCompareProtocol  gt = (EnhancementsPack::GREATER_THAN);
	static RichCompareProtocol  le = (EnhancementsPack::LESS_OR_EQ);
	static RichCompareProtocol  ge = (EnhancementsPack::GREATER_OR_EQ);

	// - binary operation protocols are listed twice for the R_xxx version
	static Protocol *slots[EnhancementsPack::NSLOTS] =
		{ &print, &str, &repr, &hash, &call,
		  &len, &getitem, &setitem, &delitem, &getslice, &setslice, &delslice,
		  &add, &add, &sub, &sub, &mul, &mul, &div, &div, &mod, &mod, 
		  &pow, &pow,
		  &bwand, &bwand, &bwor, &bwor, &bwxor, &bwxor, &bwnot,
		  &lshift, &lshift, &rshift, &rshift, &oct, &hex, &cint, &cfloat,
		  &getsubscript, &setsubscript, &delsubscript, &mapsize,
		  &eq, &eq, &ne, &ne, &lt, &lt, &gt, &gt, &le, &le, &ge, &ge
		};

	return *(slots[slot]);}
	

/**
 * A factory routine that returns a Protocol instance for deploying the
 * given slot. The slot is given by a string literal.
 */
Protocol& Protocol::deployerBySlotName(const std::string& slotname)
{
	EnhancementsPack::SlotID slot = EnhancementsPack::slotByName(slotname);

	if (slot == EnhancementsPack::NSLOTS)
		throw EnhancementsPack::NoSuchSlotException();

	return deployerBySlot(slot);
}

/**
 * Applies automatic enhancements to a class by "guessing" the intended
 * protocols from method names.
 */
void Protocol::autoEnhance(ClassObject *type)
{
	EnhancementByMethodAssociation *enh;
	for (enh = implicit_method_enhancements; enh->methodname != 0; ++enh) {
		if (type->getUnderlying()->hasInstanceMethod(enh->methodname)) {
			// Create a handler which operates by invoking the specified
			// instance method
			Handle<PyCallableWithInstance> handler
				(new InstanceMethodFunctor(enh->methodname));
			// Deploy handle to specified slot
			deployerBySlot(enh->slot).deploy(type, enh->slot, handler);
		}
	}
}


/**
 * PythonConversion constructor. The 'functor' given here is stored and
 * invoked when the conversion is applied.
 */
PythonConversion::PythonConversion(Handle<PyCallableWithInstance> functor)
	: m_functor(functor)
{
}

scripting_element PythonConversion::apply(scripting_element value) const
{
	PyObject *args = PyTuple_New(0);
	PyObject *result = m_functor->callUpon((InstanceObject*)value, args);
	Py_DECREF(args);
	if (!result)
		reportConversionError();

	return result;
}

/**
 * Throws std::runtime_error with a description of the Python error that
 * just occurred.
 */
void PythonConversion::reportConversionError() const
{
	// Ouch! Exception raised by conversion function.
	// Translate Python error into a C++ exception.
	PyObject *exc_type, *exc_value, *exc_tb;
	PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
	PyObject *exc_type_repr = PyObject_GetAttrString(exc_type, "__name__");
	PyObject *exc_value_repr = PyObject_Str(exc_value);
	// - format an error string
	std::string error_string = "Python conversion failed - ";
	error_string += PyString_AsString(exc_type_repr);
	(error_string += ": ") += PyString_AsString(exc_value_repr);
	// - cleanup
	Py_XDECREF(exc_type); Py_XDECREF(exc_value); Py_XDECREF(exc_tb);
	Py_XDECREF(exc_type_repr); Py_XDECREF(exc_value_repr);
	PyErr_Clear();
	// - throw
	throw std::runtime_error(error_string);
}


/**
 * PythonConversionWithWeigher constructor. The 'functor' is the same as
 * in PythonConversion, the 'weigher' is stored and invoked when the weight
 * is requested based on the insight.
 */
PythonConversionWithWeigher::
PythonConversionWithWeigher(Handle<PyCallableWithInstance> functor,
							PyObject *weigher)
	: PythonConversion(functor), m_weigher(weigher)
{
	Py_XINCREF(m_weigher);
}

PythonConversionWithWeigher::~PythonConversionWithWeigher()
{
	Py_XDECREF(m_weigher);
}

/**
 * Calculates the weight by calling 
 */
Conversion::Weight PythonConversionWithWeigher::weight(Insight insight) const
{
	PyObject *pyinsight = reinterpret_cast<PyObject *>(insight.i_ptr);
	if (!pyinsight)
		return weight();

	// Invoke the weigher function
	PyObject *result = PyObject_CallFunction(m_weigher, "O", pyinsight);
	if (result) {
		if (PyTuple_Check(result) && PyTuple_Size(result) == 4) {
			// Break the tuple into 4 integral elements
			int eps, prom, up, user;
			if (PyArg_ParseTuple(result, "iiii", &eps, &prom, &up, &user)) {
				Py_XDECREF(result);
				Conversion::Weight cweight(eps, prom, up, user);
				return cweight;
			}
			else {
				Py_XDECREF(result);
				reportConversionError();
			}
		}
		else if (result == Py_None) {
			return weight();
		}
		throw std::runtime_error("invalid value returned by weigher");
	}
	else {
		reportConversionError();
	}	

	return weight();
}


} // end of namespace Python

} // end of namespace Robin

