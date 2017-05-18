from pelarslas import *
import pelarslas
import sys,json
import argparse
from subprocess import call

if __name__ == "__main__":

	import argparse

	parser = argparse.ArgumentParser(description='Download Session')
	parser.add_argument('--user')
	parser.add_argument('--no-interactive',dest="interactive",action="store_false")
	parser.add_argument('--interactive',dest="interactive",action="store_true")
	parser.add_argument('--keychain',action="store_true")
	parser.add_argument('--password')
	parser.add_argument('--dest',help="destination folder",default=".")
	parser.add_argument('--list',action="store_true")
	parser.add_argument('--goodlist',action="store_true")
	parser.add_argument('--force',action="store_true",help="forces download")
	parser.add_argument('sessions',type=int,nargs="*")

	#pelarslas.extendargs(parser)
	args = parser.parse_args()
	c = pelarslas.autoconnect(args)
	
	fp = lambda x: os.path.join(args.dest,x)
	session_list = []

	if args.list or args.goodlist:
		s = c.getsessions(args.goodlist)
		#pprint.pprint(s)
		#json.dump(s,open(fp(args.list and "sessions.json" or "goodsessions.json"),"wb"))
		#TODO: parse session ids from s
		for p in s:
			session_list.append(p["session"])
		print session_list

	else:
		session_list = args.sessions

	for sid in session_list:
		if(sid > 1499):
			videoname = "./data/hands_" + str(sid) + ".mp4"
			#w_video , k_video = c.getvideos(sid)
			call(["scp", "poweredge@pelars.sssup.it:uploads/session_videos/"+str(sid)+"/webcam.mp4", videoname])
			call(["/home/lando/pelars/arucojson/build/aruco2json", "/home/lando/pelars/pelars_doc/python/c920_800_2.yml", "3.5", videoname, "true"])
			call(["rm", videoname])
			call(["gzip", videoname + ".json"])
			jsonfile = videoname + ".json.gz"
			call(["scp", jsonfile, "poweredge@pelars.sssup.it:pelars/handsnew/"])
