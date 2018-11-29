import sys
import operator

for line in sys.stdin:
	line = line.strip('\n')
	key_value_pair = line.split("::")
 	print key_value_pair[1].strip() + "::" + key_value_pair[0].strip()