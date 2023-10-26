#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt

fn = sys.argv[1]

with open(fn) as f:
    signal = np.array([int(l) for l in f.read().splitlines()])


signal = signal/(1<<31);

plt.plot(signal)

plt.show()


        
