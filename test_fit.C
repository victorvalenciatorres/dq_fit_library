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

void test_fit(){
  // Generate a signal (gaus) + background (exponential)
  TF1 *funcBkg = new TF1("funcBkg", "expo", 0., 5.);
  funcBkg->SetParameter(0, 0.00);
  funcBkg->SetParameter(1, -0.5);

  TF1 *funcSig = new TF1("funcSig", "gaus", 0., 5.);
  funcSig->SetParameter(0, 1.0);
  funcSig->SetParameter(1, 3.0);
  funcSig->SetParameter(2, 0.25);

  TH1F *hist = new TH1F("hist", "hist", 100, 0., 5.);
  hist->FillRandom("funcBkg", 100000);
  hist->FillRandom("funcSig", 10000);

  // Class parameter settings
  //Double_t     parameters[] = {100.,   -0.1,   1000.,  3.,     0.2};            // Init values of the parameters
  //TString   fixParameters[] = {"free", "free", "free", "free", "free"};         // List of fixed / free parameters
  //TString  nameParameters[] = {"p0",   "p1",   "p2",   "mean", "width"};        // Name parameters

  // Test with standard root fit
  Double_t    paramValues[] = {100.,   -0.1,   1000.,  3.,     0.2};
  Double_t minParamLimits[] = {0,      -0.2,   0,      2.,     0.};
  Double_t maxParamLimits[] = {200,     0.4,   2000,   4.,     0.4};
  TString  nameParameters[] = {"p0",   "p1",   "p2",   "mean", "width"};

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter("AnalysisResults.root");
  // Set the histogram to fit
  dq_fitter.SetHistogram(hist);

  // Set the fitting function
  dq_fitter.SetFunction(DQFitter::kFuncExpGaus);
  // Inizialize the fitting paramters
  dq_fitter.InitParameters(5, paramValues, minParamLimits, maxParamLimits, nameParameters);

  Double_t minFitRange[] = {0.1, 0.3, 0.5};
  Double_t maxFitRange[] = {4.9, 4.7, 4.5};
  for (int i = 0;i < 3;i++) {
    // Set the fit range
    dq_fitter.SetFitRange(minFitRange[i], maxFitRange[i]);
    // Set the fit method
    dq_fitter.SetFitMethod("S0");
    // Fit the spectrum
    dq_fitter.BinnedFitInvMassSpectrum(Form("trial_%i", i));
  }

  // Test with RooFit
  Double_t    rooParamValues[] = {3,      0.1,     0.1, 2,   5000,   50000};
  Double_t rooMinParamLimits[] = {2,      0,       0,   -10, 5000,   50000};
  Double_t rooMaxParamLimits[] = {4,      0.2,     0.2,  10, 20000,  200000};
  TString  rooNameParameters[] = {"mean", "width", "a", "b", "nsig", "nbkg"};

  // Inizialize the fitting paramters for RooFit
  dq_fitter.InitParameters(6, rooParamValues, rooMinParamLimits, rooMaxParamLimits, rooNameParameters);
  // Set the PDF for RooFit
  dq_fitter.SetPDF(DQFitter::kFuncExpGaus);
  // Fit the spectrum
  dq_fitter.UnbinnedFitInvMassSpectrum("trialUnbinned");
  // close the output file when all trials are finished
  dq_fitter.CloseOutputFile();
  // close the output file when all trials are finished
  dq_fitter.CloseOutputFile();
}
