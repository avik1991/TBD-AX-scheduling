#!/usr/bin/python3
import sys

if len(sys.argv) != 2:
	sys.exit("Not enough args")
NDA = str(sys.argv[1])

print (NDA,end='   ')
NDA = int(NDA)
n = 18
F = 18

summands = [1,2,4,9,18]

def checklist(A):
    counter = 0
    for item in A:
        if item in summands:
            counter+=1
            
    if (counter == len(A) and (counter < NDA)):
        return 1
    else:
        return 0

    
def checklist2(A):
    counter = 0
    for item in A:
        if item in summands:
            counter+=1
            
    if ((counter == len(A)) and (counter == NDA)):
        return 1
    else:
        return 0
    
B = []
i = 0
A = []
for iskra in range(n):
    A.append(1)
    
while A < [n]:
    if sum(A) + A[-1] <= n:
        A.append(A[-1])
    elif sum(A)<n:
        A[-1]+=n-sum(A)
    else:
        if (sum(A)==n):
            if checklist(A):
                #print A
                B.append(A)
                B[i] = []
                B[i].extend(A)
                i = i+1
            del A[-1]
            A[-1] += 1

B.sort(key=lambda x: len(x), reverse=False)

for n in range(2,F+1):
    A = []
    for iskra in range(n):
        A.append(1)
    
    while A != [n]:
        if sum(A) + A[-1] <= n:
            A.append(A[-1])
        elif sum(A)<n:
            A[-1]+=n-sum(A)
        else:
            if (checklist2(A)):
                #print A
                B.append(A)
                B[i] = []
                B[i].extend(A)
                i = i+1
            del A[-1]
            A[-1] += 1

def giveconf(B):
    C = [0,0,0,0,0]
    for item in B:
        for counter in range(len(summands)):
            if item == summands[counter]:
                C[counter]+=1
    
    return C

conf = []

conf.append([0,0,0,0,1])
for item in B:
    conf.append(giveconf(item))  


if NDA == 1:
     conf.append([0,0,0,1,0])
     conf.append([0,0,1,0,0])
     conf.append([0,1,0,0,0])
     conf.append([1,0,0,0,0])

print(len(conf))
thefile = open('nda.h', 'a')
thefile.write("\n{\n")

for item in conf:
	thefile.write('{')
	thefile.write(', '.join(map(repr, item)))
	thefile.write('},')
	thefile.write("\n")

thefile.write("},")
thefile.close()
