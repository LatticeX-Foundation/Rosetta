import numpy as np
import sys
file = sys.argv[1]
print(file, end=' ')
arr = np.loadtxt(file, dtype=float)
arr = arr/1024.0/1024.0
print('mem-max(GB):', np.max(arr))
