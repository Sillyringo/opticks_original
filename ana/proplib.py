#!/usr/bin/env python
#
# Copyright (c) 2019 Opticks Team. All Rights Reserved.
#
# This file is part of Opticks
# (see https://bitbucket.org/simoncblyth/opticks).
#
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.  
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
# See the License for the specific language governing permissions and 
# limitations under the License.
#

"""
PropLib : Geocache Access
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*PropLib* provides access to optical properties of materials and surfaces 
as a function of wavelength.

Material properties::

    REFRACTIVE_INDEX 
    ABSORPTION_LENGTH 
    SCATTERING_LENGTH 
    REEMISSION_PROB 

Surface properties::

    DETECT 
    ABSORB 
    REFLECT_SPECULAR 
    REFLECT_DIFFUSE 


Example data shapes::

    In [5]: mlib.data.shape
    Out[5]: (38, 2, 39, 4)

    In [6]: slib.data.shape
    Out[6]: (48, 2, 39, 4)

    In [7]: blib.data.shape
    Out[7]: (123, 4, 2, 39, 4)


"""
import os, logging, numpy as np
import sys, codecs
b_ = lambda _:codecs.latin_1_encode(_)[0] if sys.version_info.major > 2 else _
log = logging.getLogger(__name__)


from opticks.ana.base import stamp_
from opticks.ana.dat import Dat
from opticks.ana.nload import np_load
from opticks.ana.key import keydir

KEYDIR = keydir()


class Bnd(object):
    def __init__(self, names):
        nam = np.zeros( (len(names),4 ), dtype="|S64")
        for i,name in enumerate(names):
            nam[i] = name.split("/")
        pass
        self.nam = nam
    
    mats = property(lambda self:np.unique(np.hstack([self.nam[:,0],self.nam[:,3]])))
    surs = property(lambda self:np.unique(np.hstack([self.nam[:,1],self.nam[:,2]])))

    def oms(self, mat):
        """bnd indices of omat"""
        return np.where(self.nam[:,0] == mat)[0]   

    def ims(self, mat):
        """bnd indices of imat"""
        return np.where(self.nam[:,3] == mat)[0]   



