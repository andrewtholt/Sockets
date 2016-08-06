#!/usr/bin/env python

import sysv_ipc as ipc
import parameter as p
import engine


def main():
    print "Main"

    mq=ipc.MessageQueue(1234,flags=ipc.IPC_CREAT)

    print mq

    runFlag=True

    me=engine.engine()
    me.setVerbose()

    while runFlag:
        b=mq.receive(block=True,type=0)




main()

