fp = open("../lastfm_dataset.csv")
data = fp.readlines()

for line in data:
	songDetail = line.split(",")
	if len(songDetail) == 4:
		tagList = songDetail[1].split(";")
		for tag in tagList:
			if len(tag) > 0:
				print tag.strip()