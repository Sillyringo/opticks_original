/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

//
//
/**
 * \class DsG4OpRayleigh
 *
 * \brief A slightly modified version of G4OpRayleigh
 *
 * It is modified to make the Rayleigh Scattering happen with different waters defined in /dd/Material/
 *
 * This was taken from G4.9.1p1
 *
 * zhangh@bnl.gov on 8th, July
 */
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4OpRayleigh.cc,v 1.17 2008/10/24 19:51:12 gum Exp $
// GEANT4 tag $Name: geant4-09-02 $
//
// 
////////////////////////////////////////////////////////////////////////
// Optical Photon Rayleigh Scattering Class Implementation
////////////////////////////////////////////////////////////////////////
//
// File:        DsG4OpRayleigh.cc 
// Description: Discrete Process -- Rayleigh scattering of optical 
//		photons  
// Version:     1.0
// Created:     1996-05-31  
// Author:      Juliet Armstrong
// Updated:     2005-07-28 - add G4ProcessType to constructor
//              2001-10-18 by Peter Gumplinger
//              eliminate unused variable warning on Linux (gcc-2.95.2)
//              2001-09-18 by mma
//		>numOfMaterials=G4Material::GetNumberOfMaterials() in BuildPhy
//              2001-01-30 by Peter Gumplinger
//              > allow for positiv and negative CosTheta and force the
//              > new momentum direction to be in the same plane as the
//              > new and old polarization vectors
//              2001-01-29 by Peter Gumplinger
//              > fix calculation of SinTheta (from CosTheta)
//              1997-04-09 by Peter Gumplinger
//              > new physics/tracking scheme
// mail:        gum@triumf.ca
//
////////////////////////////////////////////////////////////////////////

#include "G4ios.hh"
#include "G4OpProcessSubType.hh"
#include "G4Version.hh"

#include "DsG4OpRayleigh.h"
#include "PLOG.hh"


using CLHEP::pi ; 
using CLHEP::twopi ; 
using CLHEP::m3 ; 
using CLHEP::MeV ; 
using CLHEP::kelvin ; 
using CLHEP::h_Planck ;
using CLHEP::c_light ;



/////////////////////////
// Class Implementation
/////////////////////////

        //////////////
        // Operators
        //////////////

// DsG4OpRayleigh::operator=(const DsG4OpRayleigh &right)
// {
// }

        /////////////////
        // Constructors
        /////////////////

DsG4OpRayleigh::DsG4OpRayleigh(const G4String& processName, G4ProcessType type)
           : G4VDiscreteProcess(processName, type)
{
        SetProcessSubType(fOpRayleigh);

        thePhysicsTable = 0;

        DefaultWater = false;

        if (verboseLevel>0) {
           G4cout << GetProcessName() << " is created " << G4endl;
        }

        BuildThePhysicsTable();
}

// DsG4OpRayleigh::DsG4OpRayleigh(const DsG4OpRayleigh &right)
// {
// }

        ////////////////
        // Destructors
        ////////////////

DsG4OpRayleigh::~DsG4OpRayleigh()
{
        if (thePhysicsTable!= 0) {
           thePhysicsTable->clearAndDestroy();
           delete thePhysicsTable;
        }
}

        ////////////
        // Methods
        ////////////

