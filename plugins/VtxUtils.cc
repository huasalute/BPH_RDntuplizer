#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/Candidate/interface/Candidate.h"

// Needed for Transient Tracks
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"

// Needed for the kinematic fit
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"

#include <iostream>

#include "VtxUtils.hh"

using namespace std;

#define _MuMass_ 0.1056583745
#define _MuMassErr_ 0.0000000024
#define _KMass_ 0.493677
#define _KMassErr_ 0.000013
#define _PiMass_ 0.13957018
#define _PiMassErr_ 0.00000035
#define _KstMass_ 0.89166
#define _KstMassErr_ 0.00011
#define _D0Mass_ 1.86484
#define _D0MassErr_ 0.00017
#define _DstMass_ 2.01027
#define _DstMassErr_ 0.00017
#define _JpsiMass_ 3.09691
#define _JpsiMassErr_ 0.00001
#define _B0Mass_ 5.27955
#define _B0MassErr_ 0.00026

RefCountedKinematicTree vtxu::FitD0(const edm::EventSetup& iSetup, pat::PackedCandidate pi, pat::PackedCandidate K, bool mass_constrain) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack pi_tk = TTBuilder->build(pi.bestTrack());
  reco::TransientTrack K_tk = TTBuilder->build(K.bestTrack());

  KinematicParticleFactoryFromTransientTrack pFactory;

  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mK = _KMass_, dmK = _KMassErr_;
  float mPi = _PiMass_, dmPi = _PiMassErr_;
  parts.push_back(pFactory.particle(K_tk, mK, chi, ndf, dmK));
  parts.push_back(pFactory.particle(pi_tk, mPi, chi, ndf, dmPi));

  if (!mass_constrain) {
    KinematicParticleVertexFitter VtxFitter;
    RefCountedKinematicTree D0KinTree = VtxFitter.fit(parts);
    return D0KinTree;
  }
  else {
    ParticleMass D0mass = _D0Mass_;
    MultiTrackKinematicConstraint * D0mass_c = new TwoTrackMassKinematicConstraint(D0mass);
    KinematicConstrainedVertexFitter kcVtxFitter;
    RefCountedKinematicTree D0KinTree = kcVtxFitter.fit(parts, D0mass_c);
    return D0KinTree;
  }
}


RefCountedKinematicTree vtxu::FitKst_piK(const edm::EventSetup& iSetup, pat::PackedCandidate pi, pat::PackedCandidate K, bool mass_constrain) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack pi_tk = TTBuilder->build(pi.bestTrack());
  reco::TransientTrack K_tk = TTBuilder->build(K.bestTrack());

  KinematicParticleFactoryFromTransientTrack pFactory;

  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mK = _KMass_, dmK = _KMassErr_;
  float mPi = _PiMass_, dmPi = _PiMassErr_;
  parts.push_back(pFactory.particle(K_tk, mK, chi, ndf, dmK));
  parts.push_back(pFactory.particle(pi_tk, mPi, chi, ndf, dmPi));

  if (!mass_constrain) {
    KinematicParticleVertexFitter VtxFitter;
    RefCountedKinematicTree kinTree = VtxFitter.fit(parts);
    return kinTree;
  }
  else {
    ParticleMass mass = _KstMass_;
    MultiTrackKinematicConstraint * mass_c = new TwoTrackMassKinematicConstraint(mass);
    KinematicConstrainedVertexFitter kcVtxFitter;
    RefCountedKinematicTree kinTree = kcVtxFitter.fit(parts, mass_c);
    return kinTree;
  }
}


RefCountedKinematicTree vtxu::FitDst_fitD0wMassConstraint(const edm::EventSetup& iSetup, pat::PackedCandidate pisoft, pat::PackedCandidate pi, pat::PackedCandidate K, bool mass_constrain, int verbose = 0) {
  // Get the mass constrained D0
  auto D0KinTree = vtxu::FitD0(iSetup, pi, K, true);
  if(!D0KinTree->isValid()) return D0KinTree;
  D0KinTree->movePointerToTheTop();
  auto D0_reco = D0KinTree->currentParticle();

  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack pisoft_tk = TTBuilder->build(pisoft.bestTrack());

  KinematicParticleFactoryFromTransientTrack pFactory;

  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mPi = _PiMass_, dmPi = _PiMassErr_;
  parts.push_back(D0_reco);
  parts.push_back(pFactory.particle(pisoft_tk, mPi, chi, ndf, dmPi));

  if (!mass_constrain) {
    KinematicParticleVertexFitter VtxFitter;
    RefCountedKinematicTree DstKinTree = VtxFitter.fit(parts);
    return DstKinTree;
  }
  else {
    ParticleMass Dstmass = _DstMass_;
    MultiTrackKinematicConstraint * Dstmass_c = new TwoTrackMassKinematicConstraint(Dstmass);
    KinematicConstrainedVertexFitter kcVtxFitter;
    RefCountedKinematicTree DstKinTree = kcVtxFitter.fit(parts, Dstmass_c);
    return DstKinTree;
  }
}

