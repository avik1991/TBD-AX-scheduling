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

	ax.xaxis.label.set_fontsize(20)
	ax.yaxis.label.set_fontsize(20)
	ax.grid()
	ax.legend(loc="best", prop={'size': 14})
	plt.tight_layout()
	plt.savefig(outfile)
	plt.close()

#dict_keys(['', '-.', ' ', '-', '--', 'None', ':'])

mylist = (('4-5m.dat', 'MR', ':'), ('3-5m.dat', 'ax-MR', ':'), ('5-5m.dat', 'PF', '--'), ('2-5m.dat', 'ax-PF', '--'), ('0-5m.dat', 'SRTF', '-.'), ('1-5m.dat', 'MUTAX', '-'), ('6-5m.dat', 'MUTAX-SO', '-.'))
plot_func(mylist, '5-d.pdf', 'D',           'Average Upload Time, s',  35, 0.05)
plot_func(mylist, '5-e.pdf', 'Empty',       'Busy Channel Time Ratio', 35, 1, invert=True)
plot_func(mylist, '5-t.pdf', 'Transmitted', 'Goodput, Mbps',           35, 250, multiplier=1e-10)
plot_func(mylist, '5-tpf.pdf', 'TPF',         'Transmissions per Frame', 35, 10)
plot_func(mylist, '5-tl1.pdf', 'TL1',         'Transmissions longer than 1 slot', 35, 1)

mylist = (('4-20m.dat', 'MR', ':'), ('3-20m.dat', 'ax-MR', ':'), ('5-20m.dat', 'PF', '--'), ('2-20m.dat', 'ax-PF', '--'), ('0-20m.dat', 'SRTF', '-.'), ('1-20m.dat', 'MUTAX', '-'), ('6-20m.dat', 'MUTAX-SO', '-.'))
plot_func(mylist, '20-d.pdf', 'D',           'Average Upload Time, s',  35, 0.12)
plot_func(mylist, '20-e.pdf', 'Empty',       'Busy Channel Time Ratio', 35, 1, invert=True)
plot_func(mylist, '20-t.pdf', 'Transmitted', 'Goodput, Mbps',           35, 250, multiplier=1e-10)
plot_func(mylist, '20-tpf.pdf', 'TPF',         'Transmissions per Frame', 35, 10)
plot_func(mylist, '20-tl1.pdf', 'TL1',         'Transmissions longer than 1 slot', 35, 1)