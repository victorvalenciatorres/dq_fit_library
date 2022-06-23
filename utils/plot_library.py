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

def Set_Latex(latex):
    latex.SetTextSize(0.035)
    latex.SetNDC()
    latex.SetTextFont(42)

def Set_Legend(legend):
    legend.SetBorderSize(0)
    legend.SetFillColor(10)
    legend.SetFillStyle(1)
    legend.SetLineStyle(0)
    legend.SetLineColor(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.03)

def Load_Style():
    font = 42
    TGaxis.SetMaxDigits(2)
    gStyle.SetFrameBorderMode(0)
    gStyle.SetFrameFillColor(0)
    gStyle.SetCanvasBorderMode(0)
    gStyle.SetPadBorderMode(0)
    gStyle.SetPadColor(10)
    gStyle.SetCanvasColor(10)
    gStyle.SetTitleFillColor(10)
    gStyle.SetTitleBorderSize(1)
    gStyle.SetStatColor(10)
    gStyle.SetStatBorderSize(1)
    gStyle.SetLegendBorderSize(1)
    gStyle.SetDrawBorder(0)
    gStyle.SetTextFont(font)
    gStyle.SetStatFontSize(0.03)
    gStyle.SetStatX(0.97)
    gStyle.SetStatY(0.98)
    gStyle.SetStatH(0.05)
    gStyle.SetStatW(0.2)
    gStyle.SetTickLength(0.02,"y")
    gStyle.SetEndErrorSize(3)
    gStyle.SetLabelSize(0.035,"xyz")
    gStyle.SetLabelFont(font,"xyz")
    gStyle.SetLabelOffset(0.01,"xyz")
    gStyle.SetTitleFont(font,"xyz")
    gStyle.SetTitleOffset(0.9,"x")
    gStyle.SetTitleOffset(1.02,"y")
    gStyle.SetTitleSize(0.04,"xyz")
    gStyle.SetMarkerSize(0.5)
    #gStyle.SetOptStat("e")
    gStyle.SetOptFit(1)
    gStyle.SetEndErrorSize(0)
    gStyle.SetCanvasPreferGL(kTRUE)
    gStyle.SetHatchesSpacing(0.5)
    gStyle.SetOptTitle(0)
    gStyle.SetLineWidth(2)
    gStyle.SetPadLeftMargin(0.15)
    gStyle.SetPadBottomMargin(0.15)
    gStyle.SetPadTopMargin(0.05)
    gStyle.SetPadRightMargin(0.05)
    gStyle.SetTitleSize(0.06)
    gStyle.SetTitleSize(0.06,"Y")
    gStyle.SetTitleOffset(1.2,"Y")
    gStyle.SetTitleOffset(1.2,"X")
    gStyle.SetFrameLineWidth(2)
    gStyle.SetNdivisions(505,"X")
    gStyle.SetNdivisions(505,"Y")
    gStyle.SetPadTickX(1)
    gStyle.SetPadTickY(1)

def Draw_Ratio_Plot(hist1, hist2, dirName, plotName):
    gStyle.SetOptStat(0)
    canvas = TCanvas("canvas", "A ratio example")
    ratioPlot = TRatioPlot(hist1, hist2)
    ratioPlot.Draw()
    ratioPlot.GetLowerRefYaxis().SetRangeUser(0.,2.)
    ratioPlot.GetLowerRefYaxis().SetLabelSize(0.025)

    tmpPad = ratioPlot.GetUpperPad()

    legend = TLegend(0.15, 0.75, 0.35, 0.89, " ", "brNDC")
    legend.SetBorderSize(0)
    legend.SetFillColor(10)
    legend.SetFillStyle(1)
    legend.SetLineStyle(0)
    legend.SetLineColor(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.03)
    if (plotName.Contains("DCA")):
        legend.AddEntry(hist1, "MC (mean = %3.2f)" % hist1.GetMean(), "PL")
        legend.AddEntry(hist2, "Data (mean = %3.2f)" % hist2.GetMean(), "PL")
    else:
        legend.AddEntry(hist1, "MC", "PL")
        legend.AddEntry(hist2, "Data", "PL")

    legend.Draw()

    tmpPad.Modified() 
    tmpPad.Update() 

    canvas.Update()
    canvas.SaveAs("%s/ratio_%s.pdf" % dirName.Data(), plotName.Data())
