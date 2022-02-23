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

 //-------------------------------------------------------------
 // Fit MC to extract pT and mass shapes
 //-------------------------------------------------------------

void generate_toy_sample(){
  TFile *fIn = new TFile("AllRun_HistMCDimuHFTree.root", "READ");
  TH1D *histDimuMassFromCharm  = (TH1D*) fIn -> Get("dimu_masscut0/dimu_rec/ULS_h_dimumass_rec_fromCharm") ;
  TH1D *histDimuMassFromBeauty = (TH1D*) fIn -> Get("dimu_masscut0/dimu_rec/ULS_h_dimumass_rec_fromBeauty") ;
  TH1D *histDimuPtFromCharm  = (TH1D*) fIn -> Get("dimu_masscut0/dimu_rec/ULS_h_dimupt_rec_fromCharm") ;
  TH1D *histDimuPtFromBeauty = (TH1D*) fIn -> Get("dimu_masscut0/dimu_rec/ULS_h_dimupt_rec_fromBeauty") ;

  // Normalize the histograms (template provides the shape only)
  histDimuMassFromCharm->Scale(1./histDimuMassFromCharm->Integral());
  histDimuMassFromBeauty->Scale(1./histDimuMassFromBeauty->Integral());
  histDimuPtFromCharm->Scale(1./histDimuPtFromCharm->Integral());
  histDimuPtFromBeauty->Scale(1./histDimuPtFromBeauty->Integral());

  // Define the variables (m,pT)
  RooRealVar m("m","#it{m} (GeV/#it{c}^{2})",0,20) ;
  RooDataHist rooHistDimuMassFromCharm("rooHistDimuMassFromCharm", "rooHistDimuMassFromCharm", m, Import(*histDimuMassFromCharm)) ;
  RooDataHist rooHistDimuMassFromBeauty("rooHistDimuMassFromBeauty", "rooHistDimuMassFromBeauty", m, Import(*histDimuMassFromBeauty)) ;

  RooRealVar pt("pt","#it{p}_{T} (GeV/#it{c})",0,20) ;
  RooDataHist rooHistDimuPtFromCharm("rooHistDimuPtFromCharm", "rooHistDimuPtFromCharm", pt, Import(*histDimuPtFromCharm)) ;
  RooDataHist rooHistDimuPtFromBeauty("rooHistDimuPtFromBeauty", "rooHistDimuPtFromBeauty", pt, Import(*histDimuPtFromBeauty)) ;

  RooPlot *frameDimuMass = m.frame(Title("Dimu mass distribution")) ;
  rooHistDimuMassFromCharm.plotOn(frameDimuMass,MarkerStyle(24),MarkerColor(kRed)) ;
  rooHistDimuMassFromBeauty.plotOn(frameDimuMass,MarkerStyle(24),MarkerColor(kBlue)) ;

  RooPlot *frameDimuPt = pt.frame(Title("Dimu #it{p}_{T} distribution")) ;
  rooHistDimuPtFromCharm.plotOn(frameDimuPt,MarkerStyle(24),MarkerColor(kRed)) ;
  rooHistDimuPtFromBeauty.plotOn(frameDimuPt,MarkerStyle(24),MarkerColor(kBlue)) ;

  // Load pdf for the pT shape
  gROOT->ProcessLineSync(".x ../fit_library/PtPdf.cxx+") ;

  // Create pdf for dimu from charm and beauty
  RooWorkspace *w = new RooWorkspace("w", "workspace");
  w->factory("PtPdf::pdfDimuMassFromCharm(m[0,20], B_DimuMassFromCharm[2.85,1,100], n1_DimuMassFromCharm[2.81,1,100], n2_DimuMassFromCharm[2.43,1,100])") ;
  w->factory("PtPdf::pdfDimuPtFromCharm(pt[0,20], B_DimuPtFromCharm[2.85,1,100], n1_DimuPtFromCharm[2.81,1,100], n2_DimuPtFromCharm[2.43,1,100])") ;

  w->factory("PtPdf::pdfDimuMassFromBeauty(m[0,20], B_DimuMassFromBeauty[2.85,1,100], n1_DimuMassFromBeauty[2.81,1,100], n2_DimuMassFromBeauty[2.43,1,100])") ;
  w->factory("PtPdf::pdfDimuPtFromBeauty(pt[0,20], B_DimuPtFromBeauty[2.85,1,100], n1_DimuPtFromBeauty[2.81,1,100], n2_DimuPtFromBeauty[2.43,1,100])") ;

  auto pdfDimuMassFromCharm = w->pdf("pdfDimuMassFromCharm");
  pdfDimuMassFromCharm->fitTo(rooHistDimuMassFromCharm) ;
  pdfDimuMassFromCharm->plotOn(frameDimuMass,LineStyle(kSolid),LineColor(kRed)) ;

  auto pdfDimuPtFromCharm = w->pdf("pdfDimuPtFromCharm");
  pdfDimuPtFromCharm->fitTo(rooHistDimuPtFromCharm) ;
  pdfDimuPtFromCharm->plotOn(frameDimuPt,LineStyle(kSolid),LineColor(kRed)) ;

  auto pdfDimuMassFromBeauty = w->pdf("pdfDimuMassFromBeauty");
  pdfDimuMassFromBeauty->fitTo(rooHistDimuMassFromBeauty) ;
  pdfDimuMassFromBeauty->plotOn(frameDimuMass,LineStyle(kSolid),LineColor(kBlue)) ;

  auto pdfDimuPtFromBeauty = w->pdf("pdfDimuPtFromBeauty");
  pdfDimuPtFromBeauty->fitTo(rooHistDimuPtFromBeauty) ;
  pdfDimuPtFromBeauty->plotOn(frameDimuPt,LineStyle(kSolid),LineColor(kBlue)) ;

  // Generate the toy samples
  RooDataSet *dataDimuMassFromCharmAndBeauty = pdfDimuMassFromCharm->generate(m,10000) ;
  RooDataSet *dataDimuMassFromBeauty = pdfDimuMassFromBeauty->generate(m,1000) ;

  RooDataSet *dataDimuPtFromCharmAndBeauty = pdfDimuPtFromCharm->generate(pt,10000) ;
  RooDataSet *dataDimuPtFromBeauty = pdfDimuPtFromBeauty->generate(pt,1000) ;

  // Sum the samples from b and c
  dataDimuMassFromCharmAndBeauty->append(*dataDimuMassFromBeauty);
  dataDimuPtFromCharmAndBeauty->append(*dataDimuPtFromBeauty);

  w->import(*dataDimuMassFromCharmAndBeauty);
  w->import(*dataDimuPtFromCharmAndBeauty);

  TCanvas* canvas = new TCanvas("canvas","canvas",1200,600) ;
  canvas->Divide(2,1);
  canvas->cd(1) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; frameDimuMass->GetYaxis()->SetTitleOffset(1.4) ; frameDimuMass->Draw() ;
  canvas->cd(2) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; frameDimuPt->GetYaxis()->SetTitleOffset(1.4) ; frameDimuPt->Draw() ;

  w->writeToFile("rooWorkspace.root");
  gDirectory->Add(w);
}
//---------------------------------------------------------------------------------------//
 void simFit_mass_pt(){
   gROOT->ProcessLineSync(".x ../fit_library/PtPdf.cxx+") ;
   TFile *fIn = new TFile("rooWorkspace.root");
   RooWorkspace *w = (RooWorkspace *)fIn->Get("w");
   w->Print();

   RooRealVar *m = w->var("m");
   RooRealVar *pt = w->var("pt");

   // Fix parameters to the MC
   RooRealVar *B_DimuMassFromCharm = w->var("B_DimuMassFromCharm"); B_DimuMassFromCharm->setConstant(kTRUE);
   RooRealVar *n1_DimuMassFromCharm = w->var("n1_DimuMassFromCharm"); n1_DimuMassFromCharm->setConstant(kTRUE);
   RooRealVar *n2_DimuMassFromCharm = w->var("n2_DimuMassFromCharm"); n2_DimuMassFromCharm->setConstant(kTRUE);
   RooRealVar *B_DimuMassFromBeauty = w->var("B_DimuMassFromBeauty"); B_DimuMassFromBeauty->setConstant(kTRUE);
   RooRealVar *n1_DimuMassFromBeauty = w->var("n1_DimuMassFromBeauty"); n1_DimuMassFromBeauty->setConstant(kTRUE);
   RooRealVar *n2_DimuMassFromBeauty = w->var("n2_DimuMassFromBeauty"); n2_DimuMassFromBeauty->setConstant(kTRUE);

   RooRealVar *B_DimuPtFromCharm = w->var("B_DimuPtFromCharm"); B_DimuPtFromCharm->setConstant(kTRUE);
   RooRealVar *n1_DimuPtFromCharm = w->var("n1_DimuPtFromCharm"); n1_DimuPtFromCharm->setConstant(kTRUE);
   RooRealVar *n2_DimuPtFromCharm = w->var("n2_DimuPtFromCharm"); n2_DimuPtFromCharm->setConstant(kTRUE);
   RooRealVar *B_DimuPtFromBeauty = w->var("B_DimuPtFromBeauty"); B_DimuPtFromBeauty->setConstant(kTRUE);
   RooRealVar *n1_DimuPtFromBeauty = w->var("n1_DimuPtFromBeauty"); n1_DimuPtFromBeauty->setConstant(kTRUE);
   RooRealVar *n2_DimuPtFromBeauty = w->var("n2_DimuPtFromBeauty"); n2_DimuPtFromBeauty->setConstant(kTRUE);

   RooAbsPdf *pdfDimuMassFromCharm = w->pdf("pdfDimuMassFromCharm");
   RooAbsPdf *pdfDimuMassFromBeauty = w->pdf("pdfDimuMassFromBeauty");
   RooAbsPdf *pdfDimuPtFromCharm = w->pdf("pdfDimuPtFromCharm");
   RooAbsPdf *pdfDimuPtFromBeauty = w->pdf("pdfDimuPtFromBeauty");

   // Define the normalization from charm and beauty spectra
   RooRealVar nDimuFromC("nDimuFromC","number dimuon from c",10000,0,2000000) ;
   RooRealVar nDimuFromB("nDimuFromB","number dimuon from b",10000,0,2000000) ;

   // Define the model as sum of charm and beauty
   RooAddPdf  m_model("m_model","dimuMassFromC + dimuMassFromB",RooArgList(*pdfDimuMassFromCharm,*pdfDimuMassFromBeauty),RooArgList(nDimuFromC,nDimuFromB)) ;
   RooAddPdf  pt_model("pt_model","dimuPtFromC + dimuPtFromB",RooArgList(*pdfDimuPtFromCharm,*pdfDimuPtFromBeauty),RooArgList(nDimuFromC,nDimuFromB)) ;

   RooDataSet *dataDimuMassFromCharmAndBeauty = (RooDataSet*) w->data("pdfDimuMassFromCharmData");
   RooDataSet *dataDimuPtFromCharmAndBeauty = (RooDataSet*) w->data("pdfDimuPtFromCharmData");

   // Define the combined dataset
   RooCategory sample("sample","sample") ;
   sample.defineType("mass") ;
   sample.defineType("transversemomentum") ;
   RooDataSet *combData = new RooDataSet("combData","combined data",RooArgSet(*m,*pt),Index(sample),Import("mass",*dataDimuMassFromCharmAndBeauty),Import("transversemomentum",*dataDimuPtFromCharmAndBeauty)) ;

   // Define the pdf for the simultaneous fit
   RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;
   simPdf.addPdf(m_model,"mass") ;
   simPdf.addPdf(pt_model,"transversemomentum") ;
   simPdf.fitTo(*combData) ;

   RooPlot *m_frame = m->frame(Title("Invariant mass distribution")) ;
   combData->plotOn(m_frame,Cut("sample==sample::mass")) ;
   simPdf.plotOn(m_frame,Slice(sample,"mass"),ProjWData(sample,*combData),LineStyle(kSolid),LineColor(kRed)) ;
   simPdf.plotOn(m_frame,Slice(sample,"mass"),Components("pdfDimuMassFromCharm"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kOrange+7)) ;
   simPdf.plotOn(m_frame,Slice(sample,"mass"),Components("pdfDimuMassFromBeauty"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kAzure+7)) ;

   RooPlot *pt_frame = pt->frame(Title("Transverse momentum distribution")) ;
   combData->plotOn(pt_frame,Cut("sample==sample::transversemomentum")) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),ProjWData(sample,*combData),LineStyle(kSolid),LineColor(kRed)) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),Components("pdfDimuPtFromCharm"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kOrange+7)) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),Components("pdfDimuPtFromBeauty"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kAzure+7)) ;

   // Plot fit parameters
   simPdf.paramOn(pt_frame, Layout(0.35));

   TCanvas* canvas = new TCanvas("canvas","canvas",1200,600) ;
   canvas->Divide(2,1);
   canvas->cd(1) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; m_frame->GetYaxis()->SetTitleOffset(1.4) ; m_frame->Draw() ;
   canvas->cd(2) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; pt_frame->GetYaxis()->SetTitleOffset(1.4) ; pt_frame->Draw() ;
}
