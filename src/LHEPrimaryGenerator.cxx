#include "LHEPrimaryGenerator.h"

// Geant4
#include "G4Event.hh"
#include "G4IonTable.hh"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

namespace hpssim {

LHEPrimaryGenerator::LHEPrimaryGenerator(std::string name, LHEReader* theReader) :
        PrimaryGenerator(name), reader_(theReader), lheEvent_(nullptr) {
}

LHEPrimaryGenerator::~LHEPrimaryGenerator() {
    if (reader_) {
        delete reader_;
    }
}

void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

    G4PrimaryVertex* vertex = new G4PrimaryVertex();
    vertex->SetPosition(0, 0, 0);
    vertex->SetWeight(lheEvent_->getXWGTUP());

    std::map<LHEParticle*, G4PrimaryParticle*> particleMap;

    int particleIndex = 0;
    const std::vector<LHEParticle*>& particles = lheEvent_->getParticles();
    for (std::vector<LHEParticle*>::const_iterator it = particles.begin(); it != particles.end(); it++) {

        LHEParticle* particle = (*it);

        if (particle->getISTUP() > 0) {

            G4PrimaryParticle* primary = new G4PrimaryParticle();
            if (particle->getIDUP() == -623) { /* Tungsten ion */
                G4ParticleDefinition* tungstenIonDef = G4IonTable::GetIonTable()->GetIon(74, 184, 0.);
                if (tungstenIonDef != NULL) {
                    primary->SetParticleDefinition(tungstenIonDef);
                } else {
                    G4Exception("LHEPrimaryGenerator::GeneratePrimaryVertex", "EventGenerationError", FatalException,
                            "Failed to find particle definition for W ion.");
                }
            } else {
                primary->SetPDGcode(particle->getIDUP());
            }

            primary->Set4Momentum(particle->getPUP(0) * GeV, particle->getPUP(1) * GeV, particle->getPUP(2) * GeV,
                    particle->getPUP(3) * GeV);
            primary->SetProperTime(particle->getVTIMUP() * nanosecond);

            //UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
            //primaryInfo->setHepEvtStatus(particle->getISTUP());
            //primary->SetUserInformation(primaryInfo);

            particleMap[particle] = primary;

            /*
             * Assign primary as daughter but only if the mother is not a DOC particle.
             */
            if (particle->getMother(0) != NULL && particle->getMother(0)->getISTUP() > 0) {
                G4PrimaryParticle* primaryMom = particleMap[particle->getMother(0)];
                if (primaryMom != NULL) {
                    primaryMom->SetDaughter(primary);
                }
            } else {
                vertex->SetPrimary(primary);
            }

            //primary->Print();

        } else {
            // TODO: Handle doc particles!
        }

        //std::cout << std::endl;

        ++particleIndex;
    }

    anEvent->AddPrimaryVertex(vertex);
}

}
