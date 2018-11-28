import sys
import operator

reduceDictionary = {}
reduceCountDict = {}
previous = "notCome"

count = 0
for line in sys.stdin:
    
    flag_value = 0
    key_value = line.split(":")
    values = (key_value[1].strip("\n")).split("|||")
    newList = []
    
    #Append Co-Artist to Newlist
    if values > 1:
        for value in values:
            #Append 
            if not key_value[0] == value:
                newList.append(value)
    
    #New Key
    if key_value[0].strip() != previous:

    	
    	if previous!="notCome":
    		print previous + ":" + str(count) +  "|||" + "|||".join(list(set(reduceDictionary[previous])))
    		
            #DataDelete of previous Key
    		reduceDictionary = {}
    		reduceCountDict = {}
    	reduceCountDict[key_value[0]] = 1
    	reduceDictionary[key_value[0]] = newList
        previous = key_value[0]
        count = 1
    

    #Previous Key Continues
    else:
    	reduceCountDict[key_value[0]] += 1
    	previousList = reduceDictionary[key_value[0]]
    	newList = newList + previousList
    	reduceDictionary[key_value[0]] = newList
    	count += 1


#sorted_dict = sorted(reduceCountDict.items(), key=operator.itemgetter(1), reverse=True)

#print sorted_dict[0][0]
#print reduceDictionary[sorted_dict[0][0]]