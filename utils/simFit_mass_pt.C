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

void generate_toy_sample(Int_t nDimuFromC = 1000, Int_t nDimuFromB = 100, Int_t nDimuCombBkg = 200){
  TFile *fIn = new TFile("data/AllRun_HistMCDimuHFTree.root", "READ");
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

  // Construct combinatorial background pdf
  RooRealVar a0("a0","a0",-0.5,0.,1.) ;
  RooRealVar a1("a1","a1",-0.2,0.,1.) ;
  RooChebychev *pdfDimuMassCombBkg = new RooChebychev("pdfDimuMassCombBkg","pdfDimuMassCombBkg", m, RooArgSet(a0,a1)) ;

  RooRealVar lambda("lambda","lambda",-1.,-2,1) ;
  RooExponential *pdfDimuPtCombBkg = new RooExponential("pdfDimuPtCombBkg","pdfDimuPtCombBkg", pt, lambda) ;

  RooPlot *frameDimuMass = m.frame(Title("Dimu mass pdf")) ;
  rooHistDimuMassFromCharm.plotOn(frameDimuMass,MarkerStyle(24),MarkerColor(kRed)) ;
  rooHistDimuMassFromBeauty.plotOn(frameDimuMass,MarkerStyle(24),MarkerColor(kBlue)) ;

  RooPlot *frameDimuPt = pt.frame(Title("Dimu #it{p}_{T} pdf")) ;
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
  pdfDimuMassFromCharm->plotOn(frameDimuMass,Name("pdfDimuMassFromCharm"),LineStyle(kSolid),LineColor(kRed)) ;

  auto pdfDimuPtFromCharm = w->pdf("pdfDimuPtFromCharm");
  pdfDimuPtFromCharm->fitTo(rooHistDimuPtFromCharm) ;
  pdfDimuPtFromCharm->plotOn(frameDimuPt,Name("pdfDimuPtFromCharm"),LineStyle(kSolid),LineColor(kRed)) ;

  auto pdfDimuMassFromBeauty = w->pdf("pdfDimuMassFromBeauty");
  pdfDimuMassFromBeauty->fitTo(rooHistDimuMassFromBeauty) ;
  pdfDimuMassFromBeauty->plotOn(frameDimuMass,Name("pdfDimuMassFromBeauty"),LineStyle(kSolid),LineColor(kBlue)) ;
  pdfDimuMassCombBkg->plotOn(frameDimuMass,Name("pdfDimuMassCombBkg"),LineStyle(kSolid),LineColor(kMagenta)) ;


  auto pdfDimuPtFromBeauty = w->pdf("pdfDimuPtFromBeauty");
  pdfDimuPtFromBeauty->fitTo(rooHistDimuPtFromBeauty) ;
  pdfDimuPtFromBeauty->plotOn(frameDimuPt,Name("pdfDimuPtFromBeauty"),LineStyle(kSolid),LineColor(kBlue)) ;
  pdfDimuPtCombBkg->plotOn(frameDimuPt,Name("pdfDimuPtCombBkg"),LineStyle(kSolid),LineColor(kMagenta)) ;

  // Generate the toy samples
  RooDataSet *dataDimuMassFromCharmAndBeauty = pdfDimuMassFromCharm->generate(m,nDimuFromC) ;
  RooDataSet *dataDimuMassFromBeauty = pdfDimuMassFromBeauty->generate(m,nDimuFromB) ;
  RooDataSet *dataDimuMassCombBkg = pdfDimuMassCombBkg->generate(m,nDimuCombBkg) ;

  RooDataSet *dataDimuPtFromCharmAndBeauty = pdfDimuPtFromCharm->generate(pt,nDimuFromC) ;
  RooDataSet *dataDimuPtFromBeauty = pdfDimuPtFromBeauty->generate(pt,nDimuFromB) ;
  RooDataSet *dataDimuPtCombBkg = pdfDimuPtCombBkg->generate(pt,nDimuCombBkg) ;

  // Sum the samples from b and c
  dataDimuMassFromCharmAndBeauty->append(*dataDimuMassFromBeauty);
  dataDimuMassFromCharmAndBeauty->append(*dataDimuMassCombBkg);

  dataDimuPtFromCharmAndBeauty->append(*dataDimuPtFromBeauty);
  dataDimuPtFromCharmAndBeauty->append(*dataDimuPtCombBkg);

  RooPlot *frameDimuMassData = m.frame(Title("Dimu mass distribution toy data")) ;
  dataDimuMassFromCharmAndBeauty->plotOn(frameDimuMassData) ;

  RooPlot *frameDimuPtData = pt.frame(Title("Dimu #it{p}_{T} distribution toy data")) ;
  dataDimuPtFromCharmAndBeauty->plotOn(frameDimuPtData) ;

  w->import(*dataDimuMassFromCharmAndBeauty);
  w->import(*dataDimuPtFromCharmAndBeauty);

  w->import(*pdfDimuMassCombBkg);
  w->import(*pdfDimuPtCombBkg);

  TCanvas* canvasFit = new TCanvas("canvasFit","canvasFit",1200,600) ;
  canvasFit->Divide(2,1);
  canvasFit->cd(1) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; frameDimuMass->GetYaxis()->SetTitleOffset(1.4) ; frameDimuMass->Draw() ;
  canvasFit->cd(2) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; frameDimuPt->GetYaxis()->SetTitleOffset(1.4) ; frameDimuPt->Draw() ;

  TLegend *legend = new TLegend(0.55,0.68,0.75,0.88," ","brNDC") ;
  legend->SetBorderSize(0) ;
  legend->SetFillColor(10) ;
  legend->SetFillStyle(1) ;
  legend->SetLineStyle(0) ;
  legend->SetLineColor(0) ;
  legend->SetTextFont(42) ;
  legend->SetTextSize(0.04) ;
  legend->AddEntry("pdfDimuPtFromCharm", "#mu^{+}#mu^{-} #leftarrow #it{c}", "L") ;
  legend->AddEntry("pdfDimuPtFromBeauty", "#mu^{+}#mu^{-} #leftarrow #it{b}", "L") ;
  legend->AddEntry("pdfDimuPtCombBkg", "Comb. bkg", "L") ;
  legend->Draw() ;

  TCanvas* canvasData = new TCanvas("canvasData","canvasData",1200,600) ;
  canvasData->Divide(2,1);
  canvasData->cd(1) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; frameDimuMassData->GetYaxis()->SetTitleOffset(1.4) ; frameDimuMassData->Draw() ;
  canvasData->cd(2) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; frameDimuPtData->GetYaxis()->SetTitleOffset(1.4) ; frameDimuPtData->Draw() ;

  w->writeToFile("rooWorkspace.root");
  gDirectory->Add(w);
}
//---------------------------------------------------------------------------------------//
 void simFit_mass_pt(){
   if(gSystem->AccessPathName("rooWorkspace.root")){
     printf("Workspace does not esist! Run generate_toy_sample()\n");
     return;
   }

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
   RooAbsPdf *pdfDimuMassCombBkg = w->pdf("pdfDimuMassCombBkg");
   RooAbsPdf *pdfDimuPtFromCharm = w->pdf("pdfDimuPtFromCharm");
   RooAbsPdf *pdfDimuPtFromBeauty = w->pdf("pdfDimuPtFromBeauty");
   RooAbsPdf *pdfDimuPtCombBkg = w->pdf("pdfDimuPtCombBkg");

   // Define the normalization from charm and beauty spectra
   RooRealVar nDimuFromC("nDimuFromC","number dimuon from c",10000,0,2000000) ;
   RooRealVar nDimuFromB("nDimuFromB","number dimuon from b",10000,0,2000000) ;
   RooRealVar nDimuCombBkg("nDimuCombBkg","number dimuon from comb. bkg.",10000,0,2000000) ;

   // Define the model as sum of charm and beauty
   RooAddPdf  m_model("m_model","dimuMassFromC + dimuMassFromB + dimuMassCombBkg",RooArgList(*pdfDimuMassFromCharm,*pdfDimuMassFromBeauty,*pdfDimuMassCombBkg),RooArgList(nDimuFromC,nDimuFromB,nDimuCombBkg)) ;
   RooAddPdf  pt_model("pt_model","dimuPtFromC + dimuPtFromB + dimuPtCombBkg",RooArgList(*pdfDimuPtFromCharm,*pdfDimuPtFromBeauty,*pdfDimuPtCombBkg),RooArgList(nDimuFromC,nDimuFromB,nDimuCombBkg)) ;

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
   simPdf.plotOn(m_frame,Slice(sample,"mass"),Components("pdfDimuMassCombBkg"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kMagenta)) ;

   RooPlot *pt_frame = pt->frame(Title("Transverse momentum distribution")) ;
   combData->plotOn(pt_frame,Cut("sample==sample::transversemomentum")) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),ProjWData(sample,*combData),LineStyle(kSolid),LineColor(kRed)) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),Components("pdfDimuPtFromCharm"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kOrange+7)) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),Components("pdfDimuPtFromBeauty"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kAzure+7)) ;
   simPdf.plotOn(pt_frame,Slice(sample,"transversemomentum"),Components("pdfDimuPtCombBkg"),ProjWData(sample,*combData),LineStyle(kDashed),LineColor(kMagenta)) ;

   // Plot fit parameters
   simPdf.paramOn(pt_frame, Layout(0.35));

   TCanvas* canvas = new TCanvas("canvas","canvas",1200,600) ;
   canvas->Divide(2,1);
   canvas->cd(1) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; m_frame->GetYaxis()->SetTitleOffset(1.4) ; m_frame->Draw() ;
   canvas->cd(2) ; gPad->SetLogy(1) ; gPad->SetLeftMargin(0.15) ; pt_frame->GetYaxis()->SetTitleOffset(1.4) ; pt_frame->Draw() ;
}
