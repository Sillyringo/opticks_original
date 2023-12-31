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


// g4-
#include "G4VSolid.hh"
#include "G4VoxelLimits.hh"
#include "G4AffineTransform.hh"

// npy-
#include "NGLM.hpp"
#include "NBoundingBox.hpp"
#include "NBBox.hpp"
#include "GLMFormat.hpp"

#include "X4SolidExtent.hh"
#include "X4AffineTransform.hh"

#include "SLOG.hh"



X4SolidExtent::X4SolidExtent(const G4VSolid* solid) 
   :
      m_solid(solid)
{
}

void X4SolidExtent::extent(const G4Transform3D& tran, glm::vec3& low, glm::vec3& high, glm::vec4& ce)
{
    G4AffineTransform  atran = X4AffineTransform::FromTransform(tran);
    G4VoxelLimits      limit; // Unlimited

    G4double minX,maxX,minY,maxY,minZ,maxZ ;

    m_solid->CalculateExtent(kXAxis,limit,atran,minX,maxX);
    m_solid->CalculateExtent(kYAxis,limit,atran,minY,maxY);
    m_solid->CalculateExtent(kZAxis,limit,atran,minZ,maxZ);

    low.x = float(minX) ;
    low.y = float(minY) ;
    low.z = float(minZ) ;

    high.x = float(maxX) ;
    high.y = float(maxY) ;
    high.z = float(maxZ) ;

    ce.x = float((minX + maxX)/2.) ; 
    ce.y = float((minY + maxY)/2.) ; 
    ce.z = float((minZ + maxZ)/2.) ; 
    ce.w = NBoundingBox::extent(low, high);


    LOG(debug) << "X4SolidExtent::extent"
              << " low " << gformat(low)
              << " high " << gformat(high)
              ;

}

nbbox* X4SolidExtent::Extent(const G4VSolid* solid)
{
    G4Transform3D tran ; 
    X4SolidExtent cs(solid); 
    nbbox bb ; 
    glm::vec4 ce ; 
    cs.extent( tran, bb.min, bb.max, ce ); 
    return new nbbox(bb) ;
}

nbbox* X4SolidExtent::BoundingLimits(const G4VSolid* solid)  // static 
{
    G4ThreeVector pMin ; 
    G4ThreeVector pMax ;
    solid->BoundingLimits(pMin, pMax); 

    nbbox bb ; 

    bb.min.x = float(pMin.x());
    bb.min.y = float(pMin.y());
    bb.min.z = float(pMin.z());

    bb.max.x = float(pMax.x());
    bb.max.y = float(pMax.y());
    bb.max.z = float(pMax.z());

    return new nbbox(bb) ;
}



