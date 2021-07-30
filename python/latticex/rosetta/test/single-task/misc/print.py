#!/usr/bin/python

"""
This file just for checking the libraries loading
"""
import latticex.rosetta as cb
print(dir())
print("doc: ", cb.__doc__)
print("name:", cb.__name__)
print("package: ", cb.__package__)
