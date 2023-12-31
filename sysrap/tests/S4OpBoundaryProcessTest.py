#!/usr/bin/env python

import os, numpy as np

from opticks.ana.fold import Fold
from opticks.ana.pvplt import *
import matplotlib.pyplot as mp

SIZE = np.array([1280, 720])
MODE=int(os.environ.get("MODE","3"))

if __name__ == '__main__':
    f = Fold.Load(symbol="f")
    print(repr(f))
    f_base = os.path.basename(f.base)


    FOLD = os.environ.get("FOLD")

    q = f.q 
    a = q[:,:3] 
    value = f.q_meta.value[0]
    valuename = f.q_meta.valuename[0]  

    label = "S4OpBoundaryProcessTest.sh :"
    label += " %s white %s:%s " % ( FOLD, valuename, value  )
    label_h = "%s:%s" % ( valuename, value  )

    if MODE == 3:
        pl = pvplt_plotter(label=label)
        pvplt_viewpoint( pl )

        pos = np.array( [[0,0,0]] )
        vec = np.array( [[0,0,1]] ) 
        pvplt_lines( pl, pos, vec )

        pl.add_points( a , color="white" )

        cpos = pl.show()

    elif MODE == 2:

        fig, ax = mp.subplots(figsize=SIZE/100.)
        fig.suptitle(label)
        ax.set_aspect("equal")

        ax.scatter( a[:,0], a[:,1], s=0.1, c="b" )
        fig.show()
         
    elif MODE == 1:

        nrm = np.array( [0,0,1], dtype=np.float32 )  ## unsmeared normal is +Z direction  
        ## dot product with Z direction picks Z coordinate 
        ## so np.arccos should be the final alpha 

        angle = np.arccos(np.dot( a, nrm )) 

        bins = np.linspace(0,0.4,100)
        angle_h = np.histogram(angle, bins=bins )[0]

        fig, ax = mp.subplots(figsize=SIZE/100.)
        fig.suptitle(label)

        ax.plot( bins[:-1], angle_h,  drawstyle="steps-post", label=label_h )

        ax.legend()
        fig.show()
    pass
pass

