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
sys.path.append('/Users/valencia/Desktop/dq_fit_library')
from DQFitter import DQFitter
sys.path.append('/Users/valencia/Desktop/dq_fit_library/utils')
from utils_library import DoSystematics

def GenerateTutorialSample():
    print("----------- GENERATE TUTORIAL SAMPLE -----------")
    nEvents = 100000
    SigOverBkg1 = 0.03
    SigOverBkg2 = SigOverBkg1 / 10.
    
    fOut = TFile("tuto1.root", "RECREATE")

    funcMassBkg = TF1("funcMassBkg", "expo", 0., 5.)
    funcMassBkg.SetParameter(0, 0.00)
    funcMassBkg.SetParameter(1, -0.5)

    funcMassSig1 = TF1("funcMassSig1", "gaus", 0., 5.)
    funcMassSig1.SetParameter(0, 1.0)
    funcMassSig1.SetParameter(1, 3.096)
    funcMassSig1.SetParameter(2, 0.070)

    funcMassSig2 = TF1("funcMassSig2", "gaus", 0., 5.)
    funcMassSig2.SetParameter(0, 1.0)
    funcMassSig2.SetParameter(1, 3.686)
    funcMassSig2.SetParameter(2, 1.05 * 0.070)

    histMass = TH1F("histMass", "histMass", 100, 0., 5.)
    histMass.FillRandom("funcMassBkg", int(nEvents - (nEvents * SigOverBkg1)))
    histMass.FillRandom("funcMassSig1", int(nEvents * SigOverBkg1))
    histMass.FillRandom("funcMassSig2", int(nEvents * SigOverBkg2))
    histMass.Write()

    print("Data histogram")
    print("counter J/psi = %f" % (int(nEvents * SigOverBkg1)))
    print("counter Psi(2S) = %f" % (int(nEvents * SigOverBkg2)))

    counterSig1 = 0
    counterSig2 = 0

    m = array('f', [0.])
    tree = TTree("data", "data")
    tree.Branch("m", m, "m/F")

    for iEvent in range(0, nEvents):
        seed = gRandom.Rndm()
        if seed > SigOverBkg1:
            m[0] = funcMassBkg.GetRandom()
        else:
            if seed > SigOverBkg2:
                m[0] = funcMassSig1.GetRandom()
                counterSig1 = counterSig1 + 1
            else:
                m[0] = funcMassSig2.GetRandom()
                counterSig2 = counterSig2 + 1
        tree.Fill()
    tree.Write()

    fOut.Close()

    print("Data tree")
    print("counter J/psi = %f" % (counterSig1))
    print("counter Psi(2S) = %f" % (counterSig2))

def main():
    print('start')
    parser = argparse.ArgumentParser(description='Arguments to pass')
    parser.add_argument('cfgFileName', metavar='text', default='config.yml', help='config file name')
    parser.add_argument("--gen_tutorial", help="generate tutorial sample", action="store_true")
    parser.add_argument("--do_fit", help="run the multi trial", action="store_true")
    parser.add_argument("--do_systematics", help="do systematic on signal extraction", action="store_true")
    parser.add_argument("--optimize_fit", help="optimize the fit parameters", action="store_true")
    args = parser.parse_args()
    print(args)
    print('Loading task configuration: ...', end='\r')



    with open(args.cfgFileName, 'r') as jsonCfgFile:
        #inputCfg = json.load(jsonCfgFile)
        inputCfg = yaml.load(jsonCfgFile, yaml.FullLoader)
    print('Loading task configuration: Done!')

    if args.gen_tutorial:
        GenerateTutorialSample()
    
    if args.do_fit:
        dqFitter = DQFitter(inputCfg["input"]["input_file_name"], inputCfg["input"]["input_name"], inputCfg["output"]["output_file_name"])
        dqFitter.SetFitConfig(inputCfg["input"]["pdf_dictionary"])
        dqFitter.MultiTrial()

    if args.do_systematics:
        fIn = TFile.Open("1")
        DoSystematics(fIn, "sig_Jpsi")

    if args.optimize_fit:
        dqFitter = DQFitter(inputCfg["input"]["input_file_name"], inputCfg["input"]["input_name"], inputCfg["output"]["output_file_name"])
        dqFitter.SetFitConfig(inputCfg["input"]["pdf_dictionary"])
        dqFitter.OptimizeFit()



if __name__ == '__main__':
    main()
