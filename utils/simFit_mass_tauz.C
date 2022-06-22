/// \author Luca Micheletti <luca.micheletti@to.infn.it>, INFN

 #include "RooRealVar.h"
 #include "RooDataSet.h"
 #include "RooGaussian.h"
 #include "RooConstVar.h"
 #include "RooChebychev.h"
 #include "RooAddPdf.h"
 #include "RooSimultaneous.h"
 #include "RooCategory.h"
 #include "TCanvas.h"
 #include "TAxis.h"
 #include "RooPlot.h"
 using namespace RooFit ;

void InitPromptJpsiPdf(){
  TFile *fIn = new TFile("data/AnalysisResults_PromptCharmonia.root", "READ") ;
  TH1D *histTauzPromptJpsi  = (TH1D*) fIn->Get("histTauzJpsi_sig") ;

  // Normalize the histograms (template provides the shape only)
  histTauzPromptJpsi->Scale(1./histTauzPromptJpsi->Integral()) ;

  RooRealVar tauz("tauz","#it{#tau} (ps)",-0.01,0.01) ;

  RooDataHist rooHistTauzPromptJpsi("rooHistTauzPromptJpsi", "rooHistTauzPromptJpsi", tauz, Import(*histTauzPromptJpsi)) ;

  // Load pdf for the pT shape
  gROOT->ProcessLineSync(".x ../fit_library/ResPdf.cxx+") ;

  // Create the pdf for resolutions
  RooWorkspace *w = new RooWorkspace("w", "workspace");
  w->factory("ResPdf::pdfResPromptSig(tauz[-10,10], alpha[0.005,0,1], lambda[1,-10,10])") ;
  auto pdfResPromptSig = w->pdf("pdfResPromptSig");

  RooRealVar sigma1("sigma1", "sigma1", 0.001, 0, 0.1);
  RooRealVar mean1("mean1", "mean1", 0, -1, 1);
  RooGaussian gauss1("gauss1", "gauss1", tauz, mean1, sigma1);

  RooRealVar sigma2("sigma2", "sigma2", 0.007, 0, 0.1);
  RooRealVar mean2("mean2", "mean2", 0, -1, 1);
  RooGaussian gauss2("gauss2", "gauss2", tauz, mean2, sigma2);

  // Define the normalization from charm and beauty spectra
  RooRealVar w1("w1","weight G1",0.5,0,1) ;
  RooRealVar w2("w2","weight G2",0.5,0,1) ;
  RooRealVar w3("w3","weight ResPromptSig",0.5,0,1) ;

  RooAddPdf  ResPromptJpsi("ResPromptJpsi","G1 + G2 + ResPromptSig",RooArgList(gauss1,gauss2,*pdfResPromptSig),RooArgList(w1,w2,w3)) ;
  ResPromptJpsi.fitTo(rooHistTauzPromptJpsi) ;
  w->import(ResPromptJpsi) ;

  RooPlot *tauz_frame = tauz.frame(Title("Pseudo proper time distribution")) ;
  ResPromptJpsi.plotOn(tauz_frame,LineStyle(kSolid),LineColor(kRed)) ;

  rooHistTauzPromptJpsi.plotOn(tauz_frame,MarkerStyle(24),MarkerColor(kBlack)) ;
  ResPromptJpsi.plotOn(tauz_frame,Components("gauss1"),LineStyle(kDashed),LineColor(kOrange+7)) ;
  ResPromptJpsi.plotOn(tauz_frame,Components("gauss2"),LineStyle(kDashed),LineColor(kAzure+7)) ;
  ResPromptJpsi.plotOn(tauz_frame,Components("pdfResPromptSig"),LineStyle(kDashed),LineColor(kMagenta)) ;

  // Plot fit parameters
  //ResPromptJpsi.paramOn(tauz_frame, Layout(0.55));

  TCanvas* canvas = new TCanvas("canvas","canvas",600,600) ;
  gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; tauz_frame->GetYaxis()->SetTitleOffset(1.4) ; tauz_frame->Draw() ;

  w->writeToFile("templatePromptJpsi.root");
  gDirectory->Add(w);

  /*
  TFile *fInTemplate = new TFile("data/XnonPrompt.root", "READ") ;
  TH1D *tmpHistTauzNonPromptJpsi_sig = (TH1D*) fInTemplate->Get("kPseudoProperDecayTimeMC_PtMC_py") ;
  std::cout << tmpHistTauzNonPromptJpsi_sig->GetBinWidth(1) << std::endl;
  TH1D *histTauzNonPromptJpsi_sig = new TH1D("histTauzNonPromptJpsi_sig", "", 1000, -0.01, 0.01);
  for (int i = 0;i < 1000;i++) {
    histTauzNonPromptJpsi_sig->SetBinContent(i+1,tmpHistTauzNonPromptJpsi_sig->GetBinContent(i+1));
    histTauzNonPromptJpsi_sig->SetBinError(i+1,tmpHistTauzNonPromptJpsi_sig->GetBinError(i+1));
  }

  histTauzNonPromptJpsi_sig->Scale(1./histTauzNonPromptJpsi_sig->Integral()) ;
  RooDataHist rooHistTauzNonPromptJpsi_sig("rooHistTauzNonPromptJpsi_sig", "rooHistTauzNonPromptJpsi_sig", tauz, Import(*histTauzNonPromptJpsi_sig)) ;

  //RooHistPdf CsiB("CsiB","CsiB",tauz,rooHistTauzNonPromptJpsi_sig,4) ;
  RooHistPdf CsiB("CsiB","CsiB",tauz,rooHistTauzNonPromptJpsi_sig, 7) ;

  RooRealVar sigma3("sigma3", "sigma3", 0.00001, 0, 0.1);
  RooRealVar mean3("mean3", "mean3", -0.001, -1, 1);
  RooGaussian gauss3("gauss3", "gauss3", tauz, mean3, sigma3);

  RooFFTConvPdf FBx_FF("FBx_FF", "FBx_FF", tauz, CsiB, model);
  //FBx_FF.plotOn(tauz_frame,LineStyle(kSolid),LineColor(kBlue)) ;

  RooRealVar NPrompt("NPrompt","NPrompt",0.5,0,1) ;
  RooRealVar NNonPrompt("NNonPrompt","NNonPrompt",0.5,0,1) ;
  RooAddPdf  model2("model2","P + NP",RooArgList(model, FBx_FF),RooArgList(NPrompt,NNonPrompt)) ;
  model2.fitTo(rooHistTauzPromptJpsi) ;

  model2.plotOn(tauz_frame,Components("model"),LineStyle(kDashed),LineColor(kOrange+7)) ;
  model2.plotOn(tauz_frame,Components("FBx_FF"),LineStyle(kDashed),LineColor(kAzure+7)) ;
  */
}
//---------------------------------------------------------------------------------------//
void InitNonPromptJpsiPdf(){
  if(gSystem->AccessPathName("templatePromptJpsi.root")){
    printf("Workspace does not esist!\n");
    return;
  }

  gROOT->ProcessLineSync(".x ../fit_library/ResPdf.cxx+") ;
  TFile *fIn = new TFile("templatePromptJpsi.root");
  RooWorkspace *w = (RooWorkspace *)fIn->Get("w");
  w->Print();

  RooRealVar *tauz = w->var("tauz");

  RooRealVar *alpha  = w->var("alpha");  alpha->setConstant(kTRUE);
  RooRealVar *lambda = w->var("lambda"); lambda->setConstant(kTRUE);
  RooRealVar *mean1  = w->var("mean1");  mean1->setConstant(kTRUE);
  RooRealVar *mean2  = w->var("mean2");  mean2->setConstant(kTRUE);
  RooRealVar *sigma1 = w->var("sigma1"); sigma1->setConstant(kTRUE);
  RooRealVar *sigma2 = w->var("sigma2"); sigma2->setConstant(kTRUE);
  RooRealVar *w1     = w->var("w1");     w1->setConstant(kTRUE);
  RooRealVar *w2     = w->var("w2");     w2->setConstant(kTRUE);
  RooRealVar *w3     = w->var("w3");     w3->setConstant(kTRUE);

  RooAbsPdf *pdfResPromptJpsi = w->pdf("ResPromptJpsi");

  TFile *fInTemplate = new TFile("data/XnonPrompt.root", "READ") ;
  TH1D *tmpHistTauzNonPromptJpsi = (TH1D*) fInTemplate->Get("kPseudoProperDecayTimeMC_PtMC_py") ;
  TH1D *histTauzNonPromptJpsi = new TH1D("histTauzNonPromptJpsi", "", 1000, -0.01, 0.01);
  for (int i = 0;i < 1000;i++) {
    histTauzNonPromptJpsi->SetBinContent(i+1,tmpHistTauzNonPromptJpsi->GetBinContent(i+1));
    histTauzNonPromptJpsi->SetBinError(i+1,tmpHistTauzNonPromptJpsi->GetBinError(i+1));
  }
  histTauzNonPromptJpsi->Scale(1./histTauzNonPromptJpsi->Integral()) ;
  RooDataHist rooHistTauzNonPromptJpsi("rooHistTauzNonPromptJpsi", "rooHistTauzNonPromptJpsi", *tauz, Import(*histTauzNonPromptJpsi)) ;
  RooHistPdf CsiB("CsiB","CsiB",*tauz,rooHistTauzNonPromptJpsi, 7) ;
  //RooFFTConvPdf FBx_FF("FBx_FF", "FBx_FF", *tauz, CsiB, pdfResPromptJpsi);

  /*
  RooPlot *tauz_frame = tauz->frame(Title("Pseudo proper time distribution")) ;
  pdfResPromptJpsi->plotOn(tauz_frame,LineStyle(kSolid),LineColor(kRed)) ;

  TCanvas* canvasTest = new TCanvas("canvasTest","canvasTest",600,600) ;
  gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; tauz_frame->GetYaxis()->SetTitleOffset(1.4) ; tauz_frame->Draw() ;
  */
}
//---------------------------------------------------------------------------------------//
void simFit_mass_tauz(){
  InitPromptJpsiPdf();
  InitNonPromptJpsiPdf();
}