// PostStepDoIt
// -------------
//
G4VParticleChange* 
DsG4OpRayleigh::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep)
{
    aParticleChange.Initialize(aTrack);

    const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();

    if (verboseLevel>0) 
    {
        G4cout << "Scattering Photon!" << G4endl;
        G4cout << "Old Momentum Direction: " << aParticle->GetMomentumDirection() << G4endl;
        G4cout << "Old Polarization: " << aParticle->GetPolarization() << G4endl;
    }

    // find polar angle w.r.t. old polarization vector

    //LOG(info) << "DsG4OpRayleigh::PostStepDoIt" ; 

    G4double rand = G4UniformRand();

    G4double CosTheta = std::pow(rand, 1./3.);
    G4double SinTheta = std::sqrt(1.-CosTheta*CosTheta);

    if(G4UniformRand() < 0.5) CosTheta = -CosTheta;

    // find azimuthal angle w.r.t old polarization vector 

    rand = G4UniformRand();

    G4double Phi = twopi*rand;
    G4double SinPhi = std::sin(Phi); 
    G4double CosPhi = std::cos(Phi); 
   
    G4double unit_x = SinTheta * CosPhi; 
    G4double unit_y = SinTheta * SinPhi;  
    G4double unit_z = CosTheta; 
   
    G4ThreeVector NewPolarization (unit_x,unit_y,unit_z);

    // Rotate new polarization direction into global reference system 

    G4ThreeVector OldPolarization = aParticle->GetPolarization();
    OldPolarization = OldPolarization.unit();

    NewPolarization.rotateUz(OldPolarization);
    NewPolarization = NewPolarization.unit();
   
    // -- new momentum direction is normal to the new
    // polarization vector and in the same plane as the
    // old and new polarization vectors --

    G4ThreeVector NewMomentumDirection = OldPolarization - NewPolarization * CosTheta;

    if(G4UniformRand() < 0.5) NewMomentumDirection = -NewMomentumDirection;
    NewMomentumDirection = NewMomentumDirection.unit();




    aParticleChange.ProposePolarization(NewPolarization);
    aParticleChange.ProposeMomentumDirection(NewMomentumDirection);

    if (verboseLevel>0) 
    {
        G4cout << "New Polarization: " << NewPolarization << G4endl;
        G4cout << "Polarization Change: " << *(aParticleChange.GetPolarization()) << G4endl;  
        G4cout << "New Momentum Direction: " << NewMomentumDirection << G4endl;
        G4cout << "Momentum Change: " << *(aParticleChange.GetMomentumDirection()) << G4endl; 
    }

    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
}

// BuildThePhysicsTable for the Rayleigh Scattering process
// --------------------------------------------------------
//
void DsG4OpRayleigh::BuildThePhysicsTable()
{
//      Builds a table of scattering lengths for each material

        if (thePhysicsTable) return;

        const G4MaterialTable* theMaterialTable=
                               G4Material::GetMaterialTable();
        G4int numOfMaterials = G4Material::GetNumberOfMaterials();

        // create a new physics table

        thePhysicsTable = new G4PhysicsTable(numOfMaterials);

        // loop for materials

        for (G4int i=0 ; i < numOfMaterials; i++)
        {
            G4MaterialPropertyVector* ScatteringLengths =
                                new G4MaterialPropertyVector();

            G4MaterialPropertiesTable *aMaterialPropertiesTable =
                         (*theMaterialTable)[i]->GetMaterialPropertiesTable();
                                                                                
            if(aMaterialPropertiesTable){

              G4MaterialPropertyVector* AttenuationLengthVector =
                            aMaterialPropertiesTable->GetProperty("RAYLEIGH");

              if(!AttenuationLengthVector){

                if ((*theMaterialTable)[i]->GetName() == "/dd/Materials/Water" || 
		    (*theMaterialTable)[i]->GetName() == "/dd/Materials/OwsWater" ||
		    (*theMaterialTable)[i]->GetName() == "/dd/Materials/IwsWater"
		    )
                {
		   // Call utility routine to Generate
		   // Rayleigh Scattering Lengths

                   DefaultWater = true;

		   ScatteringLengths =
		   RayleighAttenuationLengthGenerator(aMaterialPropertiesTable);
                }
              }
	    }

	    thePhysicsTable->insertAt(i,ScatteringLengths);
        } 
}

