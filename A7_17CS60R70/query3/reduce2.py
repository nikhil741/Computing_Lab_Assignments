import sys
import os

if not os.path.exists("../output"):
	os.mkdir("../output")

NUM_SONGS = 2


fp = open("../output/query3Result.txt", "w")

for line in sys.stdin:
	line = line.strip("\n")
	key_value_pair = line.split("::")
	if int(key_value_pair[1]) > NUM_SONGS:
		print key_value_pair[0]
		fp.write(key_value_pair[0])
		fp.write("\n")

fp.close()