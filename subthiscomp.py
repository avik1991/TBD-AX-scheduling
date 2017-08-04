import subprocess

f0 = open("0-20m.dat", "w")
f0.write("seed\tN\tF\tT\tDRA\tD\tDeSta\tSLETSL\ttraperflow\t\n")
f0.flush()

p = subprocess.call(['gcc', 'sim.c', '-lm','-lgsl','-lgslcblas'])

counter = 0
for nsta in range(1,30):
    for seed in range(1,6+1):
        #                   seed oraclemode nsta tsim arrival size frbmin
        p1 = subprocess.call(['./a.out',str(seed),'0','0',str(nsta),'1000'], stdout=f0)
        counter +=1
        print(str(counter), end='\r')

f0.close()

f1 = open("1-20m.dat", "w")
f1.write("seed\tN\tF\tT\tDRA\tD\tDeSta\tSLETSL\ttraperflow\t\n")
f1.flush()

p = subprocess.call(['gcc', 'sim.c', '-lm','-lgsl','-lgslcblas'])

counter = 0
for nsta in range(1,30):
    for seed in range(1,6+1):
        #                   seed oraclemode nsta tsim arrival size frbmin
        p1 = subprocess.call(['./a.out',str(seed),'1','0',str(nsta),'1000'], stdout=f1)
        counter +=1
        print(str(counter), end='\r')

f1.close()

f2 = open("2-20m.dat", "w")
f2.write("seed\tN\tF\tT\tDRA\tD\tDeSta\tSLETSL\ttraperflow\t\n")
f2.flush()

p = subprocess.call(['gcc', 'sim.c', '-lm','-lgsl','-lgslcblas'])

counter = 0
for nsta in range(1,30):
    for seed in range(1,6+1):
        #                   seed oraclemode nsta tsim arrival size frbmin
        p1 = subprocess.call(['./a.out',str(seed),'2','0',str(nsta),'1000'], stdout=f2)
        counter +=1
        print(str(counter), end='\r')

f1.close()