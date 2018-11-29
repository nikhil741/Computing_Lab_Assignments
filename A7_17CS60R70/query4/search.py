import sys

artist = sys.argv[1]

try:
	fp = open("./index/" + artist.replace("/", "_")[:100])
	songs = fp.read()
	songList = songs.split("|||")

	for song in songList:
		if song != "\n" and len(song)>0:
			print song

except:
	print "Error"