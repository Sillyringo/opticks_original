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

#include <cstring>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>

#include <boost/algorithm/string.hpp>


// okc-
#include "OpticksAttrSeq.hh"
#include "OpticksColors.hh"

// npy-
#include "NGLM.hpp"
#include "NPY.hpp"
#include "Index.hpp"
#include "Types.hpp"

#include "NQuad.hpp"


#include "GColorMap.hh"
#include "GVector.hh"
#include "GItemIndex.hh"

#include "SLOG.hh"


const plog::Severity GItemIndex::LEVEL = SLOG::EnvLevel("GItemIndex", "DEBUG") ; 


GItemIndex* GItemIndex::load(const char* idpath, const char* itemtype, const char* reldir)
{
    GItemIndex* idx = new GItemIndex(itemtype, reldir) ;    // itemname->index
    idx->loadIndex(idpath);
    return idx ; 
}

GItemIndex::GItemIndex(const char* itemtype, const char* reldir)
   : 
   m_index(NULL),
   m_colors(NULL),
   m_colormap(NULL),
   m_colorbuffer(NULL),
   m_types(NULL),
   m_handler(NULL)
{
   init(itemtype, reldir);
   setLabeller(DEFAULT);
}

GItemIndex::GItemIndex(Index* index)
   : 
   m_index(index),
   m_colors(NULL),
   m_colormap(NULL),
   m_colorbuffer(NULL),
   m_types(NULL),
   m_handler(NULL)
{
   setLabeller(DEFAULT);
}


void GItemIndex::setLabeller(GItemIndexLabellerPtr labeller)
{
   m_labeller = labeller ; 
}
void GItemIndex::setColorSource(OpticksColors* colors)
{
   m_colors = colors ; 
}
void GItemIndex::setColorMap(GColorMap* colormap)
{
   m_colormap = colormap ; 
}
void GItemIndex::setTypes(Types* types)
{
   m_types = types ; 
}
void GItemIndex::setHandler(OpticksAttrSeq* handler)
{
   m_handler = handler ; 
}



Types* GItemIndex::getTypes()
{
   return m_types ;
}


OpticksColors* GItemIndex::getColorSource()
{
   return m_colors ; 
}
GColorMap* GItemIndex::getColorMap()
{
   return m_colormap ; 
}

Index* GItemIndex::getIndex()
{
   return m_index ; 
}


std::vector<unsigned int>& GItemIndex::getCodes()
{
   return m_codes ; 
}

std::vector<std::string>& GItemIndex::getLabels()
{
   return m_labels ; 
}

const char* GItemIndex::getLabel(unsigned index)
{
   return index < m_labels.size() ? m_labels[index].c_str() : NULL ;
}

const char* GItemIndex::getShortLabel(unsigned index)
{
   return index < m_labels_short.size() ? m_labels_short[index].c_str() : NULL ;
}


void GItemIndex::init(const char* itemtype, const char* reldir)
{
    m_index = new Index(itemtype, reldir);
}

void GItemIndex::setTitle(const char* title)
{
   m_index->setTitle(title);
}

int GItemIndex::getSelected()
{
   return m_index->getSelected();
}
const char* GItemIndex::getSelectedKey()
{
   return m_index->getSelectedKey();
}
const char* GItemIndex::getSelectedLabel()
{
   int sel = m_index->getSelected();
   return sel == 0 ? "All" : getShortLabel(sel-1);
}


bool GItemIndex::hasIndex() const
{
    return m_index != NULL ;  
}



void GItemIndex::loadIndex(const char* idpath, const char* override_)
{
    const char* itemtype = override_ ? override_ : m_index->getItemType() ;
    const char* reldir = m_index->getRelDir();

    if(override_)
    {
        LOG(error)<<"GItemIndex::loadIndex using override itemtype " << itemtype << " instead of default " << m_index->getItemType() ;
    }

    // hmm stomps existing index ?
    m_index = Index::load(idpath, itemtype, reldir);

    LOG_IF(error, !m_index)
        << " failed for "
        << " idpath " << idpath
        << " reldir " << ( reldir ? reldir : "-" )
        << " override " << ( override_ ? override_ : "NULL" )
        ; 

}

void GItemIndex::add(const char* name, unsigned int source)
{
    assert(m_index);
    m_index->add(name, source);
}

