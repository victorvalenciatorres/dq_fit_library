import matplotlib.pyplot as plt
import array as arr
import numpy as np
import os
import sys
import argparse
import ROOT
from os import path
from ROOT import TCanvas, TF1, TFile, TPaveText, TMath, TH1F, TString, TLegend, TRatioPlot, TGaxis
from ROOT import gROOT, gBenchmark, gPad, gStyle, kTRUE, kFALSE

def Store_Histograms_From_File(fIn, histType):
    histArray = []
    for key in fIn.GetListOfKeys():
        kname = key.GetName()
        if (fIn.Get(kname).ClassName() == histType):
            histArray.append(fIn.Get(kname))
    return histArray