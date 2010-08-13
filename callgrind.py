def load(file):
    import robin_v1_1, os.path, __builtin__
    if os.path.islink(file): file = os.readlink(file)
    here = os.path.dirname(file)
    machine = os.getenv("MACHINE")
    libtemplate = "%(sopre)s%%s%(platspec)s-%(ver)s%(pyspec)s%(soext)s"
    lib = libtemplate % robin_v1_1.__dict__ % "robin_callgrind"
    robin_v1_1.loadLibrary(__name__, lib)

load(__file__)
toggleCollect.__doc__ = \
"""
It instructs callgrind to start or stop collecting data about the function run.
It only has effects if python is running under callgrind.

Usually callgrind will start collecting at the beggining of the run unless you call it with
with the parameter '--collect-atstart=no'
"""

startInstrumentation.__doc__ = \
"""
Starts instrumentating the binary code of this process by callgrind.
It only has effects if the process is running under  callgrind.
When the process is not instrumentated it runs as if it was normally running
on valgrind without any tool (dummy tool) , at the moment it is instrumentated
valgrind adds code that checks and registers more information for callgrind.

Notice that startInstrumentation is different from toggleCollect, because a program
can still be instrumentated but the data is not being stored. Instrumentation is the 
most important factor that affects speed, while collecting only affects the final reports.

WARNING: Callgrind is less exact if part of the program was not instrumentated, to know
deeply how this affects callgrind, please read callgrind's help.

NOTICE: Callgrind will start instrumentating at the beggining of the run unless you call it with
parameter --instr-atstart=no
""" 

stopInstrumentation.__doc__ = \
"""
Stops instrumentating the binary code of this process by valgrind.
Read about startInstrumentation to understand more.
""" 

del load