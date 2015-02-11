#include "GMaterial.hh"
#include "GProperty.hh"

#include <string>
#include <vector>


int main(int argc, char* argv[])
{
    GMaterial* mat = new GMaterial("demo");

    float domain[]={1.f,2.f,3.f,4.f,5.f,6.f,7.f};
    float vals[]  ={1.f,2.f,3.f,4.f,5.f,6.f,7.f};

    mat->AddProperty("pname", vals, domain, sizeof(domain)/sizeof(domain[0]) );

    GProperty<float>* prop = mat->GetProperty("pname");
    prop->Dump("prop dump");


    return 0 ;
}


