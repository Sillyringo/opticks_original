#!/usr/bin/env python

import os

efloat_ = lambda ekey, fallback:float(os.environ.get(ekey,fallback))
efloatlist_ = lambda ekey,fallback:list(map(float, filter(None, os.environ.get(ekey,fallback).split(","))))

eint_ = lambda ekey, fallback:int(os.environ.get(ekey,fallback))

def eintlist_(ekey, fallback):
    """ 
    empty string envvar yields None
    """
    slis = os.environ.get(ekey,fallback)
    if slis is None or len(slis) == 0: return None
    slis = slis.split(",")
    return list(map(int, filter(None, slis)))



if __name__ == '__main__':

    tmin0 = efloat_("TMIN",0.5)
    tmin1 = efloat_("TMIN","0.5")
    assert tmin0 == tmin1


    eye0 = efloatlist_("EYE", "1,-1,1")
    print("%10.4f %10.4f %10.4f " % tuple(eye0) )

    