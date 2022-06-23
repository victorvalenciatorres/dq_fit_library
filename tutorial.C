// \author Luca Micheletti <luca.micheletti@cern.ch>

#if !defined(__CINT__) || defined(__MAKECINT__)
// STL includes
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

// Root includes
#include <TROOT.h>
#include <TObjArray.h>
#include <TMinuit.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TPad.h>
#include <TSystem.h>
#include <TGraphErrors.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TFitResult.h>
#include <TMatrixDSym.h>
#include <TPaveText.h>
#include <TCollection.h>
#include <TKey.h>
#include <TGaxis.h>
#include <TStopwatch.h>
#include <TList.h>

// RooFit includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
#include "RooPlot.h"
#include "RooHist.h"
using namespace RooFit;

// my includes
#include "DQFitter.h"
#endif

Long_t *dummy1 = 0, *dummy2 = 0, *dummy3 = 0, *dummy4 = 0;
TString outputDirName = "tutorial/output";

void tutorial(){

  if(gSystem->GetPathInfo("tutorial/hist.root",dummy1,dummy2,dummy3,dummy4) != 0 || gSystem->GetPathInfo("tutorial/tree.root",dummy1,dummy2,dummy3,dummy4) != 0){
    printf("Input file does not exist, run create_tutorial_dataset.C\n");
    return;
  }

  if(gSystem->GetPathInfo(Form("%s",outputDirName.Data()),dummy1,dummy2,dummy3,dummy4) != 0){
    gSystem->Exec(Form("mkdir -p %s",outputDirName.Data()));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Test with standard binned root fit
  //////////////////////////////////////////////////////////////////////////////
  TFile *fInHist = new TFile("tutorial/hist.root", "READ");
  TH1F *hist = (TH1F*) fInHist->Get("histMass");

  Double_t    paramValues[] = {100.,   -0.1,   1000.,  3.,     0.07};
  Double_t minParamLimits[] = {0,      -0.2,   0,      2.,     0.};
  Double_t maxParamLimits[] = {200,     0.4,   2000,   4.,     0.4};
  TString  nameParameters[] = {"p0",   "p1",   "p2",   "mean", "width"};

  Double_t minFitRange[] = {0.1, 0.2, 0.3};
  Double_t maxFitRange[] = {4.9, 4.8, 4.7};

  const char *nameUserFunc[3] = {"UserFunctionLibrary::fFuncGaus()", "UserFunctionLibrary::fFuncExp()", "UserFunctionLibrary::fFuncExpGaus()"};
  Int_t nParamsUserFunc[3] = {3, 2, 5};

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter_binned(Form("%s/tutorial_root_fitter.root", outputDirName.Data()));
  // Set the histogram to fit
  dq_fitter_binned.SetHistogram(hist);
  // Set the fitting function
  dq_fitter_binned.SetUserFunction(nameUserFunc, nParamsUserFunc);
  // Inizialize the fitting paramters
  dq_fitter_binned.InitParameters(5, paramValues, minParamLimits, maxParamLimits, nameParameters);

  for (int i = 0;i < 3;i++) {
    // Set the fit range
    dq_fitter_binned.SetFitRange(minFitRange[i], maxFitRange[i]);
    // Set the fit method
    dq_fitter_binned.SetFitMethod("SL0Q");
    // Fit the spectrum
    dq_fitter_binned.BinnedFitInvMassSpectrum(Form("trial_%i", i));
  }
  dq_fitter_binned.CloseOutputFile();

  //////////////////////////////////////////////////////////////////////////////
  // Test with unbinned roofit fit
  //////////////////////////////////////////////////////////////////////////////
  TFile *fInTree = new TFile("tutorial/tree.root", "READ");
  TTree *tree = (TTree*) fInTree->Get("data");

  Double_t    rooParamValues[] = {3.1,    0.07,    0.1,  2,  1000,   100000};
  Double_t rooMinParamLimits[] = {2,      0,       0,   -10, 0,      0};
  Double_t rooMaxParamLimits[] = {4,      0.2,     0.2,  10, 50000,  500000};
  TString  rooNameParameters[] = {"mean", "width", "a", "b", "nsig", "nbkg"};

  const char *nameUserPDF[3] = {"GausPdf", "ExpPdf", "SUM"};
  Int_t nParamsUserPDF[3] = {2, 2, 2};

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter_unbinned(Form("%s/tutorial_roofit_fitter.root", outputDirName.Data()));
  // Set the tree to fit
  dq_fitter_unbinned.SetTree(tree);
  // Inizialize the fitting paramters for RooFit
  dq_fitter_unbinned.InitParameters(6, rooParamValues, rooMinParamLimits, rooMaxParamLimits, rooNameParameters);
  // Set the PDF for RooFit
  dq_fitter_unbinned.SetUserPDF(nameUserPDF, nParamsUserPDF);
  // Fit the spectrum
  dq_fitter_unbinned.UnbinnedFitInvMassSpectrum("trialRooFit");
  // close the output file when all trials are finished
  dq_fitter_unbinned.CloseOutputFile();
}
