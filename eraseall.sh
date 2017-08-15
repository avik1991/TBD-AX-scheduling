for run in 1 2 3 4 5 6
do
	for mode in 1 6
	do
		for radius in $(seq 5 15 30)
		do
			for nsta in $(seq 1 40)
			do
				rm output/${run}-${mode}-${radius}-${nsta}-10000.out
			done
		done
	done
done