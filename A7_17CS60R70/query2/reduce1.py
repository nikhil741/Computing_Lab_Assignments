import sys
import operator

count = 0

previous = "notSeen"

for line in sys.stdin:
	line = line.strip('\n')
 	key_value = line

	#New Key
 	if key_value != previous:
 		if previous != "notSeen":
 			print previous, "::", str(count)
		previous = key_value
		count = 1
	
	#Previous Key
	else:
		count += 1
