for mode in 0 1 2 3 4 5 6
do
	for radius in $(seq 5 15 30)
	do
		echo -e 'seed\tN\tT\tF\tDRA\tD\tDSTA\tEmpty\tTransmitted\tTPF\tTL1' > ${mode}-${radius}m.dat
	done
done

for run in 1 2 3 4 5 6
do
	for mode in 0 1 2 3 4 5 6
	do
		for radius in $(seq 5 15 30)
		do
			for nsta in $(seq 1 40)
			do
				for file in output/${run}-${mode}-${radius}-${nsta}-10000.out
				do
					tail -n 1 $file >> ${mode}-${radius}m.dat
				done
			done
		done
	done
done

#seed = opt(range(1, 7), lambda x: '{}'.format(x))
#mode = opt(range(0, 7), lambda x: '{}'.format(x))
#radius = opt(range(10,30,10), lambda x: '{}'.format(x))
#nsta = opt(range(1, 41),      lambda x: '{}'.format(x))
#Tsim = opt((10000,), lambda x: '{}'.format(x))