class PropLib(object):

    # 1st set of 4 [0] 
    M_REFRACTIVE_INDEX = 0,0
    M_ABSORPTION_LENGTH = 0,1
    M_SCATTERING_LENGTH = 0,2
    M_REEMISSION_PROB = 0,3

    L_GROUP_VELOCITY = 1,0

    M_LABELS = "RINDEX ABSLEN RAYLEIGH REEMPROB GROUPVEL".split()


    # 2nd set of 4 [0] startswith GROUPVEL, currently not used

    S_DETECT = 0,0 
    S_ABSORB = 0,1 
    S_REFLECT_SPECULAR = 0,2 
    S_REFLECT_DIFFUSE = 0,3 

    B_OMAT = 0
    B_OSUR = 1
    B_ISUR = 2
    B_IMAT = 3
     
    COARSE_DOMAIN = np.linspace(60.,820., 39)
    FINE_DOMAIN = np.linspace(60., 820., 761) 

    @classmethod
    def load_GBndLib(cls, base):
        t, t_paths = np_load(base,"GBndLib/GBndLib.npy")
        o, o_paths = np_load(base,"GBndLib/GBndLibOptical.npy")
        if t is None or o is None:
            log.warning("missing GBndLib data : cannot create blib Proplib")
            blib = None
        else:
            blib = cls("GBndLib", data=t, names=os.path.join(base,"GItemList/GBndLib.txt"), optical=o )
            ## hmm GBndLib.txt no longer exists ? presumably due top more dynamic nature of boundaries?
        pass
        return blib 

    def __init__(self, kls="GMaterialLib", data=None, names=None, optical=None):
        """
        :param kls:
        :param data: 
        :param names:

        *data* and *names* arguments can take several types:

        None
            load standard paths
        str 
            load the given path
        ndarray
             for data, adopt the array
        list 
             for names, adopt the list

        """
        self.kls = kls
        self.paths = []
        log.info("names : %s " % names)

        if names is None:
            npath=os.path.join(KEYDIR, "GItemList/%(kls)s.txt" % locals())
        elif type(names) is str:
            npath = os.path.expandvars(names)
        elif type(names) is np.ndarray or type(names) is list:
            npath = None
        pass 

        log.info("npath : %s " % npath)
        if npath is None:
            log.warning("direct names override")
        else:  
            self.paths.append(npath)
            names = list(map(lambda _:_[:-1],open(npath,"r").readlines()))
        pass
        log.info("names : %s " % names)
        self.names = names

        if data is None:
            dpath=os.path.join(KEYDIR, "%(kls)s/%(kls)s.npy" % locals())
        elif type(data) is str:
            dpath = data
        elif type(data) is np.ndarray:
            dpath = None
        pass

        if dpath is None:
            log.warning("direct data override")
            data_ = data
        else:     
            dpath = os.path.expandvars(dpath)
            self.paths.append(dpath)
            data_ = np.load(dpath)
        pass

        if len(self.names) != data_.shape[0]:
            data = data_.reshape(-1,4,2,data_.shape[0],4)
            log.warning("reshaped %s from  %r -> %r  " % (dpath, data_.shape, data.shape) )
        else:
            data = data_
        pass 
        self.data = data
        #self.domain = self.COARSE_DOMAIN
        self.domain = self.FINE_DOMAIN

        assert len(self.names) == self.data.shape[0]
        pass

        if kls == "GBndLib":
            if optical is None:
                opticalpath = idp_("%(kls)s/%(kls)sOptical.npy" % locals())
                self.optical = np.load(opticalpath)
                self.paths.append(opticalpath)
            else:
                self.optical = optical  
            pass
            self.dat = Dat(data, names, "omat osur isur imat".split(), "g0 g1".split() )
            self.bnd = Bnd(names) 
        pass

    brief = property(lambda self:"%s" % ( self.kls ))


    def _set_names(self, names):
        """
        ndarray has advantage of easy grabbing masks and index lists
        """
        self._names = np.zeros( (len(names)), dtype="|S128")   # 64 truncates
        self._names[:] = names
    def _get_names(self):
        return self._names
    names = property(_get_names, _set_names)


    def __repr__(self):
        return "\n".join([self.brief] + self.paths + map(stamp_, self.paths)) 

    def index(self, name):
        names = list(map(lambda _:_.decode("utf-8"), self.names ))  
        return names.index(name)
        #return np.where( self._names == b_(name) )[0][0]

    def interp(self, name, wavelengths, jl):
        """
        :param name: eg material name "Water"
        :param wavelengths: values or array
        :param jl: (j,l) tuple  

                           i  j   k  l 
        data shape is eg (17, 2, 39, 4) 
               
        i: material index 0:16
        j: 0:1 property group 
        k: 0:38 wavelenth samples
        l: 0:3 property from the "float4" 

        """
        j,l = jl

        #bname = b_(name)
        #if not bname in self.names:
        #    log.fatal("bname %s is not in list %s " % (bname, self.names))
        #pass
        #idx = list(self.names).index(bname)
        idx = self.index(name)
        return np.interp( wavelengths, self.domain, self.data[idx,j,:,l] ) 
 
    def __call__(self, name):
        idx = self.index(name)
        return self.data[idx]

    def as_mlib(self, order=None):
        return self.asmateriallib(self, order=order)
   
    @classmethod
    def asmateriallib(cls, blib, order=None):
        """
        :param blib: boundary lib
        :return mlib: material lib 

        Convert a boundary lib with shape like (123, 4, 2, 39, 4)
        into material lib with shape like (38, 2, 39, 4)

        Duplicated material property values from 
        boundary buffer are collapsed into material buffer layout, 
        checking that all occurences of the material in the boundary buffer 
        are exact matches.
        """
        assert blib.kls == "GBndLib"
        bsh = blib.data.shape
        assert len(bsh) == 5 
        names = blib.bnd.mats 

        if not order is None:
            order = map(str, order)  # convert from ndarray to simple list
            names = sorted(names, key=lambda name:order.index(name) if name in order else len(order))
        pass

        mdat = np.zeros( ( len(names), bsh[2], bsh[3], bsh[4] ), dtype=np.float32 ) 

        for imat,mat in enumerate(names):
            oms = blib.bnd.oms(mat)   # indices of omat
            ims = blib.bnd.ims(mat)   # indices of imat

            for om in oms:
                bom = blib.data[om,0]

                if np.all(mdat[imat] == 0.):
                    mdat[imat] = bom 
                else:
                    assert np.all(mdat[imat] == bom)
                pass
            pass

            for im in ims:
                bim = blib.data[im,3]

                if np.all(mdat[imat] == 0.):
                    mdat[imat] = bim 
                else:
                    assert np.all(mdat[imat] == bim)
                pass
            pass
        pass
        return cls("GMaterialLib", data=mdat, names=names)




if __name__ == '__main__':
    from opticks.ana.main import opticks_main
    ok = opticks_main()

    log.info("---")

    mlib = PropLib("GMaterialLib") 
    slib = PropLib("GSurfaceLib") 

    idx = mlib.index("Water")
    print("idx:%s " % idx)

    m1 = "Water"
    wavelength = 442  
    ri = mlib.interp(m1, wavelength, mlib.M_REFRACTIVE_INDEX) 

    print("m1 %s wl %s ri %s " % (m1, wavelength, ri))

   
    # not working , perhaps due to the move to dynamic long ago 
    #blib = PropLib("GBndLib") 
    #op = blib.optical
    #print("op:%s" % op)


