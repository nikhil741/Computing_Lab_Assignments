import sys
import operator
import os

if not os.path.exists("../output"):
	os.mkdir("../output")

fp = open("../output/query2Result.txt", "w")

MAX_TAGS = 10

count = 0

previous = "notSeen"
for line in sys.stdin:
	count += 1
	line = line.strip('\n')
	key_value_pair = line.split("::")
	#print key_value_pair
 	result = "Tag=" + key_value_pair[1].strip() + " Frequency=" + key_value_pair[0].strip()
 	print result
 	fp.write(result)
 	fp.write("\n")
 	if count == MAX_TAGS:
 		fp.close()
 		break