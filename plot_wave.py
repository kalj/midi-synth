#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt

fn = sys.argv[1]

with open(fn) as f:
    lines = f.read().splitlines()
try:
    signal = np.array([int(l) for l in lines])
    signal = signal / (1 << 31)
except:
    signal = np.array([float(l) for l in lines])

plt.plot(signal)

plt.show()
