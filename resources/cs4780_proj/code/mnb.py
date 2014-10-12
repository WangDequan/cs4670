#!/usr/bin/python

import sys
import pickle
import math

document_total = 0
neg_doc_total = 0
neg_words_total = 0
pos_doc_total = 0
pos_words_total = 0
total_vocab_size = 0

correct_pos_cost = 0
correct_neg_cost = 0
false_pos_cost = 1
false_neg_cost = 1

def getWordConditionalProb(wordMap, wordIndex, totalWordCount):
    return math.log((1 + wordMap[wordIndex-1])) - math.log((total_vocab_size + totalWordCount))

def getProbOfY(y):
    if int(y) == 1:
        return float(pos_doc_total)/float(document_total)
    else:
        return float(neg_doc_total)/float(document_total)

def calculateDocumentProbability(split_line,posWordMap,negWordMap):
    negProb = math.log(getProbOfY(-1))
    posProb = math.log(getProbOfY(1))
    for pair in split_line:
        [wordIndex, wordCount] = pair.split(":")
        negProb = negProb + float(wordCount)*getWordConditionalProb(negWordMap,int(wordIndex),neg_words_total)
        posProb = posProb + float(wordCount)*getWordConditionalProb(posWordMap,int(wordIndex),pos_words_total)
    return (negProb, posProb)

def testLine(raw_line, posWordMap, negWordMap):
    data_points = raw_line.split()
    label = data_points[0]
    (negProb,posProb) = calculateDocumentProbability(data_points[1:], posWordMap, negWordMap)
    return (negProb, posProb)

def testMNB(testFile, pickleNeg, picklePos, num_pos, num_neg):
    f = open(testFile, 'r')
    neg_word_map = pickle.load(open(pickleNeg,'r'))
    pos_word_map = pickle.load(open(picklePos,'r'))
    global neg_doc_total
    neg_doc_total = num_neg
    global neg_words_total
    neg_words_total = sum(neg_word_map)
    global pos_doc_total
    pos_doc_total = num_pos
    global pos_words_total
    pos_words_total = sum(pos_word_map)
    global total_vocab_size
    total_vocab_size = len(neg_word_map)
    global document_total
    document_total = neg_doc_total + pos_doc_total

    totalRuns = 0
    correct = 0
    false_pos = 0
    false_neg = 0
    for line in f:
        totalRuns = totalRuns + 1
        (label, prediction)  = testLine(line, pos_word_map, neg_word_map)
        if (int(label)==prediction):
            correct = correct + 1
        elif (prediction==-1):
            false_neg = false_neg + 1
        else:
            false_pos = false_pos + 1
        if (totalRuns % 5000 == 0):
            print "%d Documents Analyzed" % totalRuns
    return (correct,totalRuns,false_pos,false_neg)

def classifyFile(positiveWordMap, positiveWordCount, negativeWordMap, negativeWordCount, testFileName, outputFileName):
    testFile = open(testFileName, 'r')
    
    global neg_doc_total
    neg_doc_total = int(negativeWordCount)
    global neg_words_total
    neg_words_total = sum(negativeWordMap)
    global pos_doc_total
    pos_doc_total = int(positiveWordCount)
    global pos_words_total
    pos_words_total = sum(positiveWordMap)
    global total_vocab_size
    total_vocab_size = len(negativeWordMap)
    global document_total
    document_total = neg_doc_total + pos_doc_total

    print "Positive Doc: %d Negative Doc: %d Total: %d" % (pos_doc_total,neg_doc_total,document_total)
    if (pos_doc_total == 0 or neg_doc_total ==0):
        return
    outputFile = open(outputFileName,'w')
    for line in testFile:
        (negProb, posProb) = testLine(line, positiveWordMap, negativeWordMap)
        outputFile.write("%f\n" % posProb)
    

    
if __name__=="__main__a":
    if len(sys.argv)< 6:
        print "Usage: mnb.py NEGATIVE_WORDMAP_PICKLE POSITIVE_WORDMAP_PICKLE POSITIVE_DOCUMENT_COUNT NEGATIVE_DOCUMENT_COUNT TEST_SET [FALSE_POS_COST] [FALSE_NEG_COST]"
    else:
        if len(sys.argv)==8:
            false_pos_cost = int(sys.argv[6])
            false_neg_cost = int(sys.argv[7])
        (correct,totalRuns,false_pos,false_neg)= testMNB(sys.argv[5], sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
        print "Accuracy: %f (%d correct, %d total) | %d False Positives and %d False Negatives" % ((float(correct)/float(totalRuns)),correct,totalRuns, false_pos, false_neg)
    

