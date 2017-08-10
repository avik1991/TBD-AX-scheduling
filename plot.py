#!/usr/bin/python3

import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy.optimize import fsolve
import math

def plot_func(list_of_plots, outfile, parameter, ylabel, xmax, ymax, multiplier=1, invert=False):
	fig, ax = plt.subplots(1, 1, figsize=(8, 6), dpi=100)

	ax.set_xlim(0, xmax)
	ax.set_ylim(0, ymax)
	def plots(fname, name, linest):
		data = pd.read_table(fname)
		N = data.groupby('N')['N'].mean()
		D = data.groupby('N')[parameter].mean()
		dD = data.groupby('N')[parameter].std()
		if invert:
			ax.errorbar(N, 1 - D, dD / math.sqrt(5), linestyle=linest, label=name, linewidth=2)
		else:
			ax.errorbar(N, D * multiplier, dD * multiplier / math.sqrt(5), linestyle=linest, label=name, linewidth=2)

	for p in list_of_plots:
		plots(p[0], p[1], p[2])

	ax.set_xlabel('Number of STAs')
	ax.set_ylabel(ylabel)
	ax.grid()
	ax.legend(loc="best")
	plt.tight_layout()
	plt.savefig(outfile)
	plt.close()

#dict_keys(['', '-.', ' ', '-', '--', 'None', ':'])

"""mylist = (('0-5m.dat', 'SRTF', '-.'), ('1-5m.dat', 'MUTAX', '-')), ('2-5m.dat', 'PF', '--'), ('3-5m.dat', 'MR', ':')          )#, ('6-5m.dat', 'MUTAX-SO', '-.'))
plot_func(mylist, '5-d.pdf', 'D',           'Average Upload Time, s',  40, 0.05)
plot_func(mylist, '5-e.pdf', 'Empty',       'Busy Channel Time Ratio', 40, 1, invert=True)
plot_func(mylist, '5-t.pdf', 'Transmitted', 'Goodput, Mbps',           40, 250, multiplier=1e-10)
plot_func(mylist, '5-tpf.pdf', 'TPF',         'Transmissions per Frame', 40, 10)
"""
mylist = (('0-20m.dat', 'SRTF', '-.'), ('1-20m.dat', 'MUTAX', '-'), ('2-20m.dat', 'PF', '--'), ('3-20m.dat', 'MR', ':')       )#, ('6-20m.dat', 'MUTAX-SO', '-.'))
plot_func(mylist, '20-d.pdf', 'D',           'Average Upload Time, s',  40, 0.12)
plot_func(mylist, '20-e.pdf', 'Empty',       'Busy Channel Time Ratio', 40, 1, invert=True)
plot_func(mylist, '20-t.pdf', 'Transmitted', 'Goodput, Mbps',           40, 250, multiplier=1e-10)
plot_func(mylist, '20-tpf.pdf', 'TPF',         'Transmissions per Frame', 40, 10)
