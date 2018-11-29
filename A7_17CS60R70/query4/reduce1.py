import sys
import operator
import os

#Create Index Directory
if not os.path.exists("./index"):
	os.mkdir("./index")


fp = open("./index.txt", "w")


previousKey = "notSeen"
reduceDictionary = {}


for line in sys.stdin:
    line = line.strip("\n")
    key_value = line.split("::")

    #NewKey
    if previousKey != key_value[0]:
        if previousKey != "notSeen":
            fp.write(previousKey + "::" + "|||".join(reduceDictionary[previousKey]) + "\n")
            
            if len(previousKey) != 0:
            	fileIndex = open("./index/" + previousKey.replace("/", "_")[:100], "w")
            	fileIndex.write("|||".join(reduceDictionary[previousKey]) + "\n")
            
            fileIndex.close()
            reduceDictionary = {}
        previousKey = key_value[0]
        newList = []
        newList.append(key_value[1].strip())
        reduceDictionary[key_value[0]] = newList
    
    #Old Key Continues
    else:
        newList = reduceDictionary[key_value[0]]
        newList.append(key_value[1].strip())
        reduceDictionary[key_value[0]] = newList