import pdb
import numpy as np
import os
import sys
import csv
from operator import itemgetter
import scipy.io
import matplotlib.pyplot as plt
import math
import random 
import matplotlib.pyplot as plt

def AccuracyEvaluation (accuracy, index, blackOut):
	for x in range(locationSet):
		acuLOS = 0.0
		for y in range(len(index)):
			acuLOS = acuLOS + los_ap_sta[i][x][index[y]]
		if acuLOS/len(index)>=0.5 and los_ap_sta[i][x][targetSta-1]==1:
			accuracy = accuracy+1
		elif acuLOS/len(index)<0.5 and los_ap_sta[i][x][targetSta-1]==0:
			accuracy = accuracy+1
	return accuracy

staNo = 200
locationSet = 31
dataSet = 1

## Code to read files
setIter = -1
clientIter = 0
locIter = 0
losApSta = [[[0 for x in range(staNo)] for y in range(locationSet)] for z in range(dataSet)]
stringName = "APLoS.txt"

with open(stringName) as f:
    c = csv.reader(f, delimiter=' ', skipinitialspace=True)
    for line in c:
	if "i:" in line:
		setIter = setIter + 1
		locIter = 0
	else:
		for i in range(staNo):
			losApSta[setIter][locIter][i] = int(line[i])
		locIter=locIter+1

setIter = -1
clientIter = 0
locIter = 0
losStaSta = [[[0 for x in range(staNo)] for y in range(staNo)] for z in range(dataSet)]
stringName = "ClientLoS.txt"

with open(stringName) as f:
    c = csv.reader(f, delimiter=' ', skipinitialspace=True)
    for line in c:
	if "i:" in line:
		setIter = setIter + 1
		locIter = 0
	else:
		for i in range(staNo):
			losStaSta[setIter][locIter][i] = int(line[i])
		locIter=locIter+1

setIter = -1
clientIter = 0
locIter = 0
loc_client = [[[0 for x in range(3)] for y in range(staNo)] for z in range(dataSet)]
stringName = "ClientPos.txt"

