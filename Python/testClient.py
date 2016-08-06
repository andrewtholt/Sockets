#!/usr/bin/env python

import sysv_ipc as ipc
import parameter as value
import sys

SUPER=1234

def sendSet(to,name,value):

    msg="SET " + name + " " + value

    print msg
    to.send(msg)

def sendDump(to):
    msg="DUMP"
    print msg
    to.send(msg)

def main():
    print "Main"

    IAM=1235
    myMq=ipc.MessageQueue(IAM,flags=ipc.IPC_CREAT)

    superMq=ipc.MessageQueue(SUPER)

    sendSet(superMq,"TEST/QID",str(myMq.key))

    print "==========++"
    print myMq.key
    print superMq.key
    print "==========++"


    sendDump(superMq)
    runFlag=True

    stuff={}

    sys.exit(0)

    while runFlag:
        b=mq.receive(block=True,type=0)

        data=b[0].split()

        if data[0] == "GET":
            print "GET"
            result=stuff.get(data[1],"UNKNOWN")
            print result
        elif data[0] == "SET":
            print "SET"
        else:
            print "What ?"



main()

