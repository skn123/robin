from sourceanalysis.dox import DoxygenAnalyzer
from backends.robin import Launcher

d = DoxygenAnalyzer("xml")
d.processIndex()
