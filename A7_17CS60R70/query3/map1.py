NUM_TAGS = 5

fp = open("../lastfm_dataset.csv")
data = fp.readlines()

i=0
for line in data:
	songDetail = line.split(",")
	artistList = songDetail[0].split(";")
	tagList = songDetail[1].split(";")

	if len(tagList) >= NUM_TAGS and len(artistList) > 1 and len(songDetail) == 4:
		for i in range(0, len(artistList)):
			for j in range(i+1, len(artistList)):
				if artistList[i].strip() < artistList[j].strip():
					print artistList[i].strip() + "__" + artistList[j].strip()
				else:
					print artistList[i].strip() + "__" + artistList[j].strip()