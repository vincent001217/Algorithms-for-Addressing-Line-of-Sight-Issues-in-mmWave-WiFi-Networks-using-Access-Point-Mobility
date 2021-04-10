#/* 
# * Author: Yubing Jian <yjian9@gatech.edu>
#

# Floor ceiling wall performance
if false; then
shapeCategary=1
for centerLocation in $(seq 6 9)
do  
	for i in $(seq 0 30)	
	do  
		echo "ii: $i " >>  2019-2-17-shapeCategary-${shapeCategary}-centerLocation-${centerLocation}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --centerLocation=${centerLocation}--shapeCategary=${shapeCategary}" 2>> 2019-2-17-shapeCategary-${shapeCategary}-centerLocation-${centerLocation}.txt
		done
	done
done

for z in $(seq 0 3 3)
do 
	if [ $z -eq 0 ]
	then 
		centerLocation=4
	else
		centerLocation=5
	fi
	for i in $(seq 0 30)
	do  
		echo "ii: $i " >>  2019-2-17-shapeCategary-${shapeCategary}-centerLocation-${centerLocation}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --z=${z}" 2>> 2019-2-17-shapeCategary-${shapeCategary}-centerLocation-${centerLocation}.txt
		done
	done
done
fi

# Center Location
if false; then
for centerLocation in $(seq 0 1 5)
do 
	for i in $(seq 0 30)
	do  
		echo "ii: $i " >>  2019-2-17-centerLocation-${centerLocation}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --centerLocation=${centerLocation}" 2>> 2019-2-17-centerLocation-${centerLocation}.txt
		done
	done
done
fi

# Platform Size
if true; then
for platformSize in $(seq 10 10 60)
do 
	for i in $(seq 0 $platformSize)
	do  
		echo "ii: $i " >>  2019-2-17-platformSize-${platformSize}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --platformSize=${platformSize}" 2>> 2019-2-17-platformSize-${platformSize}.txt
		done
	done
done
fi

# Multi-client
if false; then
clientNo=10
clientDistType=3
hermesFlag=1
for i in $(seq 0 30)
do  
	echo "ii: $i " >>  2019-02-20-fixed-AP-6.txt
	for clientRS in $(seq 1 100)
	do
		./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --hermesFlag=${hermesFlag} --clientNo=${clientNo} --clientDistType=${clientDistType}" 2>> 2019-02-20-fixed-AP-6.txt
	done
done


hermesFlag=0
for i in $(seq 1 5)
do  
	for ii in $(seq 1 $i)
	do
		echo "ii: $i " >>  2019-02-20-fixed-AP-${i}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --ii=${ii} --clientRS=${clientRS} --hermesFlag=${hermesFlag} --clientNo=${clientNo} --clientDistType=${clientDistType}" 2>> 2019-02-20-fixed-AP-${i}.txt
		done
	done
done
fi

# Client Cluster Distribution
if false; then
hermesFlag=1
clientDistType=1
for distRS in $(seq 1 5)
do 
	for i in $(seq 0 30)
	do  
		echo "ii: $i " >>  2019-02-26-fixed-AP-6-distRS-${distRS}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --distRS=${distRS} --clientDistType=${clientDistType}" 2>> 2019-02-26-fixed-AP-6-distRS-${distRS}.txt
		done
	done
done

hermesFlag=0
clientDistType=1
for distRS in $(seq 1 5)
do
	for i in $(seq 1 5)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-02-26-fixed-AP-${i}-distRS-${distRS}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --ii=${ii} --clientRS=${clientRS} --hermesFlag=${hermesFlag} --distRS=${distRS} --clientDistType=${clientDistType}" 2>> 2019-02-26-fixed-AP-${i}-distRS-${distRS}.txt
			done
		done
	done
done
fi

# ON obstacle distribution
if false; then
hermesFlag=1
clientDistType=3
for distRS in $(seq 1 5)
do 
	for i in $(seq 0 30)
	do  
		echo "ii: $i " >>  2019-02-26-fixed-AP-6-distRS-${distRS}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --distRS=${distRS} --clientDistType=${clientDistType}" 2>> 2019-02-26-fixed-AP-6-distRS-${distRS}.txt
		done
	done
done

hermesFlag=0
clientDistType=3
for distRS in $(seq 1 5)
do
	for i in $(seq 1 5)
	do  
		for apIter in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-02-26-fixed-AP-${i}-distRS-${distRS}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --apIter=${apIter} --clientRS=${clientRS} --hermesFlag=${hermesFlag} --distRS=${distRS} --clientDistType=${clientDistType}" 2>> 2019-02-26-fixed-AP-${i}-distRS-${distRS}.txt
			done
		done
	done
done
fi


# Dependent Distribution
if false; then
clientDistType=1
hermesFlag=1
for depSD in 1 2 3 4 5 6 7 8 9 10
do 
	for i in $(seq 0 30)
	do  
		echo "ii: $i " >>  2019-2-18-hermes-depSD-${depSD}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --depSD=${depSD}  --hermesFlag=${hermesFlag} --clientDistType=${clientDistType}" 2>> 2019-2-18-hermes-depSD-${depSD}.txt
		done
	done
