#!/usr/bin/python

from bayes import makeFileMap

import sys

def findBest(currentBestFile, currentBestVal, nextFile, nextVal):
    if currentBestFile == None or currentBestVal <nextVal:
        return (nextFile, nextVal)
    return (currentBestFile, currentBestVal)

def computeAccuracy(valFile, computedFiles, outputFile=None):
    correct = 0
    total = 0
    for line in valFile:
        total = total + 1
        correctValue = int(line.split()[0])
        currentBestFile = None
        currentBestVal = None
        for fIndex in computedFiles:
            line = computedFiles[fIndex].readline().rstrip()
            if len(line)==0:
                continue
            (currentBestFile, currentBestVal) = findBest(currentBestFile,currentBestVal, fIndex, float(line))
        if (outputFile!=None):
            outputFile.write('%d %d\n' % (currentBestFile,correctValue))
        if currentBestFile == correctValue:
            correct = correct + 1
    return float(correct)/float(total)

def main():
    files = []
    for i in range(10):
        files[len(files):] = [open('output.%s' % i, 'r')]
    print computeAccuracy(open('digits.test','r'), files)

def evaluateFiles(valFile, outputFileDir, outputFile):
    fMap = makeFileMap(outputFileDir)
    computedFiles = {}
    for f in fMap:
        computedFiles[int(f)] = open(fMap[f],'r')
    f = open(outputFile,'w')
    accuracy = computeAccuracy(valFile,computedFiles,f)
    f.close()
    print "Accuracy: %f" % accuracy
    return accuracy
    
if __name__ == '__main__':
    if len(sys.argv)!= 4:
        print "USAGE: VAL_FILE OUTPUT_DIR OUTPUT_FILE"
    else:
        print evaluateFiles(open(sys.argv[1]),sys.argv[2], sys.argv[3])
