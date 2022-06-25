#!/usr/bin/env python 
"""
U4RecorderTest_ab.py
======================

Usage::

   cd ~/opticks/u4/tests   # u4t
   ./U4RecorderTest_ab.sh  

"""
import numpy as np

from opticks.ana.fold import Fold
from opticks.ana.p import * 

from opticks.sysrap.xfold import XFold
from opticks.sysrap.stag import stag  
from opticks.u4.U4Stack import U4Stack

np.set_printoptions(edgeitems=16) 

tag = stag()
stack = U4Stack()

if __name__ == '__main__':

    if "A_FOLD" in os.environ:
        a = Fold.Load("$A_FOLD", symbol="a")
        A = XFold(a, symbol="A")
    else:
        a = None 
    pass

    if "B_FOLD" in os.environ:
        b = Fold.Load("$B_FOLD", symbol="b")
        B = XFold(b, symbol="B")
    else:
        b = None
    pass
    ab = (not a is None) and (not b is None)
    if ab: 
        assert (a.inphoton - b.inphoton).max() < 1e-10 
        assert np.all( A.ts == B.ts2 ) 
        assert np.all( A.ts2 == B.ts )  
    pass



