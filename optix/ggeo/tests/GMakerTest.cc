//  ggv --gmaker

#include "Opticks.hh"

#include "GMaker.hh"
#include "GSolid.hh"
#include "GMesh.hh"

#include "glm/glm.hpp"

int main(int argc, char** argv)
{
    Opticks* opticks = new Opticks(argc, argv, "gmaker.log");

    GMaker* maker = new GMaker(opticks);

    glm::vec4 param(0.f,0.f,0.f,100.f) ; 

    const char* spec = "Rock//perfectAbsorbSurface/Vacuum" ; 

    std::vector<GSolid*> solids = maker->make(0u, 'S', param, spec );

    for(unsigned int i=0 ; i < solids.size() ; i++)
    {
        GSolid* solid = solids[i] ;

        solid->Summary();

        GMesh* mesh = solid->getMesh();

        mesh->dump();

    }

}

