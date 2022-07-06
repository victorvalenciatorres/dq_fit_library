from re import TEMPLATE
import matplotlib.pyplot as plt
import array as arr
import numpy as np
from array import array
import os
import sys
import argparse
import ROOT
from os import path
from ROOT import TGraphErrors, TCanvas, TF1, TFile, TPaveText, TMath, TH1F, TH2F, TString, TLegend, TRatioPlot, TGaxis, TLine, TLatex
from ROOT import gROOT, gBenchmark, gPad, gStyle, kTRUE, kFALSE, kBlack, kRed, kDashed
from plot_library import LoadStyle, SetLatex

def StoreHistogramsFromFile(fIn, histType):
    '''
    Method which returns all the histograms of a certain class from a given file
    '''
    histArray = []
    for key in fIn.GetListOfKeys():
        kname = key.GetName()
        if (fIn.Get(kname).ClassName() == histType):
            histArray.append(fIn.Get(kname))
    return histArray

def ComputeRMS(parValArray):
    '''
    Method to evaluate the RMS of a sample
    '''
    histDispersion = TH1F("histDispersion", "histDispersion", 10000, 0, 10000)
    for parVal in parValArray:
        histDispersion.Fill(parVal)
    return histDispersion.GetRMS()

def DoSystematics(fIn, parName):
    '''
    Method to evaluate the systematic errors from signal extraction
    '''
    LoadStyle()
    gStyle.SetOptStat(0)
    gStyle.SetOptFit(0)
    nameTrialArray = []
    trialIndexArray  = array( 'f', [] )
    parValArray  = array( 'f', [] )
    parErrArray = array( 'f', [] )

    index = 0.5
    for key in fIn.GetListOfKeys():
        kname = key.GetName()
        if "fit_results" in fIn.Get(kname).GetName():
            trialIndexArray.append(index)
            nameTrialArray.append(fIn.Get(kname).GetName().replace("fit_results_", ""))
            parValArray.append(fIn.Get(kname).GetBinContent(fIn.Get(kname).GetXaxis().FindBin(parName)))
            parErrArray.append(fIn.Get(kname).GetBinError(fIn.Get(kname).GetXaxis().FindBin(parName)))
            index = index + 1

    graParVal = TGraphErrors(len(parValArray), trialIndexArray, parValArray, 0, parErrArray)
    graParVal.SetMarkerStyle(24)
    graParVal.SetMarkerSize(1.2)
    graParVal.SetMarkerColor(kBlack)
    graParVal.SetLineColor(kBlack)

    funcParVal = TF1("funcParVal", "[0]", 0, len(trialIndexArray))
    graParVal.Fit(funcParVal, "R0")
    funcParVal.SetLineColor(kRed)
    funcParVal.SetLineWidth(2)

    trialIndexWidthArray = array( 'f', [] )
    parValSystArray = array( 'f', [] )
    parErrSystArray = array( 'f', [] )
    for i in range(0, len(parValArray)):
        trialIndexWidthArray.append(0.5)
        parValSystArray.append(funcParVal.GetParameter(0))
        parErrSystArray.append(ComputeRMS(parValArray))

    print(ComputeRMS(parValArray), funcParVal.GetParError(0))

    graParSyst = TGraphErrors(len(parValArray), trialIndexArray, parValSystArray, trialIndexWidthArray, parErrSystArray)
    graParSyst.SetFillColorAlpha(kRed, 0.3)

    lineParStatUp = TLine(0, funcParVal.GetParameter(0) + funcParVal.GetParError(0), len(trialIndexArray), funcParVal.GetParameter(0) + funcParVal.GetParError(0))
    lineParStatUp.SetLineStyle(kDashed)
    lineParStatUp.SetLineColor(kRed)

    lineParStatDown = TLine(0, funcParVal.GetParameter(0) - funcParVal.GetParError(0), len(trialIndexArray), funcParVal.GetParameter(0) - funcParVal.GetParError(0))
    lineParStatDown.SetLineStyle(kDashed)
    lineParStatDown.SetLineColor(kRed)

    latexTitle = TLatex()
    SetLatex(latexTitle)

    canvasParVal = TCanvas("canvasParVal", "canvasParVal", 800, 600)
    histGrid = TH2F("histGrid", "", len(parValArray), 0, len(parValArray), 100, 0, 2 * max(parValArray))
    indexLabel = 1
    for nameTrial in nameTrialArray:
        histGrid.GetXaxis().SetBinLabel(indexLabel, nameTrial)
        indexLabel += 1
    histGrid.Draw("same")
    funcParVal.Draw("same")
    lineParStatUp.Draw("same")
    lineParStatDown.Draw("same")
    graParSyst.Draw("E2same")
    graParVal.Draw("EPsame")
    latexTitle.DrawLatex(0.37, 0.85, "N_{J/#psi} = #bf{%3.2f} #pm #bf{%3.2f} (%2.1f %%) #pm #bf{%3.2f} (%2.1f %%)" % (funcParVal.GetParameter(0), funcParVal.GetParError(0), (funcParVal.GetParError(0)/funcParVal.GetParameter(0))*100, ComputeRMS(parValArray), (ComputeRMS(parValArray)/funcParVal.GetParameter(0))*100))

    canvasParVal.SaveAs("test.pdf")

    input()


    