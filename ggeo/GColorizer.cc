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

#include <cstddef>
#include <iomanip>

#include "OpticksAttrSeq.hh"
#include "OpticksColors.hh"
#include "NQuad.hpp"

#include "GVector.hh"
#include "GMesh.hh"
#include "GMergedMesh.hh"
#include "GVolume.hh"
#include "GItemIndex.hh"

#include "GNodeLib.hh"
#include "GGeoLib.hh"
#include "GBndLib.hh"
#include "GSurfaceLib.hh"
#include "GColorizer.hh"

#include "SLOG.hh"
// trace/debug/info/warning/error/fatal

const plog::Severity GColorizer::LEVEL = SLOG::EnvLevel("GColorizer","DEBUG") ; 


GColorizer::GColorizer(GNodeLib* nodelib, GGeoLib* geolib, GBndLib* blib, OpticksColors* colors, GColorizer::Style_t style ) 
    :
    m_target(NULL),
    m_nodelib(nodelib),
    m_geolib(geolib),
    m_blib(blib),
    m_slib(blib->getSurfaceLib()),
    m_colors(colors),
    m_style(style),
    m_cur_vertices(0),
    m_num_colorized(0),
    m_repeat_index(0)
{
    init();
}

void GColorizer::init()
{
    LOG_IF(fatal, !m_colors) << "m_colors NULL " ; 
    assert( m_colors ); 
}


/**
GColorizer::setTarget
----------------------

Sets where the vertex colors are written.

**/

void GColorizer::setTarget(nvec3* target)
{
    m_target =  target ; 
}
void GColorizer::setRepeatIndex(unsigned ridx)
{
    m_repeat_index = ridx ; 
}


/**
GColorizer::writeVertexColors
------------------------------

Huh why only mesh0 ?  Have observed that vertex colors (E key) are 
flat mid grey for instanced volumes.

TODO: do all mm

**/

void GColorizer::writeVertexColors()
{
    GMergedMesh* mesh0 = m_geolib->getMergedMesh(0); // mesh0-ok
    const GVolume* root = m_nodelib->getRootVolume();
    writeVertexColors( mesh0, root );   // mesh0-ok
}

void GColorizer::writeVertexColors(GMergedMesh* mm, const GVolume* root)
{
    assert(mm);
    unsigned ridx = mm->getIndex();  
    LOG(LEVEL) << "[ ridx " << ridx ; 

    gfloat3* vertex_colors = mm->getColors();

    setTarget( reinterpret_cast<nvec3*>(vertex_colors) );
    setRepeatIndex(ridx); 

    traverse(root);

    LOG(LEVEL) << "] ridx " << ridx ; 
}


/**
GColorizer::traverse
----------------------

Visits all vertices of selected volumes setting the 
vertex colors of the GMergedMesh based on indices of
objects configured via the style.

**/

void GColorizer::traverse(const GVolume* root)
{
    if(!m_target)
    {
        LOG(fatal) << "GColorizer::traverse must setTarget before traverse " ;
        return ;  
    }
    LOG(LEVEL) << "[" ; 

    traverse_r(root, 0);

    LOG(LEVEL) << "] num_colorized  " << m_num_colorized ; 
}

/**
GColorizer::traverse_r
-----------------------

Recurses the node tree, but writes to the flat m_target array from mm0
for selected.  Coloring approach determined by m_style

SURFACE_INDEX
    based on node boundary 
PSYCHEDELIC_VERTEX,PSYCHEDELIC_NODE,PSYCHEDELIC_MESH
    controls the index that dictates how often to change the color


**/

void GColorizer::traverse_r( const GNode* node, unsigned depth)
{
    const GVolume* volume = dynamic_cast<const GVolume*>(node) ;
    const GMesh* mesh = volume->getMesh();
    unsigned nvert = mesh->getNumVertices();


    bool selected = volume->isSelected() && volume->getRepeatIndex() == m_repeat_index ;

    LOG(verbose) << "GColorizer::traverse"
              << " depth " << depth
              << " node " << ( node ? node->getIndex() : 0 )
              << " nvert " << nvert
              << " selected " << selected
              ; 


    if(selected)
    {
        if( m_style == SURFACE_INDEX )
        { 
            nvec3 surfcolor = getSurfaceColor( node );
            ++m_num_colorized ; 
            for(unsigned int i=0 ; i<nvert ; ++i ) m_target[m_cur_vertices+i] = surfcolor ; 
        } 
        else if( m_style == PSYCHEDELIC_VERTEX || m_style == PSYCHEDELIC_NODE || m_style == PSYCHEDELIC_MESH )  // every VERTEX/SOLID/MESH a different color 
        {
            for(unsigned int i=0 ; i<nvert ; ++i ) 
            {
                unsigned int index ; 
                switch(m_style)
                {
                    case PSYCHEDELIC_VERTEX : index = i                ;break; 
                    case PSYCHEDELIC_NODE   : index = node->getIndex() ;break; 
                    case PSYCHEDELIC_MESH   : index = mesh->getIndex() ;break; 
                    default                 : index = 0                ;break; 
                }

                if(m_colors)
                { 
                    m_target[m_cur_vertices+i] = m_colors->getPsychedelic(index) ;
                }
      
            } 
        }

        m_cur_vertices += nvert ;      // offset within the flat arrays
    }
    for(unsigned i = 0; i < node->getNumChildren(); i++) traverse_r(node->getChild(i), depth + 1 );
}


/**
GColorizer::getSurfaceColor
-----------------------------

Arrive at a color based on isur/osur from the boundary of the node/volume.

**/

nvec3 GColorizer::getSurfaceColor(const GNode* node)
{
    const GVolume* volume = dynamic_cast<const GVolume*>(node) ;

    unsigned int boundary = volume->getBoundary();

    guint4 bnd = m_blib->getBnd(boundary);
    unsigned int isur_ = bnd.z ;  
    unsigned int osur_ = bnd.w ;  

    const char* isur = m_slib->getName(isur_);
    const char* osur = m_slib->getName(osur_);

    unsigned int colorcode(UINT_MAX) ; 
    if(isur)
    {
        colorcode = m_slib->getAttrNames()->getColorCode(isur);    
    } 
    else if(osur)
    {
        colorcode = m_slib->getAttrNames()->getColorCode(osur);    
    }  

    bool expected_color = colorcode != UINT_MAX ; 
    LOG_IF(debug, expected_color) << "GColorizer::getSurfaceColor " 
              << " isur " << std::setw(3) << isur_ << std::setw(30) <<  ( isur ? isur : "-" )
              << " osur " << std::setw(3) << osur_ << std::setw(30) <<  ( osur ? osur : "-" )
              << " colorcode " << std::hex << colorcode  << std::dec 
              ; 

   return colorcode == UINT_MAX ? make_nvec3(1,0,1) : GItemIndex::makeColor(colorcode) ; 
}


