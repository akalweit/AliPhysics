/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

//-------------------------------------------------------------------------
// Jet kinematics reader 
// MC reader for jet analysis
// Author: Andreas Morsch (andreas.morsch@cern.ch)
//-------------------------------------------------------------------------

// From root ...
#include <TClonesArray.h>
#include <TPDGCode.h>
#include <TParticle.h>
#include <TParticlePDG.h>
#include <TLorentzVector.h>
#include <TSystem.h>
#include <TDatabasePDG.h>
#include <TRandom.h>
// From AliRoot ...
#include "AliAODJet.h"
#include "AliJetKineReaderHeader.h"
#include "AliJetKineReader.h"
#include "AliMCEventHandler.h"
#include "AliMCEvent.h"
#include "AliStack.h"
#include "AliHeader.h"
#include "AliGenPythiaEventHeader.h"
#include "AliLog.h"

ClassImp(AliJetKineReader)

AliJetKineReader::AliJetKineReader():
    AliJetReader(),  
    fAliHeader(0),
    fMCEvent(0),
    fGenJets(0)
{
  // Default constructor
}

//____________________________________________________________________________

AliJetKineReader::~AliJetKineReader()
{
  // Destructor
  delete fAliHeader;
}

//____________________________________________________________________________

void AliJetKineReader::OpenInputFiles()
{
  // Opens the input file using the run loader
  // Not used anymore   
}

//____________________________________________________________________________

Bool_t AliJetKineReader::FillMomentumArray()
{
  // Fill momentum array for event
    Int_t goodTrack = 0;
    // Clear array
    // temporary storage of signal and cut flags
    Int_t* sflag  = new Int_t[50000];
    Int_t* cflag  = new Int_t[50000];
  
    ClearArray();
    // Get the stack
    AliStack* stack = fMCEvent->Stack();
    // Number of primaries
    Int_t nt = stack->GetNprimary();
      
    // Get cuts set by user and header
    Double_t ptMin = ((AliJetKineReaderHeader*) fReaderHeader)->GetPtCut();
    Float_t etaMin = fReaderHeader->GetFiducialEtaMin();
    Float_t etaMax = fReaderHeader->GetFiducialEtaMax();  
    fAliHeader = fMCEvent->Header();
      
    
    TLorentzVector p4;
    // Loop over particles
    for (Int_t it = 0; it < nt; it++) {
	TParticle *part = stack->Particle(it);
	Int_t   status  = part->GetStatusCode();
	Int_t   pdg     = TMath::Abs(part->GetPdgCode());
	Float_t pt      = part->Pt(); 
	  
	// Skip non-final state particles, neutrinos 
	// and particles with pt < pt_min 
	if ((status != 1)            
	    || (pdg == 12 || pdg == 14 || pdg == 16)) continue; 
	
	Float_t p       = part->P();
	Float_t p0      = p;
	Float_t eta     = part->Eta();
	Float_t phi     = part->Phi();
	
	// Fast simulation of TPC if requested
	if (((AliJetKineReaderHeader*)fReaderHeader)->FastSimTPC()) {
	    // Charged particles only
	    Float_t charge = 
		TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
	    if (charge == 0)               continue;
	    // Simulate efficiency
	    if (!Efficiency(p0, eta, phi)) continue;
	    // Simulate resolution
	    p = SmearMomentum(4, p0);
	} // End fast TPC
	  
	  // Fast simulation of EMCAL if requested
	if (((AliJetKineReaderHeader*)fReaderHeader)->FastSimEMCAL()) {
	    Float_t charge = 
		TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
	    // Charged particles only
	    if (charge != 0){
		// Simulate efficiency
		if (!Efficiency(p0, eta, phi)) continue;
		// Simulate resolution
		p = SmearMomentum(4, p0);
	    } // end "if" charged particles
	      // Neutral particles (exclude K0L, n, nbar)
	    if (pdg == kNeutron || pdg == kK0Long) continue;
	} // End fast EMCAL
	
	  // Fill momentum array
	Float_t r  = p/p0;
	Float_t px = r * part->Px(); 
	Float_t py = r * part->Py(); 
	Float_t pz = r * part->Pz();
	Float_t m  = part->GetMass();
	Float_t e  = TMath::Sqrt(px * px + py * py + pz * pz + m * m);
	p4 = TLorentzVector(px, py, pz, e);
	if ( (p4.Eta()>etaMax) || (p4.Eta()<etaMin)) continue;
	new ((*fMomentumArray)[goodTrack]) TLorentzVector(p4);
	
	// all tracks are signal ... ?
	sflag[goodTrack]=1;
	cflag[goodTrack]=0;
	if (pt > ptMin) cflag[goodTrack] = 1; // track surviving pt cut
	goodTrack++;
    } // track loop

// set the signal flags
    fSignalFlag.Set(goodTrack, sflag);
    fCutFlag.Set(goodTrack, cflag);
    AliInfo(Form(" Number of good tracks %d \n", goodTrack));
    
    delete[] sflag;
    delete[] cflag;
    
    return kTRUE;
}