RefCountedKinematicTree vtxu::FitJpsi_mumu(const edm::EventSetup& iSetup, pat::Muon m1, pat::Muon m2, bool mass_constrain) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack m1_tk = TTBuilder->build(m1.muonBestTrack());
  reco::TransientTrack m2_tk = TTBuilder->build(m2.muonBestTrack());

  std::vector<RefCountedKinematicParticle> parts;
  KinematicParticleFactoryFromTransientTrack pFactory;
  double chi = 0, ndf = 0;
  float mMu = _MuMass_, dmMu = _MuMassErr_;
  parts.push_back(pFactory.particle(m1_tk, mMu, chi, ndf, dmMu));
  parts.push_back(pFactory.particle(m2_tk, mMu, chi, ndf, dmMu));

  if (!mass_constrain) {
    KinematicParticleVertexFitter VtxFitter;
    RefCountedKinematicTree KinTree = VtxFitter.fit(parts);
    return KinTree;
  }
  else {
    ParticleMass mass = _JpsiMass_;
    MultiTrackKinematicConstraint * mass_c = new TwoTrackMassKinematicConstraint(mass);
    KinematicConstrainedVertexFitter kcVtxFitter;
    RefCountedKinematicTree KinTree = kcVtxFitter.fit(parts, mass_c);
    return KinTree;
  }
}

RefCountedKinematicTree vtxu::FitDst(const edm::EventSetup& iSetup, pat::PackedCandidate pisoft, const RefCountedKinematicParticle D0, bool mass_constrain) {
// Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack pisoft_tk = TTBuilder->build(pisoft.bestTrack());
  reco::TransientTrack D0_tk = D0->refittedTransientTrack();

  KinematicParticleFactoryFromTransientTrack pFactory;

  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mPi = _PiMass_, dmPi = _PiMassErr_;
  parts.push_back(pFactory.particle(pisoft_tk, mPi, chi, ndf, dmPi));
  float mD0 = D0->currentState().mass();
  float dmD0 = sqrt(D0->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(D0_tk, mD0, chi, ndf, dmD0));
  if (!mass_constrain) {
    KinematicParticleVertexFitter VtxFitter;
    cout << parts.size() << endl;
    RefCountedKinematicTree DstKinTree = VtxFitter.fit(parts);
    return DstKinTree;
  }
  else {
    ParticleMass Dstmass = _DstMass_;
    MultiTrackKinematicConstraint * Dstmass_c = new TwoTrackMassKinematicConstraint(Dstmass);
    KinematicConstrainedVertexFitter kcVtxFitter;
    RefCountedKinematicTree DstKinTree = kcVtxFitter.fit(parts, Dstmass_c);
    return DstKinTree;
  }
}

RefCountedKinematicTree vtxu::FitVtxMuDst(const edm::EventSetup& iSetup, const RefCountedKinematicParticle Dst, pat::Muon mu) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack Mu_tk = TTBuilder->build(mu.muonBestTrack());
  reco::TransientTrack Dst_tk = Dst->refittedTransientTrack();

  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mMu = _MuMass_, dmMu = _MuMassErr_;
  parts.push_back(pFactory.particle(Mu_tk, mMu, chi, ndf, dmMu));
  float mDst = Dst->currentState().mass();
  float dmDst = sqrt(Dst->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(Dst_tk, mDst, chi, ndf, dmDst));

  KinematicParticleVertexFitter VtxFitter;
  RefCountedKinematicTree BKinTree = VtxFitter.fit(parts);
  return BKinTree;
}

RefCountedKinematicTree vtxu::FitVtxJpsiKst(const edm::EventSetup& iSetup, const RefCountedKinematicParticle Jpsi, const RefCountedKinematicParticle Kst, bool mass_constrain) {
  reco::TransientTrack Jpsi_tk = Jpsi->refittedTransientTrack();
  reco::TransientTrack Kst_tk = Kst->refittedTransientTrack();

  std::vector<RefCountedKinematicParticle> parts;// = {Jpsi, Kst};
  KinematicParticleFactoryFromTransientTrack pFactory;
  double chi = 0, ndf = 0;
  float mJpsi = Jpsi->currentState().mass();
  float dmJpsi = sqrt(Jpsi->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(Jpsi_tk, mJpsi, chi, ndf, dmJpsi));
  float mKst = Kst->currentState().mass();
  float dmKst = sqrt(Kst->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(Kst_tk, mKst, chi, ndf, dmKst));

  if (!mass_constrain) {
    KinematicParticleVertexFitter VtxFitter;
    RefCountedKinematicTree KinTree = VtxFitter.fit(parts);
    return KinTree;
  }
  else {
    ParticleMass mass = _B0Mass_;
    MultiTrackKinematicConstraint * mass_c = new TwoTrackMassKinematicConstraint(mass);
    KinematicConstrainedVertexFitter kcVtxFitter;
    RefCountedKinematicTree KinTree = kcVtxFitter.fit(parts, mass_c);
    return KinTree;
  }
}

