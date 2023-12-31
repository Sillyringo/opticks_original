#!/usr/bin/env python

import logging, os 
import numpy as np
log = logging.getLogger(__name__)

eary_ = lambda ekey, edef:np.array( list(map(float, os.environ.get(ekey,edef).split(","))) )
#efloat_ = lambda ekey, edef: float( os.environ.get(ekey,edef) )

from opticks.ana.eget import efloat_

X,Y,Z = 0,1,2

from opticks.ana.axes import * 


class GridSpec(object):
    @classmethod
    def DemoPeta(cls, ix0=-2, ix1=2, iy0=-2, iy1=2, iz0=0, iz1=0, photons_per_genstep=100):
        peta = np.zeros( (1,4,4), dtype=np.float32 )
        peta.view(np.int32)[0,0] = [ix0,ix1,iy0,iy1]
        peta.view(np.int32)[0,1] = [iz0,iz1,photons_per_genstep, 0]
        ce = [ -492.566,  -797.087, 19285.   ,   264.   ]
        peta[0,2] = ce
        return peta 


    def __init__(self, peta, gsmeta ):
        """
        :param peta:
        :param gsmeta:

        

        """
        moi = gsmeta.find("moi:", None)
        midx = gsmeta.find("midx:", None)
        mord = gsmeta.find("mord:", None)
        iidx = gsmeta.find("iidx:", None)

        coords = "RTP" if not iidx is None and int(iidx) == -3 else "XYZ"   ## NB RTP IS CORRECT ORDERING radiusUnitVec:thetaUnitVec:phiUnitVec
        log.info(" moi %s midx %s mord %s iidx %s coords %s " % (moi, midx, mord, iidx, coords))


        ## CSGOptiX::setCEGS tests/CSGOptiXSimtraceTest.cc

        ix0,ix1,iy0,iy1 = peta[0,0].view(np.int32)
        iz0,iz1,photons_per_genstep,_ = peta[0,1].view(np.int32)
        gridscale = peta[0,1,3]

        ce = tuple(peta[0,2])
        sce = (" %7.2f" * 4 ) % ce

        assert photons_per_genstep != 0
        nx = (ix1 - ix0)//2
        ny = (iy1 - iy0)//2
        nz = (iz1 - iz0)//2

        log.info(" ix0 %d ix1 %d nx %d  " % (ix0, ix1, nx)) 
        log.info(" iy0 %d iy1 %d ny %d  " % (iy0, iy1, ny)) 
        log.info(" iz0 %d iz1 %d nz %d  " % (iz0, iz1, nz)) 
        log.info(" gridscale %10.4f " % gridscale )
        log.info(" sce %s " % sce )

        # below default from envvars are overridden for planar data
        eye = eary_("EYE","1.,1.,1.")

        #look = eary_("LOOK","0.,0.,0.")
        look = ce[:3]

        up  = eary_("UP","0.,0.,1.")
        off  = eary_("OFF","0.,0.,1.")


        EYES = efloat_("EYES", "6.")   # TODO: why 6 ? how to control FOV to gain more control of this


        axes = self.determine_axes(nx, ny, nz)
        planar = len(axes) == 2 

        if planar:
            H, V = axes
            axlabels =  coords[H], coords[V]
            HV = "%s%s" % (coords[H],coords[V])
            up  = Axes.Up(H,V)
            off = Axes.Off(H,V)
            eye = look + ce[3]*off*EYES

        else:
            H, V, D = axes
            HV = None 
            axlabels =  coords[H], coords[V], coords[D]
            up = XYZ.up
            off = XYZ.off
            ## hmm in 3D case makes less sense : better to just use the input EYE

            eye = ce[3]*eye*EYES 

            pass
        pass
        log.info(" planar %d  eye %s  EYES %s " % (planar, str(eye), EYES))



        self.coords = coords
        self.eye = eye
        self.look = look 
        self.up  = up
        self.off = off
        self.HV = HV 
        self.peta = peta 
        self.gsmeta = gsmeta

        self.axes = axes
        self.planar = planar
        self.axlabels = axlabels

        self.nx = nx
        self.ny = ny
        self.nz = nz
        self.ce = ce
        self.sce = sce
        self.thirdline = " ce: " + sce 
        self.photons_per_genstep = photons_per_genstep


    def pv_compose(self, pl ):
        """
        :param pl: pyvista plotter instance
        :param reset: for reset=True to succeed to auto-set the view, must do this after add_points etc.. 

        Note for greater control of the view it is better to use reset=False
        """

        PARA = "PARA" in os.environ 
        RESET = "RESET" in os.environ 
        ZOOM = efloat_("ZOOM", "1.")
        
        #eye = look + self.off

        look = self.look 
        eye = self.eye
        up = self.up

        print("pv_arrange_viewpoint look:%s eye: %s up:%s  PARA:%s RESET:%d ZOOM:%s  " % (str(look), str(eye), str(up), RESET, PARA, ZOOM ))

        if PARA:
            pl.camera.ParallelProjectionOn()
        pass

        pl.set_focus(    look )
        pl.set_viewup(   up )
        pl.set_position( eye, reset=RESET )   ## for reset=True to succeed to auto-set the view, must do this after add_points etc.. 
        pl.camera.Zoom(ZOOM)




    def __str__(self):
        return "GridSpec (nx ny nz) (%d %d %d) axes %s axlabels %s " % (self.nx, self.ny, self.nz, str(self.axes), str(self.axlabels) ) 

    def determine_axes(self, nx, ny, nz):
        """
        :param nx:
        :param nx:
        :param nx:

        With planar axes the order is arranged to make the longer axis the first horizontal one 
        followed by the shorter axis as the vertical one.  

            +------+
            |      |  nz     ->  ( Y, Z )    ny_over_nz > 1 
            +------+
               ny

            +------+
            |      |  ny     ->  ( Z, Y )    ny_over_nz < 1 
            +------+
               nz

        """
        if nx == 0 and ny > 0 and nz > 0:
            ny_over_nz = float(ny)/float(nz)
            axes = (Y,Z) if ny_over_nz > 1 else (Z,Y)
        elif nx > 0 and ny == 0 and nz > 0:
            nx_over_nz = float(nx)/float(nz)
            axes = (X,Z) if nx_over_nz > 1 else (Z,X)
        elif nx > 0 and ny > 0 and nz == 0:
            nx_over_ny = float(nx)/float(ny)
            axes = (X,Y) if nx_over_ny > 1 else (Y,X)
        else:
            axes = (X,Y,Z)
        pass
        return axes


class CrossHairs(object):
    @classmethod
    def draw(cls, pl, sz=100):

        nll = 3 ; 
        ll = np.zeros( (nll, 2, 3), dtype=np.float32 )
        ll[0,0] = (-sz,0,0)
        ll[0,1] = (sz,0,0)

        ll[1,0] = (0,-sz,0)
        ll[1,1] = (0,sz,0)

        ll[2,0] = (0,0,-sz)
        ll[2,1] = (0,0,sz)

        pl.add_points( ll.reshape(-1,3), color="magenta", point_size=16.0 )

        for i in range(len(ll)):
             pl.add_lines( ll[i].reshape(-1,3), color="blue" )
        pass  




if __name__ == '__main__':
     logging.basicConfig(level=logging.INFO)
     peta = GridSpec.DemoPeta()
     grid = GridSpec(peta)
     print(grid)


