#!/usr/bin/env python

import parameter as p

fred=p.parameter("TEST")

print fred.getValue()

stuff={}

stuff["NAME"]=fred

tst=stuff["NAME"]

print tst.getValue()

