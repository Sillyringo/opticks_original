#!/usr/bin/env python
"""
U4SimulateTest_fs.py  : FastSim ModelTrigger etc.. plotting
===============================================================


"""
import os, numpy as np
from opticks.ana.fold import Fold
from opticks.ana.p import * 

MODE = int(os.environ.get("MODE", 0))
if MODE > 0:
    from opticks.ana.pvplt import * 
pass




if __name__ == '__main__':
    t = Fold.Load(symbol="t")
    print(repr(t))

    uu = t.U4Recorder_G4StateRerun_726   

    fs = t.SFastSim_Debug   
    kInfinity = 9.000e+99    

    pos = fs[:,0,:3]
    tim = fs[:,0,3]

    mom = fs[:,1,:3]
    ds1 = fs[:,1,3]

    pol = fs[:,2,:3]
    ds2 = fs[:,2,3]

    trg = fs[:,3,0].astype(np.int64)  ## NB not ".view", as here wasting bits 
    wai = fs[:,3,1].astype(np.int64)
    c   = fs[:,3,2]
    pid = fs[:,3,3].astype(np.int64)

    ms1 = np.where(ds1 == kInfinity )[0]
    ht1 = np.where(ds1 != kInfinity )[0]
    ms2 = np.where(ds2 == kInfinity )[0]
    ht2 = np.where(ds2 != kInfinity )[0]
    
    #  enum EWhereAmI { OutOfRegion, kInGlass, kInVacuum };
    glass      = wai == 1 
    vacuum     = wai == 2 
    trg_no     = trg == 0 
    trg_yes    = trg == 1 
    glass_no   = np.logical_and( wai == 1, trg == 0 )
    glass_yes  = np.logical_and( wai == 1, trg == 1 ) 
    vacuum_no  = np.logical_and( wai == 2, trg == 0 )
    vacuum_yes = np.logical_and( wai == 2, trg == 1 )  


    PID = int(os.environ.get("PID", -1))
    if PID == -1: PID = int(os.environ.get("OPTICKS_G4STATE_RERUN", -1))

    pidsel = np.where( pid == PID )[0]  
    #pidsel = np.where( pid == PID )[0]  
    num_pidsel = len(pidsel)  
    print(" PID %d pidsel %s num_pidsel %d " % (PID, str(pidsel), num_pidsel))

    if PID > -1 and num_pidsel > 0:
        extra = np.zeros( [num_pidsel,4,4], dtype=np.float64 )
        extra[:] = fs[pidsel, :4]                   # copy, not reference, fs 
        extra[np.where( extra == kInfinity )] = -1  # replace obnoxious 9e99

        pidsel_path = os.path.expandvars("$FOLD/py_pidsel_%d.npy" % PID)
        dirpath = os.path.dirname(pidsel_path)
        if not os.path.isdir(dirpath):
            os.makedirs(dirpath)
        pass

        print("Save PID %d extra %s to pidsel_path %s (used from xxv.sh) " % (PID, str(extra.shape), pidsel_path))
        np.save(pidsel_path, extra )

        ModelTrigger = extra[np.where(extra[:,3,0].astype(np.int64) == 1)]   
    pass


    selnames = "glass vacuum trg_no trg_yes glass_no glass_yes vacuum_no vacuum_yes".split()
    for selname in selnames:
        sel = locals()[selname]
        print(" %20s : %s " % (selname, np.count_nonzero(sel) ))
    pass

    #specs = "blue:trg_no:lines cyan:trg_yes:lines"
    #specs = "blue:glass_no:lines cyan:glass_yes:lines"
    specs = "blue:glass_no:lines cyan:glass_yes:lines red:vacuum_yes:lines yellow:vacuum_no:lines"
    label = "U4SimulateTest_fs.py:SFastSim_Debug " + specs

    if MODE == 0:
        print("not plotting as MODE 0  in environ")
    else:
        pl = pvplt_plotter(label)
        os.environ["EYE"] = "0,100,165"
        os.environ["LOOK"] = "0,0,165"
        pvplt_viewpoint(pl)

        for spec in specs.split():
            color,selname,stylename = spec.split(":")    
            sel = locals()[selname]
            numsel = np.count_nonzero(sel) 
            if numsel == 0: continue

            if stylename == "lines":
                pvplt_lines(pl, pos[sel], mom[sel], color=color, factor=10 )
            elif stylename == "points":
                pl.add_points( pos[sel], color=color )
            else:
                pass
            pass
        pass
        pl.show()
    pass
pass
