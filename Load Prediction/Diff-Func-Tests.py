import numpy as np

b = np.array([2,6,3,8,5,0])

prev = []
n = 4
a = b
for i in range(n):
    prev.append(a[0])
    a = np.diff(a)

print(a)

for i in range(n):
    a = np.cumsum(np.concatenate([[prev[-i -1]],a]))

print(a)


b = np.array([2,6,3,8,5,0])
prev = []
a = b
lags = [1,2]
#diff
prev = []
for lag in lags:
    prev.append(a[:lag])
    a = a[lag:]- a[:-lag]
    print(a)

print("reverese", prev)

for k,lag in reversed(list(enumerate(lags))):
    cc = np.empty(len(a)+lag)
    for i in range(lag):
        print(k, i)
        print(prev[k][i])
        cc[i::lag] = np.cumsum(np.concatenate([[prev[k][i]],a[i::lag]]))
    print(cc)
    a = cc

    a = np.arange(100)
batch = 20
lag = 3
print(a[:len(a) - len(a)%batch].reshape(-1,batch)[:,:lag])
a = a[lag:] - a[:-lag]
print(a[:len(a) - len(a)%batch].reshape(-1,batch)[:,:lag])