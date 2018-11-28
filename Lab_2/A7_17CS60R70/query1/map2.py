import sys

for line in sys.stdin:
	line = line.strip("\n")
	key_value_list = line.split(":")
	valueList = key_value_list[1].split("|||")
	
	i=0
	for value in valueList:
		if i==0:
			i+=1
			print value, ":", key_value_list[0],
		elif value != ' ' or value!='\n':
			print "|||" + value,
	print