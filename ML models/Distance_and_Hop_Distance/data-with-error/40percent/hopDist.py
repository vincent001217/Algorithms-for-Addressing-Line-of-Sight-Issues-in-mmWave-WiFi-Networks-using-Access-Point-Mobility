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

def MinimumDistance (i, row, target, dist, path):
	global minDist
	for x in range(targetSta):
		#print "row, column, target, dist, minDist, path", row, x, target, dist, minDist, path
		if x != row and x not in path and dist < minDist:
			if x == target and hop_sta_sta[i][row][x] == 1:
				dist = dist + 1
				if dist < minDist:
					minDist = dist			
			elif hop_sta_sta[i][row][x] == 1:
				dist = dist+1
				path.append(row)
				MinimumDistance (i, x, target, dist, path)
locationSet = 31
targetSta = 6
los_sta_sta = np.load('los_sta_sta.npy')
los_ap_sta = np.load('los_ap_sta.npy')
hop_sta_sta = np.array(los_sta_sta)

accuracyEDLos = 0
accuracyIPLos = 0
accuracyCSLos = 0
dataSet = 10000
predictableSet = dataSet
print hop_sta_sta[50][:][:]

# Transfer LOS to hop distance
for i in range(dataSet):
	for row in range(targetSta):
		for column in range(targetSta):
			path = []
			minDist = sys.maxsize
			if hop_sta_sta[i][row][column] == 0:
				# identfy possible hops to target 
				MinimumDistance (i, column, row, 0, path)
				if minDist == sys.maxsize:
					hop_sta_sta[i][row][column] = targetSta
				else:
					hop_sta_sta[i][row][column] = minDist


print hop_sta_sta[50][:][:]
np.save('hop_sta_sta.npy',hop_sta_sta)


