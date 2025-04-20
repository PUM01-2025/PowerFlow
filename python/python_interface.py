import pybind11
#from solvers import PowerFlowSolver
import python_wrappers


print("Resultat av 5+5 är:==============================")
print(python_wrappers.pybind11_add(5, 5))

