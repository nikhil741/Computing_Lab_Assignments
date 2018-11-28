import sys
import operator

previousKey = "notSeen"
count = 0

for line in sys.stdin:
	line = line.strip("\n")
	key_value = line

    #New Key
	if key_value != previousKey:
		if previousKey != "notSeen":
			print previousKey + "::" + str(count)
		count = 1
		previousKey = key_value
    
	#old Key
	else:
		count += 1