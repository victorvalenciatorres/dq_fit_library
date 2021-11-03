#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TDatabasePDG.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TEfficiency.h"

// my includes
#include "DQFitter.h"
#endif

void LoadStyle();

using namespace std;

void process_qc(const char *file_name = "reader.root"){
  TFile *file = new TFile(file_name, "read");
  auto hlist = (THashList*) file -> Get("d-q-table-reader/output");
  auto list = (TList*) hlist -> FindObject("PairsMuonSEPM");
  auto histMass_Pt = (TH2F*) list -> FindObject("Mass_Pt");
  auto histMass_Int = (TH1F*) list -> FindObject("Mass");

  histMass_Pt -> RebinX(5);
  histMass_Pt -> RebinY(10);

  TH1F *histMass[20];

  for(int iPtBin = 0;iPtBin < 20;iPtBin++){
    histMass[iPtBin] = (TH1F*) histMass_Pt -> ProjectionX(Form("hMassSig_PtBin_%i", iPtBin), iPtBin+1, iPtBin+1, "e");
    histMass[iPtBin] -> GetXaxis() -> SetRangeUser(2., 5.);
    histMass[iPtBin] -> SetMarkerSize(0.8);
    histMass[iPtBin] -> SetMarkerStyle(24);
    histMass[iPtBin] -> SetMarkerColor(kBlack);
    histMass[iPtBin] -> SetLineColor(kBlack);
  }

  Double_t    paramValues[] = {9.86843e+01,-1.36136e+02,1.02017e+02,3.47559e-01,9.94951e-01,1.51834e+00,2.13579e+03,1000,3.11,0.1};
  Double_t minParamLimits[] = {-100.,-100.,-100.,-100.,-100.,-100.,0.,0.,3.0,0.07};
  Double_t maxParamLimits[] = { 100., 100., 100., 500., 100., 100.,100000,10000.,3.2,0.12};
  TString  nameParameters[] = {"p0","p1","p2","p3","p4","p5","N_bkg","N_sig","mean","width"};

  // Create a DQFitter object and open the file where results will be saved
  DQFitter dq_fitter("first_test.root");


  for(int iPt = 0;iPt < 20;iPt++){
    // Set the histogram to fit
    dq_fitter.SetHistogram(histMass[iPt]);

    // Set the fitting function
    dq_fitter.SetFunction(DQFitter::kFuncPol4ExpGaus);
    // Inizialize the fitting paramters
    dq_fitter.InitParameters(10, paramValues, minParamLimits, maxParamLimits, nameParameters);

    Double_t minFitRange[] = {2.};
    Double_t maxFitRange[] = {5.};
    for (int i = 0;i < 1;i++) {
      // Set the fit range
      dq_fitter.SetFitRange(minFitRange[i], maxFitRange[i]);
      // Set the fit method
      dq_fitter.SetFitMethod("S0");
      // Fit the spectrum
      dq_fitter.BinnedFitInvMassSpectrum(Form("Pt_%i_trial_%i", iPt, i));
    }
  }
  dq_fitter.CloseOutputFile();
}
//------------------------------------------------------------------------------
void process_output(const char *file_name = "first_test.root"){
  gStyle -> SetOptStat(0);
  LoadStyle();

  TLatex *gLatexTitle = new TLatex();
  gLatexTitle -> SetTextSize(0.045);
  gLatexTitle -> SetNDC();
  gLatexTitle -> SetTextFont(42);

  TH1F *histMeanJpsi = new TH1F("histMeanJpsi", "", 10, 0., 10.);
  histMeanJpsi -> SetMarkerStyle(24);
  histMeanJpsi -> SetMarkerColor(kRed);
  histMeanJpsi -> SetLineColor(kRed);

  TH1F *histWidthJpsi = new TH1F("histWidthJpsi", "", 10, 0., 10.);
  histWidthJpsi -> SetMarkerStyle(24);
  histWidthJpsi -> SetMarkerColor(kRed);
  histWidthJpsi -> SetLineColor(kRed);

  TFile *file = new TFile(file_name, "read");
  for(int iPt = 0;iPt < 10;iPt++){
    //auto dir = (TDirectory*) file -> Get(Form("Pt_%i_trial_0", iPt));
    auto histResults = (TH1F*) file -> Get(Form("Pt_%i_trial_0/histResults", iPt));
    histMeanJpsi -> SetBinContent(iPt+1, histResults -> GetBinContent(11));
    histMeanJpsi -> SetBinError(iPt+1, histResults -> GetBinError(11));
    histWidthJpsi -> SetBinContent(iPt+1, histResults -> GetBinContent(12));
    histWidthJpsi -> SetBinError(iPt+1, histResults -> GetBinError(12));
  }

  TH2D *histGridMeanJpsi = new TH2D("histGridMeanJpsi", "", 100, 0., 10., 100, 2., 4.);
  histGridMeanJpsi -> GetXaxis() -> SetTitle("#it{p}_{T} (GeV/#it{c})");
  histGridMeanJpsi -> GetYaxis() -> SetTitle("#it{m}_{J/#psi} (GeV/c^{2})");
  histGridMeanJpsi -> GetXaxis() -> SetTitleOffset(1.2);
  histGridMeanJpsi -> GetYaxis() -> SetTitleOffset(1.2);

  TCanvas *canvasMeanJpsi = new TCanvas("canvasMeanJpsi", "", 800, 600);
  canvasMeanJpsi -> SetLeftMargin(0.15);
  histGridMeanJpsi -> Draw();
  histMeanJpsi -> Draw("EPsame");
  gLatexTitle -> DrawLatex(0.25, 0.85, "pp #sqrt{#it{s}} = 900 GeV, J/#psi #rightarrow #mu^{+}#mu^{-}, LHC21i3d");

  TH2D *histGridWidthJpsi = new TH2D("histGridWidthJpsi", "", 100, 0., 10., 100, 0., 0.2);
  histGridWidthJpsi -> GetXaxis() -> SetTitle("#it{p}_{T} (GeV/#it{c})");
  histGridWidthJpsi -> GetYaxis() -> SetTitle("#it{#sigma}_{J/#psi} (GeV/c^{2})");
  histGridWidthJpsi -> GetXaxis() -> SetTitleOffset(1.2);
  histGridWidthJpsi -> GetYaxis() -> SetTitleOffset(1.2);

  TCanvas *canvasWidthJpsi = new TCanvas("canvasWidthJpsi", "", 800, 600);
  canvasWidthJpsi -> SetLeftMargin(0.15);
  histGridWidthJpsi -> Draw("");
  histWidthJpsi -> Draw("EPsame");
  gLatexTitle -> DrawLatex(0.25, 0.85, "pp #sqrt{#it{s}} = 900 GeV, J/#psi #rightarrow #mu^{+}#mu^{-}, LHC21i3d");

  //canvasMeanJpsi -> SaveAs("distribMeanJpsi.pdf");
  //canvasWidthJpsi -> SaveAs("distribWidthJpsi.pdf");
}
////////////////////////////////////////////////////////////////////////////////
void LoadStyle(){
  int font = 42;
  TGaxis::SetMaxDigits(2);

  gStyle -> SetFrameBorderMode(0);
  gStyle -> SetFrameFillColor(0);
  gStyle -> SetCanvasBorderMode(0);
  gStyle -> SetPadBorderMode(0);
  gStyle -> SetPadColor(10);
  gStyle -> SetCanvasColor(10);
  gStyle -> SetTitleFillColor(10);
  gStyle -> SetTitleBorderSize(1);
  gStyle -> SetStatColor(10);
  gStyle -> SetStatBorderSize(1);
  gStyle -> SetLegendBorderSize(1);
  gStyle -> SetDrawBorder(0);
  gStyle -> SetTextFont(font);
  gStyle -> SetStatFontSize(0.05);
  gStyle -> SetStatX(0.97);
  gStyle -> SetStatY(0.98);
  gStyle -> SetStatH(0.03);
  gStyle -> SetStatW(0.3);
  gStyle -> SetTickLength(0.02,"y");
  gStyle -> SetEndErrorSize(3);
  gStyle -> SetLabelSize(0.04,"xyz");
  gStyle -> SetLabelFont(font,"xyz");
  gStyle -> SetLabelOffset(0.01,"xyz");
  gStyle -> SetTitleFont(font,"xyz");
  gStyle -> SetTitleOffset(0.9,"x");
  gStyle -> SetTitleOffset(1.02,"y");
  gStyle -> SetTitleSize(0.04,"xyz");
  gStyle -> SetMarkerSize(1.3);
  gStyle -> SetOptStat(0);
  gStyle -> SetEndErrorSize(0);
  gStyle -> SetCanvasPreferGL(kTRUE);
  gStyle -> SetHatchesSpacing(0.5);


  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gStyle->SetLineWidth(2);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetTitleSize(0.06);
  gStyle->SetTitleSize(0.06,"Y");
  gStyle->SetTitleOffset(1,"Y");
  gStyle->SetTitleOffset(1,"X");
  gStyle->SetLabelSize(0.05);
  gStyle->SetLabelSize(0.05,"Y");
  gStyle->SetFrameLineWidth(2);
  gStyle->SetNdivisions(505,"X");
  gStyle->SetNdivisions(505,"Y");
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
}
