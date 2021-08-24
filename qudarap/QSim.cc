#include "PLOG.hh"
#include "SSys.hh"
#include "scuda.h"
#include "squad.h"

#include "NP.hh"
#include "QUDA_CHECK.h"
#include "QU.hh"

#include "qrng.h"
#include "qsim.h"

#include "QRng.hh"
#include "QTex.hh"
#include "QScint.hh"
#include "QBnd.hh"
#include "QProp.hh"
#include "QEvent.hh"
#include "QSim.hh"

template <typename T>
const plog::Severity QSim<T>::LEVEL = PLOG::EnvLevel("QSim", "INFO"); 

template <typename T>
const QSim<T>* QSim<T>::INSTANCE = nullptr ; 

template <typename T>
const QSim<T>* QSim<T>::Get(){ return INSTANCE ; }

/**
QSim::UploadComponents
-----------------------

This is a once only action for a geometry, encompassing 
random states, scintillation and boundary textures and 
property arrays.   It is the simulation physics equivalent 
of uploading the CSGFoundry geometry. 

Instanciates the singleton instances that subsequenct QSim 
instanciation collects together.

**/

template <typename T>
void QSim<T>::UploadComponents( const NP* icdf, const NP* bnd )
{
    // on heap, to avoid dtors

    QRng* qrng = new QRng ;  // loads and uploads curandState 
    LOG(LEVEL) << qrng->desc(); 

    const char* qsim_icdf_path = SSys::getenvvar("QSIM_ICDF_PATH", nullptr ); 
    const NP* override_icdf = qsim_icdf_path ?  NP::Load(qsim_icdf_path) : nullptr ;
 
    unsigned hd_factor = 0u ;  // 0,10,20
    QScint* qscint = new QScint( override_icdf ? override_icdf : icdf, hd_factor); // custom high-definition inverse CDF for scintillation generation
    LOG(LEVEL) << qscint->desc(); 

    QBnd* qbnd = new QBnd(bnd); // boundary texture with standard domain, used for standard fast property lookup 
    LOG(LEVEL) << qbnd->desc(); 

    QProp<T>* qprop = new QProp<T> ;  // property interpolation with per-property domains, eg used for Cerenkov RINDEX sampling 
    LOG(LEVEL) << qprop->desc(); 

}


/**
QSim:::QSim
-------------

Prior to instanciating QSim invoke QSim::Init to prepare the 
singleton components. 

**/

template <typename T>
QSim<T>::QSim()
    :
    rng(QRng::Get()),
    scint(QScint::Get()),
    bnd(QBnd::Get()),
    prop(QProp<T>::Get()),
    sim(new qsim<T>),
    d_sim(init_upload())
{
    INSTANCE = this ; 
    LOG(LEVEL) << desc() ; 
}

/**
QSim::init
------------

NB .sim (qsim.h) is a host side instance that is populated
with device side pointers and handles and then uploaded 
to the device d_sim.

Many device pointers and handles are then accessible from 
the qsim.h instance at the cost of only a single launch 
parameter argument: the d_sim pointer
or with optix launches a single Params member.

The advantage of this approach is it avoids kernel 
launches having very long argument lists and provides a natural 
place (qsim.h) to add GPU side functionality. 

**/
template <typename T>
qsim<T>* QSim<T>::init_upload()
{
    LOG(LEVEL) 
        << " rng " << rng 
        << " scint " << scint
        << " bnd " << bnd
        << " prop " << prop
        << " sim " << sim 
        << " d_sim " << d_sim 
        ;  

    unsigned hd_factor = scint->tex->getHDFactor() ;  // HMM: perhaps get this from sim rather than occupying an argument slot  
    LOG(LEVEL) 
        << " hd_factor " << hd_factor  
        ;

    if(rng)
    {
        LOG(LEVEL) << " rng " << rng->desc() ; 
        sim->rngstate = rng->qr->rng_states ; 
    } 
    if(scint)
    {
        LOG(LEVEL) << " scint.desc " << scint->desc() ; 
        sim->scint_tex = scint->tex->texObj ; 
        sim->scint_meta = scint->tex->d_meta ; 
    } 
    if(bnd)
    {
        LOG(LEVEL) << " bnd " << bnd->desc() ; 
        sim->boundary_tex = bnd->tex->texObj ; 
        sim->boundary_meta = bnd->tex->d_meta ; 
        sim->boundary_tex_MaterialLine_Water = bnd->getMaterialLine("Water") ; 
        sim->boundary_tex_MaterialLine_LS    = bnd->getMaterialLine("LS") ; 
    } 

    if(prop)
    {
        LOG(LEVEL) << " prop " << prop->desc() ; 
        sim->prop = prop->getDevicePtr() ; 
    }

    qsim<T>* device_sim = QU::UploadArray<qsim<T>>(sim, 1 );  
    return device_sim ; 
}

