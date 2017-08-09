#!/usr/bin/python2
import htcondor
import classad
import itertools
import os
import errno
schedd = htcondor.Schedd()

try:
	os.makedirs('output')
except OSError as exception:
	if exception.errno != errno.EEXIST:
		raise
os.chdir('output')

def opt(x, f):
	return zip(x, map(f, x))

seed = opt(range(1, 7), lambda x: '{}'.format(x))
mode = opt([6], lambda x: '{}'.format(x))
radius = opt(range(5,30,15), lambda x: '{}'.format(x))
nsta = opt(range(1, 31),      lambda x: '{}'.format(x))
Tsim = opt((10000,), lambda x: '{}'.format(x))

for t in itertools.product(seed, mode, radius, nsta, Tsim):
	base = '-'.join(map(str, zip(*t)[0]))
	opts = list(zip(*t)[1])
	ad = classad.ClassAd({
	'Cmd': '../sim',
	'Arguments': ' '.join(opts),
	'UserLog': base + '.log',
	'Out':     base + '.out', # stdout
	'Err':     base + '.err'  # stderr
	})
	schedd.submit(ad)
