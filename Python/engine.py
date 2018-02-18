#!/usr/bin/env python

import parameter as p

class engine:

    param={}
    verbose=False

    def __init__(self):
        if self.verbose:
            print "Engine Created"

    def __del__(self):
        if self.verbose:
            print "Engine Deleted"

    def setVerbose(self):
        self.verbose=True
        
    def setQuiet(self):
        self.verbose=False
        
    def getValueObject(self,name):
        if self.param.has_key( key ):
            return self.param[ name ]
        else:
            return None

    def command(self,cmd):
        print "Engine Command"

        data=cmd.split()

        if data[0] == "GET":
            print "GET"
            if len(data) == 2:
                key=data[1]
                return self.param[ key ].getValue()

        elif data[0] == "SET":
            print "SET"
            if len(data) == 3:
                key=data[1]
                val=data[2]
                
                if self.param.has_key( key ):
                    print "Param Exists"
                    return self.param[ key ].setValue(val)
                    
                else:
                    print "Param Create"
                    public=True
                    self.param[ key ] = p.parameter( val, public )
                    return "OK"

        elif data[0] == "DUMP":
            print "DUMP"
            
        elif data[0] == "SUB":
            if len(data) == 3:
                key=data[1]
                qid=data[2]
                if self.param.has_key( key ):
                    self.param[ key ].subscribe(qid)
                    
            
        else:
            print "What ? ", cmd
    def dump(self):
        print "Dump"
        for k in self.param:
            print k+"\t", self.param[ k ].dump()
            
        

def test():
    print "Testing"

    me=engine()
    me.setVerbose()
    print me.command("SET TEST 1234")
    print me.command("SET TEST 1234")
    print me.command("SET TESTING 4321")
    me.command("SUB TEST 4321")
    
    print "==="
    print me.command("GET TEST")
    print "==="
    
    me.dump()

if __name__ == "__main__":
    test()