RefCountedKinematicTree vtxu::FitVtxDstK(const edm::EventSetup& iSetup, const RefCountedKinematicParticle Dst, pat::PackedCandidate K, int verbose = 0) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack K_tk = TTBuilder->build(K.bestTrack());
  reco::TransientTrack Dst_tk = Dst->refittedTransientTrack();

  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mK = _KMass_, dmK = _KMassErr_;
  parts.push_back(pFactory.particle(K_tk, mK, chi, ndf, dmK));
  float mDst = Dst->currentState().mass();
  float dmDst = sqrt(Dst->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(Dst_tk, mDst, chi, ndf, dmDst));

  KinematicParticleVertexFitter VtxFitter;
  RefCountedKinematicTree BKinTree = VtxFitter.fit(parts);
  return BKinTree;
}

RefCountedKinematicTree vtxu::FitVtxDstPi(const edm::EventSetup& iSetup, const RefCountedKinematicParticle Dst, pat::PackedCandidate pi, int verbose = 0) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack Pi_tk = TTBuilder->build(pi.bestTrack());
  reco::TransientTrack Dst_tk = Dst->refittedTransientTrack();

  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mPi = _PiMass_, dmPi = _PiMassErr_;
  parts.push_back(pFactory.particle(Pi_tk, mPi, chi, ndf, dmPi));
  float mDst = Dst->currentState().mass();
  float dmDst = sqrt(Dst->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(Dst_tk, mDst, chi, ndf, dmDst));

  KinematicParticleVertexFitter VtxFitter;
  RefCountedKinematicTree KinTree = VtxFitter.fit(parts);
  return KinTree;
}

RefCountedKinematicTree vtxu::FitVtxMuDstPi(const edm::EventSetup& iSetup, const RefCountedKinematicParticle Dst, pat::PackedCandidate mu, pat::PackedCandidate pi, int verbose = 0) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack Dst_tk = Dst->refittedTransientTrack();
  reco::TransientTrack Mu_tk = TTBuilder->build(mu.bestTrack());
  reco::TransientTrack Pi_tk = TTBuilder->build(pi.bestTrack());

  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> parts;
  double chi = 0, ndf = 0;
  float mDst = Dst->currentState().mass();
  float dmDst = sqrt(Dst->currentState().kinematicParametersError().matrix()(6,6));
  parts.push_back(pFactory.particle(Dst_tk, mDst, chi, ndf, dmDst));
  float mMu = _MuMass_, dmMu = _MuMassErr_;
  parts.push_back(pFactory.particle(Mu_tk, mMu, chi, ndf, dmMu));
  float mPi = _PiMass_, dmPi = _PiMassErr_;
  parts.push_back(pFactory.particle(Pi_tk, mPi, chi, ndf, dmPi));

  KinematicParticleVertexFitter VtxFitter;
  RefCountedKinematicTree BKinTree = VtxFitter.fit(parts);
  return BKinTree;
}


pair<double,double> vtxu::computeDCA(const edm::EventSetup& iSetup, pat::PackedCandidate cand, GlobalPoint p) {
  // Get transient track builder
  edm::ESHandle<TransientTrackBuilder> TTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",TTBuilder);

  reco::TransientTrack tk = TTBuilder->build(cand.bestTrack());

  TrajectoryStateClosestToPoint stateCA = tk.trajectoryStateClosestToPoint(p);
  double dT = stateCA.perigeeParameters().transverseImpactParameter();
  double dL = stateCA.perigeeParameters().longitudinalImpactParameter();
  double dCA = hypot(dL, dT);

  double EdT = stateCA.perigeeError().transverseImpactParameterError();
  double EdL = stateCA.perigeeError().longitudinalImpactParameterError();
  double EdCA = hypot(dT*EdT, dL*EdL)/dCA;
  return make_pair(dCA,EdCA);
}

