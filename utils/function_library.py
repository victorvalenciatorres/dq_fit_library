import matplotlib.pyplot as plt
import array as arr
import numpy as np
import os
import sys
import argparse
import ROOT
from os import path
from ROOT import TCanvas, TF1, TFile, TPaveText, TMath
from ROOT import gROOT, gBenchmark, gPad, gStyle
from ctypes import cdll

def Func_Exp(x, par):
    return par[0] * TMath.Exp(-x[0] / par[1])

def Func_CB_Jpsi(x, par):
    t = (x[0] - par[3]) / par[4]
    if (par[5] < 0):
        t = -t

    absAlpha = TMath.Abs(par[5])
    if (t >= -absAlpha):
        return par[2] * (TMath.Exp(-0.5 * t * t))
    
    if (t < -absAlpha):
        a =  TMath.Power(par[6] / absAlpha, par[6]) * TMath.Exp(-0.5 * absAlpha * absAlpha)
        b = par[6] / absAlpha - absAlpha
        return par[2] * (a / TMath.Power(b - t, par[6]))

def Func_Tot_ee(x, par):
    return Func_Exp(x,par) + Func_CB_Jpsi(x,par)

def Func_VWG(x, par):
    sigma = par[2] + par[3] * ((x[0] - par[1]) / par[1])
    return par[0] * TMath.Exp(-(x[0] - par[1]) * (x[0] - par[1])/(2. * sigma * sigma))

def Func_CB2_Jpsi(x, par):
    t = (x[0] - par[5]) / par[6]
    if (par[7] < 0):
        t = -t

    absAlpha = TMath.Abs(par[7])
    absAlpha2 = TMath.Abs(par[9])

    if (t >= -absAlpha and t < absAlpha2):
        return par[4] * (TMath.Exp(-0.5 * t * t))

    if (t < -absAlpha):
        a =  TMath.Power(par[8] / absAlpha, par[8]) * TMath.Exp(-0.5 * absAlpha * absAlpha)
        b = par[8] / absAlpha - absAlpha
        return par[4] * (a / TMath.Power(b - t, par[8]))

    if (t >= absAlpha2):
        c =  TMath.Power(par[10] / absAlpha2, par[10]) * TMath.Exp(-0.5 * absAlpha2 * absAlpha2)
        d = par[10] / absAlpha2 - absAlpha2
        return  par[4] * (c/TMath.Power(d + t, par[10]))

    return 0.


def Func_CB2_Jpsi_Fixed(x, par):
    t = (x[0] - par[1]) / par[2]
    if (par[3] < 0):
        t = -t

    absAlpha = TMath.Abs(par[3])
    absAlpha2 = TMath.Abs(par[5])

    if (t >= -absAlpha and t < absAlpha2):
        return par[0] * (TMath.Exp(-0.5 * t * t))

    if (t < -absAlpha):
        a =  TMath.Power(par[4] / absAlpha, par[4]) * TMath.Exp(-0.5 * absAlpha * absAlpha)
        b = par[4] / absAlpha - absAlpha
        return par[0] * (a / TMath.Power(b - t, par[4]))

    if (t >= absAlpha2):
        c =  TMath.Power(par[6] / absAlpha2, par[6]) * TMath.Exp(-0.5 * absAlpha2 * absAlpha2)
        d = par[6] / absAlpha2 - absAlpha2
        return  par[0] * (c/TMath.Power(d + t, par[6]))

    return 0.


def Func_CB2_Psi2S(x, par):
    scaling_factor = 1.05154
    t = (x[0] - (par[5] + (3.686 - 3.097))) / (par[6] * scaling_factor)
    if (par[7] < 0):
        t = -t

    absAlpha = TMath.Abs(par[7])
    absAlpha2 = TMath.Abs(par[9])

    if (t >= -absAlpha and t < absAlpha2):
        return par[4] * par[11]*(TMath.Exp(-0.5 * t * t))

    if (t < -absAlpha):
        a =  TMath.Power(par[8] / absAlpha, par[8])*TMath.Exp(-0.5 * absAlpha * absAlpha)
        b = par[8] / absAlpha - absAlpha
        return par[4] * par[11] * (a / TMath.Power(b - t, par[8]))

    if (t >= absAlpha2):
        c =  TMath.Power(par[10] / absAlpha2,par[10]) * TMath.Exp(-0.5 * absAlpha2 * absAlpha2)
        d = par[10] / absAlpha2 - absAlpha2
        return  par[4] * par[11] * (c / TMath.Power(d + t, par[10]))
        
    return 0.

def Func_Tot_mumu(x, par):
    return Func_VWG(x,par) + Func_CB2_Jpsi(x,par) + Func_CB2_Psi2S(x,par)