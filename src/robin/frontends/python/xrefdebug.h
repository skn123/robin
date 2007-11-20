// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-
// vim:tw=80:ts=4:sw=4

#ifndef XREFDEBUG_H_1d357dfac9
#define XREFDEBUG_H_1d357dfac9

#ifdef XREFDEBUG

#include <stdio.h>

template<typename T>
const char* debug_Py_GetType(T *obj) {
    const char *name;
    if(PyType_Check(obj)) {
        name = ((PyTypeObject *)obj)->tp_name;
    } else if(PyClass_Check(obj)) {
        name = ((PyStringObject *)(((PyClassObject*)obj)->cl_name))->ob_sval;
    } else {
        name = obj->ob_type->tp_name;
    }
    return name;
}


template<typename T>
int debug_Py_INCREF(char *file, int line, T *obj) {
    fprintf(stderr, "%s:%d\t", file, line);
    fprintf(stderr, "%s at 0x%08x refcnt increased, old: %d, new: %d\n", debug_Py_GetType(obj), obj, obj->ob_refcnt, obj->ob_refcnt+1);
    return obj->ob_refcnt++;
}

template<typename T>
void debug_Py_DECREF(char *file, int line, T *obj) {
    fprintf(stderr, "%s:%d\t", file, line);
    fprintf(stderr, "%s at 0x%08x refcnt decreased, old: %d, new: %d\n", debug_Py_GetType(obj), obj, obj->ob_refcnt, obj->ob_refcnt-1);
    if(--(obj->ob_refcnt) == 0) {
        _Py_Dealloc((PyObject *)obj);
    }
}
    



#define Py_INCREF(op) debug_Py_INCREF(__FILE__, __LINE__, (op))
#define Py_DECREF(op) debug_Py_DECREF(__FILE__, __LINE__, (op))

#define Py_XINCREF(op) if ((op) == NULL) ; else Py_INCREF(op)
#define Py_XDECREF(op) if ((op) == NULL) ; else Py_DECREF(op)
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None

#endif


#endif