with open(stringName) as f:
    c = csv.reader(f, delimiter=' ', skipinitialspace=True)
    for line in c:
	if "i:" in line:
		setIter = setIter + 1
		locIter = 0
	else:
		for i in range(staNo*3):
			loc_client[setIter][i//3][i%3] = float(line[i])
		locIter=locIter+1

setIter = -1
clientIter = 0
locIter = 0
disStaSta = [[[0 for x in range(staNo)] for y in range(staNo)] for z in range(dataSet)]
stringName = "ClientDistance.txt"

with open(stringName) as f:
    c = csv.reader(f, delimiter=' ', skipinitialspace=True)
    for line in c:
	if "i:" in line:
		setIter = setIter + 1
		locIter = 0
	else:
		for i in range(staNo):
			disStaSta[setIter][locIter][i] = float(line[i])
		locIter=locIter+1

targetSta = 6
scenarioNo = 1
timeStep = 10000
minDist = 0

staMatrix = [[[0 for x in range(targetSta)] for y in range(targetSta)] for z in range(scenarioNo*timeStep)]
staDisMatrix = [[[0 for x in range(targetSta)] for y in range(targetSta)] for z in range(scenarioNo*timeStep)]
apMatrix = [[[0 for x in range(targetSta)] for y in range(locationSet)] for z in range(scenarioNo*timeStep)]
locationMatrix = [[[0 for x in range(3)] for y in range(targetSta)] for z in range(scenarioNo*timeStep)]
#print staMatrix[1][0][0]
for i in range(scenarioNo):
	# uniform select 6 STA index and initialize 6 STAs connectivity matrix
	staSet = []
	for itSta in range(targetSta):
		staSet.append(random.randint(0, staNo-1))
	#print staSet
	for timeIt in range(timeStep):
		if timeIt == 0:
			for row in range(targetSta):
				for column in range(targetSta):
					if staSet[column] >= staSet[row]:
						#print i, timeStep, timeIt
						staMatrix[i*timeStep+timeIt][column][row] = losStaSta[0][staSet[column]][staSet[row]]
					else:
						staMatrix[i*timeStep+timeIt][column][row] = losStaSta[0][staSet[row]][staSet[column]]
					staDisMatrix[i*timeStep+timeIt][column][row] = disStaSta[0][staSet[column]][staSet[row]]
					
				for column in range(locationSet):
					apMatrix[i*timeStep+timeIt][column][row] = losApSta[0][column][staSet[row]]
				locationMatrix[i*timeStep+timeIt][row][:] = loc_client[0][row][:]
		else:
			dStaNo = np.random.poisson(1, 1)
			#print dStaNo
			for staDynamicIt in range(dStaNo):
				dynamicStaIndex = random.randint(0, len(staSet)-1)
				#print "dynamic", dynamicStaIndex
				staSet[dynamicStaIndex] = random.randint(0, staNo-1)
				#print "set", staSet
			for row in range(targetSta):
				for column in range(targetSta):
					if staSet[column] >= staSet[row]:
						staMatrix[i*timeStep+timeIt][column][row] = losStaSta[0][staSet[column]][staSet[row]]
					else:
						staMatrix[i*timeStep+timeIt][column][row] = losStaSta[0][staSet[row]][staSet[column]]
					staDisMatrix[i*timeStep+timeIt][column][row] = disStaSta[0][staSet[column]][staSet[row]]
				for column in range(locationSet):
					apMatrix[i*timeStep+timeIt][column][row] = losApSta[0][column][staSet[row]]
				locationMatrix[i*timeStep+timeIt][row][:] = loc_client[0][staSet[row]][:]
		#print "s", timeIt, staMatrix[i*timeStep+timeIt][:][:]
		#print "a", timeIt, apMatrix[i*timeStep+timeIt][:][:]
		#print "l", timeIt, locationMatrix[i*timeStep+timeIt][:][:]


los_sta_sta = np.array(staMatrix)
los_ap_sta = np.array(apMatrix)
dis_sta_sta = np.array(staDisMatrix)
loc_sta = locationMatrix
#print dis_sta_sta[10][:][:]
np.save('los_ap_sta.npy',los_ap_sta)
np.save('los_sta_sta.npy',los_sta_sta)
np.save('loc_sta.npy',loc_sta)
np.save('dis_sta_sta.npy',dis_sta_sta)

los_sta_sta = np.load('los_sta_sta.npy')
los_ap_sta = np.load('los_ap_sta.npy')
print len(los_sta_sta)

accuracyEDLos = 0
accuracyIPLos = 0
accuracyCSLos = 0
dataSet = scenarioNo*timeStep
predictableSet = dataSet

for i in range(dataSet):
	indexED = []
	indexIP = []
	indexCS = []
	minED = sys.maxsize;
	maxIP = -sys.maxsize;
	maxCS = -sys.maxsize;
	blackOut = False
	newCV = los_sta_sta[i][targetSta-1][:]
	for j in range(targetSta-1):
		if np.linalg.norm(los_sta_sta[i][j][:]) == 0:
			break
		if np.linalg.norm(newCV-los_sta_sta[i][j][:]) < minED:
			minED = np.linalg.norm(newCV-los_sta_sta[i][j][:])
			if len(indexED)!=0:
				indexED = []
			indexED.append(j)	
		elif np.linalg.norm(newCV-los_sta_sta[i][j][:]) == minED:
			indexED.append(j)
		if np.inner(newCV, los_sta_sta[i][j][:]) > maxIP:
			maxIP = np.inner(newCV, los_sta_sta[i][j][:])
			if len(indexIP)!=0:
				indexIP = []
			indexIP.append(j)	
		elif np.inner(newCV, los_sta_sta[i][j][:]) == maxIP:
			indexIP.append(j)
		if float(np.inner(newCV, los_sta_sta[i][j][:])/(np.linalg.norm(newCV)*np.linalg.norm(los_sta_sta[i][j][:]))) > maxCS:
			maxCS = np.inner(newCV, los_sta_sta[i][j][:])
			if len(indexCS)!=0:
				indexCS = []
			indexCS.append(j)	
		elif np.inner(newCV, los_sta_sta[i][j][:]) == maxCS:
			indexCS.append(j)
	#print minED
	if minED > 1*math.sqrt(targetSta):
		predictableSet = predictableSet-1
	else:    	
		# identify difference bewteen expected accuracy with ground truth
		accuracyEDLos = AccuracyEvaluation (accuracyEDLos, indexED, blackOut)
		#accuracyIPLos = AccuracyEvaluation (accuracyIPLos, indexIP, blackOut)
		#accuracyCSLos = AccuracyEvaluation (accuracyCSLos, indexCS, blackOut) 
print accuracyEDLos, accuracyIPLos, accuracyCSLos, float(accuracyEDLos)/(predictableSet*locationSet), float(accuracyIPLos)/(predictableSet*locationSet), float(accuracyCSLos)/(predictableSet*locationSet), float(predictableSet)/(dataSet)


		
