for mode in 6
do
	for radius in $(seq 5 15 30)
	do
		echo -e 'seed\tN\tT\tF\tDRA\tD\tDSTA\tEmpty\tTransmitted\tTPF' > ${mode}-${radius}m.dat
	done
done

for run in 1 2 3 4 5 6
do
	for mode in 6
	do
		for radius in $(seq 5 15 30)
		do
			for nsta in $(seq 1 30)
			do
				for file in output/${run}-${mode}-${radius}-${nsta}-10000.out
				do
					tail -n 1 $file >> ${mode}-${radius}m.dat
				done
			done
		done
	done
done

#seed = opt(range(1, 6), lambda x: '{}'.format(x))
#mode = opt(range(0, 5), lambda x: '{}'.format(x))
#radius = opt(range(10,30,10), lambda x: '{}'.format(x))
#nsta = opt(range(1, 21),      lambda x: '{}'.format(x))
#Tsim = opt((10000,), lambda x: '{}'.format(x))