unsigned int GItemIndex::getIndexLocal(const char* name, unsigned int missing)
{
    assert(m_index);
    return m_index->getIndexLocal(name, missing);
}
unsigned int GItemIndex::getIndexSource(const char* name, unsigned int missing)
{
    assert(m_index);
    return m_index->getIndexSource(name, missing);
}
unsigned GItemIndex::getIndexSourceStarting(const char* name, unsigned int missing)
{
    assert(m_index);
    return m_index->getIndexSourceStarting(name, missing);
}




const char*  GItemIndex::getNameSource(unsigned source, const char* missing)  // source index 
{
    assert(m_index);
    return m_index->getNameSource(source, missing);
}
const char*  GItemIndex::getNameLocal( unsigned local, const char* missing)   // local index
{
    assert(m_index);
    return m_index->getNameLocal(local, missing);
}





bool GItemIndex::hasItem(const char* key)
{
    assert(m_index);
    return m_index->hasItem(key);
}


void GItemIndex::save(const char* idpath)
{
    assert(m_index);
    m_index->save(idpath);
}

unsigned int GItemIndex::getNumItems()
{
    return m_index ? m_index->getNumItems() : 0 ;
}

void GItemIndex::test(const char* msg, bool verbose)
{
    assert(m_index);
    m_index->test(msg, verbose);
}

void GItemIndex::dump(const char* msg)
{
   if(!m_index)
   {
       LOG(warning) << msg << " NULL index "; 
       return ; 
   }  

   LOG(info) << msg << " itemtype: " << m_index->getItemType()  ; 

   typedef std::vector<std::string> VS ; 
   VS names = m_index->getNames();
   for(VS::iterator it=names.begin() ; it != names.end() ; it++ )
   {
       std::string iname = *it ; 
       const char*  cname = m_colormap ? m_colormap->getItemColor(iname.c_str(), NULL) : NULL ; 
       unsigned int ccode = m_colors   ? m_colors->getCode(cname, 0xFFFFFF) : 0xFFFFFF ; 

       unsigned int source = m_index->getIndexSource(iname.c_str()) ;
       unsigned int local  = m_index->getIndexLocal(iname.c_str()) ;
       std::cout 
            << " iname  " << std::setw(35) <<  iname
            << " source " << std::setw(4) <<  std::dec << source
            << " local  " << std::setw(4) <<  std::dec << local
            << " 0x " << std::setw(4)     <<  std::hex << local
            << " cname  " << std::setw(20) <<  ( cname ? cname : "no-colormap-or-missing" )
            << " ccode  " << std::setw(20) << std::hex <<  ccode 
            << std::dec
            << std::endl ; 
   }
}



/**
GItemIndex::getLabel
----------------------

Operates via the OpticksAttrSeq handler


**/


std::string GItemIndex::getLabel(const char* key, unsigned int& colorcode)
{
    // Trojan Handler : out to kill most of this class
    return m_handler ? 
                       m_handler->getLabel(m_index, key, colorcode) 
                     : 
                      (*m_labeller)(this, key, colorcode);
}


void GItemIndex::setLabeller(Labeller_t labeller )
{
    switch(labeller)
    {
        case    DEFAULT  : setLabeller(&GItemIndex::defaultLabeller)     ; break ; 
        case   COLORKEY  : setLabeller(&GItemIndex::colorKeyLabeller)    ; break ; 
        case MATERIALSEQ : setLabeller(&GItemIndex::materialSeqLabeller) ; break ; 
        case  HISTORYSEQ : setLabeller(&GItemIndex::historySeqLabeller)  ; break ; 
    }
}

std::string GItemIndex::defaultLabeller(GItemIndex* /*self*/, const char* key, unsigned int& colorcode)
{
   colorcode = 0xFFFFFF ; 
   return key ;  
}

std::string GItemIndex::colorKeyLabeller(GItemIndex* self, const char* key, unsigned int& colorcode )
{
    // function pointers have to be static, so access members python style
    colorcode = self->getColorCode(key);

    Index* index = self->getIndex();
    unsigned int local  = index->getIndexLocal(key) ;

    std::stringstream ss ; 
    ss  << std::setw(5)  << std::dec << local 
        << std::setw(25) << key
        << std::setw(10) << std::hex << colorcode 
        ;

    return ss.str();
}


const char* GItemIndex::getColorName(const char* key)
{
    return m_colormap ? m_colormap->getItemColor(key, NULL) : NULL ; 
}

unsigned int GItemIndex::getColorCode(const char* key )
{
    const char*  colorname =  getColorName(key) ;
    unsigned int colorcode  = m_colors ? m_colors->getCode(colorname, 0xFFFFFF) : 0xFFFFFF ; 
    return colorcode ; 
}


