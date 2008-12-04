from sourceanalysis import Specifiers
from sourceanalysis.view import Traverse
from sourceanalysis.hints import IncludedViaHeader


EasyBMP_h = pdb.lookupSourceFile("EasyBMP.h")
internal_h = [pdb.lookupSourceFile("EasyBMP_BMP.h"),
              pdb.lookupSourceFile("EasyBMP_DataStructures.h"),
              pdb.lookupSourceFile("EasyBMP_VariousBMPutilities.h")]


for inter in internal_h:
    inter.addHint(IncludedViaHeader(EasyBMP_h))

