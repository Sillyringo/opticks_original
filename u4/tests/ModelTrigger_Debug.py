#!/usr/bin/env python

import os, logging, numpy as np
log = logging.getLogger(__name__)

MODE = int(os.environ.get("MODE","2")) 
PIDX = int(os.environ.get("PIDX","-1")) 
N = int(os.environ.get("VERSION","-1")) 
 
from opticks.ana.fold import Fold, AttrBase


class ModelTrigger_Debug(AttrBase):
    @classmethod
    def Create(cls, t, symbol="mtd", prefix="", publish=False):
        meta = t.ModelTrigger_Debug_meta if hasattr(t, 'ModelTrigger_Debug_meta') else None
        mtd = t.ModelTrigger_Debug  if hasattr(t, 'ModelTrigger_Debug' ) else None
        incomplete = (meta is None) or (mtd is None)
        return None if incomplete else cls(t, symbol=symbol, prefix=prefix, publish=publish) 
        
    def __init__(self, t, symbol="mtd", prefix="", publish=False ):
        """
        :param t: Fold instance
        :param prefix: prefix for symbols planted into global scope
        :param publish: symbols into global scope
        """
        AttrBase.__init__(self, symbol=symbol, prefix=prefix, publish=publish)

        meta = t.ModelTrigger_Debug_meta
        mtd = t.ModelTrigger_Debug 
        
        if mtd is None or mtd.shape[0] == 0:
            log.fatal("loaded SEvt has missing or empty ModelTrigger_Debug mtd.shape:%s" % (str(mtd.shape)))
            log.fatal("Ensure are using SEvt created whilst running with the old FastSim POM active, eg with N=0 ")
        pass   


        PV = np.array(meta.PV) 
        MLV = np.array(meta.MLV) 
        WAI = np.array( ["OutOfRegion", "kInGlass   ", "kInVacuum  ", "kUnset     "] )

        IMPL = meta.IMPL[0]   

        pos  = mtd[:,0,:3]
        time = mtd[:,0,3]

        dir  = mtd[:,1,:3]
        energy = mtd[:,1,3]


        dist1 = mtd[:,2,0]
        dist2 = mtd[:,2,1]
        mlv_   = mtd[:,2,2].view(np.uint64) 
        etrig = mtd[:,2,3].view("|S8") 


        index     = mtd[:,3,0].view(np.uint64)  # photon index for each candidate ModelTrigger
        pv_       = mtd[:,3,1].view(np.uint64) 
        whereAmI_ = mtd[:,3,2].view(np.uint64) 
        trig      = mtd[:,3,3].view(np.uint64) 

        next_pos  = mtd[:,4,:3]
        next_mct  = mtd[:,4,3]

        next_norm = mtd[:,5,:3]
        impl      = mtd[:,5,3].view(np.uint64)

        EInside1  = mtd[:,6,0].view(np.uint64)
        s61       = mtd[:,6,1].view(np.uint64)
        s62       = mtd[:,6,2].view(np.uint64)
        s63       = mtd[:,6,3].view(np.uint64)

        mlv = MLV[mlv_]
        pv  = PV[pv_]
        whereAmI = WAI[whereAmI_]

        dist2[dist2 == 9e99] = np.inf  ## avoid obnoxious 9e99 kInfinity
        dist1[dist1 == 9e99] = np.inf

        #tr = np.array([[0.000,0.000,-1.000,0.000],[0.000,1.000,0.000,0.000],[1.000,0.000,0.000,0.000],[-250.000,0.000,0.000,1.000]],dtype=np.float64)
        tr = np.eye(4) 

        lpos = np.ones( (len(pos),4) )
        lpos[:,:3] = pos

        ldir = np.zeros( (len(dir),4) )
        ldir[:,:3] = dir

        gpos = np.dot( lpos, tr )  
        gdir = np.dot( ldir, tr )

        lnext_pos = np.ones( (len(next_pos),4) )
        lnext_pos[:,:3] = next_pos

        lnext_norm = np.zeros( (len(next_norm),4) )
        lnext_norm[:,:3] = next_norm

        gnext_pos  = np.dot( lnext_pos , tr )  
        gnext_norm = np.dot( lnext_norm, tr )


        #for k, v in locals().items():print("        self.%s = %s" % (k,k))
        #
        # generate the below with the above line
        # attempts to automate this cause weird ipython crashes 
        #so resort to code generation        
 
        self.PV = PV
        self.MLV = MLV
        self.WAI = WAI
        self.mtd = mtd
        self.meta = meta
        self.IMPL = IMPL
        self.pos = pos
        self.time = time
        self.dir = dir
        self.energy = energy
        self.dist1 = dist1
        self.dist2 = dist2
        self.mlv_ = mlv_
        self.etrig = etrig
        self.index = index
        self.pv_ = pv_
        self.whereAmI_ = whereAmI_
        self.trig = trig
        self.next_pos = next_pos
        self.next_mct = next_mct
        self.next_norm = next_norm
        self.impl = impl
        self.mlv = mlv
        self.pv = pv
        self.whereAmI = whereAmI
        self.tr = tr
        self.lpos = lpos
        self.ldir = ldir
        self.gpos = gpos
        self.gdir = gdir
        self.lnext_pos = lnext_pos
        self.lnext_norm = lnext_norm
        self.gnext_pos = gnext_pos
        self.gnext_norm = gnext_norm
        self.EInside1 = EInside1
        self.s61 = s61
        self.s62 = s62
        self.s63 = s63


    def __str__(self):
        mtd = self 
        lines = []

        expr = "np.c_[mtd.index, mtd.whereAmI, mtd.trig, mtd.etrig, mtd.pv, mtd.mlv][mtd.index == PIDX]"
        lines.append("\n%s ## ModelTrigger_Debug mlv and pv for PIDX " % expr)
        lines.append(repr(eval(expr)))

        expr = " np.c_[mtd.index, mtd.pos[:,2],mtd.time, mtd.gpos[:,:3], mtd.gdir[:,:3], mtd.dist1, mtd.dist2][mtd.index == PIDX] "
        lines.append("\n%s ## ModelTrigger_Debug for PIDX " % expr)
        lines.append(repr(eval(expr)))

        return "\n".join(lines)



if __name__ == '__main__':
    t = Fold.Load(symbol="t")
    print(repr(t))
   
    print("MODE:%d" % (MODE) )
    print("PIDX:%d" % (PIDX) )
    print("N:%d" % (N) )

    mtd = ModelTrigger_Debug(t, symbol="mtd", publish=False)  # publish:True is crashing  
    print(mtd)