nvec3 GItemIndex::makeColor( unsigned int rgb )
{
    unsigned int red   =  ( rgb & 0xFF0000 ) >> 16 ;  
    unsigned int green =  ( rgb & 0x00FF00 ) >>  8 ;  
    unsigned int blue  =  ( rgb & 0x0000FF ) ;  

    float d(0xFF);
    float r = float(red)/d ;
    float g = float(green)/d ;
    float b = float(blue)/d ;

    return make_nvec3( r, g, b) ;
}



std::string GItemIndex::materialSeqLabeller(GItemIndex* self, const char* key_, unsigned int& colorcode)
{
   colorcode = 0xFFFFFF ; 
   std::string key(key_);
   Types* types = self->getTypes();
   assert(types); 

   std::string seqmat = types->abbreviateHexSequenceString(key, Types::MATERIALSEQ);  
   std::stringstream ss ; 
   ss << std::setw(16) << key_ 
      << " "
      << seqmat 
      ;

   return ss.str() ;  
}

std::string GItemIndex::historySeqLabeller(GItemIndex* self, const char* key_, unsigned int& colorcode)
{
   colorcode = 0xFFFFFF ; 
   std::string key(key_);

   Types* types = self->getTypes();
   assert(types); 
   std::string seqhis = types->abbreviateHexSequenceString(key, Types::HISTORYSEQ);  

   std::stringstream ss ; 
   ss << std::setw(16) << key_ 
      << " "
      << seqhis
      ;

   return ss.str() ;  
}



void GItemIndex::formTable(bool compact)
{
   m_codes.clear(); 
   m_labels.clear(); 
   m_labels_short.clear(); 

   // collect keys (item names) into vector and sort into ascending local index order 

   typedef std::vector<std::string> VS ; 


   VS& names = m_index->getNames(); 
   for(VS::iterator it=names.begin() ; it != names.end() ; it++ )
   {
       std::string key = *it ; 
       unsigned int colorcode(0x0) ; 
       std::string label = getLabel(key.c_str(), colorcode );

       const char* colorname = getColorName(key.c_str());
       if(colorname)
       {
          label += "   " ; 
          label += colorname ; 
       }

       LOG(LEVEL)
            << std::setw(30) << key 
            << " : " 
            << label 
            ;

       m_codes.push_back(colorcode);
       m_labels.push_back(label);

       unsigned ntail = m_handler ? m_handler->getValueWidth() : 50 ; 
       m_labels_short.push_back(ShortenLabel(label.c_str(), ntail ));
   }
}


/**
GItemIndex::ShortenLabel
----------------------------

Returns the ntail last characters from the full label.

The shortened label is used in the GUI to present
the selected flag sequence eg "TO SC BT BT BT BT SD".

**/

const char* GItemIndex::ShortenLabel(const char* label, unsigned ntail )
{
    unsigned len = strlen(label);
    const char* tail_ = len > ntail ? strdup( label + len - ntail ) : label ;

    std::string tail(tail_);
    boost::trim(tail);
    return strdup(tail.c_str()) ; 
}



NPY<unsigned char>* GItemIndex::makeColorBuffer()
{
   LOG_IF(warning, m_colors==nullptr) << "GItemIndex::makeColorBuffer no colors defined will provide defaults"  ; 

   formTable(); 
   LOG(info) << "GItemIndex::makeColorBuffer codes " << m_codes.size() ;  
   return m_colors->make_buffer(m_codes) ; 
}

NPY<unsigned char>* GItemIndex::getColorBuffer()
{
   if(!m_colorbuffer)
   {
       m_colorbuffer = makeColorBuffer();
   }  
   return m_colorbuffer ; 
}



std::string GItemIndex::gui_radio_select_debug()
{
    assert(m_index);
    Index* ii = m_index ; 

    std::stringstream ss ; 
    typedef std::vector<std::string> VS ;

    VS  names = ii->getNames();
    VS& labels = getLabels(); 

    LOG(info) << "GItemIndex::gui_radio_select_debug"
              << " names " << names.size()
              << " labels " << labels.size()
              ;

    assert(names.size() == labels.size());

    ss << " title " << ii->getTitle() << std::endl ;
    for(unsigned int i=0 ; i < labels.size() ; i++) 
            ss << std::setw(3) << i 
               << " name  " << std::setw(20) << names[i]
               << " label " << std::setw(50) << labels[i]
               << std::endl ; 

    return ss.str();
}


