fp = open("../lastfm_dataset.csv")
#fp = open("nikhil.txt")
data = fp.readlines()

i=0
for line in data:
	songDetail = line.split(",")
	if len(songDetail) == 4:
		artistList = songDetail[0].split(";")
		for artist in artistList:
			print artist.strip() + "::" + songDetail[3].strip()