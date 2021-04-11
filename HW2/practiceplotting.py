import matplotlib.pyplot as plt
import matplotlib.cbook as cbook

import numpy as np
import pandas as pd

sigA = pd.read_csv('sigA.csv')
sigB = pd.read_csv('sigB.csv')
sigC = pd.read_csv('sigC.csv')
sigD = pd.read_csv('sigD.csv')

plt.plotfile(sigA)



# import csv
# with open(curfile, 'r', newline = '') as csvfile:
#     sigX = csv.reader(csvfile, delimiter = ' ', quotechar='|')
#     for row in sigX:
#         # whatever

