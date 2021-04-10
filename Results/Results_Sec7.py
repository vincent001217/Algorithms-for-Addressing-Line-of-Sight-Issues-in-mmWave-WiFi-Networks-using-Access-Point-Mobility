import pdb
import numpy as np
import os
import sys
import csv
from operator import itemgetter 
import plotly.plotly as py
import plotly.graph_objs as go
import operator
import math

overallTime = 300

throughputSum = [280, 8.79, 56, 317, 203, 290, 334, 108.4, 108.4, 279.44, 151, 308, 398, 349, 349, 396, 320, 321, 1, 1.31, 1, 110, 110, 348, 141.2, 126.4, 221, 415, 391, 392]
losSample = [0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 3, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2]
jainsFairnessSample = [0.5, 0.903663, 0.944305, 0.98656, 0.5, 0.999952, 0.663242, 0.39027, 0.39027, 0.624403, 0.760566, 0.916401, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.653573, 0.527621, 0.85503, 0.995141, 0.992051, 0.999064]

ThptStatic = [0 for x in range(overallTime)]
ThptBF = [0 for x in range(overallTime)]
ThptWiM = [0 for x in range(overallTime)]

LosStatic = [0 for x in range(overallTime)]
LosBF = [0 for x in range(overallTime)]
LosWiM = [0 for x in range(overallTime)]

FairnessStatic = [0 for x in range(overallTime)]
FairnessBF = [0 for x in range(overallTime)]
FairnessWiM = [0 for x in range(overallTime)]

for timeIter in range(overallTime):
	if timeIter%60 == 0:
		ThptStatic[timeIter] = throughputSum[int(math.floor(timeIter/60))*6 + 2]
		initialApLoc = 0
		for dupIter in range(60):
			#print int(math.floor(timeIter/60)) + 2, throughputSum[int(math.floor(timeIter/60))*6 + 2]
			ThptStatic[int(math.floor(timeIter/60))*60+dupIter] = throughputSum[int(math.floor(timeIter/60))*6 + 2]
			LosStatic[int(math.floor(timeIter/60))*60+dupIter] = losSample[int(math.floor(timeIter/60))*6 + 2]
			FairnessStatic[int(math.floor(timeIter/60))*60+dupIter] = jainsFairnessSample[int(math.floor(timeIter/60))*6 + 2]
	if (timeIter<60 or timeIter>=240 or (timeIter>=120 and timeIter<180)) and timeIter%5 == 0:	
		for dupIter in range(5):
			if timeIter%60<30:	
				ThptBF[timeIter+dupIter] = throughputSum[initialApLoc+int(math.floor(timeIter/60))*6]
				ThptWiM[timeIter+dupIter] = throughputSum[initialApLoc+int(math.floor(timeIter/60))*6]
				LosBF[timeIter+dupIter] = losSample[initialApLoc+int(math.floor(timeIter/60))*6]
				LosWiM[timeIter+dupIter] = losSample[initialApLoc+int(math.floor(timeIter/60))*6]
				FairnessBF[timeIter+dupIter] = jainsFairnessSample[initialApLoc+int(math.floor(timeIter/60))*6]
				FairnessWiM[timeIter+dupIter] = jainsFairnessSample[initialApLoc+int(math.floor(timeIter/60))*6]
			else:
				ThptBF[timeIter+dupIter] = ThptBF[timeIter+dupIter-1]
				ThptWiM[timeIter+dupIter] = ThptWiM[timeIter+dupIter-1]
				LosBF[timeIter+dupIter] = LosBF[timeIter+dupIter-1]
				LosWiM[timeIter+dupIter] = LosWiM[timeIter+dupIter-1]
				FairnessBF[timeIter+dupIter] = FairnessBF[timeIter+dupIter-1]
				FairnessWiM[timeIter+dupIter] = FairnessWiM[timeIter+dupIter-1]
			if timeIter>60:
				ThptWiM[timeIter+dupIter] = throughputSum[(int(math.floor(timeIter/60))+1)*6-1]
				LosWiM[timeIter+dupIter] = losSample[(int(math.floor(timeIter/60))+1)*6-1]
				FairnessWiM[timeIter+dupIter] = jainsFairnessSample[(int(math.floor(timeIter/60))+1)*6-1]
		initialApLoc = initialApLoc+1	
	elif ((timeIter>=60 and timeIter<120) or (timeIter>=180 and timeIter<240)) and timeIter%5 == 0:	
		for dupIter in range(5):
			if timeIter%60 == 30:
				initialApLoc = 0
			if timeIter%60<30:	
				ThptBF[timeIter+dupIter] = throughputSum[(int(math.floor(timeIter/60))+1)*6-initialApLoc-1]
				ThptWiM[timeIter+dupIter] = throughputSum[(int(math.floor(timeIter/60))+1)*6-1]
				LosBF[timeIter+dupIter] = losSample[(int(math.floor(timeIter/60))+1)*6-initialApLoc-1]
				LosWiM[timeIter+dupIter] = losSample[(int(math.floor(timeIter/60))+1)*6-1]
				FairnessBF[timeIter+dupIter] = jainsFairnessSample[(int(math.floor(timeIter/60))+1)*6-initialApLoc-1]
				FairnessWiM[timeIter+dupIter] = jainsFairnessSample[(int(math.floor(timeIter/60))+1)*6-1]
			elif timeIter%60<55 and timeIter%60>=30:
				ThptBF[timeIter+dupIter] = throughputSum[initialApLoc+int(math.floor(timeIter/60))*6+1]
				ThptWiM[timeIter+dupIter] = throughputSum[(int(math.floor(timeIter/60))+1)*6-1]
				LosBF[timeIter+dupIter] = losSample[initialApLoc+int(math.floor(timeIter/60))*6+1]
				LosWiM[timeIter+dupIter] = losSample[(int(math.floor(timeIter/60))+1)*6-1]
				FairnessBF[timeIter+dupIter] = jainsFairnessSample[initialApLoc+int(math.floor(timeIter/60))*6+1]
				FairnessWiM[timeIter+dupIter] = jainsFairnessSample[(int(math.floor(timeIter/60))+1)*6-1]
			else:
				ThptBF[timeIter+dupIter] = ThptBF[timeIter+dupIter-1]
				ThptWiM[timeIter+dupIter] = ThptWiM[timeIter+dupIter-1]
				LosBF[timeIter+dupIter] = LosBF[timeIter+dupIter-1]
				LosWiM[timeIter+dupIter] = LosWiM[timeIter+dupIter-1]
				FairnessBF[timeIter+dupIter] = FairnessBF[timeIter+dupIter-1]
				FairnessWiM[timeIter+dupIter] = FairnessWiM[timeIter+dupIter-1]

		initialApLoc = initialApLoc+1	
				