done

clientDistType=1
hermesFlag=0
for depSD in 1 2 3 4 5 6 7 8 9 10
do 
	for i in $(seq 1 5)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-02-18-static-AP-${i}-depSD-${depSD}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --ii=${ii} --clientRS=${clientRS} --depSD=${depSD} --hermesFlag=${hermesFlag} --clientDistType=${clientDistType}" 2>> 2019-02-18-static-AP-${i}-depSD-${depSD}.txt
			done
		done
	done
done
fi

# Obstacle coverage
if false; then
hermesFlag=1
for obsNumber in 12 24 36 48 60 72 84 96 108
do 
	for i in $(seq 0 30)
	do 
		echo "ii: $i " >>  2019-2-19-obstacle-coverage-${obsNumber}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --obsNumber=${obsNumber} --hermesFlag=${hermesFlag}" 2>> 2019-2-19-obstacle-coverage-${obsNumber}.txt
		done
	done
done

hermesFlag=0
for obsNumber in 12 24 36 48 60 72 84 96 108
do 
	for i in $(seq 1 5)
	do  
		for ii in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-2-19-static-AP-${i}-obstacle-coverage-${obsNumber}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --ii=${ii} --clientRS=${clientRS} --obsNumber=${obsNumber} --hermesFlag=${hermesFlag}" 2>> 2019-2-19-static-AP-${i}-obstacle-coverage-${obsNumber}.txt
			done
		done
	done
done
fi

# test sample
if false; then
for i in $(seq 30 30)
do  
	echo "ii: $i " >>  2019-2-27-test.txt
	for clientRS in $(seq 1 100)
	do
		./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS}" 2>> 2019-2-27-test.txt
	done
done
fi

# Shape
if true; then
for shapeCategary in $(seq 0 1 3)
do 
	for i in $(seq 0 30)
	do  
		echo "ii: $i " >>  2019-10-14-shapeCategary-${shapeCategary}.txt
		for clientRS in $(seq 1 100)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --shapeCategary=${shapeCategary}" 2>> 2019-10-14-shapeCategary-${shapeCategary}.txt
		done
	done
done
fi

if false; then
#platformSize=60
for shapeCategary in $(seq 0 1 1)
do 
	#for clientDistType in $(seq 0 3 3)
	#do
	for roomX in 30 72
	do  
		#if [ $roomX -eq 9 ]
		#then 
		#	roomY=6
		if [ $roomX -eq 30 ]
		then 
			roomY=25
			platformSize=200
		else
			roomY=70
			platformSize=200
		fi
		for i in $(seq 0 $platformSize)
		do  
			echo "ii: $i " >>  2019-4-30-shapeCategary-${shapeCategary}-roomX-${roomX}-roomY-${roomY}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --shapeCategary=${shapeCategary} --roomX=${roomX} --roomY=${roomY} --platformSize=${platformSize}" 2>> 2019-4-30-shapeCategary-${shapeCategary}-roomX-${roomX}-roomY-${roomY}.txt
			done
		done
	done
done
fi

# uniform distribution
if false; then
platformSize=300
for shapeCategary in $(seq 1 1 1)
do 
	for i in $(seq 0 $platformSize)
	do 
		echo "ii: $i " >>  2019-5-6-shapeCategary-${shapeCategary}.txt
		for clientRS in $(seq 1 4900)
		do
			./waf --run "scratch/Hermes++ --i=${i} --clientRS=${clientRS} --shapeCategary=${shapeCategary} --platformSize=${platformSize}" 2>> 2019-5-6-shapeCategary-${shapeCategary}.txt
		done
	done
done
fi

# multi-mobile AP
if false; then
hermesFlag=2
clientDistType=3
for distRS in $(seq 1 5)
do 
	for i in $(seq 0 30)
	do  
		for apIter in $(seq 1 2)
		do
			echo "ii: $i " >>  2019-02-26-fixed-AP-6-distRS-${distRS}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --apIter=${apIter} --clientRS=${clientRS} --hermesFlag=${hermesFlag} --distRS=${distRS} --clientDistType=${clientDistType}" 2>> 2019-02-26-fixed-AP-6-distRS-${distRS}.txt
			done
		done
	done
done

hermesFlag=0
clientDistType=3
for distRS in $(seq 1 5)
do
	for i in $(seq 1 5)
	do  
		for apIter in $(seq 1 $i)
		do
			echo "ii: $i " >>  2019-02-26-fixed-AP-${i}-distRS-${distRS}.txt
			for clientRS in $(seq 1 100)
			do
				./waf --run "scratch/Hermes++ --i=${i} --apIter=${apIter} --clientRS=${clientRS} --hermesFlag=${hermesFlag} --distRS=${distRS} --clientDistType=${clientDistType}" 2>> 2019-02-26-fixed-AP-${i}-distRS-${distRS}.txt
			done
		done
	done
done
fi
