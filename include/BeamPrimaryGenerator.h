#include <math.h>

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"

#include "CLHEP/Random/RandGauss.h"

#include "PrimaryGenerator.h"
#include "UserPrimaryParticleInformation.h"

namespace hpssim {

class BeamPrimaryGenerator : public PrimaryGenerator {

    public:

        BeamPrimaryGenerator(std::string name) : PrimaryGenerator(name) {
            CLHEP::RandGauss::setTheEngine(G4Random::getTheEngine());
        }

        void GeneratePrimaryVertex(G4Event* anEvent) {
            if (verbose_ > 1) {
                std::cout << "BeamPrimaryGenerator: Generating " << nelectrons_
                        << " electrons in event " << anEvent->GetEventID() << std::endl;
            }
            for (int i = 0; i < nelectrons_; i++) {

                G4PrimaryVertex* vertex = new G4PrimaryVertex();
                G4ThreeVector sampledPosition;
                sampledPosition.setX(position_.x() + CLHEP::RandGauss::shoot(0, sigmaX_));
                sampledPosition.setY(position_.y() + CLHEP::RandGauss::shoot(0, sigmaY_));
                sampledPosition.setZ(position_.z());

                if (verbose_ > 2) {
                    std::cout << "BeamPrimaryGenerator: Sampled pos " << sampledPosition
                            << " for electron " << i << std::endl;
                }

                vertex->SetPosition(sampledPosition.x(), sampledPosition.y(), sampledPosition.z());
                anEvent->AddPrimaryVertex(vertex);

                G4PrimaryParticle* primaryParticle = new G4PrimaryParticle();
                static auto electronDef = G4ParticleTable::GetParticleTable()->FindParticle("e-");
                primaryParticle->SetParticleDefinition(electronDef);
                primaryParticle->SetMomentumDirection(direction_);
                primaryParticle->SetTotalEnergy(energy_);
                vertex->SetPrimary(primaryParticle);
            }
        }

        void initialize() {
            Parameters& params = getParameters();
            energy_ = params.get("energy", energy_);
            if (params.has("nelectrons")) {
                nelectrons_ = params.get("nelectrons");
                if (verbose_ > 1) {
                    std::cout << "BeamPrimaryGenerator: Number of electrons was set to " << nelectrons_ << std::endl;
                }
            } else {
                current_ = params.get("current", current_);
                computeNumberOfElectrons();
                if (verbose_ > 1) {
                    std::cout << "BeamPrimaryGenerator: Calculated number of electrons " << nelectrons_ << std::endl;
                }
            }
        }

    private:

        void computeNumberOfElectrons() {

            // electrons per second for 100 nA
            static double electronsPerSecond = 6.25 * pow(10., 11.);

            // number of bunches per second (machine parameter)
            static int nBunchesPerSecond = 500000000;

            // number of electrons per bunch
            static int electronsPerBunch = electronsPerSecond / nBunchesPerSecond;

            // convert to specified current
            nelectrons_ = electronsPerBunch * current_ / 100.;
        }

    private:

        /** Vertex position of the beam particles. */
        G4ThreeVector position_{G4ThreeVector(0, 0, -10)};

        /** Beam particle momentum in GeV. */
        G4ThreeVector direction_{G4ThreeVector(0, 0, 1.)};

        /* Beam energy in GeV. */
        double energy_{1.056 * GeV};

        /** Number of electrons to fire in one event. */
        int nelectrons_{0};

        /* Beam current in nA (may use 200 or 450 also). */
        double current_{50};

        /** Gaussian sigma of vertex X coordinate. */
        double sigmaX_{0.300};

        /** Gaussian sigma of vertex Y coordinate. */
        double sigmaY_{0.030};
};

}
