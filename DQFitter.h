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

using namespace std;

class DQFitter : public TObject
{

public:
  DQFitter();
  DQFitter(TString);
  virtual ~DQFitter();

  enum FitFunctionsList {
    kFuncExp,
    kFuncPol0,
    kFuncGaus,
    kFuncPol0Gaus,
    kFuncExpGaus,
    kNFunctions
  };

  const Int_t nParameters[kNFunctions] = {2, 2, 3, 5, 5};

  void OpenOutputFile(TString pathToFile);
  void CloseOutputFile();
  void SetHistogram(TH1F* hist);
  void SetFunction(FitFunctionsList func);
  void InitParameters(Int_t nParams, Double_t *params, TString *fixParams, TString *nameParams);
  void SetFitRange(Double_t minFitRange, Double_t maxFitRange);
  void SetFitMethod(TString fitMethod);
  void FitInvMassSpectrum(TString trialName);

protected:
  void SaveResults();

private:
  // Files
  TFile*   fFile;
  TString  fPathToFile;

  // Histograms
  TH1F*    fHist;
  TH1F*    fHistRatio;
  TH1F*    fHistResults;

  // Fitting functions
  TF1*     fFuncTot;
  TF1*     fFuncBkg;
  TF1*     fFuncSig;

  Int_t    fNParBkg;
  Int_t    fNParSig;
  Int_t    fMaxFitIterations;

  // Fit parameters
  TString  fTrialName;
  Int_t    fNParams;
  Double_t fMinFitRange;
  Double_t fMaxFitRange;
  TString  fFitMethod;

  Double_t fChiSquareNDF;
  Double_t fErrorChiSquareNDF;
  Double_t fSignal;
  Double_t fErrorSignal;

ClassDef(DQFitter,1)
};

#endif
