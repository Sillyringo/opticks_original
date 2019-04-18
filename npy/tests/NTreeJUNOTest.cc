// TEST=NTreeJUNOTest om-t

#include <vector>
#include "NTreeJUNO.hpp"
#include "NNode.hpp"
#include "NSolid.hpp"
#include "NTreeAnalyse.hpp"

#include "OPTICKS_LOG.hh"


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);


    typedef std::vector<int> VI ; 
    VI lvs = { 18, 19, 20 , 21 } ;  

    for( VI::const_iterator it=lvs.begin() ; it != lvs.end() ; it++ )
    { 
        int lv = *it ; 
        nnode* a = NSolid::create(lv); 
        if(!a) continue ;
       
        LOG(fatal) << "LV=" << lv << " label " << ( a->label ? a->label : "-" ) ; 
        LOG(error) << NTreeAnalyse<nnode>::Desc(a) ; 

        NTreeJUNO tj(a) ; 
        tj.rationalize(); 

        LOG(info) << NTreeAnalyse<nnode>::Desc(tj.root);

    }

    return 0 ; 
}



