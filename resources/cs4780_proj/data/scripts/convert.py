#!/usr/bin/python
import os
import sys


def decodeTrainLine(line):
    #Id,Title,FullDescription,LocationRaw,LocationNormalized,ContractType,ContractTime,Company,Category,SalaryRaw,SalaryNormalized,SourceName
    dirtyEls = line.split('\"')
    for i in range(len(dirtyEls)):
        if i % 2:
            dirtyEls[i] = dirtyEls[i].replace(',','')
            
    els = (''.join(dirtyEls)).split(',')
    dec = [('__title__' + x) for x in els[1].split(' ')]
    dec.extend(els[2].split(' '))
    dec.append('__location__' + els[4])
    dec.append('__ctype__' + els[5])
    dec.append('__ctime__' + els[6])
    dec.append('__cat__' + els[7])
    dec.append('__source__' + els[10])
    return (els[10],dec)

def decodeTestLine(line): #May not be needed
    pass

def placeInBucket(salary):
    return int(salary)/5000


def convertFile(inputFile, outputFile, featureMapping):
    #Assume first line isn't needed
    inputFile.readline()
    featureIndex = 1
    for line in inputFile:
        (salary, features) = decodeTrainLine(line)
        bucket = placeInBucket(salary)
        lineFeats = {}
        for feature in features:
            if not(featureMapping.has_key(feature)):
                featureMapping[feature]=featureIndex
                featureIndex +=1
                if (featureIndex % 1000 == 0):
                    print "Seen %d unique features" % featureIndex
            index = featureMapping[feature]
            if lineFeats.has_key(index):
                lineFeats[index] = lineFeats[index]+1
            else:
                lineFeats[index] = 1

        outputFile.write(str(bucket) + ' ')
        for feat in sorted(lineFeats.keys()):
            outputFile.write('%d:%d ' % (feat, lineFeats[feat]))
        outputFile.write('\n')
    print "Total feature count: %d " % featureIndex
    return featureMapping
        
def readAndConvertFile(inputFilename, outputFilename, featureMapping={}):
    i = open(inputFilename,'r')
    o = open(outputFilename,'w')
    mapping =  convertFile(i,o,featureMapping)
    i.close()
    o.close()
    return mapping


if __name__=="__main__":
    readAndConvertFile(sys.argv[1],sys.argv[2])
