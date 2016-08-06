#!/usr/bin/env python

import sysv_ipc as ipc
import parameter as p


def main():
    print "Main"

    mq=ipc.MessageQueue(1234,flags=ipc.IPC_CREAT)

    print mq

    runFlag=True

    stuff={}

    while runFlag:
        b=mq.receive(block=True,type=0)

        data=b[0].split()

        if data[0] == "GET":
            print "GET"

            if stuff.has_key(data[1]):
                print "Found"
                print (stuff.get(data[1]).getValue())
            else:
                print "UNKNOWN"

        elif data[0] == "SET":
            print "Cmd  : SET"
            print "Name :"+data[1]
            print "Value:"+data[2]

            stuff[ data[1] ] = p.parameter(data[2])
        elif data[0] == "DUMP":
            print "==== START ====="
            for key in stuff:
                print key
                print (stuff[key]).getValue()
            print "==== END   ====="
        else:
            print "What ?",b



main()

