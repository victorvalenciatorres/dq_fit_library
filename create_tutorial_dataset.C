double FuncPt(double *, double *);

Long_t *dummy1 = 0, *dummy2 = 0, *dummy3 = 0, *dummy4 = 0;

void create_tutorial_dataset(){
  const double nEvents = 100000;
  const double ratioSigBkg = 0.1;

  if(gSystem->GetPathInfo("tutorial",dummy1,dummy2,dummy3,dummy4) != 0){
    gSystem->Exec(Form("mkdir -p tutorial"));
  }

  // Generate a signal (gaus) + background (exponential)
  TF1 *funcMassBkg = new TF1("funcMassBkg", "expo", 0., 5.);
  funcMassBkg->SetParameter(0, 0.00);
  funcMassBkg->SetParameter(1, -0.5);

  TF1 *funcMassSig = new TF1("funcMassSig", "gaus", 0., 5.);
  funcMassSig->SetParameter(0, 1.0);
  funcMassSig->SetParameter(1, 3.1);
  funcMassSig->SetParameter(2, 0.07);

  TF1 *funcPtBkg = new TF1("funcPtBkg", FuncPt, 0., 20., 4);
  funcPtBkg->SetParameter(0, 10);
  funcPtBkg->SetParameter(1, 2.85);
  funcPtBkg->SetParameter(2, 1);
  funcPtBkg->SetParameter(3, 2.43);

  TF1 *funcPtSig = new TF1("funcPtSig", FuncPt, 0., 20., 4);
  funcPtSig->SetParameter(0, 10);
  funcPtSig->SetParameter(1, 2.85);
  funcPtSig->SetParameter(2, 2.81);
  funcPtSig->SetParameter(3, 2.43);

  TFile *fOutHist = new TFile("tutorial/hist_2D.root", "RECREATE");

  TH1F *histMass = new TH1F("histMass", "histMass", 100, 0., 5.);
  histMass->FillRandom("funcMassBkg", (int) nEvents - (nEvents*ratioSigBkg));
  histMass->FillRandom("funcMassSig", nEvents*ratioSigBkg);

  TH1F *histPt = new TH1F("histPt", "histPt", 100, 0., 20.);
  histPt->FillRandom("funcPtBkg", (int) nEvents - (nEvents*ratioSigBkg));
  histPt->FillRandom("funcPtSig", nEvents*ratioSigBkg);

  histMass -> Write();
  histPt -> Write();
  fOutHist -> Close();

  // Fill the tree
  TFile *fOutTree = new TFile("tutorial/tree_2D.root", "RECREATE");

  Double_t m, pT;
  Double_t seed;

  TTree *tree = new TTree("data", "data");
  tree->Branch("m", &m, "m/D");
  tree->Branch("pT", &pT, "pT/D");

  for(int iEvent = 0;iEvent < nEvents;iEvent++){
    seed = gRandom->Rndm();
    if(seed > ratioSigBkg){
      m  = funcMassBkg->GetRandom();
      pT = funcPtBkg->GetRandom();
    } else {
      m  = funcMassSig->GetRandom();
      //pT = funcPtSig->GetRandom();
    }
    tree->Fill();
  }
  tree->Write();
  fOutTree->Close();

}
////////////////////////////////////////////////////////////////////////////////
double FuncPt(double *x, double *par){
  double xx = x[0];
  double A = par[0];
  double B = par[1];
  double n1 = par[2];
  double n2 = par[3];

  return A*(xx/TMath::Power(1 + TMath::Power(xx/B,n1),n2));
}
