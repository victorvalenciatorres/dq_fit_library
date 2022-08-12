from traceback import print_tb
import yaml
import json
import sys
import argparse
from array import array
from os import path
import ROOT
from ROOT import TFile, TF1, TH1F, TTree
from ROOT import gRandom
sys.path.append('../')
from DQFitter import DQFitter
sys.path.append('../utils')
from utils_library import DoSystematics

def main():
    parser = argparse.ArgumentParser(description='Arguments to pass')
    parser.add_argument('cfgFileName', metavar='text', default='config.yml', help='config file name')
    parser.add_argument("--do_fit", help="run the multi trial", action="store_true")
    parser.add_argument("--do_systematics", help="do systematic on signal extraction", action="store_true")
    parser.add_argument("--optimize_fit", help="optimize the fit parameters", action="store_true")
    args = parser.parse_args()

    print('Loading task configuration: ...', end='\r')
    with open(args.cfgFileName, 'r') as jsonCfgFile:
        inputCfg = json.load(jsonCfgFile)
        #inputCfg = yaml.load(jsonCfgFile, yaml.FullLoader)
    print('Loading task configuration: Done!')

    #fIn = TFile.Open(inputCfg["input"]["input_file_name"])
    #hlist = fIn.Get("analysis-same-event-pairing/output")
    #list = hlist.FindObject("PairsMuonSEPM_muonQualityCuts")
    #histMass = list.FindObject("Mass")
    
    if args.do_fit:
        dqFitter = DQFitter(inputCfg["input"]["input_file_name"], inputCfg["input"]["input_name"])
        dqFitter.SetFitConfig(inputCfg["input"]["pdf_dictionary"])
        dqFitter.MultiTrial()

    if args.do_systematics:
        fIn = TFile.Open("FitResults.root")
        DoSystematics(fIn, "sig_Jpsi")

    if args.optimize_fit:
        dqFitter = DQFitter(inputCfg["input"]["input_file_name"], inputCfg["input"]["input_name"])
        dqFitter.SetFitConfig(inputCfg["input"]["pdf_dictionary"])
        dqFitter.OptimizeFit()

main()