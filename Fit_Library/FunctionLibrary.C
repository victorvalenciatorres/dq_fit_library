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
// Pol1 function
Double_t FuncPol1(Double_t *x, Double_t *par){
  return par[0] + par[1] * x[0];
}
//==============================================================================
// Pol4 x Negative Exponential function
Double_t FuncPol4Exp(Double_t *x, Double_t *par){
  Double_t X = x[0];
  return par[6]*(par[0] + par[1]*X + par[2]*X*X + par[3]*X*X*X + par[4]*X*X*X*X) * TMath::Exp(-par[5]*X);
}
//==============================================================================
// Gaussian function
Double_t FuncGaus(Double_t *x, Double_t *par){
  return par[0] * TMath::Exp(-0.5 * ((x[0]-par[1])/par[2]) * ((x[0]-par[1])/par[2]));
}
//==============================================================================
// Pol1 function + Gaussian function
Double_t FuncPol1Gaus(Double_t *x, Double_t *par){
  return par[0] + x[0] * par[1] + par[2] * TMath::Exp(-0.5 * ((x[0]-par[3])/par[4]) * ((x[0]-par[3])/par[4]));
}
//==============================================================================
// Negative Exponential function + Gaussian function
Double_t FuncExpGaus(Double_t *x, Double_t *par){
  return par[0] * TMath::Exp(-par[1] * x[0]) + par[2] * TMath::Exp(-0.5 * ((x[0]-par[3])/par[4]) * ((x[0]-par[3])/par[4]));
}
//==============================================================================
// Negative Exponential function + Gaussian function
Double_t FuncPol4ExpGaus(Double_t *x, Double_t *par){
  Double_t X = x[0];
  return par[6]*(par[0] + par[1]*X + par[2]*X*X + par[3]*X*X*X + par[4]*X*X*X*X) * TMath::Exp(-par[5]*X) + par[7]*TMath::Exp(-0.5 * ((X-par[8])/par[9]) * ((X-par[8])/par[9]));
}
//==============================================================================
