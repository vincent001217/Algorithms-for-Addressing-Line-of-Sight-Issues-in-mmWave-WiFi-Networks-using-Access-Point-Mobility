#/* 
# * Author: Yubing Jian <yjian9@gatech.edu>
#

# Platform Size
if false; then
for i in $(seq 0 30)
do  
	echo "ii: $i " >>  2019-1-29-square-ceiling-cost.txt
	for clientRS in $(seq 1 100)
	do
		./waf --run "scratch/Cost-Analysis --i=${i} --clientRS=${clientRS}" 2>> 2019-1-29-square-ceiling-cost.txt
	done
done
fi

# Fixed AP
if true; then
for i in $(seq 1 1)
do  
	for ii in $(seq 1 $i)
	do
		echo "ii: $ii " >>  2019-01-29-fixed-AP-${i}-cost.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Cost-Analysis-Fixed-AP --i=${i} --ii=${ii} --clientRS=${clientRS}" 2>> 2019-01-29-fixed-AP-${i}-cost.txt
		done
	done
done
fi
