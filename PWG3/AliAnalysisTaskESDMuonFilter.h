#ifndef ALIANALYSISTASKESDMUONFILTER_H
#define ALIANALYSISTASKESDMUONFILTER_H
 
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id: AliAnalysisTaskESDMuonFilter.h 24429 2008-03-12 10:27:50Z jgrosseo $ */

#include <TList.h> 
/*#include "/n60raid3/alice/roberta/Trunk/AliRoot/ANALYSIS/AliAnalysisTaskSE.h"*/
#include "AliAnalysisTaskSE.h"

class AliAnalysisFilter;

class AliAnalysisTaskESDMuonFilter : public AliAnalysisTaskSE
{
 public:
    AliAnalysisTaskESDMuonFilter();
    AliAnalysisTaskESDMuonFilter(const char* name);
    virtual ~AliAnalysisTaskESDMuonFilter() {;}
    // Implementation of interface methods
    virtual void UserCreateOutputObjects();
    virtual void Init();
    virtual void LocalInit() {Init();}
    virtual void UserExec(Option_t *option);
    virtual void Terminate(Option_t *option);

    virtual void ConvertESDtoAOD();

    // Setters
    virtual void SetTrackFilter(AliAnalysisFilter* trackF) {fTrackFilter = trackF;}
    virtual void SetKinkFilter (AliAnalysisFilter*  KinkF) {fKinkFilter  =  KinkF;}
    virtual void SetV0Filter   (AliAnalysisFilter*    V0F) {fV0Filter    =    V0F;}

 private:
    AliAnalysisTaskESDMuonFilter(const AliAnalysisTaskESDMuonFilter&);
    AliAnalysisTaskESDMuonFilter& operator=(const AliAnalysisTaskESDMuonFilter&);
    AliAnalysisFilter* fTrackFilter; //  Track Filter
    AliAnalysisFilter* fKinkFilter;  //  Kink  Filter
    AliAnalysisFilter* fV0Filter;    //  V0    Filter    
    ClassDef(AliAnalysisTaskESDMuonFilter, 1); // Analysis task for standard ESD filtering
};
 
#endif
