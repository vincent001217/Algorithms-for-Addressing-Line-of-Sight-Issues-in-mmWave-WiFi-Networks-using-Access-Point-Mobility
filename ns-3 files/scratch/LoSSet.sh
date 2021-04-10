#/* 
# * Author: Yubing Jian <yjian9@gatech.edu>
#

if true; then
for clientRS in $(seq 1 1 10)
do 
	echo "i: $clientRS " >>  ClientLoS.txt	
	echo "i: $clientRS " >>  ClientDistance.txt
	echo "i: $clientRS " >>  ClientPos.txt
	echo "i: $clientRS " >>  APLoS.txt	
	echo "i: $clientRS " >>  APDistance.txt
	echo "i: $clientRS " >>  RSS.txt
	echo "i: $clientRS " >>  MCS.txt
	echo "i: $clientRS " >>  Throughput.txt
	for i in $(seq 0 30)
	do  
		./waf --run "scratch/LoSDataGeneration --i=${i} --clientRS=${clientRS}"
	done
done
fi

                  