template <typename T>
qsim<T>* QSim<T>::getDevicePtr() const 
{
    return d_sim ; 
}
 




template <typename T>
char QSim<T>::getScintTexFilterMode() const 
{
    return scint->tex->getFilterMode() ; 
}


template<typename T>
std::string QSim<T>::desc() const
{
    std::stringstream ss ; 
    ss << "QSim"
       << " sim->rngstate " << sim->rngstate 
       << " sim->scint_tex " << sim->scint_tex 
       << " sim->scint_meta " << sim->scint_meta
       << " sim->boundary_tex " << sim->boundary_tex 
       << " sim->boundary_meta " << sim->boundary_meta
       << " d_sim " << d_sim 
       ; 
    std::string s = ss.str(); 
    return s ; 
}



template<typename T>
void QSim<T>::configureLaunch(unsigned width, unsigned height ) 
{
    QU::ConfigureLaunch(numBlocks, threadsPerBlock, width, height); 
}

template<typename T>
void QSim<T>::configureLaunch2D(unsigned width, unsigned height ) 
{
    QU::ConfigureLaunch2D(numBlocks, threadsPerBlock, width, height); 
}


template <typename T>
extern void QSim_rng_sequence_0(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, T* rs, unsigned num_items ); 


template<typename T>
void QSim<T>::rng_sequence_0( T* rs, unsigned num_items )
{
    configureLaunch(num_items, 1 ); 

    T* d_rs = QU::device_alloc<T>(num_items ); 

    QSim_rng_sequence_0<T>(numBlocks, threadsPerBlock, d_sim, d_rs, num_items );  

    QU::copy_device_to_host_and_free<T>( rs, d_rs, num_items ); 

    LOG(LEVEL) << "]" ; 
}

/**
QSim::rng_sequence mass production with multiple launches...
--------------------------------------------------------------

Split output files too ?::

    epsilon:opticks blyth$ np.py *.npy 
    a :                                            TRngBufTest_0.npy :      (10000, 16, 16) : 8f9b27c9416a0121574730baa742b5c9 : 20210715-1227 
    epsilon:opticks blyth$ du -h TRngBufTest_0.npy
     20M	TRngBufTest_0.npy

    In [6]: (16*16*4*2*10000)/1e6
    Out[6]: 20.48

Upping to 1M would be 100x 20M = 2000M  2GB

* using floats would half storage to 1GB, just promote to double in cks/OpticksRandom::flat 
  THIS MAKES SENSE AS curand_uniform IS GENERATING float anyhow 
* 100k blocks would mean 10*100Mb files for 1M : can do in 10 launches to work on any GPU 

**/


template <typename T>
extern void QSim_rng_sequence(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, T*  seq, unsigned ni, unsigned nj, unsigned ioffset ); 


template <typename T>
void QSim<T>::rng_sequence( T* seq, unsigned ni_tranche, unsigned nv, unsigned ioffset )
{
    configureLaunch(ni_tranche, 1 ); 

    unsigned num_rng = ni_tranche*nv ;  

    T* d_seq = QU::device_alloc<T>(num_rng); 

    QSim_rng_sequence<T>(numBlocks, threadsPerBlock, d_sim, d_seq, ni_tranche, nv, ioffset );  

    QU::copy_device_to_host_and_free<T>( seq, d_seq, num_rng ); 
}


template <typename T>
const char* QSim<T>::PREFIX = "rng_sequence" ; 

/**
QSim::rng_sequence
---------------------

The ni is split into tranches of ni_tranche_size each.

**/

