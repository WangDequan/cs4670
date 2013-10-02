#!/bin/bash
EXE1=code/Features
EXE2=samplesolution/F*x
SDIR=imagesets
SET=yosemite
ODIR=data

$EXE1 computeFeatures $SDIR/$SET/$SET"1.jpg" $ODIR/"1hs.f" 2 1
$EXE1 computeFeatures $SDIR/$SET/$SET"1.jpg" $ODIR/"1hm.f" 2 2
$EXE1 computeFeatures $SDIR/$SET/$SET"2.jpg" $ODIR/"2hs.f" 2 1
$EXE1 computeFeatures $SDIR/$SET/$SET"2.jpg" $ODIR/"2hm.f" 2 2

$EXE2 computeFeatures $SDIR/$SET/$SET"1.jpg" $ODIR/"1hsa.f" 2 1
$EXE2 computeFeatures $SDIR/$SET/$SET"1.jpg" $ODIR/"1hma.f" 2 2
$EXE2 computeFeatures $SDIR/$SET/$SET"2.jpg" $ODIR/"2hsa.f" 2 1
$EXE2 computeFeatures $SDIR/$SET/$SET"2.jpg" $ODIR/"2hma.f" 2 2

$EXE1 roc $ODIR/"1hs.f" $ODIR/"2hs.f" $SDIR/$SET/"H1to2p" 1 rocfile1hs aucfile1hs
$EXE1 roc $ODIR/"1hm.f" $ODIR/"2hm.f" $SDIR/$SET/"H1to2p" 1 rocfile1hm aucfile1hm
$EXE1 roc $ODIR/"1hs.f" $ODIR/"2hs.f" $SDIR/$SET/"H1to2p" 2 rocfile2hs aucfile2hs
$EXE1 roc $ODIR/"1hm.f" $ODIR/"2hm.f" $SDIR/$SET/"H1to2p" 2 rocfile2hm aucfile2hm

$EXE2 roc $ODIR/"1hsa.f" $ODIR/"2hsa.f" $SDIR/$SET/"H1to2p" 1 rocfile1hsa aucfile1hsa
$EXE2 roc $ODIR/"1hma.f" $ODIR/"2hma.f" $SDIR/$SET/"H1to2p" 1 rocfile1hma aucfile1hma
$EXE2 roc $ODIR/"1hsa.f" $ODIR/"2hsa.f" $SDIR/$SET/"H1to2p" 2 rocfile2hsa aucfile2hsa
$EXE2 roc $ODIR/"1hma.f" $ODIR/"2hma.f" $SDIR/$SET/"H1to2p" 2 rocfile2hma aucfile2hma

gnuplot plot.roc.txt
gnuplot plota.roc.txt
