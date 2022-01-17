#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TSystem.h"
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
#include "TRatioPlot.h"

// my includes
#include "DQFitter.h"
#endif

void run_production_comparison(TString , TString);
void LoadStyle();
void DrawRatioPlot(TH1F *, TH1F *, TString, TString);
void FitInvariantMass(TH2F *, TString);

Long_t *dummy1 = 0, *dummy2 = 0, *dummy3 = 0, *dummy4 = 0;
TString outputFigureDirName = "figures/qc";

using namespace std;

void run_qc(TString input_file_name = "data/table_reader_output.root", TString output_file_name = "data/qc_test.root", TString input_type = "reader"){
  LoadStyle();

  if(gSystem -> GetPathInfo(Form("%s",outputFigureDirName.Data()),dummy1,dummy2,dummy3,dummy4) != 0){
    gSystem -> Exec(Form("mkdir -p %s",outputFigureDirName.Data()));
  }

  // Histograms and and variables configurations
  TH1F *hist1dVar[10][10];
  TH2F *hist2dVar[10][10];
  TString mainDirName;
  std::vector<TString> dirName;
  std::vector<TString> hist1dName;
  std::vector<TString> hist2dName;
  int dirNum;
  int hist1dNum;
  int hist2dNum;

  if(input_type.Contains("reader")){
    mainDirName = "d-q-table-reader";
    TString initDirName[] = {"PairsMuonSEPM", "PairsMuonSEPP", "PairsMuonSEMM"};
    TString initHist1dName[] = {"Mass", "Tauz"};
    TString initHist2dName[] = {"Mass_Pt"};
    dirNum = sizeof(initDirName)/sizeof(initDirName[0]);
    hist1dNum = sizeof(initHist1dName)/sizeof(initHist1dName[0]);
    hist2dNum = sizeof(initHist2dName)/sizeof(initHist2dName[0]);
    copy(initDirName,    initDirName+dirNum,       back_inserter(dirName));
    copy(initHist1dName, initHist1dName+hist1dNum, back_inserter(hist1dName));
    copy(initHist2dName, initHist2dName+hist2dNum, back_inserter(hist2dName));
  }

  if(input_type.Contains("maker")){
    mainDirName = "table-maker";
    TString initDirName[] = {"TrackBarrel_BeforeCuts"};
    TString initHist1dName[] = {"Pt", "Eta", "Phi", "DCAxy", "DCAz"};
    TString initHist2dName[] = {"TPCdedx_pIN", "TPCnSigEle_pIN"};
    dirNum = sizeof(initDirName)/sizeof(initDirName[0]);
    hist1dNum = sizeof(initHist1dName)/sizeof(initHist1dName[0]);
    hist2dNum = sizeof(initHist2dName)/sizeof(initHist2dName[0]);
    copy(initDirName,    initDirName+dirNum,       back_inserter(dirName));
    copy(initHist1dName, initHist1dName+hist1dNum, back_inserter(hist1dName));
    copy(initHist2dName, initHist2dName+hist2dNum, back_inserter(hist2dName));
  }

  TFile *file = new TFile(input_file_name, "read");
  auto hlist = (THashList*) file -> Get(Form("%s/output", mainDirName.Data()));
  for(int iDir = 0;iDir < dirNum;iDir++){
    auto list = (TList*) hlist -> FindObject(dirName[iDir].Data());
    for(int iHist1d = 0;iHist1d < hist1dNum;iHist1d++){
      hist1dVar[iDir][iHist1d] = (TH1F*) list -> FindObject(hist1dName[iHist1d].Data());
      hist1dVar[iDir][iHist1d] -> SetName(dirName[iDir]);
    }
    for(int iHist2d = 0;iHist2d < hist2dNum;iHist2d++){
      hist2dVar[iDir][iHist2d] = (TH2F*) list -> FindObject(hist2dName[iHist2d].Data());
      hist2dVar[iDir][iHist2d] -> SetName(dirName[iDir]);
    }
  }

  for(int iHist1d = 0;iHist1d < hist1dNum;iHist1d++){
    auto canvasVar = new TCanvas("canvasVar", "",  600*dirNum, 600);
    canvasVar -> Divide(dirNum,1);
    for(int iDir = 0;iDir < dirNum;iDir++){
      canvasVar -> cd(iDir+1);
      gPad -> SetLogy(1);
      hist1dVar[iDir][iHist1d] -> Draw("H");
    }
    canvasVar -> SaveAs(Form("%s/%s.pdf", outputFigureDirName.Data(), hist1dName[iHist1d].Data()));
    delete canvasVar;
  }

  for(int iHist2d = 0;iHist2d < hist2dNum;iHist2d++){
    auto canvasVar = new TCanvas("canvasVar", "",  600*dirNum, 600);
    canvasVar -> Divide(dirNum,1);
    for(int iDir = 0;iDir < dirNum;iDir++){
      canvasVar -> cd(iDir+1);
      if(hist2dName[iHist2d].Contains("dedx")){
        gPad -> SetLogx(1);
      }
      hist2dVar[iDir][iHist2d] -> Draw("COLZ");
    }
    canvasVar -> SaveAs(Form("%s/%s.pdf", outputFigureDirName.Data(), hist2dName[iHist2d].Data()));
    delete canvasVar;
  }

  // select the TH2D with mass and pT
  if(input_type.Contains("reader")){
    FitInvariantMass(hist2dVar[0][0], output_file_name);
  }
}
////////////////////////////////////////////////////////////////////////////////
void FitInvariantMass(TH2F *histMassPt, TString output_file_name){
  // Fit the Mass distribution in different pt bins
  // Rebin the mass - pT histograms
  histMassPt -> RebinX(5);
  histMassPt -> RebinY(10);

  TH1F *histMass[20];
  for(int iPtBin = 0;iPtBin < 20;iPtBin++){
    histMass[iPtBin] = (TH1F*) histMassPt -> ProjectionX(Form("hMassSig_PtBin_%i", iPtBin), iPtBin+1, iPtBin+1, "e");
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
  DQFitter dq_fitter(output_file_name);

  for(int iPt = 0;iPt < 10;iPt++){
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

  // Plot the mass peak position and the width of the J/psi
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

  TFile *file_fit = new TFile(output_file_name, "read");
  for(int iPt = 0;iPt < 10;iPt++){
    auto canvasFit = (TCanvas*) file_fit -> Get(Form("Pt_%i_trial_0/canvasFit_Pt_%i_trial_0", iPt, iPt));
    canvasFit -> SaveAs(Form("%s/fit_Pt_%i.pdf", outputFigureDirName.Data(), iPt));
    auto histResults = (TH1F*) file_fit -> Get(Form("Pt_%i_trial_0/histResults", iPt));
    histMeanJpsi -> SetBinContent(iPt+1, histResults -> GetBinContent(11));
    histMeanJpsi -> SetBinError(iPt+1, histResults -> GetBinError(11));
    histWidthJpsi -> SetBinContent(iPt+1, histResults -> GetBinContent(12));
    histWidthJpsi -> SetBinError(iPt+1, histResults -> GetBinError(12));
  }

  // J/psi mass peak position
  TCanvas *canvasMeanJpsi = new TCanvas("canvasMeanJpsi", "", 800, 600);
  canvasMeanJpsi -> SetLeftMargin(0.15);
  TH2D *histGridMeanJpsi = new TH2D("histGridMeanJpsi", "", 100, 0., 10., 100, 2., 4.);
  histGridMeanJpsi -> GetXaxis() -> SetTitle("#it{p}_{T} (GeV/#it{c})");
  histGridMeanJpsi -> GetYaxis() -> SetTitle("#it{m}_{J/#psi} (GeV/c^{2})");
  histGridMeanJpsi -> GetXaxis() -> SetTitleOffset(1.2);
  histGridMeanJpsi -> GetYaxis() -> SetTitleOffset(1.2);
  histGridMeanJpsi -> Draw();
  histMeanJpsi -> Draw("EPsame");
  gLatexTitle -> DrawLatex(0.25, 0.85, "pp #sqrt{#it{s}} = 900 GeV, J/#psi #rightarrow #mu^{+}#mu^{-}, LHC21i3d");

  // J/psi width
  TCanvas *canvasWidthJpsi = new TCanvas("canvasWidthJpsi", "", 800, 600);
  canvasWidthJpsi -> SetLeftMargin(0.15);
  TH2D *histGridWidthJpsi = new TH2D("histGridWidthJpsi", "", 100, 0., 10., 100, 0., 0.2);
  histGridWidthJpsi -> GetXaxis() -> SetTitle("#it{p}_{T} (GeV/#it{c})");
  histGridWidthJpsi -> GetYaxis() -> SetTitle("#it{#sigma}_{J/#psi} (GeV/c^{2})");
  histGridWidthJpsi -> GetXaxis() -> SetTitleOffset(1.2);
  histGridWidthJpsi -> GetYaxis() -> SetTitleOffset(1.2);
  histGridWidthJpsi -> Draw();
  histWidthJpsi -> Draw("EPsame");
  gLatexTitle -> DrawLatex(0.25, 0.85, "pp #sqrt{#it{s}} = 900 GeV, J/#psi #rightarrow #mu^{+}#mu^{-}, LHC21i3d");

  // Save results
  canvasMeanJpsi -> SaveAs(Form("%s/distribMeanJpsi.pdf", outputFigureDirName.Data()));
  canvasWidthJpsi -> SaveAs(Form("%s/distribWidthJpsi.pdf", outputFigureDirName.Data()));
}
////////////////////////////////////////////////////////////////////////////////
void run_production_comparison(TString process = "table-maker", TString output_dir_name = "figures/qc/pilot_run"){
  TH1F *hist1dVar_pass1[10][10];
  TH2F *hist2dVar_pass1[10][10];
  TH1F *hist1dVar_pass2[10][10];
  TH2F *hist2dVar_pass2[10][10];
  std::vector<TString> dirName;
  std::vector<TString> hist1dName;
  std::vector<TString> hist2dName;
  int dirNum;
  int hist1dNum;
  int hist2dNum;

  TString initDirName[] = {"TrackBarrel_BeforeCuts", "TrackBarrel_jpsiO2MCdebugCuts"};
  TString initHist1dName[] = {"Pt", "Eta", "Phi"};
  TString initHist2dName[] = {"TPCdedx_pIN", "TPCnSigEle_pIN"};
  dirNum = sizeof(initDirName)/sizeof(initDirName[0]);
  hist1dNum = sizeof(initHist1dName)/sizeof(initHist1dName[0]);
  hist2dNum = sizeof(initHist2dName)/sizeof(initHist2dName[0]);
  copy(initDirName,    initDirName+dirNum,       back_inserter(dirName));
  copy(initHist1dName, initHist1dName+hist1dNum, back_inserter(hist1dName));
  copy(initHist2dName, initHist2dName+hist2dNum, back_inserter(hist2dName));

  TFile *fIn_pass1 = new TFile("data/pilot_run/AnalysisResults_apass1.root", "READ");
  auto hlist_pass1 = (THashList*) fIn_pass1 -> Get(Form("%s/output", process.Data()));
  for(int iDir = 0;iDir < dirNum;iDir++){
    auto list = (TList*) hlist_pass1 -> FindObject(dirName[iDir].Data());
    for(int iHist1d = 0;iHist1d < hist1dNum;iHist1d++){
      hist1dVar_pass1[iDir][iHist1d] = (TH1F*) list -> FindObject(hist1dName[iHist1d].Data());
      hist1dVar_pass1[iDir][iHist1d] -> SetName(dirName[iDir]);
      hist1dVar_pass1[iDir][iHist1d] -> Scale(1./hist1dVar_pass1[iDir][iHist1d] -> Integral());
      hist1dVar_pass1[iDir][iHist1d] -> SetMarkerStyle(20);
      hist1dVar_pass1[iDir][iHist1d] -> SetMarkerColor(kBlack);
      hist1dVar_pass1[iDir][iHist1d] -> SetLineColor(kBlack);

      if(hist1dName[iHist1d].Contains("Pt")){
        hist1dVar_pass1[iDir][iHist1d] -> GetXaxis() -> SetRangeUser(0, 5);
        hist1dVar_pass1[iDir][iHist1d] -> Rebin(40);
      } else {
        hist1dVar_pass1[iDir][iHist1d] -> Rebin(5);
      }
    }
    for(int iHist2d = 0;iHist2d < hist2dNum;iHist2d++){
      hist2dVar_pass1[iDir][iHist2d] = (TH2F*) list -> FindObject(hist2dName[iHist2d].Data());
      hist2dVar_pass1[iDir][iHist2d] -> SetName(dirName[iDir]);
    }
  }

  TFile *fIn_pass2 = new TFile("data/pilot_run/AnalysisResults_apass2.root", "READ");
  auto hlist_pass2 = (THashList*) fIn_pass2 -> Get(Form("%s/output", process.Data()));
  for(int iDir = 0;iDir < dirNum;iDir++){
    auto list = (TList*) hlist_pass2 -> FindObject(dirName[iDir].Data());
    for(int iHist1d = 0;iHist1d < hist1dNum;iHist1d++){
      hist1dVar_pass2[iDir][iHist1d] = (TH1F*) list -> FindObject(hist1dName[iHist1d].Data());
      hist1dVar_pass2[iDir][iHist1d] -> SetName(dirName[iDir]);
      hist1dVar_pass2[iDir][iHist1d] -> Scale(1./hist1dVar_pass2[iDir][iHist1d] -> Integral());
      hist1dVar_pass2[iDir][iHist1d] -> SetMarkerStyle(24);
      hist1dVar_pass2[iDir][iHist1d] -> SetMarkerSize(0.3);
      hist1dVar_pass2[iDir][iHist1d] -> SetMarkerColor(kRed);
      hist1dVar_pass2[iDir][iHist1d] -> SetLineColor(kRed);

      if(hist1dName[iHist1d].Contains("Pt")){
        hist1dVar_pass1[iDir][iHist1d] -> GetXaxis() -> SetRangeUser(0, 5);
        hist1dVar_pass2[iDir][iHist1d] -> Rebin(40);
      } else {
        hist1dVar_pass2[iDir][iHist1d] -> Rebin(5);
      }
    }
    for(int iHist2d = 0;iHist2d < hist2dNum;iHist2d++){
      hist2dVar_pass2[iDir][iHist2d] = (TH2F*) list -> FindObject(hist2dName[iHist2d].Data());
      hist2dVar_pass2[iDir][iHist2d] -> SetName(dirName[iDir]);
    }
  }

  for(int iDir = 0;iDir < dirNum;iDir++){
    for(int iHist1d = 0;iHist1d < hist1dNum;iHist1d++){
      DrawRatioPlot(hist1dVar_pass1[iDir][iHist1d], hist1dVar_pass2[iDir][iHist1d], output_dir_name,initHist1dName[iHist1d] + initDirName[iDir]);
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
void DrawRatioPlot(TH1F *hist1, TH1F *hist2, TString dirName, TString plotName){
  gStyle->SetOptStat(0);
  auto canvas = new TCanvas("canvas", "A ratio example");
  auto ratioPlot = new TRatioPlot(hist1, hist2);
  if(plotName.Contains("Pt")){
    gPad -> SetLogy(1);
  }
  ratioPlot -> Draw();
  if(plotName.Contains("Pt")){
    ratioPlot -> GetLowerRefYaxis() -> SetRangeUser(0.,2.);
  }
  canvas -> Update();
  canvas -> SaveAs(Form("%s/ratio_%s.pdf", dirName.Data(), plotName.Data()));
  delete canvas;
  delete ratioPlot;
}
////////////////////////////////////////////////////////////////////////////////
void LoadStyle(){
  int font = 42;
  //TGaxis::SetMaxDigits(2);
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
  gStyle -> SetOptStat(111111);
  gStyle -> SetEndErrorSize(0);
  gStyle -> SetCanvasPreferGL(kTRUE);
  gStyle -> SetHatchesSpacing(0.5);
  gStyle -> SetOptTitle(0);
  gStyle -> SetLineWidth(2);
  gStyle -> SetPadLeftMargin(0.15);
  gStyle -> SetPadBottomMargin(0.15);
  gStyle -> SetPadTopMargin(0.05);
  gStyle -> SetPadRightMargin(0.05);
  gStyle -> SetTitleSize(0.06);
  gStyle -> SetTitleSize(0.06,"Y");
  gStyle -> SetTitleOffset(1,"Y");
  gStyle -> SetTitleOffset(1,"X");
  gStyle -> SetLabelSize(0.05);
  gStyle -> SetLabelSize(0.05,"Y");
  gStyle -> SetFrameLineWidth(2);
  gStyle -> SetNdivisions(505,"X");
  gStyle -> SetNdivisions(505,"Y");
  gStyle -> SetPadTickX(1);
  gStyle -> SetPadTickY(1);
}
