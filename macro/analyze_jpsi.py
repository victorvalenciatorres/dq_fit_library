from ast import parse
from time import process_time_ns
from tkinter import Canvas
from turtle import color, right
import matplotlib.pyplot as plt
import array as arr
import numpy as np
import pandas as pd
from pandas.plotting import scatter_matrix
import os
import sys
import argparse
import ROOT
from os import path
from ROOT import TCanvas, TFile, TF1
from ROOT import gStyle, gPad, kBlack
from ctypes import cdll
sys.path.append('../utils/')
from plot_library import Load_Style
from function_library import Func_CB2_Jpsi_Fixed

histMass = []

###
def do_fit(simName):
    Load_Style()
    gStyle.SetOptStat(0)
    gStyle.SetOptFit(1)

    fIn = TFile.Open("{}".format(simName))
    hlistDir = fIn.Get("analysis-same-event-pairing/output")
    listDir = hlistDir.FindObject("PairsMuonSEPM_matchedGlobal_mumuFromJpsi")
    histMass = listDir.FindObject("Mass")
    histMass.SetMarkerStyle(24)
    histMass.SetMarkerColor(kBlack)
    histMass.SetLineColor(kBlack)
    histMass.GetXaxis().SetTitle("#it{m} (GeV/c^{2})")
    histMass.GetXaxis().SetTitleSize(0.04)
    histMass.GetXaxis().SetLabelSize(0.04)
    histMass.GetXaxis().SetRangeUser(0, 20)
    histMass.GetXaxis().SetTitleOffset(1.2)
    histMass.GetYaxis().SetTitle("d#it{N}/d#it{m} (GeV/c^{2})^{-1}")
    histMass.GetYaxis().SetTitleSize(0.04)
    histMass.GetYaxis().SetLabelSize(0.04)
    histMass.GetYaxis().SetRangeUser(1, 1e5)

    par_signal_mumu = [1000.,3.096,1.2e-01,1.089,3.393,2.097,8.694,0.01]

    funcMass = TF1("funcMass", Func_CB2_Jpsi_Fixed, 2.0, 4.0, 7)
    funcMass.SetNpx(1000)
    funcMass.SetParameter(0, par_signal_mumu[0])
    funcMass.SetParameter(1, par_signal_mumu[1]) 
    funcMass.SetParName(1, "#mu_{J/#psi}")
    funcMass.SetParameter(2, par_signal_mumu[2])
    funcMass.SetParName(2, "#sigma_{J/#psi}")
    funcMass.SetParameter(3, par_signal_mumu[3])
    funcMass.SetParameter(4, par_signal_mumu[4])
    funcMass.SetParameter(5, par_signal_mumu[5])
    funcMass.SetParameter(6, par_signal_mumu[6])
    histMass.Fit(funcMass, "RL")

    canvas = TCanvas("canvas", "canvas", 800, 600)
    gPad.SetLogy()
    histMass.Draw("H")
    funcMass.Draw("same")
    canvas.Update()

    input()


def main():
    do_fit("data/AnalysisResults.root")

main()