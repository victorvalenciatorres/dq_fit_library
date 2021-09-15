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
  Double_t     parameters[] = {100.,   -0.1,   1000.,  3.,     0.2};            // Init values of the parameters
  TString   fixParameters[] = {"free", "free", "free", "free", "free"};         // List of fixed / free parameters
  TString  nameParameters[] = {"p0",   "p1",   "p2",   "mean", "width"};        // Name parameters

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter("AnalysisResults.root");
  // Set the histogram to fit
  dq_fitter.SetHistogram(hist);
  // Set the fitting function
  dq_fitter.SetFunction(DQFitter::kFuncExpGaus);
  // Inizialize the fitting paramters
  dq_fitter.InitParameters(5, parameters, fixParameters, nameParameters);


  Double_t minFitRange[] = {0.1, 0.3, 0.5};
  Double_t maxFitRange[] = {4.9, 4.7, 4.5};
  for (int i = 0;i < 3;i++) {
    // Set the fit range
    dq_fitter.SetFitRange(minFitRange[i], maxFitRange[i]);
    // Set the fit method
    dq_fitter.SetFitMethod("S0");
    // Fit the spectrum
    dq_fitter.FitInvMassSpectrum(Form("trial_%i", i));
  }
  // close the output file when all trials are finished
  dq_fitter.CloseOutputFile();

}
