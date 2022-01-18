#!/usr/bin/env python

import sys
import time
import select

def hexit( text ):
	"""Prevede retezec na hexadecimalni zapis. Jednotlive znaky jsou oddeleny 
	mezerou."""
	h = []
	for c in text:
		h.append( "%02X" % (ord(c)) )
	return " ".join(h)


if len(sys.argv) < 2 or len(sys.argv) > 3:
	print "./listener.py PORT [verbose]"
	sys.exit(0)

file_input = open(sys.argv[1], "r")

samp_freq = 23

last_data = 0
last_ts = 0

data_out = ""

try:
	while True:
		line = file_input.readline()
		if len(line) == 0:
			break
		if line[0] == 'S':
			continue

		s = line.split(',')
		ts = int(s[0])
		data = int(s[1])

		if not last_data == data:
			count = int(round(float(ts - last_ts)/samp_freq))
			write = True

			if len(data_out) == 0 and last_data == 0:
				write = False

			if write:
				data_out += str(last_data) * count

#			print "dts = %d, count = %f(%d), data = %d, write = %d, data_oout = %s" % (ts-last_ts, float(ts-last_ts)/float(samp_freq), count, last_data, write, data_out)

#		print "data = %d, dts = %d" % (last_data, ts-last_ts)
		if last_data == 0:
			if (ts - last_ts) > 100*samp_freq:
				#if len(data_out) > 0:
				print data_out.rstrip("0")
				data_out = ""


		last_data = data
		last_ts = ts


except EOFError:
	pass

print data_out.rstrip("0")
