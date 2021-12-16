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
TString outputFigureDirName = "figures/test_fit";

void run_test_fit(){

  if(gSystem -> GetPathInfo(Form("%s",outputFigureDirName.Data()),dummy1,dummy2,dummy3,dummy4) != 0){
    gSystem -> Exec(Form("mkdir -p %s",outputFigureDirName.Data()));
  }

  // Generate a signal (gaus) + background (exponential)
  TF1 *funcBkg = new TF1("funcBkg", "expo", 0., 5.);
  funcBkg->SetParameter(0, 0.00);
  funcBkg->SetParameter(1, -0.5);

  TF1 *funcSig = new TF1("funcSig", "gaus", 0., 5.);
  funcSig->SetParameter(0, 1.0);
  funcSig->SetParameter(1, 3.1);
  funcSig->SetParameter(2, 0.07);

  TH1F *hist = new TH1F("hist", "hist", 100, 0., 5.);
  hist->FillRandom("funcBkg", 100000);
  hist->FillRandom("funcSig", 10000);


  //////////////////////////////////////////////////////////////////////////////
  // Test with standard root fit
  //////////////////////////////////////////////////////////////////////////////
  Double_t    paramValues[] = {100.,   -0.1,   1000.,  3.,     0.07};
  Double_t minParamLimits[] = {0,      -0.2,   0,      2.,     0.};
  Double_t maxParamLimits[] = {200,     0.4,   2000,   4.,     0.4};
  TString  nameParameters[] = {"p0",   "p1",   "p2",   "mean", "width"};

  Double_t minFitRange[] = {0.1, 0.2, 0.3};
  Double_t maxFitRange[] = {4.9, 4.8, 4.7};

  //----------------------------------------------------------------------------
  // functions internal to the class
  //----------------------------------------------------------------------------
  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter_1("data/test_fit_results_internal_func.root");
  // Set the histogram to fit
  dq_fitter_1.SetHistogram(hist);
  // Set the fitting function
  dq_fitter_1.SetFunction(DQFitter::kFuncExpGaus);
  // Inizialize the fitting paramters
  dq_fitter_1.InitParameters(5, paramValues, minParamLimits, maxParamLimits, nameParameters);

  for (int i = 0;i < 3;i++) {
    // Set the fit range
    dq_fitter_1.SetFitRange(minFitRange[i], maxFitRange[i]);
    // Set the fit method
    dq_fitter_1.SetFitMethod("SL0Q");
    // Fit the spectrum
    dq_fitter_1.BinnedFitInvMassSpectrum(Form("trial_%i", i));
  }
  dq_fitter_1.CloseOutputFile();

  //----------------------------------------------------------------------------
  // functions external to the class
  //----------------------------------------------------------------------------
  const char *nameUserFunc[3] = {"UserFunctionLibrary::fFuncGaus()", "UserFunctionLibrary::fFuncExp()", "UserFunctionLibrary::fFuncExpGaus()"};
  Int_t nParamsUserFunc[3] = {3, 2, 5};

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter_2("data/test_fit_results_external_func.root");
  // Set the histogram to fit
  dq_fitter_2.SetHistogram(hist);
  // Set the fitting function
  dq_fitter_2.SetUserFunction(nameUserFunc, nParamsUserFunc);
  // Inizialize the fitting paramters
  dq_fitter_2.InitParameters(5, paramValues, minParamLimits, maxParamLimits, nameParameters);


  for (int i = 0;i < 3;i++) {
    // Set the fit range
    dq_fitter_2.SetFitRange(minFitRange[i], maxFitRange[i]);
    // Set the fit method
    dq_fitter_2.SetFitMethod("SL0Q");
    // Fit the spectrum
    dq_fitter_2.BinnedFitInvMassSpectrum(Form("trial_%i", i));
  }
  dq_fitter_2.CloseOutputFile();

  //////////////////////////////////////////////////////////////////////////////
  // Test with standard rooFit
  //////////////////////////////////////////////////////////////////////////////
  Double_t    rooParamValues[] = {3.1,    0.07,    0.1,  2,  10000,  100000};
  Double_t rooMinParamLimits[] = {2,      0,       0,   -10, 5000,   10000};
  Double_t rooMaxParamLimits[] = {4,      0.2,     0.2,  10, 20000,  200000};
  TString  rooNameParameters[] = {"mean", "width", "a", "b", "nsig", "nbkg"};

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter_3("data/test_fit_results_internal_func_rooFit.root");
  // Set the histogram to fit
  dq_fitter_3.SetHistogram(hist);
  // Set the fitting function
  dq_fitter_3.SetFunction(DQFitter::kFuncExpGaus);
  // Inizialize the fitting paramters
  // Inizialize the fitting paramters for RooFit
  dq_fitter_3.InitParameters(6, rooParamValues, rooMinParamLimits, rooMaxParamLimits, rooNameParameters);
  // Set the PDF for RooFit
  dq_fitter_3.SetPDF(DQFitter::kFuncExpGaus);
  // Fit the spectrum
  dq_fitter_3.UnbinnedFitInvMassSpectrum("trialRooFit");
  // close the output file when all trials are finished
  dq_fitter_3.CloseOutputFile();

  /*
  // Save results are pdf
  TFile *file_fit = new TFile("data/test_fit_results.root", "read");
  for(int i = 0;i < 3;i++){
    auto canvasFit = (TCanvas*) file_fit -> Get(Form("trial_%i/canvasFit_trial_%i", i, i));
    canvasFit -> SaveAs(Form("%s/fit_trial_%i.pdf", outputFigureDirName.Data(), i));
  }
  */
}
