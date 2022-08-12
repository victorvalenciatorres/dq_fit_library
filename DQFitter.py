import ROOT
from ROOT import TCanvas, TFile, TH1F, TPaveText, RooRealVar, RooDataSet, RooWorkspace, RooDataHist, RooArgSet
from ROOT import gPad, gROOT, kRed, kBlue, kGreen

class DQFitter:
    def __init__(self, fInName, fInputName):
        self.fPdfDict          = {}
        self.fFileOut          = TFile("FitResults.root", "RECREATE")
        self.fFileIn           = TFile.Open(fInName)
        self.fInput            = self.fFileIn.Get(fInputName)
        self.fRooWorkspace     = RooWorkspace('w','workspace')
        self.fParNames         = []
        self.fFitMethod        = "likelyhood"
        self.fFitRangeMin      = []
        self.fFitRangeMax      = []
        self.fTrialName        = ""
        self.fRooMass          = RooRealVar("m", "#it{M} (GeV/#it{c}^{2})", 2, 5)

    def SetFitConfig(self, pdfDict):
        '''
        Method set the configuration of the fit
        '''
        self.fPdfDict = pdfDict
        self.fFitMethod = pdfDict["fitMethod"]
        self.fFitRangeMin = pdfDict["fitRangeMin"]
        self.fFitRangeMax = pdfDict["fitRangeMax"]

        pdfList = []
        for pdf in self.fPdfDict["pdf"][:-1]:
            self.fTrialName = self.fTrialName + pdf + "_"
        
        for i in range(0, len(self.fPdfDict["pdf"])):
            if not self.fPdfDict["pdf"][i] == "SUM":
                gROOT.ProcessLineSync(".x ../fit_library/{}Pdf.cxx+".format(self.fPdfDict["pdf"][i]))
        
        for i in range(0, len(self.fPdfDict["pdf"])):
            parVal = self.fPdfDict["parVal"][i]
            parLimMin = self.fPdfDict["parLimMin"][i]
            parLimMax = self.fPdfDict["parLimMax"][i]
            parName = self.fPdfDict["parName"][i]

            if not self.fPdfDict["pdf"][i] == "SUM":
                # Filling parameter list
                for j in range(0, len(parVal)):
                    if ("sum" in parName[j]) or ("prod" in parName[j]):
                        self.fRooWorkspace.factory("{}".format(parName[j]))
                        # Replace the exression of the parameter with the name of the parameter
                        r1 = parName[j].find("::") + 2
                        r2 = parName[j].find("(", r1)
                        parName[j] = parName[j][r1:r2]
                    else:
                        self.fRooWorkspace.factory("{}[{},{},{}]".format(parName[j], parVal[j], parLimMin[j], parLimMax[j]))
                        self.fParNames.append(parName[j]) # only free parameters will be reported in the histogram of results

                # Define the pdf associating the parametes previously defined
                nameFunc = self.fPdfDict["pdf"][i]
                nameFunc += "Pdf::{}Pdf(m[2,5]".format(self.fPdfDict["pdfName"][i])
                pdfList.append(self.fPdfDict["pdfName"][i])

                for j in range(0, len(parVal)):
                    nameFunc += ",{}".format(parName[j])
                nameFunc += ")"
                self.fRooWorkspace.factory(nameFunc)
            else:
                nameFunc = self.fPdfDict["pdf"][i]
                nameFunc += "::sum("

                for j in range(0, len(pdfList)):
                    nameFunc += "{}[{},{},{}]*{}Pdf".format(parName[j], parVal[j], parLimMin[j], parLimMax[j], pdfList[j])
                    self.fParNames.append(parName[j])
                    if not j == len(pdfList) - 1:
                        nameFunc += ","
                nameFunc += ")"
                print(nameFunc)
                self.fRooWorkspace.factory(nameFunc)

        self.fRooWorkspace.Print()

    def FitInvMassSpectrum(self, fitMethod, fitRangeMin, fitRangeMax):
        '''
        Method to perform the fit to the invariant mass spectrum
        '''
        trialName = self.fTrialName + "_" + str(fitRangeMin) + "_" + str(fitRangeMax)
        self.fRooWorkspace.Print()
        pdf = self.fRooWorkspace.pdf("sum")
        self.fRooMass.setRange("range", fitRangeMin, fitRangeMax)
        fRooPlot = self.fRooMass.frame(ROOT.RooFit.Title(trialName))
        if "TTree" in self.fInput.ClassName():
            print("########### Perform unbinned fit ###########")
            rooDs = RooDataSet("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))
        else:
            print("########### Perform binned fit ###########")
            rooDs = RooDataHist("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))

        # Select the fit method
        if fitMethod == "likelyhood":
            print("########### Perform likelyhood fit ###########")
            rooFitRes = ROOT.RooFitResult(pdf.fitTo(rooDs, ROOT.RooFit.Extended(ROOT.kTRUE), ROOT.RooFit.Save()))
        if fitMethod == "chi2":
            print("########### Perform X2 fit ###########")
            rooFitRes = ROOT.RooFitResult(pdf.chi2FitTo(rooDs, ROOT.RooFit.Save()))

        index = 1
        histResults = TH1F("fit_results_{}".format(trialName), "fit_results_{}".format(trialName), len(self.fParNames), 0., len(self.fParNames))
        for parName in self.fParNames:
            histResults.GetXaxis().SetBinLabel(index, parName)
            histResults.SetBinContent(index, self.fRooWorkspace.var(parName).getVal())
            histResults.SetBinError(index, self.fRooWorkspace.var(parName).getError())
            index += 1

        # Print the fit result
        rooFitRes.Print()

        rooDs.plotOn(fRooPlot, ROOT.RooFit.MarkerStyle(20), ROOT.RooFit.MarkerSize(0.6))
        pdf.plotOn(fRooPlot, ROOT.RooFit.VisualizeError(rooFitRes, 1), ROOT.RooFit.FillColor(ROOT.kRed-10), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        rooDs.plotOn(fRooPlot, ROOT.RooFit.MarkerStyle(20), ROOT.RooFit.MarkerSize(0.6))
        pdf.plotOn(fRooPlot, ROOT.RooFit.LineColor(ROOT.kRed), ROOT.RooFit.LineWidth(1), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        for i in range(0, len(self.fPdfDict["pdf"])):
            if not self.fPdfDict["pdfName"][i] == "SUM":
                pdf.plotOn(fRooPlot, ROOT.RooFit.Components("{}Pdf".format(self.fPdfDict["pdfName"][i])), ROOT.RooFit.LineColor(self.fPdfDict["pdfColor"][i]), ROOT.RooFit.LineStyle(2), ROOT.RooFit.LineWidth(1), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        
        paveText = TPaveText(0.60, 0.45, 0.99, 0.94, "brNDC")
        paveText.SetTextFont(42)
        paveText.SetTextSize(0.025)
        paveText.SetFillColor(ROOT.kWhite)
        for parName in self.fParNames:
            paveText.AddText("{} = {:.4f} #pm {:.4f}".format(parName, self.fRooWorkspace.var(parName).getVal(), self.fRooWorkspace.var(parName).getError()))
            for i in range(0, len(self.fPdfDict["pdfName"])):
                if self.fPdfDict["pdfName"][i] in parName:
                    (paveText.GetListOfLines().Last()).SetTextColor(self.fPdfDict["pdfColor"][i])

        # Print the chiSquare value
        nPars = rooFitRes.floatParsFinal().getSize()
        nBins = fRooPlot.GetXaxis().FindBin(fitRangeMax) - fRooPlot.GetXaxis().FindBin(fitRangeMin)
        paveText.AddText("#bf{#chi^{2} = %3.2f}" % (fRooPlot.chiSquare()))
        paveText.AddText("n Par = %3.2f" % (nPars))
        paveText.AddText("n Bins = %3.2f" % (nBins))
        paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % (fRooPlot.chiSquare()/(nBins - nPars)))
        fRooPlot.addObject(paveText)

        # Fit plot
        canvasFit = TCanvas("fit_plot_{}".format(trialName), "fit_plot_{}".format(trialName), 800, 600)
        canvasFit.SetLeftMargin(0.15)
        gPad.SetLeftMargin(0.15)
        fRooPlot.GetYaxis().SetTitleOffset(1.4)
        fRooPlot.Draw()

        # Ratio plot
        rooHistRatio = fRooPlot.residHist()
        rooPlotRatio = self.fRooMass.frame(ROOT.RooFit.Title("Residual Distribution"))
        rooPlotRatio.addPlotable(rooHistRatio,"P")
        canvasRatio = TCanvas("ratio_plot_{}".format(trialName), "ratio_plot_{}".format(trialName), 600, 600)
        canvasRatio.SetLeftMargin(0.15)
        rooPlotRatio.GetYaxis().SetTitleOffset(1.4)
        rooPlotRatio.Draw()

        # Pull plot
        rooHistPull = fRooPlot.pullHist()
        rooPlotPull = self.fRooMass.frame(ROOT.RooFit.Title("Pull Distribution"))
        rooPlotPull.addPlotable(rooHistPull,"P")
        canvasPull = TCanvas("pull_plot_{}".format(trialName), "pull_plot_{}".format(trialName), 600, 600)
        canvasPull.SetLeftMargin(0.15)
        rooPlotPull.GetYaxis().SetTitleOffset(1.4)
        rooPlotPull.Draw()

        # Correlation matrix histogram
        histCorrMat = rooFitRes.correlationHist("hist_corr_mat_{}".format(trialName))
        canvasCorrMat = TCanvas("corr_mat_{}".format(trialName), "corr_mat_{}".format(trialName), 600, 600)
        histCorrMat.Draw("COLZ")

        # Save results
        self.fFileOut.cd()
        histResults.Write()
        canvasFit.Write()
        canvasRatio.Write()
        canvasPull.Write()
        canvasCorrMat.Write()
        rooFitRes.Write("info_fit_results_{}".format(trialName))
    
    def MultiTrial(self):
        '''
        Method to perform a multi-trial fit
        '''
        for iRange in range(0, len(self.fFitRangeMin)):
            self.FitInvMassSpectrum(self.fFitMethod, self.fFitRangeMin[iRange], self.fFitRangeMax[iRange])
        self.fFileOut.Close()



#################################################
    def OptimizeFit(self):
        '''
        Optimize fit parameters and perform bin counting
        '''

        pdf = self.fRooWorkspace.pdf("BkgPdf")
        background_norm = ROOT.RooRealVar("background_norm", "background_norm", 1e4, 0, 1e8)
        model = ROOT.RooAddPdf("model","model",ROOT.RooArgList(pdf),ROOT.RooArgList(background_norm))

        if "TTree" in self.fInput.ClassName():
            print("########### Perform unbinned fit ###########")
            rooDs = RooDataSet("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))
        else:
            print("########### Perform binned fit ###########")
            rooDs = RooDataHist("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))

        # Fit pdf to all data
        r_full = model.fitTo(rooDs, ROOT.RooFit.Save(ROOT.kTRUE))
        
        # Fit partial range
        # ----------------------------------
        # Define "signal" range in x as [-3,3]
        self.fRooMass.setRange("left", 2., 2.7)
        self.fRooMass.setRange("right", 4, 5)
        
        # Fit pdf only to data in "signal" range
        r_left = model.fitTo(rooDs, ROOT.RooFit.Save(ROOT.kTRUE))
        
        # Plot/print results
        # ---------------------------------------
        
        # Make plot frame in x and add data and fitted model
        frame = self.fRooMass.frame(ROOT.RooFit.Title("Fitting a sub range"))
        rooDs.plotOn(frame)
        model.plotOn(frame, ROOT.RooFit.Range("left,right"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kRed)) # Add shape in full ranged dashed
        model.plotOn(frame)  # By default only fitted range is shown


        c = ROOT.TCanvas("rf203_ranges", "rf203_ranges", 600, 600)
        ROOT.gPad.SetLeftMargin(0.15)
        frame.GetYaxis().SetTitleOffset(1.4)
        frame.Draw()
        c.Update()
 
        input()

        #self.fRooMass.setRange("FULL", 2, 5)
        #self.fRooMass.setRange("LEFT", 2.1, 2.5)
        #self.fRooMass.setRange("RIGHT", 4, 5)
        #fRooPlot = self.fRooMass.frame(ROOT.RooFit.Title("test"))

        #rooFitRes = ROOT.RooFitResult(model.fitTo(rooDs, ROOT.RooFit.Range("FULL"), ROOT.RooFit.Save()))
        #rooFitRes.Print()
        
        #model.fitTo(rooDs, ROOT.RooFit.Range(2, 2.5))

        #rooDs.plotOn(fRooPlot, ROOT.RooFit.MarkerStyle(20), ROOT.RooFit.MarkerSize(0.6))
        #model.plotOn(fRooPlot, ROOT.RooFit.LineColor(ROOT.kRed), ROOT.RooFit.LineWidth(2))

        # Fit plot
        #canvasFit = TCanvas("canvasFit", "canvasFit", 600, 600)
        #canvasFit.SetLeftMargin(0.15)
        #gPad.SetLeftMargin(0.15)
        #fRooPlot.GetYaxis().SetTitleOffset(1.4)
        #fRooPlot.Draw()
        #canvasFit.Update()

        input()