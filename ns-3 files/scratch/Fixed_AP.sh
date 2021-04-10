#/* 
# * Author: Yubing Jian <yjian9@gatech.edu>
#

# floor vs. ceiling
if false; then
for z in $(seq 3 3 3)
do   
	for i in $(seq 1 5)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $ii " >>  2018-01-23-fixed-AP-${i}-z-${z}-poisson.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Motivation-Expected-Thpt-Fixed-APs --i=${i} --ii=${ii} --clientRS=${clientRS} --z=${z}" 2>> 2019-01-23-fixed-AP-${i}-z-${z}-poisson.txt
			done
		done
	done
done
fi

# Dependent Distribution
if false; then
for depVariance in 1 5 10 15 20 30
do 
	for i in $(seq 1 5)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-01-28-fixed-AP-${i}-depVariance-${depVariance}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Motivation-Expected-Thpt-Fixed-APs --i=${i} --clientRS=${clientRS} --depVariance=${depVariance}" 2>> 2019-01-28-fixed-AP-${i}-depVariance-${depVariance}.txt
			done
		done
	done
done
fi

# Client Distribution
if true; then
for distRS in $(seq 1 5)
do
	for i in $(seq 1 5)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-01-20-fixed-AP-${i}-distRS-${distRS}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Motivation-Expected-Thpt-Fixed-APs --i=${i} --clientRS=${clientRS} --distRS=${distRS}" 2>> 2019-01-20-fixed-AP-${i}-distRS-${distRS}.txt
			done
		done
	done
done
fi

# Obstacle coverage
if false; then
for obsNumber in 12 30 61 93
do 
	for i in $(seq 1 1)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-02-01-fixed-AP-${i}-obsNumber-${obsNumber}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Motivation-Expected-Thpt-Fixed-APs --i=${i} --clientRS=${clientRS} --obsNumber=${obsNumber}" 2>> 2019-02-01-fixed-AP-${i}-obsNumber-${obsNumber}.txt
			done
		done
	done
done
fi
