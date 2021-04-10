#/* 
# * Author: Yubing Jian <yjian9@gatech.edu>
#

# static AP
if false; then
hermesFlag=0
for clientRS in $(seq 1 1 3)
do
	echo "ii: $clientRS " >>  2019-03-15-final-evaluation-static-ap.txt
	for timeStep in $(seq 0 60 600)
	do  
		./waf --run "scratch/LoSDataGeneration --timeStep=${timeStep} --clientRS=${clientRS} --hermesFlag=${hermesFlag}" 2>> 2019-03-15-final-evaluation-static-ap.txt
	done
done
fi

# brute force
if true; then
hermesFlag=2
for clientRS in $(seq 2 1 2)
do
	echo "ii: $clientRS " >>  2019-03-15-final-evaluation-bruteforce.txt
	for timeStep in $(seq 0 1 900)
	do  
		./waf --run "scratch/LoSDataGeneration --timeStep=${timeStep} --clientRS=${clientRS} --hermesFlag=${hermesFlag}" 2>> 2019-03-15-final-evaluation-bruteforce.txt
	done
done
fi

                  

