#!/usr/bin/python

import sys
import pickle

def parseLine(pairs, wordMap):
    for pair in pairs:
        [index, count] = pair.split(":")
        wordMap[int(index)-1]= wordMap[int(index)-1] + int(count)

# Main function to use with a filename and word count to get a word map
def tallyFile(sampleFile, size):
    f = open(sampleFile,'r')
    posMap = [0]*int(size)
    negMap = [0]*int(size)
    posCount = 0
    negCount = 0
    for line in f:
        vals = line.split()
        if vals[0] == "1":
            parseLine(vals[1:],posMap)
            posCount += 1
        else:
            parseLine(vals[1:],negMap)
            negCount += 1
    return (posMap, negMap, posCount, negCount)


        
