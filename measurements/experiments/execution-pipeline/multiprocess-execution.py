import numpy as np

x = np.loadtxt(open("execution-pipeline/multitask-B.csv", "r"), delimiter=",")
for instance in x.T:
    print(np.average([np.max(group) for group in np.split(instance, 10)]), end=", ")
print("\n")

x = np.loadtxt(open("execution-pipeline/multitask-D.csv", "r"), delimiter=",")
for instance in x.T:
    print(np.average([np.max(group) for group in np.split(instance, 10)]), end=", ")
print()
