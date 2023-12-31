#!/usr/bin/env python
"""
stats.py 
==========

Count the number of files with different entensions in each package directory 
and create an RST table presenting this.::

    cd ~/opticks/bin
    ./stats.sh 

::

    epsilon:bin blyth$ ./stats.sh 
    +---------------+----------+----------+----------+----------+----------+----------+----------+
    |            pkg|       .hh|        .h|      .hpp|       .cc|      .cpp|       .cu|       .py|
    +===============+==========+==========+==========+==========+==========+==========+==========+
    |            ana|         1|         0|         0|         3|         0|         0|       239|
    +---------------+----------+----------+----------+----------+----------+----------+----------+
    |           okop|        14|         0|         0|        10|         0|         1|         0|
    +---------------+----------+----------+----------+----------+----------+----------+----------+
    |       CSGOptiX|         2|        18|         0|        14|         0|         4|         2|
    +---------------+----------+----------+----------+----------+----------+----------+----------+
    |        cudarap|        14|         2|         0|         8|         0|         3|         0|
    +---------------+----------+----------+----------+----------+----------+----------+----------+

TODO:

0. dependency ordering of packages (see bin/CMakeLists.py)
1. presentation selection, pkg notes in right hand column  
2. consolidate .hh/.hpp and .cc/.cpp

"""
import os, numpy as np
from opticks.ana.rsttable import RSTTable


class Pkg(object):
    EXTMAP = {".hh":".hh", ".h":".hh", ".hpp":".hh" , ".cc":".cc", ".cpp":".cc", ".cu":".cu", ".py":".py" }
    EXTMAPV = ".hh .cc .cu .py".split()

    EXCLUDE = ".pyc .log .swp .txt .rst .in .old .sh .bash .cfg".split()
    EXTS = ".hh .h .hpp .cc .cpp .cu .py".split()
    def __init__(self, fold):
        names = os.listdir(fold)
        pkg = os.path.basename(fold)

        exts = {}
        exms = {}
        for name in names:
            stem, ext = os.path.splitext(name)
            if ext == "" or ext in self.EXCLUDE: continue
            if not ext in self.EXTS: print("unexpected ext %s " % ext )
            if not ext in exts: exts[ext] = 0
            exts[ext]+=1   

            exm = self.EXTMAP.get(ext, None)
            assert not exm is None 
            if not exm in exms: exms[exm] = 0
            exms[exm]+=1   
        pass

        mapped = True

        if mapped == False:
            labels = ["pkg"] + self.EXTS
            stats = np.zeros( (1+len(self.EXTS),), dtype=np.object )
            for i, ext in enumerate(self.EXTS): 
                stats[1+i] = exts.get(ext, 0)
            pass
        else:
            labels = ["pkg"] + self.EXTMAPV
            stats = np.zeros( (1+len(self.EXTMAPV),), dtype=np.object )
            for i, exm in enumerate(self.EXTMAPV): 
                stats[1+i] = exms.get(exm, 0)
            pass
        pass
        stats[0] = pkg

        pass
        self.labels = labels
        self.fold = fold
        self.pkg = pkg
        self.names = names
        self.exts = exts  
        self.exms = exms  
        self.stats = stats 

    def __repr__(self):
        return "Pkg : %3d : %15s : %s " % (len(self.names), self.pkg, repr(self.exts)) 
 

class Stats(object):
    HOME = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    def __init__(self, home=HOME):
        pkgs = [] 
        for dirpath, dirs, names in os.walk(home):
            if "CMakeLists.txt" in names:
                if dirpath == home or dirpath.find("examples") > -1 or dirpath.find("tests") > 1: continue
                pkg = Pkg(dirpath) 
                pkgs.append(pkg)
            pass 
        pass
        labels = pkgs[0].labels
        stats = np.zeros( ( len(pkgs), len(labels) ), dtype=np.object )
        for i, pkg in enumerate(pkgs):
            stats[i] = pkg.stats
        pass
        self.labels = labels
        self.pkgs = pkgs
        self.stats = stats
    pass
    def __str__(self):
        return RSTTable.Rdr(self.stats, self.labels, rfm="%10d", left_wid=15, left_rfm="%15s", left_hfm="%15s" )
    def __repr__(self):
        return "\n".join(list(map(repr, self.pkgs)))
    def save(self, path):
        np.save(path, self.stats)
    pass

if __name__ == '__main__':
    st = Stats()
    st.save("/tmp/stats.npy")
    print(st)
     