print np.mean(ThptWiM, axis=0)/np.mean(ThptStatic, axis=0), np.mean(ThptWiM, axis=0)/np.mean(ThptBF, axis=0), 
print np.mean(FairnessWiM, axis=0)/np.mean(FairnessStatic, axis=0), np.mean(FairnessWiM, axis=0)/np.mean(FairnessBF, axis=0)

maxThpt = max([max(ThptStatic), max(ThptBF), max(ThptWiM)])
for i in range(300):
	ThptStatic[i] = float(ThptStatic[i])/float(maxThpt)
	ThptBF[i] = float(ThptBF[i])/float(maxThpt)
	ThptWiM[i] = float(ThptWiM[i])/float(maxThpt)

LosStatic = np.sort(LosStatic, axis=0)
LosBF = np.sort(LosBF, axis=0)
LosWiM = np.sort(LosWiM, axis=0)

xaxis = [0 for x in range(overallTime)]
for i in range(overallTime):
	xaxis[i]=float(i)/overallTime

# Create traces
trace0 = go.Scatter(
    x = xaxis,
    y = LosStatic,
    line = dict(width=5),	    
    name = 'Single Static AP'

)
trace1 = go.Scatter(
    x = xaxis,
    y = LosBF,
    line = dict(width=5,
		dash='dot'),
    name = 'BruteForce'
)
trace2 = go.Scatter(
    x = xaxis,
    y = LosWiM,
    line = dict(width=5,
		dash='dashdot'),
    name = 'WiMove'
)
data = [trace0, trace1, trace2]
layout = go.Layout(
    font=dict(family='Roboto', size=26, color='#444444'),
    xaxis=dict(
	title='CDF',
	automargin=True,
    ),
    yaxis=dict(
	title='Number of LOS STAs',
	automargin=True,
    ),
    legend=dict(orientation="h",
		x=-0.1, y=1.2)	
)
fig = go.Figure(data=data, layout=layout)
py.iplot(fig, filename='2019-03-17-final-evaluation-los-exp')

ThptStatic = np.sort(ThptStatic, axis=0)
ThptBF = np.sort(ThptBF, axis=0)
ThptWiM = np.sort(ThptWiM, axis=0)

trace0 = go.Scatter(
    x = xaxis,
    y = ThptStatic,
    line = dict(width=5),	
    name = 'Single Static AP'
)
trace1 = go.Scatter(
    x = xaxis,
    y = ThptBF,
    line = dict(width=5,
		dash='dot'),
    name = 'BruteForce'
)
trace2 = go.Scatter(
    x = xaxis,
    y = ThptWiM,
    line = dict(width=5,
		dash='dashdot'),
    name = 'WiMove'
)
data = [trace0, trace1, trace2]
layout = go.Layout(
    font=dict(family='Roboto', size=26, color='#444444'),
    xaxis=dict(
	title='CDF',
	automargin=True,
    ),
    yaxis=dict(
	title='Normalized Throughput',
	automargin=True,
    ),
    legend=dict(orientation="h",
		x=-0.1, y=1.2)		
)
fig = go.Figure(data=data, layout=layout)
py.iplot(fig, filename='2019-03-17-final-evaluation-thpt-exp')

FairnessStatic = np.sort(FairnessStatic, axis=0)
FairnessBF = np.sort(FairnessBF, axis=0)
FairnessWiM = np.sort(FairnessWiM, axis=0)

trace0 = go.Scatter(
    x = xaxis,
    y = FairnessStatic,
    line = dict(width=5),
    name = 'Single Static AP'
)
trace1 = go.Scatter(
    x = xaxis,
    y = FairnessBF,
    line = dict(width=5,
		dash='dot'),
    name = 'BruteForce'
)
trace2 = go.Scatter(
    x = xaxis,
    y = FairnessWiM,
    line = dict(width=5,
		dash='dashdot'),
    name = 'WiMove'
)
data = [trace0, trace1, trace2]
layout = go.Layout(
    font=dict(family='Roboto', size=26, color='#444444'),
    xaxis=dict(
	title='CDF',
	automargin=True,
    ),
    yaxis=dict(
	title='Jain''s Fairness Index',
	automargin=True,
    ),
    legend=dict(orientation="h",
		x=0.1, y=1.2)	
)
fig = go.Figure(data=data, layout=layout)
py.iplot(fig, filename='2019-03-17-final-evaluation-fairness-exp')
