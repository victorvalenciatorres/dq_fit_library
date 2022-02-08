void create_tutorial_dataset(){
  const double nEvents = 100000;
  const double ratioSigBkg = 0.1;

  if(gSystem->GetPathInfo("tutorial",dummy1,dummy2,dummy3,dummy4) != 0){
    gSystem->Exec(Form("mkdir -p tutorial"));
  }

  // Generate a signal (gaus) + background (exponential)
  TF1 *funcBkg = new TF1("funcBkg", "expo", 0., 5.);
  funcBkg->SetParameter(0, 0.00);
  funcBkg->SetParameter(1, -0.5);

  TF1 *funcSig = new TF1("funcSig", "gaus", 0., 5.);
  funcSig->SetParameter(0, 1.0);
  funcSig->SetParameter(1, 3.1);
  funcSig->SetParameter(2, 0.07);

  TFile *fOutHist = new TFile("tutorial/hist.root", "RECREATE");

  TH1F *hist = new TH1F("hist", "hist", 100, 0., 5.);
  hist->FillRandom("funcBkg", (int) nEvents - (nEvents*ratioSigBkg));
  hist->FillRandom("funcSig", nEvents*ratioSigBkg);

  hist -> Write();
  fOutHist -> Close();

  // Fill the tree
  TFile *fOutTree = new TFile("tutorial/tree.root", "RECREATE");

  Double_t m;
  Double_t seed;

  TTree *tree = new TTree("data", "data");
  tree->Branch("m", &m, "m/D");

  for(int iEvent = 0;iEvent < nEvents;iEvent++){
    seed = gRandom->Rndm();
    if(seed > ratioSigBkg){
      m =  funcBkg->GetRandom();
    } else {
      m =  funcSig->GetRandom();
    }
    tree->Fill();
  }
  tree->Write();
  fOutTree->Close();

}
