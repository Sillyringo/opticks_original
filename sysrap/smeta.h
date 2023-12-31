#pragma once
/**
smeta.h
==========

BASH_SOURCE fails to export

**/

#include "sproc.h"
#include "ssys.h"
#include "stamp.h"

#include "NP.hh"

struct smeta
{
    static constexpr const char* VARS = R"( 
HOME
USER
SCRIPT
PWD
CMDLINE
CHECK
LAYOUT
TEST
VERSION
GEOM
EXECUTABLE
COMMANDLINE
DIRECTORY
${GEOM}_GEOMList
)" ; 
// HIGHER ORDER KEYS WITH TOKENS ARE HANDLED IN ssys::_getenv
    static void Collect(std::string& meta, const char* source=nullptr) ;      
    static void CollectEnv(std::string& meta ) ;      
};

/**
smeta::Collect
----------------

This is used for example to populate (SEvt)sevt.meta by:

1. G4CXOpticks::init_SEvt for SEvt::EGPU meta 
2. U4Recorder::init_SEvt for SEvt::ECPU meta
3. CSGFoundry::init for the CSGFoundry::meta 

**/

inline void smeta::Collect(std::string& meta, const char* source)
{
    uint64_t t = stamp::Now(); 
    std::string tf = stamp::Format(t) ;

    if(source) NP::SetMeta<std::string>(meta, "source", source );
    NP::SetMeta<std::string>(meta, "creator", sproc::ExecutableName() );
    NP::SetMeta<uint64_t>(meta, "stamp", t);
    NP::SetMeta<std::string>(meta, "stampFmt", tf);
    NP::SetMeta<std::string>(meta, "uname", ssys::uname("-a"));

    CollectEnv(meta); 
}

inline void smeta::CollectEnv(std::string& meta)
{
    typedef std::pair<std::string, std::string> KV ; 
    std::vector<KV> kvs ; 
    ssys::getenv_(kvs, VARS); 
    NP::SetMetaKV(meta, kvs); 
}

