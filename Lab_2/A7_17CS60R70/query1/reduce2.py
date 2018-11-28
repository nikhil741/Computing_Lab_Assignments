import sys
import os

if not os.path.exists("../output"):
	os.mkdir("../output")

fp = open("../output/query1Result.txt", "w")

for line in sys.stdin:
	line = line.strip("\n")
	key_value_pair = line.split(":")
	artistList = key_value_pair[1].split("|||")
	
	message = "Artist=" + artistList[0].strip() + " Frequency=" + str(key_value_pair[0]) + "\n"
	
	message += "Co-Artist are as following:-"
	print message
	fp.write(message + "\n")
	i=0
	for artist in artistList:
		if i!=0:
			print artist.strip()
			fp.write(artist.strip())
		else:
			i += 1
	fp.close()
	break