import ROOT
import os
import sys

def init_prompt_jpsi_pdf() :
    fIn = ROOT.TFile("data/AnalysisResults_PromptCharmonia.root", "READ")
    histTauzPromptJpsi = fIn.Get("histTauzJpsi_sig")

    # Normalize the histograms (template provides the shape only)
    histTauzPromptJpsi.Scale(1./histTauzPromptJpsi.Integral())

    # Define the pseudo-proper time
    tauz = ROOT.RooRealVar("tauz","#it{#tau} (ps)",-0.01,0.01)

    # Import the Prompt Jpsi histogram
    rooHistTauzPromptJpsi = ROOT.RooDataHist("rooHistTauzPromptJpsi", "rooHistTauzPromptJpsi", tauz, ROOT.RooFit.Import(histTauzPromptJpsi))

    # Load pdf for the resolution
    ROOT.gROOT.ProcessLineSync(".x ../fit_library/ResPdf.cxx+")

    # Create the pdf for resolutions
    w = ROOT.RooWorkspace("w", "workspace")
    w.factory("ResPdf::pdfResPromptSig(tauz[-10,10], alpha[0.005,0,1], lambda[1,-10,10])")
    pdfResPromptSig = w.pdf("pdfResPromptSig")

    sigma1 = ROOT.RooRealVar("sigma1", "sigma1", 0.001, 0, 0.1);
    mean1  = ROOT.RooRealVar("mean1", "mean1", 0, -1, 1);
    gauss1 = ROOT.RooGaussian("gauss1", "gauss1", tauz, mean1, sigma1)

    sigma2 = ROOT.RooRealVar("sigma2", "sigma2", 0.007, 0, 0.1);
    mean2  = ROOT.RooRealVar("mean2", "mean2", 0, -1, 1);
    gauss2 = ROOT.RooGaussian("gauss2", "gauss2", tauz, mean2, sigma2)

    w1 = ROOT.RooRealVar("w1","weight G1",0.5,0,1)
    w2 = ROOT.RooRealVar("w2","weight G2",0.5,0,1)
    w3 = ROOT.RooRealVar("w3","weight ResPromptSig",0.5,0,1)

    ResPromptJpsi = ROOT.RooAddPdf("ResPromptJpsi","G1 + G2 + ResPromptSig", ROOT.RooArgList(gauss1, gauss2, pdfResPromptSig), ROOT.RooArgList(w1, w2, w3))
    ResPromptJpsi.fitTo(rooHistTauzPromptJpsi)

    # See https://sft.its.cern.ch/jira/browse/ROOT-6785
    getattr(w, 'import')(ResPromptJpsi)

    # Create frame for plotting
    tauz_frame = tauz.frame(ROOT.RooFit.Title("Pseudo proper time distribution"))
    ResPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.LineStyle(ROOT.kSolid), ROOT.RooFit.LineColor(ROOT.kRed))
    rooHistTauzPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.MarkerStyle(24), ROOT.RooFit.MarkerColor(ROOT.kBlack))
    ResPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.Components("gauss1"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kOrange+7))
    ResPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.Components("gauss2"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kAzure+7))
    ResPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.Components("pdfResPromptSig"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kMagenta))

    # Draw and save the fit results
    canvas = ROOT.TCanvas("canvas","canvas",600,600)
    ROOT.gPad.SetLogy(1)
    ROOT.gPad.SetLeftMargin(0.15)
    tauz_frame.GetYaxis().SetTitleOffset(1.4)
    tauz_frame.Draw()

    canvas.SaveAs("test.pdf")

    w.writeToFile("templatePromptJpsi.root");
    ROOT.gDirectory.Add(w);

