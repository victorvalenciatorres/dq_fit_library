from ast import parse
from queue import PriorityQueue
from time import process_time_ns
from tkinter import Canvas
from turtle import color, right
import matplotlib.pyplot as plt
import array as arr
import numpy as np
import os
from os import path
import sys
import argparse
import ROOT
from ROOT import TCanvas, TFile, TF1, TH1F, RooRealVar, RooDataSet, RooWorkspace, RooAddPdf, RooExtendPdf, RooPlot, RooHist, RooDataHist, RooArgSet
from ROOT import gStyle, gPad, kBlack, gROOT, gSystem


class DQFitter:
    def __init__(self, fInName, fInputName):
        self.fPdfDict          = {}
        self.fFileOut          = TFile("FitResults.root", "RECREATE")
        self.fFileIn           = TFile.Open(fInName)
        self.fInput            = self.fFileIn.Get(fInputName)
        self.fRooWorkspace     = RooWorkspace('w','workspace')
        self.fParNames         = []
        self.fFitRangeMin      = []
        self.fFitRangeMax      = []
        self.fRooMass          = RooRealVar("m", "#it{M} (GeV/#it{c}^{2})", 2, 5)
        self.fMaxFitIterations = 100
        self.fFitMethod        = "SRL"
        self.fFitStatus        = "Undefined"

    def SetFitConfig(self, pdfDict):
        '''
        Method to set the fit PDFs
        '''
        self.fPdfDict = pdfDict
        self.fFitRangeMin = pdfDict["fitRangeMin"]
        self.fFitRangeMax = pdfDict["fitRangeMax"]

        pdfList = []
        for i in range(0, len(self.fPdfDict["pdf"])):
            parVal = self.fPdfDict["parVal"][i]
            parLimMin = self.fPdfDict["parLimMin"][i]
            parLimMax = self.fPdfDict["parLimMax"][i]
            parName = self.fPdfDict["parName"][i]

            if not self.fPdfDict["pdf"][i] == "SUM":
                gROOT.ProcessLineSync(".x ../fit_library/{}Pdf.cxx+".format(self.fPdfDict["pdf"][i]))
                nameFunc = self.fPdfDict["pdf"][i]
                nameFunc += "Pdf::{}Pdf(m[2,5]".format(self.fPdfDict["pdf"][i])
                pdfList.append(self.fPdfDict["pdf"][i])

                for j in range(0, len(parVal)):
                    nameFunc += ",{}[{},{},{}]".format(parName[j], parVal[j], parLimMin[j], parLimMax[j])
                    self.fParNames.append(parName[j])
                nameFunc += ")"
                print(nameFunc)
                self.fRooWorkspace.factory(nameFunc)
            else:
                nameFunc = self.fPdfDict["pdf"][i]
                nameFunc += "::sum("

                for j in range(0, len(pdfList)):
                    nameFunc += "{}[{},{},{}]*{}Pdf".format(parName[j], parVal[j], parLimMin[j], parLimMax[j], pdfList[j])
                    self.fParNames.append(parName[j])
                    if not j == len(pdfList) - 1:
                        nameFunc += ","
                nameFunc += ")"
                print(nameFunc)
                self.fRooWorkspace.factory(nameFunc)

    def FitInvMassSpectrum(self, trialName, fitRangeMin, fitRangeMax):
        '''
        Method to perform unbinned fit to a ROOT TTree
        '''
        self.fRooWorkspace.Print()
        pdf = self.fRooWorkspace.pdf("sum")
        self.fRooMass.setRange("range", fitRangeMin, fitRangeMax)
        fRooPlot = self.fRooMass.frame(ROOT.RooFit.Title(trialName))
        if "TTree" in self.fInput.ClassName():
            print("Perform unbinned fit")
            rooDs = RooDataSet("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))
        else:
            print("Perform binned fit")
            rooDs = RooDataHist("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))
        pdf.fitTo(rooDs)

        index = 1
        histResults = TH1F("histResults", "", len(self.fParNames), 0., len(self.fParNames))
        for parName in self.fParNames:
            histResults.GetXaxis().SetBinLabel(index, parName)
            histResults.SetBinContent(index, self.fRooWorkspace.var(parName).getVal())
            histResults.SetBinContent(index, self.fRooWorkspace.var(parName).getError())
            index += 1

        rooDs.plotOn(fRooPlot)
        pdf.plotOn(fRooPlot)
        pdf.paramOn(fRooPlot, ROOT.RooFit.Layout(0.55))

        canvasFit = TCanvas("canvasFit_{}".format(trialName), "canvasFit_{}".format(trialName), 600, 600)
        canvasFit.SetLeftMargin(0.15)
        gPad.SetLeftMargin(0.15)
        fRooPlot.GetYaxis().SetTitleOffset(1.4)
        fRooPlot.Draw()

        self.fFileOut.cd()
        canvasFit.Write()
        histResults.Write()

    
    def MultiTrial(self):
        for iRange in range(0, len(self.fFitRangeMin)):
            self.FitInvMassSpectrum("trial_{}_{}".format(self.fFitRangeMin[iRange], self.fFitRangeMax[iRange]), self.fFitRangeMin[iRange], self.fFitRangeMax[iRange])
        self.fFileOut.Close()