Float_t AliJetKineReader::SmearMomentum(Int_t ind, Float_t p)
{
  //  The relative momentum error, i.e. 
  //  (delta p)/p = sqrt (a**2 + (b*p)**2) * 10**-2,
  //  where typically a = 0.75 and b = 0.16 - 0.24 depending on multiplicity
  //  (the lower value is for dn/d(eta) about 2000, and the higher one for 8000)
  //
  //  If we include information from TRD, b will be a factor 2/3 smaller.
  //
  //  ind = 1: high multiplicity
  //  ind = 2: low  multiplicity
  //  ind = 3: high multiplicity + TRD
  //  ind = 4: low  multiplicity + TRD
  
  Float_t pSmeared;
  Float_t a = 0.75;
  Float_t b = 0.12;
  
  if (ind == 1) b = 0.12;
  if (ind == 2) b = 0.08;
  if (ind == 3) b = 0.12;    
  if (ind == 4) b = 0.08;    
  
  Float_t sigma =  p * TMath::Sqrt(a * a + b * b * p * p)*0.01;
  pSmeared = p + gRandom->Gaus(0., sigma);
  return pSmeared;
}


Bool_t AliJetKineReader::Efficiency(Float_t p, Float_t /*eta*/, Float_t phi)
{
  // Fast simulation of geometrical acceptance and tracking efficiency
 
  //  Tracking
  Float_t eff = 0.99;
  if (p < 0.5) eff -= (0.5-p)*0.2/0.3;
  // Geometry
  if (p > 0.5) {
    phi *= 180. / TMath::Pi();
    // Sector number 0 - 17
    Int_t isec  = Int_t(phi / 20.);
    // Sector centre
    Float_t phi0 = isec * 20. + 10.;
    Float_t phir = TMath::Abs(phi-phi0);
    // 2 deg of dead space
    if (phir > 9.) eff = 0.;
  }

  if (gRandom->Rndm() > eff) {
    return kFALSE;
  } else {
    return kTRUE;
  }

}

TClonesArray*  AliJetKineReader::GetGeneratedJets()
{
    //
    // Get generated jets from mc header
    //
    if (!fGenJets) {
	fGenJets = new TClonesArray("AliAODJets", 100);
    } else {
	fGenJets->Clear();
    }
    
    
    AliHeader* alih = GetAliHeader(); 
    if (alih == 0) return 0;
    AliGenEventHeader * genh = alih->GenEventHeader();
    if (genh == 0) return 0;

    Int_t nj =((AliGenPythiaEventHeader*)genh)->NTriggerJets(); 
    for (Int_t i = 0; i < nj; i++) {
	Float_t p[4];
	((AliGenPythiaEventHeader*)genh)->TriggerJet(i,p);
	new ((*fGenJets)[i]) AliAODJet(p[0], p[1], p[2], p[3]);
    }

    return fGenJets;
}

void AliJetKineReader::SetInputEvent(TObject* /*esd*/, TObject* /*aod*/, TObject* mc)
{
    // Connect the input event
    fMCEvent = (AliMCEvent*) mc;
}