// GetMeanFreePath()
// -----------------
//
G4double DsG4OpRayleigh::GetMeanFreePath(const G4Track& aTrack,
                                     G4double ,
                                     G4ForceCondition* )
{
        const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
        const G4Material* aMaterial = aTrack.GetMaterial();

        G4double thePhotonEnergy = aParticle->GetTotalEnergy();

        G4double AttenuationLength = DBL_MAX;

	if ((strcmp(aMaterial->GetName(), "/dd/Materials/Water") == 0 ||
	     strcmp(aMaterial->GetName(), "/dd/Materials/OwsWater") == 0 ||
	     strcmp(aMaterial->GetName(), "/dd/Materials/IwsWater") == 0 )
	    && DefaultWater){

           G4bool isOutRange;

           AttenuationLength =
                (*thePhysicsTable)(aMaterial->GetIndex())->
                           GetValue(thePhotonEnergy, isOutRange);
        }
        else {

           G4MaterialPropertiesTable* aMaterialPropertyTable =
                           aMaterial->GetMaterialPropertiesTable();

           if(aMaterialPropertyTable){
             G4MaterialPropertyVector* AttenuationLengthVector =
                   aMaterialPropertyTable->GetProperty("RAYLEIGH");
             if(AttenuationLengthVector){

#if ( G4VERSION_NUMBER > 1000 )
               AttenuationLength = AttenuationLengthVector->Value(thePhotonEnergy);
#else
               AttenuationLength = AttenuationLengthVector ->
                                    GetProperty(thePhotonEnergy);
#endif
             }
             else{
//               G4cout << "No Rayleigh scattering length specified" << G4endl;
             }
           }
           else{
//             G4cout << "No Rayleigh scattering length specified" << G4endl; 
           }
        }

        return AttenuationLength;
}

// RayleighAttenuationLengthGenerator()
// ------------------------------------
// Private method to compute Rayleigh Scattering Lengths (for water)
//
G4MaterialPropertyVector* 
DsG4OpRayleigh::RayleighAttenuationLengthGenerator(G4MaterialPropertiesTable *aMPT) 
{
        // Physical Constants

        // isothermal compressibility of water
        G4double betat = 7.658e-23*m3/MeV;

        // K Boltzman
        G4double kboltz = 8.61739e-11*MeV/kelvin;

        // Temperature of water is 10 degrees celsius
        // conversion to kelvin:
        // TCelsius = TKelvin - 273.15 => 273.15 + 10 = 283.15
        G4double temp = 283.15*kelvin;

        // Retrieve vectors for refraction index
        // and photon energy from the material properties table

        G4MaterialPropertyVector* Rindex = aMPT->GetProperty("RINDEX");

        G4double refsq;
        G4double e;
        G4double xlambda;
        G4double c1, c2, c3, c4;
        G4double Dist;
        G4double refraction_index;

        G4MaterialPropertyVector *RayleighScatteringLengths = 
				new G4MaterialPropertyVector();

        if (Rindex ) {

#if ( G4VERSION_NUMBER > 1000 )
          for(size_t ii=0 ; ii < Rindex->GetVectorLength() ; ii++)
          {
                e = Rindex->Energy(ii);
                refraction_index = (*Rindex)[ii];
#else
           Rindex->ResetIterator();
           while (++(*Rindex)) {
                e = (Rindex->GetPhotonEnergy());
                refraction_index = Rindex->GetProperty();
#endif
                refsq = refraction_index*refraction_index;
                xlambda = h_Planck*c_light/e;

	        if (verboseLevel>0) {

#if ( G4VERSION_NUMBER > 1000 )
        	        G4cout << e << " MeV\t";
#else
        	        G4cout << Rindex->GetPhotonEnergy() << " MeV\t";
#endif
                	G4cout << xlambda << " mm\t";
		}

                c1 = 1 / (6.0 * pi);
                c2 = std::pow((2.0 * pi / xlambda), 4);
                c3 = std::pow( ( (refsq - 1.0) * (refsq + 2.0) / 3.0 ), 2);
                c4 = betat * temp * kboltz;

                Dist = 1.0 / (c1*c2*c3*c4);

	        if (verboseLevel>0) {
	                G4cout << Dist << " mm" << G4endl;
		}

#if ( G4VERSION_NUMBER > 1000 )
                RayleighScatteringLengths->InsertValues(e, Dist);
#else
                RayleighScatteringLengths->
			InsertValues(Rindex->GetPhotonEnergy(), Dist);
#endif


           }

        }  // SCB: Rindex

	return RayleighScatteringLengths;
}
