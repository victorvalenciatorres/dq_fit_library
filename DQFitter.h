// \author Luca Micheletti <luca.micheletti@cern.ch>

#ifndef DQFITTER_H
#define DQFITTER_H
#include "TObject.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TString.h"
#include "TTree.h"
#include <string>
#include <vector>

// RooFit includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
#include "RooPlot.h"

using namespace RooFit;
using namespace std;

class DQFitter : public TObject
{

public:
  DQFitter();
  DQFitter(TString);
  virtual ~DQFitter();

  void OpenOutputFile(TString pathToFile);
  void CloseOutputFile();
  void SetHistogram(TH1F* hist);
  void SetTree(TTree *tree);
  void SetUserFunction(const char *nameFunc[3], Int_t nParams[3]);
  void SetUserPDF(const char *nameFunc[3], Int_t nParams[3]);
  void InitParameters(Int_t nParams, Double_t *params, Double_t *minParamLimits, Double_t *maxParamLimits, TString *nameParams);
  void SetFitRange(Double_t minFitRange, Double_t maxFitRange);
  void SetFitMethod(TString fitMethod);
  void BinnedFitInvMassSpectrum(TString trialName);
  void UnbinnedFitInvMassSpectrum(TString trialName);

protected:
  void SaveResults();

private:
  // Configurations
  Bool_t   fDoRooFit;

  // Files
  TFile*   fFile;
  TString  fPathToFile;

  // Histograms
  TH1F*    fHist;
  TH1F*    fHistBkg;
  TH1F*    fHistResiduals;
  TH1F*    fHistRatio;
  TH1F*    fHistResults;

  // Trees
  TTree*   fTree;

  // Fitting functions
  TF1*     fFuncTot;
  TF1*     fFuncBkg;
  TF1*     fFuncSig;

  Int_t    fNParBkg;
  Int_t    fNParSig;
  Int_t    fMaxFitIterations;

  // Fit parameters
  TString   fTrialName;
  TString   fFitStatus;
  Int_t     fNParams;
  Double_t* fParams;
  Double_t* fMinParamLimits;
  Double_t* fMaxParamLimits;
  TString*  fParamNames;
  Double_t  fMinFitRange;
  Double_t  fMaxFitRange;
  TString   fFitMethod;

  Double_t  fChiSquareNDF;
  Double_t  fErrorChiSquareNDF;
  Double_t  fSignal;
  Double_t  fErrorSignal;

  // RooFit variables
  RooRealVar   fRooMass;
  RooWorkspace fRooWorkspace;
  RooRealVar*  fRooParameters[100];
  RooPlot*     fRooPlot;


ClassDef(DQFitter,1)
};

#endif
