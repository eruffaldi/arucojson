
# MISSING
#  pelars.sssup.it/pelars/live/{session_id}
#
#   http://pelars.sssup.it/pelars/op/[op_id]
#           http://pelars.sssup.it/pelars/op/{op_id}/result
#   http://pelars.sssup.it/pelars/content/{session_id}/[{name}]
#   http://pelars.sssup.it/pelars/content/{name}

import requests
import json,os,pprint
import getpass
import datetime
import argparse
import sys,json


class Session:
    def __init__(self,c,id):
        self.connection = c
        self.id = id
    

class Connection:
    def __init__(self,interactive=True):
        self.session = requests.Session()
        if "PELARSTOKEN" in os.environ:
            token = os.environ["PELARSTOKEN"]
        else:
            token = None
            if os.path.isfile("token"):
                token = open("token","r").read().strip()
        self.token = token
        self.interactive = interactive
        self.prefix = 'http://pelars.sssup.it/pelars'
    def islogged(self):
        return self.token is not None
    def login(self,user,password=None):
        if self.token is not None:
            return True
        if password is None:
            if self.interactive:
                password = getpass.getpass("User password")
            else:
                return False
        r = self.session.post(self.prefix + '/password',params={"user":user,"pwd":password})
        if r.status_code != 200:
            return False
        else:
            self.token = json.loads(r.text)["token"]
            print "new token:",self.token
            open("token","w").write(self.token)
            return True
    def getsessions(self,goodonly=False):
        r = self.session.get(self.prefix + (goodonly and "/goodsession" or "/session"),params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return json.loads(r.text)
    def getsession(self,sessionid):
        r = self.session.get(self.prefix + "/session/%d" % sessionid,params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return json.loads(r.text)
    def getdatasraw(self,sessionid,type=None):
        r = self.session.get(self.prefix + "/data/%d" % sessionid,params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return r.text
    def getdatas(self,sessionid,type=None):
        r = self.session.get(self.prefix + "/data/%d" % sessionid,params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            q = json.loads(r.text)
            if type is not None:
                return [x for x in q if x["type"] == type]
            else:
                return q;
    def getdata(self,sessionid,dataid):
        r = self.session.get(self.prefix + "/data/%d/%d" % (sessionid),params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return json.loads(r.text)
    def getmultimedias(self,sessionid):
        r = self.session.get(self.prefix + "/multimedia/%d" % (sessionid),params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return json.loads(r.text)
    def downloadfile(self,u,fp):
        s = self.session.get(self.prefix + u,stream=True)
        print s.headers
        if s.status_code == 200:
            with open(fp,"wb") as f:
                for chunk in s.iter_content(chunk_size=1024): 
                    if chunk: # filter out keep-alive new chunks
                        f.write(chunk)
            return fp
        else:
            return ""
    def getmultimedia(self,sessionid,mediaid):
        """TODO: performance issue with large content"""
        r = self.session.get(self.prefix + "/multimedia/%d/%d" % (sessionid,mediaid),params={"token":self.token})
        if r.status_code != 200:
            return (None,r.headers,r.status_code)
        else:
            return (r.content,r.headers,r.status_code)
    def getphases(self,sessionid):
        r = self.session.get(self.prefix + "/phase/%d" % (sessionid),params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return json.loads(r.text)
    def getphase(self,sessionid,phaseid):
        """TODO: performance issue with large content"""
        r = self.session.get(self.prefix + "/phase/%d/%d" % (sessionid,phaseid),params={"token":self.token})
        if r.status_code != 200:
            return (None,r.headers,r.status_code)
        else:
            return (r.content,r.headers,r.status_code)    

    def getMeta(self,sessionid,multimediaid):
        r = self.session.get(self.prefix + "/multimedia/%d/%d/meta" % (sessionid,int(multimediaid)),params={"token":self.token})
        if r.status_code != 200:
            return (None,r.headers,r.status_code)
        else:
            return json.loads(r.content) 

    #TODO delete ops
    def get(self,suffix):
        if suffix.startswith("http:"):
            x = suffix
        else:
            x = self.prefix + suffix
        return self.session.get(x,params={"token":self.token})
    def getallops(self):
        r = self.session.get(self.prefix + "/op",params={"token":self.token})
        if r.status_code != 200:
            return None
        else:
            return json.loads(r.text)
    def makeop(self,session,json,name):
        return self.session.get
    def waitop(self,session,name):
        pass
    def getvideos(self,session):
        return ["http://pelars.sssup.it/pelars/completevideo/%d/webcam" % session,"http://pelars.sssup.it/pelars/completevideo/%d/kinect" % session]

    def getstatusbyid(self,id):
        r = self.session.get(self.prefix + "/op/%d" % (id),params={"token":self.token})
        if r:
            return json.loads(r.text)["status"]
        else:
            return "UNKNOWN"
    def hasop(self,session,name):
        return self.getopbyname(session,name)
    def getresultbyid(self,id):
        r = self.session.get(self.prefix + "/op/result/%d" % (id),params={"token":self.token})
        return r


    #def getcontents(self,sessionid):
    #    """TODO: performance issue with large content"""
    #   r = self.session.get(self.prefix + "/content/%d" % (sessionid),params={"token":self.token})
    #    if r.status_code != 200:
    #        return (None,r.headers,r.status_code)
    #    else:
    #        return json.loads(r.text)
    #def getcontent(self,sessionid,name):
    #    """TODO: performance issue with large content"""
    #    r = self.session.get(self.prefix + "/content/%d/%s" % (sessionid,name),params={"token":self.token})
    #    if r.status_code != 200:
    #        return (None,r.headers,r.status_code)
    #    else:
    #        return json.loads(r.text)
    def getcontents(self,session):
        r = self.session.get(self.prefix + "/content/%d" % session,params={"token":self.token})
        if r.status_code == 200:
            l = json.loads(r.text)
            if type(l) == dict:
                return []
            else:
                return l
        else:
            return []
    def getcontentnames(self,session):
        ss = self.getcontents(session)
        return [s["name"] for s in ss]
    def getcontent(self,session,name):
        r = self.session.get(self.prefix + "/content/%d/%s" % (session,name),params={"token":self.token})
        if r.status_code == 200:
            r = json.loads(r.text)
            if type(r) is dict and r["status"] == "Empty":
                return None
            else:
                return r
        else:
            return None        
    def delsessioncontent(self,session):
        return self.session.delete(self.prefix + "/content/%d" % (session),params={"token":self.token})
    def delcontent(self,session,name):
        return self.session.delete(self.prefix + "/content/%d/%s" % (session,name),params={"token":self.token})
    #def delcontent(self,opename):
    #    return self.session.delete(self.prefix + "/content/%s" % (opename),params={"token":self.token})

    def setvalid(self,session):
        return self.session.post(self.prefix+"/session/%d" % session,data=dict(is_valid=True),params={"token":self.token})
    def _doop(self,op):
        return self.session.put(self.prefix+"/op",data=json.dumps(op),params={"token":self.token})
    def ophandspeed(self,session,name="handspeed"):
        op = dict(type="hand_speed",session=session,name=name)
        return self._doop(op)
    def opmean(self,session,table,field,name):
        op = dict(type="mean",session=session,name=name,table=table,field=field)
        return self._doop(op)
    def oppresence(self,session,name="presence"):
        op = dict(type="presence",session=session,name=name)
        return self._doop(op)
    def getcalibration(self,id):
        r = self.session.get(self.prefix + "/calibration/%d" % (id),params={"token":self.token})
        if r:
            return json.loads(r.text)
        else:
            return "UNKNOWN"

#operation mean
#tables: tables.txt

#PUT http://pelars.sssup.it/pelars/op with JSON content
#GET http://pelars.sssup.it/pelars/op/{op_id}
#    Result is RESULT
#GET http://pelars.sssup.it/pelars/result/{op_id}
#    Result is STATUS
#GET http://pelars.sssup.it/pelars/content/{session_id}
#    Result is list of Operations with status
#GET http://pelars.sssup.it/pelars/content/{session_id}/{name}
#    Result is content of Operation
#DELETE http://pelars.sssup.it/pelars/content/{session_id}/{name}
#DELETE http://pelars.sssup.it/pelars/content/{session_id}

def timestr2epochms(s):
    #2016/02/03 12:27:25
    return (datetime.datetime.strptime(s, "%Y/%m/%d %H:%M:%S") - datetime.datetime.utcfromtimestamp(0)).total_seconds()*1000
def epochms2datetime(s):
    return datetime.datetime.utcfromtimestamp(s/1000.0)

def extendargs(parser):
    parser.add_argument('--no-interactive',dest="interactive",action="store_false")
    parser.add_argument('--interactive',dest="interactive",action="store_true")
    parser.add_argument('--keychain',action="store_true")
    parser.add_argument('--password')
    parser.add_argument('--user')
    return

def autoconnect(args):
    if args.interactive is None:
        args.interactive = True

    if args.user and args.keychain:
        import keyring
        if args.password is not None:
            keyring.set_password("pelarssession", args.user, args.password)
        else:
            print "getting password from keychain"
            args.password = keyring.get_password("pelarssession", args.user)
            if args.password == "" and args.interactive:
                import getpass
                args.password = getpass.getpass("pelars password for user " + args.user)
                keyring.set_password("pelarssession", args.user, args.password)
    elif args.user:
        if args.password is None and args.interactive:
            args.password = getpass.getpass("pelars password for user " + args.user)
    print "login ",args.user,args.password

    c = Connection()
    c.login(args.user,args.password)
    return c


if __name__ == "__main__":
    c = Connection()
    if not c.token:
        c.login("giacomo.dabisias@gmail.com")
    pprint.pprint(c.getsessions())

def downloadAll(session_list,c,fp,args):
    for sid in session_list:
        sid = int(sid)
        s = c.getsession(sid)
        k = fp("S%d_session.json" % sid)
        if args.force or not os.path.isfile(k):
            open(k,"w").write(json.dumps(s))
        k = fp("S%d_datas.json" % sid)
        if args.force or not os.path.isfile(k):
            open(k,"w").write(json.dumps(c.getdatas(sid)))

        k = fp("S%d_multimedias.json" % sid)
        sm = c.getmultimedias(sid)
        if sm is None:
            sm = []
        print "multimedia entities are ",len(sm),"items"
        if args.force or not os.path.isfile(k) and sm is not None:
            for x in sm:
                x["data"] = x["data"].replace("http://pelars.sssup.it/pelars","")
            open(k,"w").write(json.dumps(sm))

        k = fp("S%d_phases.json" % sid)
        if args.force or not os.path.isfile(k):
            open(k,"w").write(json.dumps(c.getphases(sid)))

        sd = fp("S%d/media/" % sid)
        if args.force or not os.path.isdir(sd):
            os.makedirs(sd)

        tsd = fp("S%d/thumb/" % sid)
        if args.force or not os.path.isdir(tsd):
            os.makedirs(tsd)

        k = fp("S%d_content.json" % sid)
        if args.force or not os.path.isfile(k):
            cm  = c.getcontents(sid)
            open(k,"w").write(json.dumps(cm))

        k = fp("S%d_calibration.json" % sid)
        if args.force or not os.path.isfile(k):
            xsm  = c.getcalibration(sid)
            open(k,"w").write(json.dumps(xsm))

        if sm is not None:
            for s in sm:
                mid = fp(os.path.join(sd,"%d.%s" % (s["id"],s["mimetype"])))
                if args.force or not os.path.isfile(mid):
                    print "check item",mid
                    r = c.get(s["data"])
                    print mid,s["data"],r.status_code
                    if r.status_code == 200:
                        open(mid,"wb").write(r.content)
                if s["type"] == "image":
                    tmid = fp(os.path.join(tsd,"%d.%s" % (s["id"],s["mimetype"])))
                    if args.force or not os.path.isfile(tmid):
                        r = c.get(s["data"]+"/thumb")
                        print tmid,r.status_code
                        if r.status_code == 200:
                            open(tmid,"wb").write(r.content)
        pureurl = ["/completevideo/%d/webcam" % sid, "/completevideo/%d/kinect" % sid]
        purefiles = [x.split("/")[-1] + ".mp4" for x in pureurl]
        for u,f in zip(pureurl,purefiles):
            ffp = os.path.join(fp("S%d" % sid),f)
            if (args.force or not os.path.isfile(ffp)):
                print "download big",u,ffp
                #c.downloadfile(u,ffp)


