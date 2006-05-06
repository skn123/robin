from sourceanalysis import Specifiers
from sourceanalysis.view import Traverse
from sourceanalysis.hints import IncludedViaHeader


globalns = pdb.getGlobalNamespace()
hinting_h = pdb.lookupSourceFile("hinting.h")
hinted_h = pdb.lookupSourceFile("hinted.h")
hinting_h.addHint(IncludedViaHeader(hinted_h))

#Traverse().traverse(globalns.getScope(), ClassInfo())
