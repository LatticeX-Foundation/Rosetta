from matplotlib.pyplot import MultipleLocator
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def sigmoid(x):
    s = 1 / (1 + np.exp(-x))
    return s


x = []
y = []
for i in range(5000):
    d = -5.0 + i/500
    x.append(d)
    y.append(sigmoid(d))
# print(x[:32])
# print(y[:32])

rtty = pd.read_csv("/root/x/000/cb/build/bin/y.csv",
                   header=None).values.tolist()

# print(len(y))
# print(len(rtty))


# plot
x_major_locator = MultipleLocator(500)
y_major_locator = MultipleLocator(0.1)

plt.title("numpy & rtt sigmoid")
plt.xlabel("x")
plt.ylabel("y=sigmoid(x)")
plt.plot(y)
plt.plot(rtty)
plt.plot([0, 5000], [0.5, 0.5], linewidth=0.5)
plt.plot([2500, 2500], [0, 1], linewidth=0.5)

xx = [-6.0]
for i in range(0, len(x), 500):
    xx.append(x[i])
xx.append(5.0)

plt.xticks(range(len(xx)), xx)
ax = plt.gca()
ax.xaxis.set_major_locator(x_major_locator)
ax.yaxis.set_major_locator(y_major_locator)
plt.savefig(
    "/root/x/000/cb/cc/modules/protocol/mpc/tests/helix/internal_sigmoid.png")
plt.clf()

diff = []
for i in range(len(y)):
    diff.append(y[i] - rtty[i])

plt.title("numpy - rtt")
plt.ylabel("diff=numpy - rtt")
plt.xlabel("x")
plt.plot(diff)
plt.plot([0, 5000], [0, 0], linewidth=0.5)
plt.plot([2500, 2500], [-0.03, 0.03], linewidth=0.5)

plt.xticks(range(len(xx)), xx)
ax = plt.gca()
y_major_locator = MultipleLocator(0.01)
ax.xaxis.set_major_locator(x_major_locator)
ax.yaxis.set_major_locator(y_major_locator)

plt.savefig(
    "/root/x/000/cb/cc/modules/protocol/mpc/tests/helix/internal_sigmoid-diff.png")
plt.clf()

print("done!")
