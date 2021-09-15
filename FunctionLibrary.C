#include <iostream>
#include "TMath.h"
#include "TMath.h"
#include "TF1.h"
#include "TROOT.h"

//==============================================================================
// Negative Exponential function
Double_t FuncExp(Double_t *x, Double_t *par){
  return par[0] * TMath::Exp(-par[1] * x[0]);
}
//==============================================================================
// Pol0 function
Double_t FuncPol0(Double_t *x, Double_t *par){
  return par[0] + par[1] * x[0];
}
//==============================================================================
// Gaussian function
Double_t FuncGaus(Double_t *x, Double_t *par){
  return par[0] * TMath::Exp(-0.5 * ((x[0]-par[1])/par[2]) * ((x[0]-par[1])/par[2]));
}
//==============================================================================
// Pol0 function + Gaussian function
Double_t FuncPol0Gaus(Double_t *x, Double_t *par){
  return par[0] + x[0] * par[1] + par[2] * TMath::Exp(-0.5 * ((x[0]-par[3])/par[4]) * ((x[0]-par[3])/par[4]));
}
//==============================================================================
// Negative Exponential function + Gaussian function
Double_t FuncExpGaus(Double_t *x, Double_t *par){
  return par[0] * TMath::Exp(-par[1] * x[0]) + par[2] * TMath::Exp(-0.5 * ((x[0]-par[3])/par[4]) * ((x[0]-par[3])/par[4]));
}
//==============================================================================
