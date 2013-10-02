#!/bin/bash
EXE1=code/Features
EXE2=samplesolution/F*x
SDIR=imagesets
SET=graf
ODIR=data
PRE=img
EXT=ppm

rm $ODIR/*

$EXE1 computeFeatures $SDIR/$SET/$PRE"1."$EXT $ODIR/"1hs.f" 2 1
$EXE1 computeFeatures $SDIR/$SET/$PRE"1."$EXT $ODIR/"1hm.f" 2 2
$EXE1 computeFeatures $SDIR/$SET/$PRE"2."$EXT $ODIR/"2hs.f" 2 1
$EXE1 computeFeatures $SDIR/$SET/$PRE"2."$EXT $ODIR/"2hm.f" 2 2

$EXE2 computeFeatures $SDIR/$SET/$PRE"1."$EXT $ODIR/"1hsa.f" 2 1
$EXE2 computeFeatures $SDIR/$SET/$PRE"1."$EXT $ODIR/"1hma.f" 2 2
$EXE2 computeFeatures $SDIR/$SET/$PRE"2."$EXT $ODIR/"2hsa.f" 2 1
$EXE2 computeFeatures $SDIR/$SET/$PRE"2."$EXT $ODIR/"2hma.f" 2 2

$EXE1 roc $ODIR/"1hs.f" $ODIR/"2hs.f" $SDIR/$SET/"H1to2p" 1 $ODIR/rocfile1hs $ODIR/aucfile1hs
$EXE1 roc $ODIR/"1hm.f" $ODIR/"2hm.f" $SDIR/$SET/"H1to2p" 1 $ODIR/rocfile1hm $ODIR/aucfile1hm
$EXE1 roc $ODIR/"1hs.f" $ODIR/"2hs.f" $SDIR/$SET/"H1to2p" 2 $ODIR/rocfile2hs $ODIR/aucfile2hs
$EXE1 roc $ODIR/"1hm.f" $ODIR/"2hm.f" $SDIR/$SET/"H1to2p" 2 $ODIR/rocfile2hm $ODIR/aucfile2hm

$EXE2 roc $ODIR/"1hsa.f" $ODIR/"2hsa.f" $SDIR/$SET/"H1to2p" 1 $ODIR/rocfile1hsa $ODIR/aucfile1hsa
$EXE2 roc $ODIR/"1hma.f" $ODIR/"2hma.f" $SDIR/$SET/"H1to2p" 1 $ODIR/rocfile1hma $ODIR/aucfile1hma
$EXE2 roc $ODIR/"1hsa.f" $ODIR/"2hsa.f" $SDIR/$SET/"H1to2p" 2 $ODIR/rocfile2hsa $ODIR/aucfile2hsa
$EXE2 roc $ODIR/"1hma.f" $ODIR/"2hma.f" $SDIR/$SET/"H1to2p" 2 $ODIR/rocfile2hma $ODIR/aucfile2hma

gnuplot plot.roc.txt
gnuplot plota.roc.txt