pair<double,double> vtxu::computeDCA(reco::TransientTrack tk, GlobalPoint p) {
  TrajectoryStateClosestToPoint stateCA = tk.trajectoryStateClosestToPoint(p);
  double dT = stateCA.perigeeParameters().transverseImpactParameter();
  double dL = stateCA.perigeeParameters().longitudinalImpactParameter();
  double dCA = hypot(dL, dT);

  double EdT = stateCA.perigeeError().transverseImpactParameterError();
  double EdL = stateCA.perigeeError().longitudinalImpactParameterError();
  double EdCA = hypot(dT*EdT, dL*EdL)/dCA;
  return make_pair(dCA,EdCA);
}

TLorentzVector vtxu::getTLVfromKinPart(const RefCountedKinematicParticle p) {
  auto pvec = p->currentState().globalMomentum();
  auto mass = p->currentState().mass();
  TLorentzVector out;
  out.SetXYZM(pvec.x(), pvec.y(), pvec.z(), mass);
  return out;
}


TLorentzVector vtxu::getTLVfromTrack(reco::Track t, double mass) {
  TLorentzVector out;
  auto p3 = t.momentum();
  auto pt = sqrt(p3.perp2());
  out.SetPtEtaPhiM(pt, p3.eta(), p3.phi(), mass);
  return out;
}

TLorentzVector vtxu::getTLVfromCand(pat::PackedCandidate p, double mass) {
  TLorentzVector out;
  out.SetPtEtaPhiM(p.pt(), p.eta(), p.phi(), mass);
  return out;
}

double vtxu::dPhi(double p1, double p2) {
  double dPhi = p1 - p2;
  double pi = 3.14159265358979323846;
  while (fabs(dPhi) > pi) {
    int sgn = dPhi > 0? 1 : -1;
    dPhi -= sgn*2*pi;
  }
  return dPhi;
}

double vtxu::dR(double p1, double p2, double e1, double e2) {
  return hypot(dPhi(p1, p2), e1 - e2);
}

std::pair<double,double> vtxu::vtxsDistance(reco::VertexRef v1, RefCountedKinematicVertex v2){
  double dx = v2->position().x() - v1->x();
  double dy = v2->position().y() - v1->y();
  double dz = v2->position().z() - v1->z();

  double d = sqrt(dx*dx + dy*dy + dz*dz);

  double dd[3] = {dx/d, dy/d, dz/d};
  auto e1 = v1->covariance();
  auto e2 = v2->error().matrix();

  double Ed2 = 0;
  for(uint i=0; i<3; ++i){
    for(uint j=0; j<3; ++j){
      Ed2 += dd[i]* ( e1.At(i,j)+e2.At(i,j) ) *dd[j];
    }
  }

  return make_pair(d,sqrt(Ed2));
}

std::pair<double,double> vtxu::vtxsDistance(reco::Vertex v1, RefCountedKinematicVertex v2){
  double dx = v2->position().x() - v1.x();
  double dy = v2->position().y() - v1.y();
  double dz = v2->position().z() - v1.z();

  double d = sqrt(dx*dx + dy*dy + dz*dz);

  double dd[3] = {dx/d, dy/d, dz/d};
  auto e1 = v1.covariance();
  auto e2 = v2->error().matrix();

  double Ed2 = 0;
  for(uint i=0; i<3; ++i){
    for(uint j=0; j<3; ++j){
      Ed2 += dd[i]* ( e1.At(i,j)+e2.At(i,j) ) *dd[j];
    }
  }

  return make_pair(d,sqrt(Ed2));
}

double vtxu::computePointingCos(reco::Vertex vtxP, const RefCountedKinematicVertex vtxKinPartDecay, const RefCountedKinematicParticle p) {
  TVector3 dvtx(vtxKinPartDecay->position().x() - vtxP.position().x(),
                vtxKinPartDecay->position().y() - vtxP.position().y(),
                vtxKinPartDecay->position().z() - vtxP.position().z()
               );

  TVector3 p3(p->currentState().globalMomentum().x(),
              p->currentState().globalMomentum().y(),
              p->currentState().globalMomentum().z()
            );

  double dalpha = dvtx.Angle(p3);
  return cos(dalpha);
}

double vtxu::computeIP(reco::Candidate::Point axis, reco::Candidate::Point trajPoint, reco::Candidate::Vector trajVector, bool linearApprox){
  if(linearApprox) {
    double nx = trajVector.x() / hypot(trajVector.x(), trajVector.y());
    double ny = trajVector.y() / hypot(trajVector.x(), trajVector.y());

    double amp_x = axis.x() - trajPoint.x();
    double amp_y = axis.y() - trajPoint.y();
    double dx = amp_x - nx * (amp_x * nx + amp_y * ny);
    double dy = amp_y - ny * (amp_x * nx + amp_y * ny);

    return hypot(dx, dy);
  }
  else {
    cout << "Not yet implemented" << endl;
    return -1;
  }
}
