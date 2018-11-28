fp = open("../lastfm_dataset.csv")
data = fp.readlines()

i=0
for line in data:
	line = line.strip("\n")
	songDetail = line.split(",")
	#Check Data Validity
	if len(songDetail) != 4:
		continue
	
	listOfArtist = songDetail[0].split(";")
	
	artistAsString = '|||'.join(listOfArtist)
	#print artistAsString
	for artist in listOfArtist:
		print artist.strip() + ":" + artistAsString.strip()	
	i+=1	
	# if i==500:
	# 	break