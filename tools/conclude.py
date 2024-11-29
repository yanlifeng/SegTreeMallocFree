#!/home/export/online1/mdt00/shisuan/sweq/anaconda3/bin/python
# -*- coding: UTF-8 -*-
import os
import sys
import time

import numpy as np

root = (sys.argv[1])
if(root[len(root)-1] != '/'):
	root += '/'
items_t = os.listdir(root)
items = []

for item in items_t:
	if(item[:18] == 'htmalloccount_rank'):
		items.append(item)

items.sort()

calls     = []
time_avgs = []
time_maxs = []
time_mins = []
size_avgs = []
size_maxs = []
size_mins = []

nrank = 0
for item in items:
	rank = int(item[18:24])+1
	if(rank > nrank):
		nrank = rank

for item in items:
	rank = int(item[18:24])
	fp = open(root + item)
	print('Processing ' + root + item + '...')
	lines = fp.readlines()

	line = lines[1]
	words = line.split()

	call, time_avg, time_max, time_min, size_avg, size_max, size_min = int(words[0]), float(words[1]), float(words[2]), float(words[3]), float(words[4]), float(words[5]), float(words[6])
	calls.append(    call)
	time_avgs.append(time_avg)
	time_maxs.append(time_max)
	time_mins.append(time_min)
	size_avgs.append(size_avg)
	size_maxs.append(size_max)
	size_mins.append(size_min)

fp = open(root + 'htmalloccount_conclusion.dat', 'w')
fp.write('   Rank           CALLED      TIME_TOT(s)     TIME_AVG(μs)     TIME_MAX(μs)     TIME_MIN(μs)         SIZE_AVG         SIZE_MAX         SIZE_MIN\n')
for i in range(0, nrank):
	fp.write('%7d  %15d  %15f  %15f  %15f  %15f  %15f  %15f  %15f\n' % (i, calls[i], time_avgs[i]/1000000.0*calls[i], time_avgs[i], time_maxs[i], time_mins[i], size_avgs[i], size_maxs[i], size_mins[i]))

fp.close()