template <typename T>
void QSim<T>::rng_sequence( const char* dir, unsigned ni, unsigned nj, unsigned nk, unsigned ni_tranche_size  )
{
    assert( ni % ni_tranche_size == 0 ); 
    unsigned num_tranche = ni/ni_tranche_size ; 
    unsigned nv = nj*nk ; 
    unsigned size = ni_tranche_size*nv ; 
    std::string reldir = QU::rng_sequence_reldir<T>(PREFIX, ni, nj, nk, ni_tranche_size  ) ;  

    std::cout 
        << "QSim::rng_sequence" 
        << " ni " << ni
        << " ni_tranche_size " << ni_tranche_size
        << " num_tranche " << num_tranche 
        << " reldir " << reldir.c_str()
        << " nj " << nj
        << " nk " << nk
        << " nv(nj*nk) " << nv 
        << " size(ni_tranche_size*nv) " << size 
        << " typecode " << QU::typecode<T>() 
        << std::endl 
        ; 

    NP* seq = NP::Make<T>(ni_tranche_size, nj, nk) ; 
    T* values = seq->values<T>(); 

    for(unsigned t=0 ; t < num_tranche ; t++)
    {
        unsigned ioffset = ni_tranche_size*t ; 
        std::string name = QU::rng_sequence_name<T>(PREFIX, ni_tranche_size, nj, nk, ioffset ) ;  

        std::cout 
            << std::setw(3) << t 
            << std::setw(10) << ioffset 
            << std::setw(100) << name.c_str()
            << std::endl 
            ; 

        rng_sequence( values, ni_tranche_size, nv, ioffset );  
        seq->save(dir, reldir.c_str(), name.c_str()); 
    }
}




/**
QSim::scint_wavelength
----------------------------------

Setting envvar QSIM_DISABLE_HD disables multiresolution handling
and causes the returned hd_factor to be zero rather then 
the typical values of 10 or 20 which depend on the buffer creation.

**/

template <typename T>
extern void QSim_scint_wavelength(   dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, T* wavelength, unsigned num_wavelength, unsigned hd_factor ); 

template <typename T>
void QSim<T>::scint_wavelength( T* wavelength, unsigned num_wavelength, unsigned& hd_factor )
{
    bool qsim_disable_hd = SSys::getenvbool("QSIM_DISABLE_HD"); 
    hd_factor = qsim_disable_hd ? 0u : scint->tex->getHDFactor() ; 
    // HMM: perhaps get this from sim rather than occupying an argument slot  
    LOG(LEVEL) << "[" << " qsim_disable_hd " << qsim_disable_hd << " hd_factor " << hd_factor ; 

    configureLaunch(num_wavelength, 1 ); 

    T* d_wavelength = QU::device_alloc<T>(num_wavelength); 

    QSim_scint_wavelength<T>(numBlocks, threadsPerBlock, d_sim, d_wavelength, num_wavelength, hd_factor );  

    QU::copy_device_to_host_and_free<T>( wavelength, d_wavelength, num_wavelength ); 

    LOG(LEVEL) << "]" ; 
}

/**
QSim::cerenkov_wavelength
---------------------------

**/

template <typename T>
extern void QSim_cerenkov_wavelength(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, T* wavelength, unsigned num_wavelength ); 

template <typename T>
void QSim<T>::cerenkov_wavelength( T* wavelength, unsigned num_wavelength )
{
    LOG(LEVEL) << "[ num_wavelength " << num_wavelength ;
 
    configureLaunch(num_wavelength, 1 ); 

    T* d_wavelength = QU::device_alloc<T>(num_wavelength); 

    QSim_cerenkov_wavelength(numBlocks, threadsPerBlock, d_sim, d_wavelength, num_wavelength );  

    QU::copy_device_to_host_and_free<T>( wavelength, d_wavelength, num_wavelength ); 

    LOG(LEVEL) << "]" ; 
}




template <typename T>
extern void QSim_cerenkov_photon(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, quad4* photon, unsigned num_photon, int print_id );


template <typename T>
void QSim<T>::cerenkov_photon( quad4* photon, unsigned num_photon, int print_id )
{
    LOG(LEVEL) << "[ num_photon " << num_photon ;
 
    configureLaunch(num_photon, 1 ); 

    quad4* d_photon = QU::device_alloc<quad4>(num_photon); 

    QSim_cerenkov_photon<T>(numBlocks, threadsPerBlock, d_sim, d_photon, num_photon, print_id );  

    QU::copy_device_to_host_and_free<quad4>( photon, d_photon, num_photon ); 

    LOG(LEVEL) << "]" ; 
}



