import weakref
import copy_reg


# =============
#  weakref.ref
# =============

def weakref_unpickler(obj):
	# 'obj' is the referenced object (held in a singleton-tuple)
	return weakref.ref(obj)

def weakref_pickler(ref):
	return weakref_unpickler, (ref(),)

copy_reg.pickle(weakref.ReferenceType, weakref_pickler, weakref_unpickler)