def init_non_prompt_jpsi_pdf() :
    if not os.path.isfile("templatePromptJpsi.root") :
        printf("Workspace does not esist!\n")
        return

    # Load pdf for the resolution
    ROOT.gROOT.ProcessLineSync(".x ../fit_library/ResPdf.cxx+")
    fIn = ROOT.TFile("templatePromptJpsi.root", "READ")
    w = fIn.Get("w");
    w.Print();

    tauz = w.var("tauz");

    alphaRes  = w.var("alpha"); alphaRes.setConstant(ROOT.kTRUE)
    lambdaRes = w.var("lambda"); lambdaRes.setConstant(ROOT.kTRUE)
    mean1     = w.var("mean1"); mean1.setConstant(ROOT.kTRUE)
    mean2     = w.var("mean2"); mean2.setConstant(ROOT.kTRUE)
    sigma1    = w.var("sigma1"); sigma1.setConstant(ROOT.kTRUE)
    sigma2    = w.var("sigma2"); sigma2.setConstant(ROOT.kTRUE)
    w1        = w.var("w1"); w1.setConstant(ROOT.kTRUE)
    w2        = w.var("w2"); w2.setConstant(ROOT.kTRUE)
    w3        = w.var("w3"); w3.setConstant(ROOT.kTRUE)


    pdfResPromptJpsi = w.pdf("ResPromptJpsi")

    fInTemplate = ROOT.TFile("data/XnonPrompt.root", "READ")
    tmpHistTauzNonPromptJpsi = fInTemplate.Get("kPseudoProperDecayTimeMC_PtMC_py")
    histTauzNonPromptJpsi = ROOT.TH1D("histTauzNonPromptJpsi", "", 1000, -0.01, 0.01);

    for i in range(0, 1000) :
        histTauzNonPromptJpsi.SetBinContent(i+1,tmpHistTauzNonPromptJpsi.GetBinContent(i+1));
        histTauzNonPromptJpsi.SetBinError(i+1,tmpHistTauzNonPromptJpsi.GetBinError(i+1));

    histTauzNonPromptJpsi.Scale(1./histTauzNonPromptJpsi.Integral()) ;
    rooHistTauzNonPromptJpsi = ROOT.RooDataHist("rooHistTauzNonPromptJpsi", "rooHistTauzNonPromptJpsi", tauz, ROOT.RooFit.Import(histTauzNonPromptJpsi))
    CsiB = ROOT.RooHistPdf("CsiB","CsiB",tauz, rooHistTauzNonPromptJpsi, 7)
    pdfResNonPromptJpsi = ROOT.RooFFTConvPdf("pdfResNonPromptJpsi", "pdfResNonPromptJpsi", tauz, CsiB, pdfResPromptJpsi);


    weight1 = ROOT.RooRealVar("weight1","weight1",0.75,0,1)
    weight2 = ROOT.RooRealVar("weight2","weight2",0.15,0,1)
    ResNonPromptJpsi = ROOT.RooAddPdf("ResNonPromptJpsi", "Prompt + NonPrompt", ROOT.RooArgList(pdfResPromptJpsi, pdfResNonPromptJpsi), ROOT.RooArgList(weight1, weight2))

    tauz_frame = tauz.frame(ROOT.RooFit.Title("Pseudo proper time distribution for non-prompt"))
    pdfResNonPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kBlue))
    pdfResPromptJpsi.plotOn(tauz_frame, ROOT.RooFit.LineStyle(ROOT.kSolid), ROOT.RooFit.LineColor(ROOT.kRed))

    tauz_frame_tot = tauz.frame(ROOT.RooFit.Title("Pseudo proper time distribution for non-prompt + prompt"))
    ResNonPromptJpsi.plotOn(tauz_frame_tot, ROOT.RooFit.LineStyle(ROOT.kSolid), ROOT.RooFit.LineColor(ROOT.kBlack))
    ResNonPromptJpsi.plotOn(tauz_frame_tot, ROOT.RooFit.Components("pdfResNonPromptJpsi"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kOrange+7))
    ResNonPromptJpsi.plotOn(tauz_frame_tot, ROOT.RooFit.Components("pdfResPromptJpsi"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kAzure+7))

    canvasTest = ROOT.TCanvas("canvasTest", "canvasTest", 600, 600) ;
    ROOT.gPad.SetLogy(1)
    ROOT.gPad.SetLeftMargin(0.15)
    tauz_frame.GetYaxis().SetTitleOffset(1.4)
    tauz_frame.Draw()

    canvasTestTot = ROOT.TCanvas("canvasTestTot", "canvasTestTot", 600, 600) ;
    ROOT.gPad.SetLogy(1)
    ROOT.gPad.SetLeftMargin(0.15)
    tauz_frame_tot.GetYaxis().SetTitleOffset(1.4)
    tauz_frame_tot.Draw()

    canvasTest.SaveAs("testNonPrompt.pdf")
    canvasTestTot.SaveAs("testPrompt_NonPrompt.pdf")


    w_non_prompt = ROOT.RooWorkspace("w", "workspace non-prompt Jpsi")
    getattr(w_non_prompt, 'import')(pdfResNonPromptJpsi)
    w_non_prompt.writeToFile("templateNonPromptJpsi.root");
    ROOT.gDirectory.Add(w_non_prompt);


def combine_components() :
    fInPromptJpsi = ROOT.TFile("templatePromptJpsi.root", "READ")
    wPromptJpsi = fInPromptJpsi.Get("w");
    wPromptJpsi.Print();
    pdfResPromptJpsi = wPromptJpsi.pdf("pdfResPromptSig")

    fInNonPromptJpsi = ROOT.TFile("templateNonPromptJpsi.root", "READ")
    wNonPromptJpsi = fInNonPromptJpsi.Get("w");
    wNonPromptJpsi.Print();
    pdfResNonPromptJpsi = wNonPromptJpsi.pdf("pdfResNonPromptSig")

    #weight1 = ROOT.RooRealVar("weight1","weight1",0.75,0,1)
    #weight2 = ROOT.RooRealVar("weight2","weight2",0.15,0,1)
    #pdfResJpsi = ROOT.RooAddPdf("pdfResJpsi", "Prompt + NonPrompt", ROOT.RooArgList(pdfResPromptJpsi, pdfResNonPromptJpsi), ROOT.RooArgList(weight1, weight2))


def main():
    init_prompt_jpsi_pdf()
    init_non_prompt_jpsi_pdf()
    #combine_components()

main()