template <typename T>
extern void QSim_cerenkov_photon_enprop(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, quad4* photon, unsigned num_photon, int print_id );


template <typename T>
void QSim<T>::cerenkov_photon_enprop( quad4* photon, unsigned num_photon, int print_id )
{
    LOG(LEVEL) << "[ num_photon " << num_photon ;
 
    configureLaunch(num_photon, 1 ); 

    quad4* d_photon = QU::device_alloc<quad4>(num_photon); 

    QSim_cerenkov_photon_enprop<T>(numBlocks, threadsPerBlock, d_sim, d_photon, num_photon, print_id );  

    QU::copy_device_to_host_and_free<quad4>( photon, d_photon, num_photon ); 

    LOG(LEVEL) << "]" ; 
}




template <typename T>
extern void QSim_cerenkov_photon_expt(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, quad4* photon, unsigned num_photon, int print_id );


template <typename T>
void QSim<T>::cerenkov_photon_expt( quad4* photon, unsigned num_photon, int print_id )
{
    LOG(LEVEL) << "[ num_photon " << num_photon ;
 
    configureLaunch(num_photon, 1 ); 

    quad4* d_photon = QU::device_alloc<quad4>(num_photon); 

    QSim_cerenkov_photon_expt<T>(numBlocks, threadsPerBlock, d_sim, d_photon, num_photon, print_id );  

    QU::copy_device_to_host_and_free<quad4>( photon, d_photon, num_photon ); 

    LOG(LEVEL) << "]" ; 
}





template <typename T>
void QSim<T>::dump_wavelength( T* wavelength, unsigned num_wavelength, unsigned edgeitems )
{
    LOG(LEVEL); 
    for(unsigned i=0 ; i < num_wavelength ; i++)
    {
        if( i < edgeitems || i > num_wavelength - edgeitems)
        {
            std::cout 
                << std::setw(10) << i 
                << std::setw(10) << std::fixed << std::setprecision(3) << wavelength[i] 
                << std::endl 
                ; 
        }
    }
}


template <typename T>
extern void QSim_scint_photon( dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, quad4* photon , unsigned num_photon ); 


template <typename T>
void QSim<T>::scint_photon( quad4* photon, unsigned num_photon )
{
    LOG(LEVEL) << "[" ; 

    configureLaunch( num_photon, 1 ); 

    quad4* d_photon = QU::device_alloc<quad4>(num_photon) ; 

    QSim_scint_photon(numBlocks, threadsPerBlock, d_sim, d_photon, num_photon );  

    QU::copy_device_to_host_and_free<quad4>( photon, d_photon, num_photon ); 

    LOG(LEVEL) << "]" ; 
}




template <typename T>
extern void QSim_generate_photon(dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* sim, qevent* evt )  ; 
 

template <typename T>
void QSim<T>::generate_photon(QEvent* evt)
{
    LOG(LEVEL) << "[" ; 

    unsigned num_photon = evt->getNumPhotons() ;  
    LOG(info) << " num_photon " << num_photon ; 

    if( num_photon == 0 )
    {
        LOG(fatal) 
           << " num_photon zero : MUST QEvent::setGenstep before QSim::generate_photon "  
           ; 
        return ; 
    }

    qevent* d_evt = evt->getDevicePtr(); 

    assert( d_evt ); 
    assert( d_sim ); 

    configureLaunch( num_photon, 1 ); 

    LOG(info) << "QSim_generate_photon... " ; 

    QSim_generate_photon(numBlocks, threadsPerBlock, d_sim, d_evt );  

    LOG(LEVEL) << "]" ; 
}



template <typename T>
extern void QSim_boundary_lookup_all(    dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, quad* lookup  , unsigned width, unsigned height ); 

