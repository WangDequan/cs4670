#!/usr/bin/python

# Simple script take's a directory full of training sets and build's a multivariate Naive Bayes interpretter out of them.
# The extension of each file in the directory is presumed to be it's target label value

# System level imports
from os import listdir
from os.path import isfile, join
import sys

# Import bayes stuff
import wordCounter
import mnb

def makeFileMap(dirName):
    files = [ join(dirName,f) for f in listdir(dirName) if isfile(join(dirName,f)) ]
    fMap = {}
    for f in files:
        print 'Found file %s ' % f
        extension = f[f.rindex('.')+1:]
        fMap[extension] = f
    return fMap

def buildAndRunClassifier(trainFile, size, testFile, outputFile):
    (posMap, negMap, posCount, negCount) = wordCounter.tallyFile(trainFile,size)
    mnb.classifyFile(posMap, posCount, negMap, negCount, testFile, outputFile)

def buildAndSaveClassifier(trainFile,size,outputFile):
    output = open(outputFile,'w')
    (posMap,negMap,posCount,negCount) = wordCounter.tallyFile(trainFile,size)
    pos = (posMap,posCount)
    neg = (negMap,negCount)
    both = (pos,neg)
    pickle.dump(both,output)
    output.close()

def runAllClassifiers(sampleDir, outputDir, featureCount, testFile):
    fMap = makeFileMap(sampleDir)
    for f in fMap:
        outputFile = join(outputDir,"output.%s" % f)
        buildAndRunClassifier(fMap[f], featureCount, testFile, outputFile)

if __name__=="__main__":
    #if len(sys.argv)!=4:
    #    print "Usage: bayes.py TRAINING_DIRECTORY FEATURE_COUNT OUTPUT_FILE"
    #else:
    #    buildAndSaveClassifier(sys.argv[1],sys.argv[2],sys.argv[3])
        
    if len(sys.argv)!=5:    
        print "Usage: bayes.py TRAINING_DIRECTORY OUTPUT_DIRECTORY FEATURE_COUNT TEST_FILE"
    else:
        runAllClassifiers(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])

    




