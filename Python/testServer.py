#!/usr/bin/env python

import sysv_ipc as ipc
import parameter as value


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
            result=stuff.get(data[1],"UNKNOWN")
            print result
        elif data[0] == "SET":
            print "Cmd  : SET"
            print "Name :"+data[1]
            print "Value:"+data[2]

            stuff[ data[1] ] = data[2]
            print stuff
        else:
            print "What ?",b



main()