template <typename T>
void QSim<T>::boundary_lookup_all(quad* lookup, unsigned width, unsigned height )
{
    LOG(LEVEL) << "[" ; 
    assert( bnd ); 
    assert( width <= getBoundaryTexWidth()  ); 
    assert( height <= getBoundaryTexHeight()  ); 

    unsigned num_lookup = width*height ; 
    LOG(LEVEL) 
        << " width " << width 
        << " height " << height 
        << " num_lookup " << num_lookup
        ;
   

    configureLaunch(width, height ); 

    quad* d_lookup = QU::device_alloc<quad>(num_lookup) ; 

    QSim_boundary_lookup_all(numBlocks, threadsPerBlock, d_sim, d_lookup, width, height );  

    QU::copy_device_to_host_and_free<quad>( lookup, d_lookup, num_lookup ); 

    LOG(LEVEL) << "]" ; 

}

template <typename T>
extern void QSim_boundary_lookup_line(    dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, quad* lookup, T* domain, unsigned num_lookup, unsigned line, unsigned k ); 


template <typename T>
void QSim<T>::boundary_lookup_line( quad* lookup, T* domain, unsigned num_lookup, unsigned line, unsigned k ) 
{
    LOG(LEVEL) 
        << "[" 
        << " num_lookup " << num_lookup
        << " line " << line 
        << " k " << k 
        ; 

    configureLaunch(num_lookup, 1  ); 

    T* d_domain = QU::device_alloc<T>(num_lookup) ; 

    QU::copy_host_to_device<T>( d_domain, domain, num_lookup ); 

    quad* d_lookup = QU::device_alloc<quad>(num_lookup) ; 

    QSim_boundary_lookup_line<T>(numBlocks, threadsPerBlock, d_sim, d_lookup, d_domain, num_lookup, line, k );  

    QU::copy_device_to_host_and_free<quad>( lookup, d_lookup, num_lookup ); 

    QU::device_free<T>( d_domain ); 


    LOG(LEVEL) << "]" ; 
}





/**
QSim::prop_lookup
--------------------

suspect problem when have fine domain and many pids due to 2d launch config,
BUT when have 1d launch there is no problem to launch millions of threads : hence the 
below *prop_lookup_onebyone* 

**/


template <typename T>
extern void QSim_prop_lookup( dim3 numBlocks, dim3 threadsPerBlock, qsim<T>* d_sim, T* lookup, const T* domain, unsigned domain_width, unsigned* pids, unsigned num_pids ); 

template <typename T>
void QSim<T>::prop_lookup( T* lookup, const T* domain, unsigned domain_width, const std::vector<unsigned>& pids ) 
{
    unsigned num_pids = pids.size() ; 
    unsigned num_lookup = num_pids*domain_width ; 
    LOG(LEVEL) 
        << "[" 
        << " num_pids " << num_pids
        << " domain_width " << domain_width 
        << " num_lookup " << num_lookup
        ; 

    configureLaunch(domain_width, num_pids  ); 

    unsigned* d_pids = QU::device_alloc<unsigned>(num_pids) ; 
    T* d_domain = QU::device_alloc<T>(domain_width) ; 
    T* d_lookup = QU::device_alloc<T>(num_lookup) ; 

    QU::copy_host_to_device<T>( d_domain, domain, domain_width ); 
    QU::copy_host_to_device<unsigned>( d_pids, pids.data(), num_pids ); 

    QSim_prop_lookup(numBlocks, threadsPerBlock, d_sim, d_lookup, d_domain, domain_width, d_pids, num_pids );  

    QU::copy_device_to_host_and_free<T>( lookup, d_lookup, num_lookup ); 
    QU::device_free<T>( d_domain ); 
    QU::device_free<unsigned>( d_pids ); 

    LOG(LEVEL) << "]" ; 
}



/**
Hmm doing lookups like this is a very common pattern, could do with 
a sub context to carry the pieces to simplify doing that.
**/

template <typename T>
extern void QSim_prop_lookup_one(
    dim3 numBlocks, 
    dim3 threadsPerBlock, 
    qsim<T>* sim, 
    T* lookup, 
    const T* domain, 
    unsigned domain_width, 
    unsigned num_pids, 
    unsigned pid, 
    unsigned ipid 
);

