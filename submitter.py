#!/usr/bin/python2
import htcondor
import classad
import itertools
import os
import errno

from shutil import copyfile


schedd = htcondor.Schedd()

copyfile('sim', 'onlyforcondor')

try:
	os.makedirs('output')
except OSError as exception:
	if exception.errno != errno.EEXIST:
		raise
os.chdir('output')


def opt(x, f):
	return zip(x, map(f, x))

#mode 0 1 2 3 6
seed = opt(range(1, 7), lambda x: '{}'.format(x))
mode = opt([6], lambda x: '{}'.format(x))
radius = opt(range(5,30,15), lambda x: '{}'.format(x))
nsta = opt(range(1, 41),      lambda x: '{}'.format(x))
Tsim = opt((10000,), lambda x: '{}'.format(x))

for t in itertools.product(seed, mode, radius, nsta, Tsim):
	base = '-'.join(map(str, zip(*t)[0]))
	opts = list(zip(*t)[1])
	ad = classad.ClassAd({
	'Cmd': '../onlyforcondor',
	'Arguments': ' '.join(opts),
	'UserLog': base + '.log',
	'Out':     base + '.out', # stdout
	'Err':     base + '.err'  # stderr
	})
	schedd.submit(ad)