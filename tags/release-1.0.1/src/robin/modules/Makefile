modules: lib/$(MACHINE)/stl_robin.so lib/$(MACHINE)/numpy_robin.so

include ../../../pythonapi.mak

lib/$(MACHINE)/stl_robin.so: stl/stl_robin.cc
	mkdir -p lib/$(MACHINE)
	$(LDSHARED) -o $@ stl/stl_robin.cc

lib/$(MACHINE)/numpy_robin.so: numpy/numpy_robin.cc
	mkdir -p lib/$(MACHINE)
	$(LDSHARED) -g -o $@ numpy/numpy_robin.cc -I../.. $(PYTHONAPI_VERSION_2.2|CXXFLAGS) -L../../../lib/DEBUG/$(MACHINE) -lrobin_pyfe


include ../../../compilers.mak


CXX := $(CXX!$(MACHINE))
CC := $(CC!$(MACHINE))
LD := $(LD!$(MACHINE))
LDSHARED := $(LDSHARED!$(MACHINE))


#======================================================================
# Installation
#======================================================================
define nl
$(empty)
$(empty)
endef

[install-at] := $([install-at])/modules

[install-libs] = stl_robin.so numpy_robin.so
[install-libs-from] = ${foreach MODEL, $([install-models]), \
    ${foreach MACHINE, $(platform), \
     ${foreach lib, $([install-libs]), lib/$(MACHINE)/$(lib)}}}
[install-libs-to] = $([install-at])/lib/$(platform)
[install-libs-cmd] = mkdirhier $([install-libs-to]) && \
  install $([install-libs-from]) $([install-libs-to])

[install-py-to] = $([install-at])
[install-py-from] = stl.pyc numpy.pyc robinhelp.pyc document.pyc \
                    pickle_weakref.pyc
[install-pyhtml-from] = \
                    html/__init__.pyc html/textformat.pyc
[install-py-cmd] = mkdirhier $([install-py-to]) && \
  install $([install-py-from]) $([install-py-to]) && \
  mkdirhier $([install-py-to])/html && \
  install $([install-pyhtml-from]) $([install-py-to])/html


install:
	@echo "=================================================="
	@echo " Modules"
	@echo "=================================================="
	@python -Sc "import cpy" stl.py robinhelp.py document.py pickle_weakref.py\
                             html/__init__.py html/textformat.py
	$([install-py-cmd])
	@${foreach platform, $([install-platforms]), $([install-libs-cmd]) && \
		echo Installed $(platform)$(nl)}