template <typename T>
void QSim<T>::prop_lookup_onebyone( T* lookup, const T* domain, unsigned domain_width, const std::vector<unsigned>& pids ) 
{
    unsigned num_pids = pids.size() ; 
    unsigned num_lookup = num_pids*domain_width ; 
    LOG(LEVEL) 
        << "[" 
        << " num_pids " << num_pids
        << " domain_width " << domain_width 
        << " num_lookup " << num_lookup
        ; 

    configureLaunch(domain_width, 1  ); 

    T* d_domain = QU::device_alloc<T>(domain_width) ; 
    QU::copy_host_to_device<T>( d_domain, domain, domain_width ); 

    T* d_lookup = QU::device_alloc<T>(num_lookup) ; 

    // separate launches for each pid
    for(unsigned ipid=0 ; ipid < num_pids ; ipid++)
    {
        unsigned pid = pids[ipid] ; 
        QSim_prop_lookup_one<T>(numBlocks, threadsPerBlock, d_sim, d_lookup, d_domain, domain_width, num_pids, pid, ipid );  
    }

    QU::copy_device_to_host_and_free<T>( lookup, d_lookup, num_lookup ); 

    QU::device_free<T>( d_domain ); 

    LOG(LEVEL) << "]" ; 
}




template <typename T>
unsigned QSim<T>::getBoundaryTexWidth() const 
{
    return bnd->tex->width ; 
}
template <typename T>
unsigned QSim<T>::getBoundaryTexHeight() const 
{
    return bnd->tex->height ; 
}
template <typename T>
const NP* QSim<T>::getBoundaryTexSrc() const
{
    return bnd->src ; 
}

template <typename T>
void QSim<T>::dump_photon( quad4* photon, unsigned num_photon, const char* opt_, unsigned edgeitems )
{
    LOG(LEVEL); 

    std::string opt = opt_ ; 

    bool f0 = opt.find("f0") != std::string::npos ; 
    bool f1 = opt.find("f1") != std::string::npos ; 
    bool f2 = opt.find("f2") != std::string::npos ; 
    bool f3 = opt.find("f3") != std::string::npos ; 

    bool i0 = opt.find("i0") != std::string::npos ; 
    bool i1 = opt.find("i1") != std::string::npos ; 
    bool i2 = opt.find("i2") != std::string::npos ; 
    bool i3 = opt.find("i3") != std::string::npos ; 
 

    for(unsigned i=0 ; i < num_photon ; i++)
    {
        if( i < edgeitems || i > num_photon - edgeitems)
        {
            const quad4& p = photon[i] ;  



            std::cout 
                << std::setw(10) << i 
                ; 

            if(f0) std::cout 
                << " q0.f.xyzw " 
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q0.f.x  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q0.f.y
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q0.f.z  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q0.f.w
                ;

            if(f1) std::cout 
                << " q1.f.xyzw " 
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q1.f.x  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q1.f.y
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q1.f.z  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q1.f.w
                ;

            if(f2) std::cout 
                << " q2.f.xyzw " 
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q2.f.x  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q2.f.y
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q2.f.z  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q2.f.w
                ;

            if(f3) std::cout 
                << " q3.f.xyzw " 
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q3.f.x  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q3.f.y
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q3.f.z  
                << std::setw(10) << std::fixed << std::setprecision(3) << p.q3.f.w
                ;

            if(i0) std::cout 
                << " q0.i.xyzw " 
                << std::setw(10) << p.q0.i.x  
                << std::setw(10) << p.q0.i.y
                << std::setw(10) << p.q0.i.z  
                << std::setw(10) << p.q0.i.w  
                ;

            if(i1) std::cout 
                << " q1.i.xyzw " 
                << std::setw(10) << p.q1.i.x  
                << std::setw(10) << p.q1.i.y  
                << std::setw(10) << p.q1.i.z  
                << std::setw(10) << p.q1.i.w  
                ;

            if(i2) std::cout
                << " q2.i.xyzw " 
                << std::setw(10) << p.q2.i.x  
                << std::setw(10) << p.q2.i.y  
                << std::setw(10) << p.q2.i.z  
                << std::setw(10) << p.q2.i.w  
                ;

            if(i3) std::cout
                << " q3.i.xyzw " 
                << std::setw(10) << p.q3.i.x  
                << std::setw(10) << p.q3.i.y  
                << std::setw(10) << p.q3.i.z  
                << std::setw(10) << p.q3.i.w  
                ;
      
            std::cout 
                << std::endl 
                ; 
        }
    }
}









template struct QSim<float> ; 
template struct QSim<double> ;

 
