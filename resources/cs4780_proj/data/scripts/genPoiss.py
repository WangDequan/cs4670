#!/usr/bin/python


import numpy
import sys

#Simple script that generates n poisson samples and outputs to stdout
#Takes two argument: lambda and n



def genAndPrintSamples(l, n):
    samples = numpy.random.poisson(l,n)
    for s in samples:
        print s


if __name__=="__main__":
    if len(sys.argv)!=3:
        print "USAGE: genPoiss.py lambda n"
    else:
        genAndPrintSamples(int(sys.argv[1]),int(sys.argv[2]))
