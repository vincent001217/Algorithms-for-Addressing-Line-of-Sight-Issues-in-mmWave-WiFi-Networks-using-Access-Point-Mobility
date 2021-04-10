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

def AccuracyEvaluation (accuracy, index):
	for x in range(locationSet):
		acuLOS = 0.0
		for y in range(len(index)):
			acuLOS = acuLOS + los_ap_sta[i][x][index[y]]
		if acuLOS/len(index)>=0.5 and los_ap_sta[i][x][targetSta-1]==1:
			accuracy = accuracy+1
		elif acuLOS/len(index)<0.5 and los_ap_sta[i][x][targetSta-1]==0:
			accuracy = accuracy+1
	return accuracy


locationSet = 31
scenarioNo = 1
timeStep = 10000
los_sta_sta = np.load('los_sta_sta.npy')
los_ap_sta = np.load('los_ap_sta.npy')
loc_sta = np.load('loc_sta.npy')
dis_sta = np.load('dis_sta_sta.npy')
targetSta = 6

for i in range(scenarioNo*timeStep):
	for row in range(targetSta):
		for column in range(targetSta):
			#random.seed(scenarioNo*targetSta*targetSta)
			temp = random.randint(0, 99)
			if temp <= 4:
				los_sta_sta[i][row][column] = 1-los_sta_sta[i][row][column]
	for row in range(locationSet):
		for column in range(targetSta):
			temp = random.randint(0, 99)		
			if temp <= 4:
				los_ap_sta[i][row][column] = 1-los_ap_sta[i][row][column]			

	
for i in range(scenarioNo*timeStep):
	for row in range(targetSta):
		column = random.randint(0, 2) 
		option = random.randint(0, 9) 
		if option == 0:
			loc_sta[i][row][column] = loc_sta[i][row][column]
		if option == 1:
			optionSign = random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.1
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.1
		if option == 2:
			optionSign = random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.2
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.2
		if option == 3:
			optionSign = random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.3
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.3
		if option == 4:
			optionSign = random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.4
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.4
		if option == 5:
			optionSign =  random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.45
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.45
		if option == 6:
			optionSign =  random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.5
			if optionSign == 1:
				loc_sta[i][row][column] =loc_sta[i][row][column] - 0.5
		if option == 7:
			optionSign =  random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.6
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.6
		if option == 8:
			optionSign =  random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.7
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.7
		if option == 9:
			optionSign =  random.randint(0, 1)
			if optionSign == 0:
				loc_sta[i][row][column] = loc_sta[i][row][column] + 0.9
			if optionSign == 1:
				loc_sta[i][row][column] = loc_sta[i][row][column] - 0.9
		if loc_sta[i][row][column] < 0:
			loc_sta[i][row][column] = 0
		if column==0 and loc_sta[i][row][column]>9:
			loc_sta[i][row][column]=9
		if column==1 and loc_sta[i][row][column]>6:
			loc_sta[i][row][column]=6
		if column==2 and loc_sta[i][row][column]>3:
			loc_sta[i][row][column]=3	

for i in range(scenarioNo*timeStep):
	for j in range(targetSta):
		for k in range(targetSta):
			if j != k:
				option = random.randint(0, 9) 
				if option == 0:
					dis_sta[i][j][k] = dis_sta[0][j][k]
				if option == 1:
					optionSign = random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.01
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.01
				if option == 2:
					optionSign = random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.03
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.03
				if option == 3:
					optionSign = random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.04
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.04
				if option == 4:
					optionSign = random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.04
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.04
				if option == 5:
					optionSign =  random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.05
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.05
				if option == 6:
					optionSign =  random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.06
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.06
				if option == 7:
					optionSign =  random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.075
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.075
				if option == 8:
					optionSign =  random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.15
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.15
				if option == 9:
					optionSign =  random.randint(0, 1)
					if optionSign == 0:
						dis_sta[i][j][k] = dis_sta[0][j][k] + 0.38
					if optionSign == 1:
						dis_sta[i][j][k] = dis_sta[0][j][k] - 0.38

np.save('los_ap_sta.npy',los_ap_sta)
np.save('los_sta_sta.npy',los_sta_sta)
np.save('loc_sta.npy',loc_sta)
np.save('dis_sta_sta.npy',dis_sta)

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
	#print minED
	if minED > 1*math.sqrt(targetSta):
		predictableSet = predictableSet-1
	else:    	
		# identify difference bewteen expected accuracy with ground truth
		accuracyEDLos = AccuracyEvaluation (accuracyEDLos, indexED)
print accuracyEDLos, float(accuracyEDLos)/(predictableSet*locationSet), float(predictableSet)/(dataSet)


		
