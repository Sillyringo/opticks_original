opticks_python_machinery
=============================


PYTHONPATH
-------------


First guess is wrong::

    N[blyth@localhost opticks]$ export PYTHONPATH=$PYTHONPATH:$JUNOTOP/opticks
    N[blyth@localhost opticks]$ ./ana/GParts.py 
    Traceback (most recent call last):
      File "./ana/GParts.py", line 5, in <module>
        from opticks.ana.key import keydir
    ModuleNotFoundError: No module named 'opticks'

Need to be one above, also possibly better to set it to installed location::

    N[blyth@localhost opticks]$ export PYTHONPATH=$PYTHONPATH:$JUNOTOP
    N[blyth@localhost opticks]$ ./ana/GParts.py 
    Traceback (most recent call last):
      File "./ana/GParts.py", line 48, in <module>
        gp = GParts(kd)
      File "./ana/GParts.py", line 17, in __init__
        solid = Solid(d, kd)
      File "/data/blyth/junotop/opticks/ana/prim.py", line 354, in __init__
        self.prims = self.get_prims()
      File "/data/blyth/junotop/opticks/ana/prim.py", line 367, in get_prims
        p = Prim(primIdx, prim_item, self)  
      File "/data/blyth/junotop/opticks/ana/prim.py", line 262, in __init__
        self.parts = list(map(lambda _:Part(_,trans_,d, self), parts_))   ## note that every part gets passed all the trans_ need to use the gt to determine which one to use
      File "/data/blyth/junotop/opticks/ana/prim.py", line 262, in <lambda>
        self.parts = list(map(lambda _:Part(_,trans_,d, self), parts_))   ## note that every part gets passed all the trans_ need to use the gt to determine which one to use
      File "/data/blyth/junotop/opticks/ana/prim.py", line 93, in __init__
        tcn = CSG_.desc(tc)   ## typename 
      File "/data/blyth/junotop/opticks/sysrap/OpticksCSG.py", line 52, in desc
        return kvs[0][0] if len(kvs) == 1 else "UNKNOWN"
    TypeError: object of type 'filter' has no len()
    N[blyth@localhost opticks]$ 


After update of the generated sysrap/OpticksCSG.py::


    N[blyth@localhost opticks]$ export PYTHONPATH=$PYTHONPATH:$JUNOTOP
    N[blyth@localhost opticks]$ ./ana/GParts.py 
    Solid 0 : /tmp/blyth/opticks/GParts/0 : primbuf (3084, 4) partbuf (17346, 4, 4) tranbuf (7917, 3, 4, 4) idxbuf (3084, 4) 
    Solid 1 : /tmp/blyth/opticks/GParts/1 : primbuf (5, 4) partbuf (7, 4, 4) tranbuf (5, 3, 4, 4) idxbuf (5, 4) 
    Solid 2 : /tmp/blyth/opticks/GParts/2 : primbuf (7, 4) partbuf (33, 4, 4) tranbuf (16, 3, 4, 4) idxbuf (7, 4) 
    Solid 3 : /tmp/blyth/opticks/GParts/3 : primbuf (7, 4) partbuf (61, 4, 4) tranbuf (21, 3, 4, 4) idxbuf (7, 4) 
    Solid 4 : /tmp/blyth/opticks/GParts/4 : primbuf (4, 4) partbuf (24, 4, 4) tranbuf (12, 3, 4, 4) idxbuf (4, 4) 
    Solid 5 : /tmp/blyth/opticks/GParts/5 : primbuf (1, 4) partbuf (1, 4, 4) tranbuf (1, 3, 4, 4) idxbuf (1, 4) 
    Solid 6 : /tmp/blyth/opticks/GParts/6 : primbuf (1, 4) partbuf (31, 4, 4) tranbuf (9, 3, 4, 4) idxbuf (1, 4) 
    Solid 7 : /tmp/blyth/opticks/GParts/7 : primbuf (1, 4) partbuf (3, 4, 4) tranbuf (1, 3, 4, 4) idxbuf (1, 4) 
    Solid 8 : /tmp/blyth/opticks/GParts/8 : primbuf (1, 4) partbuf (31, 4, 4) tranbuf (11, 3, 4, 4) idxbuf (1, 4) 
    Solid 9 : /tmp/blyth/opticks/GParts/9 : primbuf (130, 4) partbuf (130, 4, 4) tranbuf (130, 3, 4, 4) idxbuf (130, 4) 
    N[blyth@localhost opticks]$ 



sysrap-csg-generate is still not automated : need to run it after adding primitives or jumping python
--------------------------------------------------------------------------------------------------------

Looks like the below is not automated::

    105 
    106 sysrap-csg-generate()
    107 {
    108     local msg="$FUNCNAME : "
    109     local iwd=$PWD
    110     sysrap-cd
    111     c_enums_to_python.py OpticksCSG.h
    112 
    113     echo $msg To write above generated python to OpticksCSG.py ..
    114 
    115     local ans
    116     read -p "Enter YES ... " ans
    117 
    118     if [  "$ans" == "YES" ]; then
    119        c_enums_to_python.py OpticksCSG.h > OpticksCSG.py
    120 
    121        echo $msg checking the generated python is valid 
    122        python  OpticksCSG.py
    123 
    124     else
    125        echo $msg SKIP
    126     fi
    127 
    128     cd $iwd
    129 }
    130 



