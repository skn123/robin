from sourceanalysis import Specifiers
from sourceanalysis.view import Traverse



class ClassInfo(Traverse.AggregateVisitor):
	def visit(self, c):
		if c.getName() == "PrivateStruct":
			c.getContainer().setVisibility(Specifiers.Visibility.PRIVATE)
		print c, c.getContainer().getVisibility()



globalns = pdb.getGlobalNamespace()

Traverse().traverse(globalns.getScope(), ClassInfo())
