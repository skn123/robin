############################################################
# setup.py
#
# Important notice:
# I only made this file to create a win32 installer. Don't
# expect it to operate well on Unix; the installation method
# there is still 'make install'.
############################################################

from distutils.core import setup, Extension

setup(name='robin',
      version='1.1',
      url='robin.python-hosting.com',
      author='corwin',
      author_email='corwin.amber@gmail.com',
      scripts=['griffin', 'Griffin.jar', 'griffin.bat'],
      py_modules=['robin', 'griffin', 'stl'],
      packages=['robinlib', 'robinlib.html'],
      package_dir={'': 'src/robin/modules'},
      data_files=[('DLLs', ['robin_pyfe-1.1.dll', 'robin_stl-1.1.dll']),
	          ('Scripts/premises', ['premises/antlr.jar',
			  'premises/xmlParserAPIs.jar',
			  'premises/xercesImpl.jar',
			  'premises/jython.jar'])]
      )
