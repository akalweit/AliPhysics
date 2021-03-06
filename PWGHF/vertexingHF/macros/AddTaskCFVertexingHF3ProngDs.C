
//----------------------------------------------------

AliCFTaskVertexingHF *AddTaskCFVertexingHF3ProngDs(TString suffixName="", Int_t decayOption=AliCFVertexingHF3Prong::kCountResonant, const char* cutFile = "./DstoKKpiCuts.root", Int_t configuration = AliCFTaskVertexingHF::kSnail, Bool_t isKeepDfromB=kFALSE, Bool_t isKeepDfromBOnly=kFALSE, Int_t pdgCode = 431, Char_t isSign = 2, Bool_t useNtrkWeight=kFALSE, Bool_t isFineNtrkBin=kFALSE)
//AliCFContainer *AddTaskCFVertexingHF3ProngDs(const char* cutFile = "./DstoKKpiCuts.root", Int_t configuration = AliCFTaskVertexingHF::kSnail, Bool_t isKeepDfromB=kFALSE, Bool_t isKeepDfromBOnly=kFALSE, Int_t pdgCode = 431, Char_t isSign = 2)
{
//DEFINITION OF A FEW CONSTANTS
const Double_t ymin  = -1.2 ;
const Double_t ymax  =  1.2 ;
const Double_t cosmin = -0.7;
const Double_t cosmax =  1.05;
const Double_t cTmin = 0;  // micron
const Double_t cTmax = 500;  // micron
const Double_t phimin = 0.0;  
const Int_t    mintrackrefsTPC = 2 ;
const Int_t    mintrackrefsITS = 3 ;
const Int_t    charge  = 1 ;
const Int_t    minclustersTPC = 50 ;
// cuts
const Double_t ptmin = 0.1;
const Double_t ptmax = 9999.;
const Double_t etamin = -0.9;
const Double_t etamax = 0.9;
const Double_t zvtxmin = -15;
const Double_t zvtxmax = 15;
const Int_t    minITSClusters = 5;

const Float_t centmin_0_10 = 0.;
const Float_t centmax_0_10 = 10.;
const Float_t centmin_10_60 = 10.;
const Float_t centmax_10_60 = 60.;
const Float_t centmin_60_100 = 60.;
const Float_t centmax_60_100 = 100.;
const Float_t centmax = 100.;
const Float_t fakemin = -0.5;
const Float_t fakemax = 2.5;
const Float_t cosminXY = 0.90;
const Float_t cosmaxXY = 1.0;
const Float_t normDecLXYmin = 0;
const Float_t normDecLXYmax = 20;
const Float_t multmin_0_20 = 0;
const Float_t multmax_0_20 = 20;
const Float_t multmin_20_50 = 20;
const Float_t multmax_20_50 = 50;
const Float_t multmin_50_102 = 50;
const Float_t multmax_50_102 = 102;


	printf("Addig CF task using cuts from file %s\n",cutFile);
	if (configuration == AliCFTaskVertexingHF::kSnail){
		printf("The configuration is set to be SLOW --> all the variables will be used to fill the CF\n");
	}
	else if (configuration == AliCFTaskVertexingHF::kCheetah){
		printf("The configuration is set to be FAST --> using only pt, y, ct, phi, zvtx, centrality, fake, multiplicity to fill the CF\n");
	}
	else if (configuration == AliCFTaskVertexingHF::kFalcon){
		printf("The configuration is set to be FAST --> using only pt, y, centrality, multiplicity to fill the CF\n");
	}
	else{
		printf("The configuration is not defined! returning\n");
		return NULL;
	}
	       
	gSystem->Sleep(2000);

	// isSign = 0 --> D+s only
	// isSign = 1 --> D-s only
	// isSign = 2 --> D+s + D-s
	
	TString expected;
	if (isSign == 0 && pdgCode < 0){
		Printf("ERROR: Error setting PDG code (%d) and sign (0 --> particle (%d) only): they are not compatible, returning",pdgCode,isSign);
		return 0x0;
	}
	else if (isSign == 1 && pdgCode > 0){
		Printf("ERROR: Error setting PDG code (%d) and sign (1 --> antiparticle (%d) only): they are not compatible, returning",pdgCode,isSign);
		return 0x0;
	}
	else if (isSign > 2 || isSign < 0){
		Printf("ERROR: Sign not valid (%d, possible values are 0, 1, 2), returning",isSign);
		return 0x0;
	}

	TFile* fileCuts = TFile::Open(cutFile);
	if(!fileCuts || (fileCuts && !fileCuts->IsOpen())){ 
	  Printf("ERROR: Wrong cut file");
	  return 0x0;
	}
	AliRDHFCutsDstoKKpi *cutsDstoKKpi = (AliRDHFCutsDstoKKpi*)fileCuts->Get("AnalysisCuts");
	
	// check that the fKeepD0fromB flag is set to true when the fKeepD0fromBOnly flag is true
	//  for now the binning is the same than for all D's
	if(isKeepDfromBOnly) isKeepDfromB = true;
	
	/*
	  Double_t ptmin_0_4;
	  Double_t ptmax_0_4;
	  Double_t ptmin_4_8;
	  Double_t ptmax_4_8;
	  Double_t ptmin_8_10;
	  Double_t ptmax_8_10;
	  
	  if(!isKeepDfromB){
	  ptmin_0_4 =  0.0 ;
	  ptmax_0_4 =  4.0 ;
	  ptmin_4_8 =  4.0 ;
	  ptmax_4_8 =  8.0 ;
	  ptmin_8_10 =  8.0 ;
	  ptmax_8_10 =  10.0 ;
	  } else{
	  ptmin_0_4 =  0.0 ;
	  ptmax_0_4 =  3.0 ;
	  ptmin_4_8 =  3.0 ;
	  ptmax_4_8 =  5.0 ;
	  ptmin_8_10 =  5.0 ;
	  ptmax_8_10 =  10.0 ;
	  }
	*/

	//CONTAINER DEFINITION
	Info("AliCFTaskVertexingHF","SETUP CONTAINER");

	const Double_t phimax = 2*TMath::Pi();

	//Setting up the container grid... 
	UInt_t nstep = 10; //number of selection steps: MC with limited acceptance, MC, Acceptance, Vertex, Refit, Reco (no cuts), RecoAcceptance, RecoITSClusters (RecoAcceptance included), RecoPPR (RecoAcceptance+RecoITSCluster included), RecoPID 
// 	const Int_t nbinpt_0_4  = 8 ; //bins in pt from 0 to 4 GeV
// 	const Int_t nbinpt_4_8  = 4 ; //bins in pt from 4 to 8 GeV
// 	const Int_t nbinpt_8_10  = 1 ; //bins in pt from 8 to 10 GeV

/*
	Int_t nbinpt_0_4;
	Int_t nbinpt_4_8;
	Int_t nbinpt_8_10;
	if (!isKeepDfromB){
	  nbinpt_0_4  = 8 ; //bins in pt from 0 to 4 GeV
	  nbinpt_4_8  = 4 ; //bins in pt from 4 to 8 GeV
	  nbinpt_8_10  = 1 ; //bins in pt from 8 to 10 GeV
	}else{
	  nbinpt_0_4  = 3 ; //bins in pt from 0 to 3 GeV
	  nbinpt_4_8  = 1 ; //bins in pt from 3 to 5 GeV
	  nbinpt_8_10  = 1 ; //bins in pt from 5 to 10 GeV
	}
*/
	const Int_t nbinpt = cutsDstoKKpi->GetNPtBins(); // bins in pT
	printf("pT: nbin (from cuts file) = %d\n",nbinpt);
	const Int_t nbiny  = 24 ; //bins in y
	const Int_t nbinphi  = 18 ; //bins in phi
	const Int_t nbincT  = 2 ; //bins in cT 
	const Int_t nbinpointing  = 350 ; //bins in cosPointingAngle	
	const Int_t nbinpT1_0_4  = 8 ; //bins in pt1 from 0 to 4 GeV
	const Int_t nbinpT1_4_8  = 4 ; //bins in pt1 from 4 to 8 GeV
	const Int_t nbinpT1_8_10  = 1 ; //bins in pt1 from 8 to 10 GeV
	const Int_t nbinpT2_0_4  = 8 ; //bins in pt2 from 0 to 4 GeV
	const Int_t nbinpT2_4_8  = 4 ; //bins in pt2 from 4 to 8 GeV
	const Int_t nbinpT2_8_10  = 1 ; //bins in pt2 from 8 to 10 GeV
	const Int_t nbinpT3_0_4  = 8 ; //bins in pt3 from 0 to 4 GeV
	const Int_t nbinpT3_4_8  = 4 ; //bins in pt3 from 4 to 8 GeV
	const Int_t nbinpT3_8_10  = 1 ; //bins in pt3 from 8 to 10 GeV
	const Int_t nbinzvtx  = 30 ; //bins in z vertex
	const Int_t nbincent = 18;  //bins in centrality
	const Int_t nbincent_0_10 = 4;  //bins in centrality between 0 and 10
	const Int_t nbincent_10_60 = 10;  //bins in centrality between 10 and 60
	const Int_t nbincent_60_100 = 4;  //bins in centrality between 60 and 100
	const Int_t nbinfake = 3;  //bins in fake
	const Int_t nbinpointingXY = 50;  //bins in cosPointingAngleXY
	const Int_t nbinnormDecayLXY = 20;  //bins in NormDecayLengthXY
	const Int_t nbinmult = 48;  //bins in multiplicity (total number)
	const Int_t nbinmult_0_20 = 20; //bins in multiplicity between 0 and 20
	const Int_t nbinmult_20_50 = 15; //bins in multiplicity between 20 and 50
	const Int_t nbinmult_50_102 = 13; //bins in multiplicity between 50 and 102
	
    Int_t nbinmultTmp=nbinmult;
    Int_t nbinLimmultFine;
    Double_t* binLimmultFine;
    if(isFineNtrkBin){
        nbinLimmultFine=250;
        const UInt_t nbinMultFine = nbinLimmultFine;
        binLimmultFine = new Double_t[nbinMultFine+1];
        for (Int_t ibin0 = 0 ; ibin0<=nbinMultFine; ibin0++){
            binLimmultFine[ibin0] = ibin0;
        }
        nbinmultTmp=nbinLimmultFine;
    }
    const Int_t nbinmultTot=nbinmultTmp;

    
	//the sensitive variables, their indices
	const UInt_t ipT = 0;
	const UInt_t iy  = 1;
	const UInt_t iphi  = 2;
	const UInt_t icT  = 3;
	const UInt_t ipointing  = 4;
	const UInt_t ipT1  = 5;
	const UInt_t ipT2  = 6;
	const UInt_t ipT3  = 7;
	const UInt_t izvtx  = 8;
	const UInt_t icent = 9;
	const UInt_t ifake = 10;
	const UInt_t ipointingXY = 11;
	const UInt_t inormDecayLXY = 12;
	const UInt_t imult = 13;

	const Int_t nvarTot   = 14 ; 

	//arrays for the number of bins in each dimension
	Int_t iBin[nvarTot];
 	//iBin[ipT]=nbinpt_0_4+nbinpt_4_8+nbinpt_8_10;
	iBin[ipT]=nbinpt;
	iBin[iy]=nbiny;
	iBin[iphi]=nbinphi;
	// 	iBin[icT]=nbincT_0_4+nbincT_4_8+nbincT_8_10;
 	//iBin[4]=nbinpointing_0_4+nbinpointing_4_8+nbinpointing_8_10;
 	iBin[icT]=nbincT;
 	iBin[ipointing]=nbinpointing;
	iBin[ipT1]=nbinpt;
	iBin[ipT2]=nbinpt;
	iBin[ipT3]=nbinpt;
	iBin[izvtx]=nbinzvtx;
	iBin[icent]=nbincent;
	iBin[ifake]=nbinfake;
	iBin[ipointingXY]=nbinpointingXY;
	iBin[inormDecayLXY]=nbinnormDecayLXY;
	iBin[imult]=nbinmultTot;
	
	//arrays for lower bounds :
	Double_t *binLimpT=new Double_t[iBin[ipT]+1];
	Double_t *binLimy=new Double_t[iBin[iy]+1];
	Double_t *binLimphi=new Double_t[iBin[iphi]+1];
	Double_t *binLimcT=new Double_t[iBin[icT]+1];
	Double_t *binLimpointing=new Double_t[iBin[ipointing]+1];
	Double_t *binLimpT1=new Double_t[iBin[ipT1]+1];
	Double_t *binLimpT2=new Double_t[iBin[ipT2]+1];
	Double_t *binLimpT3=new Double_t[iBin[ipT3]+1];
	Double_t *binLimzvtx=new Double_t[iBin[izvtx]+1];
	Double_t *binLimcent=new Double_t[iBin[icent]+1];
	Double_t *binLimfake=new Double_t[iBin[ifake]+1];
	Double_t *binLimpointingXY=new Double_t[iBin[ipointingXY]+1];
	Double_t *binLimnormDecayLXY=new Double_t[iBin[inormDecayLXY]+1];
	Double_t *binLimmult=new Double_t[iBin[imult]+1];
	
	// checking limits
	/*
	  if (ptmax_0_4 != ptmin_4_8) {
	  Error("AliCFHeavyFlavourTaskMultiVarMultiStep","max lim 1st range != min lim 2nd range, please check!");
	  }
	  if (ptmax_4_8 != ptmin_8_10) {
	  Error("AliCFHeavyFlavourTaskMultiVarMultiStep","max lim 2nd range != min lim 3rd range, please check!");
	  }
	*/
	// values for bin lower bounds
	// pt
	Float_t* floatbinLimpT = cutsDstoKKpi->GetPtBinLimits();
	for (Int_t ibinpT = 0 ; ibinpT<iBin[ipT]+1; ibinpT++){
		binLimpT[ibinpT] = (Double_t)floatbinLimpT[ibinpT];
		binLimpT1[ibinpT] = (Double_t)floatbinLimpT[ibinpT];
		binLimpT2[ibinpT] = (Double_t)floatbinLimpT[ibinpT];
		binLimpT3[ibinpT] = (Double_t)floatbinLimpT[ibinpT];
	}
	for(Int_t i=0; i<=nbinpt; i++) printf("binLimpT[%d]=%f\n",i,binLimpT[i]);  
	
	/*
	  for(Int_t i=0; i<=nbinpt_0_4; i++) binLimpT[i]=(Double_t)ptmin_0_4 + (ptmax_0_4-ptmin_0_4)/nbinpt_0_4*(Double_t)i ; 
	  if (binLimpT[nbinpt_0_4] != ptmin_4_8)  {
	  Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for pt - 1st range - differs from expected!\n");
	  }
	  for(Int_t i=0; i<=nbinpt_4_8; i++) binLimpT[i+nbinpt_0_4]=(Double_t)ptmin_4_8 + (ptmax_4_8-ptmin_4_8)/nbinpt_4_8*(Double_t)i ; 
	  if (binLimpT[nbinpt_0_4+nbinpt_4_8] != ptmin_8_10)  {
	  Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for pt - 2nd range - differs from expected!\n");
	  }
	  for(Int_t i=0; i<=nbinpt_8_10; i++) binLimpT[i+nbinpt_0_4+nbinpt_4_8]=(Double_t)ptmin_8_10 + (ptmax_8_10-ptmin_8_10)/nbinpt_8_10*(Double_t)i ; 
	*/
	
	// y
	for(Int_t i=0; i<=nbiny; i++) binLimy[i]=(Double_t)ymin  + (ymax-ymin)  /nbiny*(Double_t)i ;
	
	// Phi
	for(Int_t i=0; i<=nbinphi; i++) binLimphi[i]=(Double_t)phimin  + (phimax-phimin)  /nbinphi*(Double_t)i ;
	
	// cT
	for(Int_t i=0; i<=nbincT; i++) binLimcT[i]=(Double_t)cTmin  + (cTmax-cTmin)  /nbincT*(Double_t)i ;
	
	// cosPointingAngle
	for(Int_t i=0; i<=nbinpointing; i++) binLimpointing[i]=(Double_t)cosmin  + (cosmax-cosmin)  /nbinpointing*(Double_t)i ;

	/*
	// ptPi
	for(Int_t i=0; i<=nbincT_0_4; i++) binLimcT[i]=(Double_t)ptmin_0_4 + (ptmax_0_4-ptmin_0_4)/nbincT_0_4*(Double_t)i ; 
	if (binLimcT[nbincT_0_4] != ptmin_4_8)  {
	Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for ptPi - 1st range - differs from expected!");
	}
	for(Int_t i=0; i<=nbincT_4_8; i++) binLimcT[i+nbincT_0_4]=(Double_t)ptmin_4_8 + (ptmax_4_8-ptmin_4_8)/nbincT_4_8*(Double_t)i ; 
	if (binLimcT[nbincT_0_4+nbincT_4_8] != ptmin_8_10)  {
	Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for ptPi - 2nd range - differs from expected!\n");
	}
	for(Int_t i=0; i<=nbincT_8_10; i++) binLimcT[i+nbincT_0_4+nbincT_4_8]=(Double_t)ptmin_8_10 + (ptmax_8_10-ptmin_8_10)/nbincT_8_10*(Double_t)i ; 
	
	// ptKa
	for(Int_t i=0; i<=nbinpointing_0_4; i++) binLimpointing[i]=(Double_t)ptmin_0_4 + (ptmax_0_4-ptmin_0_4)/nbinpointing_0_4*(Double_t)i ; 
	if (binLimpointing[nbinpointing_0_4] != ptmin_4_8)  {
	Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for ptKa - 1st range - differs from expected!");
	}
	for(Int_t i=0; i<=nbinpointing_4_8; i++) binLimpointing[i+nbinpointing_0_4]=(Double_t)ptmin_4_8 + (ptmax_4_8-ptmin_4_8)/nbinpointing_4_8*(Double_t)i ; 
	if (binLimpointing[nbinpointing_0_4+nbinpointing_4_8] != ptmin_8_10)  {
	Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for ptKa - 2nd range - differs from expected!\n");
	}
	for(Int_t i=0; i<=nbinpointing_8_10; i++) binLimpointing[i+nbinpointing_0_4+nbinpointing_4_8]=(Double_t)ptmin_8_10 + (ptmax_8_10-ptmin_8_10)/nbinpointing_8_10*(Double_t)i ; 
	*/
	
	// z Primary Vertex
	for(Int_t i=0; i<=nbinzvtx; i++) {
		binLimzvtx[i]=(Double_t)zvtxmin  + (zvtxmax-zvtxmin)  /nbinzvtx*(Double_t)i ;
	}
	
	// centrality 
	for(Int_t i=0; i<=nbincent_0_10; i++) binLimcent[i]=(Double_t)centmin_0_10 + (centmax_0_10-centmin_0_10)/nbincent_0_10*(Double_t)i ; 
	if (binLimcent[nbincent_0_10] != centmin_10_60)  {
	  Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for cent - 1st range - differs from expected!\n");
	}
	for(Int_t i=0; i<=nbincent_10_60; i++) binLimcent[i+nbincent_0_10]=(Double_t)centmin_10_60 + (centmax_10_60-centmin_10_60)/nbincent_10_60*(Double_t)i ;
	if (binLimcent[nbincent_0_10+nbincent_10_60] != centmin_60_100)  {
		Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for cent - 2st range - differs from expected!\n");
	}
	for(Int_t i=0; i<=nbincent_60_100; i++) binLimcent[i+nbincent_0_10+nbincent_10_60]=(Double_t)centmin_60_100 + (centmax_60_100-centmin_60_100)/nbincent_60_100*(Double_t)i ;
	
	// fake
	for(Int_t i=0; i<=nbinfake; i++) {
	  binLimfake[i]=(Double_t)fakemin  + (fakemax-fakemin)/nbinfake * (Double_t)i;
	}

	// cosPointingAngleXY
	for(Int_t i=0; i<=nbinpointingXY; i++) binLimpointingXY[i]=(Double_t)cosminXY  + (cosmaxXY-cosminXY)  /nbinpointingXY*(Double_t)i ;

	// normDecayLXY
	for(Int_t i=0; i<=nbinnormDecayLXY; i++) binLimnormDecayLXY[i]=(Double_t)normDecLXYmin  + (normDecLXYmax-normDecLXYmin)  /nbinnormDecayLXY*(Double_t)i ;

	// multiplicity
	for(Int_t i=0; i<=nbinmult_0_20; i++) binLimmult[i]=(Double_t)multmin_0_20 + (multmax_0_20-multmin_0_20)/nbinmult_0_20*(Double_t)i ; 
	if (binLimmult[nbinmult_0_20] != multmin_20_50)  {
		Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for mult - 1st range - differs from expected!\n");
	}
	for(Int_t i=0; i<=nbinmult_20_50; i++) binLimmult[i+nbinmult_0_20]=(Double_t)multmin_20_50 + (multmax_20_50-multmin_20_50)/nbinmult_20_50*(Double_t)i ; 
	if (binLimmult[nbinmult_0_20+nbinmult_20_50] != multmin_50_102)  {
		Error("AliCFHeavyFlavourTaskMultiVarMultiStep","Calculated bin lim for mult - 2nd range - differs from expected!\n");
	}
	for(Int_t i=0; i<=nbinmult_50_102; i++) binLimmult[i+nbinmult_0_20+nbinmult_20_50]=(Double_t)multmin_50_102 + (multmax_50_102-multmin_50_102)/nbinmult_50_102*(Double_t)i ; 

	TString suffixDecayType="";
	if(decayOption==AliCFVertexingHF3Prong::kCountAllDsKKpi) suffixDecayType="_All";
	else if(decayOption==AliCFVertexingHF3Prong::kCountPhipi) suffixDecayType="_Phi";
	else if(decayOption==AliCFVertexingHF3Prong::kCountK0stK) suffixDecayType="_K0star";
	else if(decayOption==AliCFVertexingHF3Prong::kCountResonant) suffixDecayType="_Reson";
	else if(decayOption==AliCFVertexingHF3Prong::kCountNonResonant) suffixDecayType="_NonReson";

	//one "container" for MC
	TString nameContainer="";
	if(!isKeepDfromB) {
		nameContainer="CFHFccontainer0_3ProngDstoKKpi_CommonFramework";
	}
	else  if(isKeepDfromBOnly){
		nameContainer="CFHFccontainer0DfromB_3ProngDstoKKpi_CommonFramework";
	}
	else  {
		nameContainer="CFHFccontainer0allD_3ProngDstoKKpi_CommonFramework";          
	}
	nameContainer+=suffixDecayType.Data();
	nameContainer+=suffixName.Data();
	
	AliCFContainer* container;
	if (configuration == AliCFTaskVertexingHF::kSnail){
		container = new AliCFContainer(nameContainer,"container for tracks",nstep,nvarTot,iBin);
		//setting the bin limits
		printf("pt\n");
		container -> SetBinLimits(ipT,binLimpT);
		printf("y\n");
		container -> SetBinLimits(iy,binLimy);
		printf("Phi\n");
		container -> SetBinLimits(iphi,binLimphi);
		printf("cT\n");
		container -> SetBinLimits(icT,binLimcT);
		printf("pointing angle\n");
		container -> SetBinLimits(ipointing,binLimpointing);
		printf("pt1\n");
		container -> SetBinLimits(ipT1,binLimpT1);
		printf("pt2\n");
		container -> SetBinLimits(ipT2,binLimpT2);
		printf("pt3\n");
		container -> SetBinLimits(ipT3,binLimpT3);
		printf("zvtx \n");
		container -> SetBinLimits(izvtx,binLimzvtx);
		printf("cent\n");
		container -> SetBinLimits(icent,binLimcent);
		printf("fake\n");
		container -> SetBinLimits(ifake,binLimfake);
		printf("pointingXY\n");
		container -> SetBinLimits(ipointingXY,binLimpointingXY);
		printf("normDecayLXY\n");
		container -> SetBinLimits(inormDecayLXY,binLimnormDecayLXY);
		printf("multiplicity\n");
        if(isFineNtrkBin) container -> SetBinLimits(imult,binLimmultFine);
        else container -> SetBinLimits(imult,binLimmult);

		container -> SetVarTitle(ipT,"pt");
		container -> SetVarTitle(iy,"y");
		container -> SetVarTitle(iphi, "phi");
		container -> SetVarTitle(icT, "ct");
		container -> SetVarTitle(ipointing, "pointing");        
		container -> SetVarTitle(ipT1, "pt1");
		container -> SetVarTitle(ipT2, "pt2");
		container -> SetVarTitle(ipT3, "pt3");
		container -> SetVarTitle(izvtx, "zvtx");
		container -> SetVarTitle(icent, "centrality");
		container -> SetVarTitle(ifake, "fake");
		container -> SetVarTitle(ipointingXY, "piointingXY");
		container -> SetVarTitle(inormDecayLXY, "normDecayLXY");
		container -> SetVarTitle(imult, "multiplicity");
	}
	else if (configuration == AliCFTaskVertexingHF::kCheetah){
		//arrays for the number of bins in each dimension
		const Int_t nvar = 8;

		const UInt_t ipTFast = 0;
		const UInt_t iyFast = 1;
		const UInt_t icTFast = 2;
		const UInt_t iphiFast = 3;
		const UInt_t izvtxFast = 4;
		const UInt_t icentFast = 5;
		const UInt_t ifakeFast = 6;
		const UInt_t imultFast = 7;

		Int_t iBinFast[nvar];
		iBinFast[ipTFast] = iBin[ipT];
		iBinFast[iyFast] = iBin[iy];
		iBinFast[icTFast] = iBin[icT];
		iBinFast[iphiFast] = iBin[iphi];
		iBinFast[izvtxFast] = iBin[izvtx];
		iBinFast[icentFast] = iBin[icent];
		iBinFast[ifakeFast] = iBin[ifake];
		iBinFast[imultFast] = iBin[imult];

		container = new AliCFContainer(nameContainer,"container for tracks",nstep,nvar,iBinFast);
		printf("pt\n");
		container -> SetBinLimits(ipTFast,binLimpT);
		printf("y\n");
		container -> SetBinLimits(iyFast,binLimy);
		printf("ct\n");
		container -> SetBinLimits(icTFast,binLimcT);
		printf("phi\n");
		container -> SetBinLimits(iphiFast,binLimphi);
		printf("zvtx\n");
		container -> SetBinLimits(izvtxFast,binLimzvtx);
		printf("centrality\n");
		container -> SetBinLimits(icentFast,binLimcent);
		printf("fake\n");
		container -> SetBinLimits(ifakeFast,binLimfake);
		printf("multiplicity\n");
        if(isFineNtrkBin) container -> SetBinLimits(imultFast,binLimmultFine);
        else container -> SetBinLimits(imultFast,binLimmult);

		container -> SetVarTitle(ipTFast,"pt");
		container -> SetVarTitle(iyFast,"y");
		container -> SetVarTitle(icTFast, "ct");
		container -> SetVarTitle(iphiFast, "phi");
		container -> SetVarTitle(izvtxFast, "zvtx");
		container -> SetVarTitle(icentFast, "centrality");
		container -> SetVarTitle(ifakeFast, "fake");
		container -> SetVarTitle(imultFast, "multiplicity");
	}
	else if (configuration == AliCFTaskVertexingHF::kFalcon){
		//arrays for the number of bins in each dimension
		const Int_t nvar = 4;

		const UInt_t ipTSuperFast = 0;
		const UInt_t iySuperFast = 1;
		const UInt_t icentSuperFast = 2;
		const UInt_t imultSuperFast = 3;

		Int_t iBinSuperFast[nvar];
		iBinSuperFast[ipTSuperFast] = iBin[ipT];
		iBinSuperFast[iySuperFast] = iBin[iy];
		iBinSuperFast[icentSuperFast] = iBin[icent];
		iBinSuperFast[imultSuperFast] = iBin[imult];

		container = new AliCFContainer(nameContainer,"container for tracks",nstep,nvar,iBinSuperFast);
		printf("pt\n");
		container -> SetBinLimits(ipTSuperFast,binLimpT);
		printf("y\n");
		container -> SetBinLimits(iySuperFast,binLimy);
		printf("centrality\n");
		container -> SetBinLimits(icentSuperFast,binLimcent);
		printf("multiplicity\n");
        if(isFineNtrkBin) container -> SetBinLimits(imultSuperFast,binLimmultFine);
        else container -> SetBinLimits(imultSuperFast,binLimmult);

		container -> SetVarTitle(ipTSuperFast,"pt");
		container -> SetVarTitle(iySuperFast,"y");
		container -> SetVarTitle(icentSuperFast, "centrality");
		container -> SetVarTitle(imultSuperFast, "multiplicity");
	}

	//return container;

	container -> SetStepTitle(0, "MCLimAcc");
	container -> SetStepTitle(1, "MC");
        container -> SetStepTitle(2, "MCAcc");
        container -> SetStepTitle(3, "RecoVertex");
        container -> SetStepTitle(4, "RecoRefit");
        container -> SetStepTitle(5, "Reco");
        container -> SetStepTitle(6, "RecoAcc");
	container -> SetStepTitle(7, "RecoITSCluster");
	container -> SetStepTitle(8, "RecoCuts");
	container -> SetStepTitle(9, "RecoPID");


	//CREATE THE  CUTS -----------------------------------------------
	
	// Gen-Level kinematic cuts
	AliCFTrackKineCuts *mcKineCuts = new AliCFTrackKineCuts("mcKineCuts","MC-level kinematic cuts");
	
	//Particle-Level cuts:  
	AliCFParticleGenCuts* mcGenCuts = new AliCFParticleGenCuts("mcGenCuts","MC particle generation cuts");
	Bool_t useAbsolute = kTRUE;
	if (isSign != 2){
		useAbsolute = kFALSE;
	}
	mcGenCuts->SetRequirePdgCode(pdgCode, useAbsolute);  // kTRUE set in order to include antiparticle
	mcGenCuts->SetAODMC(1); //special flag for reading MC in AOD tree (important)
	
	// Acceptance cuts:
	AliCFAcceptanceCuts* accCuts = new AliCFAcceptanceCuts("accCuts", "Acceptance cuts");
	AliCFTrackKineCuts *kineAccCuts = new AliCFTrackKineCuts("kineAccCuts","Kine-Acceptance cuts");
	kineAccCuts->SetPtRange(ptmin,ptmax);
	kineAccCuts->SetEtaRange(etamin,etamax);

	// Rec-Level kinematic cuts
	AliCFTrackKineCuts *recKineCuts = new AliCFTrackKineCuts("recKineCuts","rec-level kine cuts");
	
	AliCFTrackQualityCuts *recQualityCuts = new AliCFTrackQualityCuts("recQualityCuts","rec-level quality cuts");
	
	AliCFTrackIsPrimaryCuts *recIsPrimaryCuts = new AliCFTrackIsPrimaryCuts("recIsPrimaryCuts","rec-level isPrimary cuts");
	
	printf("CREATE MC KINE CUTS\n");
	TObjArray* mcList = new TObjArray(0) ;
	mcList->AddLast(mcKineCuts);
	mcList->AddLast(mcGenCuts);
	
	printf("CREATE ACCEPTANCE CUTS\n");
	TObjArray* accList = new TObjArray(0) ;
	accList->AddLast(kineAccCuts);

	printf("CREATE RECONSTRUCTION CUTS\n");
	TObjArray* recList = new TObjArray(0) ;   // not used!! 
	recList->AddLast(recKineCuts);
	recList->AddLast(recQualityCuts);
	recList->AddLast(recIsPrimaryCuts);
	
	TObjArray* emptyList = new TObjArray(0);

	//CREATE THE INTERFACE TO CORRECTION FRAMEWORK USED IN THE TASK
	printf("CREATE INTERFACE AND CUTS\n");
	AliCFManager* man = new AliCFManager() ;
	man->SetParticleContainer(container);
	man->SetParticleCutsList(0 , mcList); // MC, Limited Acceptance
	man->SetParticleCutsList(1 , mcList); // MC
	man->SetParticleCutsList(2 , accList); // Acceptance 
	man->SetParticleCutsList(3 , emptyList); // Vertex 
	man->SetParticleCutsList(4 , emptyList); // Refit 
	man->SetParticleCutsList(5 , emptyList); // AOD
	man->SetParticleCutsList(6 , emptyList); // AOD in Acceptance
	man->SetParticleCutsList(7 , emptyList); // AOD with required n. of ITS clusters
	man->SetParticleCutsList(8 , emptyList); // AOD Reco (PPR cuts implemented in Task)
	man->SetParticleCutsList(9 , emptyList); // AOD Reco PID
	
	// Get the pointer to the existing analysis manager via the static access method.
	//==============================================================================
	AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
	if (!mgr) {
	  ::Error("AddTaskCompareHF", "No analysis manager to connect to.");
	  return NULL;
	}   
	//CREATE THE TASK
	printf("CREATE TASK\n");

	// create the task
	AliCFTaskVertexingHF *task = new AliCFTaskVertexingHF("AliCFTaskVertexingHF",cutsDstoKKpi);
	task->SetConfiguration(configuration);
	task->SetFillFromGenerated(kFALSE);
	task->SetDecayChannel(33);
	if(decayOption==AliCFVertexingHF3Prong::kCountAllDsKKpi) task->SetCountAllDs();
	else if(decayOption==AliCFVertexingHF3Prong::kCountPhipi) task->SetCountDsViaPhi();
	else if(decayOption==AliCFVertexingHF3Prong::kCountK0stK)  task->SetCountDsViaK0star();
	else if(decayOption==AliCFVertexingHF3Prong::kCountResonant) task->SetCountResonantDs();
	else if(decayOption==AliCFVertexingHF3Prong::kCountNonResonant) task->SetCountNonResonantDs();
	task->SetUseWeight(kFALSE);
	task->SetCFManager(man); //here is set the CF manager
	task->SetSign(isSign);
	task->SetCentralitySelection(kFALSE);
	task->SetFakeSelection(0);
	task->SetRejectCandidateIfNotFromQuark(kTRUE); // put to false if you want to keep HIJING D0!!
	task->SetUseMCVertex(kFALSE); // put to true if you want to do studies on pp
	if (isKeepDfromB && !isKeepDfromBOnly) task->SetDselection(2);
	if (isKeepDfromB && isKeepDfromBOnly) task->SetDselection(1);		

	TF1* funcWeight = 0x0;
	if (task->GetUseWeight()) {
		funcWeight = (TF1*)fileCuts->Get("funcWeight");
		if (funcWeight == 0x0){
			Printf("FONLL Weights will be used");
		}
		else {
			task->SetWeightFunction(funcWeight);
			Printf("User-defined Weights will be used. The function being:");
			task->GetWeightFunction()->Print();
		}
	}
    
    if(useNtrkWeight){
        TH1F *hNtrkMC;
        TH1F *hNtrkMeasured;
        hNtrkMC = (TH1F*)fileCuts->Get("hNtrkMC");
        hNtrkMeasured = (TH1F*)fileCuts->Get("hNtrkMeasured");
        if(hNtrkMC) task->SetMCNchHisto(hNtrkMC);
        else {
            Printf("FATAL: Histogram for multiplicity weights not found");
            return 0x0;
        }
        if(hNtrkMeasured) task->SetMeasuredNchHisto(hNtrkMeasured);
        else {
            Printf("FATAL: Histogram for multiplicity weights not found");
            return 0x0;
        }
        task->SetUseNchTrackletsWeight(kTRUE);
    }
    
	Printf("***************** CONTAINER SETTINGS *****************");
	Printf("decay channel = %d",(Int_t)task->GetDecayChannel());
	Printf("FillFromGenerated = %d",(Int_t)task->GetFillFromGenerated());
	Printf("Dselection = %d",(Int_t)task->GetDselection());
	Printf("UseWeight = %d",(Int_t)task->GetUseWeight());
	if (task->GetUseWeight()) {
		funcWeight = (TF1*)fileCuts->Get("funcWeight");
		if (funcWeight == 0x0){
			Printf("FONLL Weights will be used");
		}
		else {
			task->SetWeightFunction(funcWeight);
			Printf("User-defined Weights will be used. The function being:");
			task->GetWeightFunction()->Print();
		}
	}
	Printf("Sign = %d",(Int_t)task->GetSign());
	Printf("Centrality selection = %d",(Int_t)task->GetCentralitySelection());
	Printf("Fake selection = %d",(Int_t)task->GetFakeSelection());
	Printf("RejectCandidateIfNotFromQuark selection = %d",(Int_t)task->GetRejectCandidateIfNotFromQuark());
	Printf("UseMCVertex selection = %d",(Int_t)task->GetUseMCVertex());
	Printf("***************END CONTAINER SETTINGS *****************\n");

        //-----------------------------------------------------------//
        //   create correlation matrix for unfolding - only eta-pt   //
        //-----------------------------------------------------------//

        Bool_t AcceptanceUnf = kTRUE; // unfold at acceptance level, otherwise PPR

        Int_t thnDim[4];
        
        //first half  : reconstructed 
        //second half : MC

        thnDim[0] = iBin[ipT];
        thnDim[2] = iBin[ipT];
        thnDim[1] = iBin[iy];
        thnDim[3] = iBin[iy];

	TString nameCorr="";
	if(!isKeepDfromB) {
		nameCorr="CFHFcorr0_3ProngDstoKKpi_CommonFramework";
	}
	else  if(isKeepDfromBOnly){
		nameCorr= "CFHFcorr0KeepDfromBOnly_3ProngDstoKKpi_CommonFramework";
	}
	else  {
		nameCorr="CFHFcorr0allD_3ProngDstoKKpi_CommonFramework";		
	}
	nameCorr+=suffixDecayType.Data();
	nameCorr+=suffixName.Data();

        THnSparseD* correlation = new THnSparseD(nameCorr,"THnSparse with correlations",4,thnDim);
        Double_t** binEdges = new Double_t*[2];

        // set bin limits

        binEdges[0]= binLimpT;
        binEdges[1]= binLimy;

        correlation->SetBinEdges(0,binEdges[0]);
        correlation->SetBinEdges(2,binEdges[0]);

        correlation->SetBinEdges(1,binEdges[1]);
        correlation->SetBinEdges(3,binEdges[1]);

        correlation->Sumw2();
  
        // correlation matrix ready
        //------------------------------------------------//

        task->SetCorrelationMatrix(correlation); // correlation matrix for unfolding
	
	// Create and connect containers for input/output
	
	// ------ input data ------
	AliAnalysisDataContainer *cinput0  = mgr->GetCommonInputContainer();
	
	// ----- output data -----
	
	TString outputfile = AliAnalysisManager::GetCommonFileName();
	TString output1name="", output2name="", output3name="", output4name="", output5name="";
	output2name=nameContainer;
	output3name=nameCorr;
	output5name= "coutProfDs";
	if(!isKeepDfromB) {
		outputfile += ":PWG3_D2H_CFtaskDstoKKpi_CommonFramework";
		output1name="CFHFchist0_3ProngDstoKKpi_CommonFramework";
		output3name+="_cOnly";
		output4name= "CutsDdirect_3ProngDs_CommonFramework";
		output5name+="_cOnly";
	}
	else  if(isKeepDfromBOnly){
		outputfile += ":PWG3_D2H_CFtaskDstoKKpiKeepDfromBOnly_CommonFramework";
		output1name="CFHFchist0DfromB_3ProngDstoKKpi_CommonFramework";
		output3name+="_bOnly";
		output4name= "CutsDfromB_3ProngDs_CommonFramework";
		output5name+="_bOnly";
	}
	else{
		outputfile += ":PWG3_D2H_CFtaskDstoKKpiKeepDfromB_CommonFramework";
		output1name="CFHFchist0allD_3ProngDstoKKpi_CommonFramework";
		output3name+="_all";
		output4name= "CutsallD_3ProngDs_CommonFramework";
		output5name+="_all";
	}

	outputfile += suffixDecayType.Data();
	output1name+= suffixDecayType.Data();
	output4name+= suffixDecayType.Data();
	output5name+= suffixDecayType.Data();
	
	outputfile += suffixName.Data();
	output1name+= suffixName.Data();
	output4name+= suffixName.Data();
	output5name+= suffixName.Data();

	//now comes user's output objects :
	// output TH1I for event counting
	AliAnalysisDataContainer *coutput1 = mgr->CreateContainer(output1name, TH1I::Class(),AliAnalysisManager::kOutputContainer,outputfile.Data());
	// output Correction Framework Container (for acceptance & efficiency calculations)
	AliAnalysisDataContainer *coutput2 = mgr->CreateContainer(output2name, AliCFContainer::Class(),AliAnalysisManager::kOutputContainer,outputfile.Data());
	// Unfolding - correlation matrix
        AliAnalysisDataContainer *coutput3 = mgr->CreateContainer(output3name, THnSparseD::Class(),AliAnalysisManager::kOutputContainer,outputfile.Data());
	AliAnalysisDataContainer *coutput4 = mgr->CreateContainer(output4name, AliRDHFCuts::Class(),AliAnalysisManager::kOutputContainer, outputfile.Data());
	// estimators list
	AliAnalysisDataContainer *coutput5 = mgr->CreateContainer(output5name, TList::Class(),AliAnalysisManager::kOutputContainer, outputfile.Data());

	mgr->AddTask(task);
	
	mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
	mgr->ConnectOutput(task,1,coutput1);
	mgr->ConnectOutput(task,2,coutput2);
        mgr->ConnectOutput(task,3,coutput3);
	mgr->ConnectOutput(task,4,coutput4);
	mgr->ConnectOutput(task,5,coutput5);

	return task;
}
