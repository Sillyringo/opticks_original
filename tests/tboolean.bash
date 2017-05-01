tboolean-source(){   echo $(opticks-home)/tests/tboolean.bash ; }
tboolean-vi(){       vi $(tboolean-source) ; }
tboolean-usage(){ cat << \EOU

tboolean- 
======================================================


TODO
--------

* CSG geometry config using python that writes
  a serialization turns out to be really convenient...
  Howabout attaching emission of torch photons to pieces
  of geometry ?

* Also there is lots of duplication in torchconfig
  between all the tests... pull out the preparation of 
  torchconfig metadata into a python script ? that 
  all the tests can use ? 

  Hmm but the sources need to correspond to geometry ...
  this needs an overhaul.
  
  

NOTES
--------

tracetest option
~~~~~~~~~~~~~~~~~~~

When using tracetest option only a single intersect is
done using oxrap/cu/generate.cu:tracetest and a special 
format of the photon buffer is used, for analysis by 
ana/tboolean.py 

However in tracetest mode the record buffer filling 
is not implemented so the visualization 
of photon paths is not operational.


bash test geometry configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* CSG tree is defined in breadth first or level order

* parameters of boolean operations currently define adhoc box 
  intended to contain the geometry, TODO: calculate from bounds of the contained tree 

* offsets arg identifies which nodes belong to which primitives by pointing 
  at the nodes that start each primitive

::

     1  node=union        parameters=0,0,0,400           boundary=Vacuum///$material 
     2  node=difference   parameters=0,0,100,300         boundary=Vacuum///$material
     3  node=difference   parameters=0,0,-100,300        boundary=Vacuum///$material
     4  node=box          parameters=0,0,100,$inscribe   boundary=Vacuum///$material
     5  node=sphere       parameters=0,0,100,$radius     boundary=Vacuum///$material
     6  node=box          parameters=0,0,-100,$inscribe  boundary=Vacuum///$material
     7  node=sphere       parameters=0,0,-100,$radius    boundary=Vacuum///$material

Perfect tree with n=7 nodes is depth 2, dev/csg/node.py (root2)::
 
                 U1                
                  o                
         D2              D3        
          o               o        
     b4      s5      b6      s7    
      o       o       o       o         


* nodes identified with 1-based levelorder index, i
* left/right child of node i at l=2i, r=2i+1, so long as l,r < n + 1


python test geometry configuration 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* python config is much more flexible than bash, allowing 
  more natural tree construction and node reuse

Running functions such as tboolean-box-sphere-py- 
construct CSG node trees for each "solid" of the geometry.
Typically the containing volume is a single node tree 
and the contained volume is a multiple node CSG tree.

These trees are serialized into numpy arrays and written to 
files within directories named after the bash function eg 
"/tmp/blyth/opticks/tboolean-box-sphere-py-". 

The bash function emits to stdout only the name of 
this directory which is captured and used in the 
commandline testconfig csgpath slot.


testconfig modes
~~~~~~~~~~~~~~~~~~

PmtInBox

     * see tpmt- for this one

BoxInBox

     * CSG combinations not supported, union/intersection/difference nodes
       appear as placeholder boxes

     * raytrace superficially looks like a union, but on navigating inside 
       its apparent that its just overlapped individual primitives


PyCsgInBox

     * requires csgpath identifying directory containing serialized CSG trees
       and csg.txt file with corresponding boundary spec strings


CsgInBox

     * DECLARED DEAD, USE PyCsgInBox
     * requires "offsets" identifying node splits into primitives eg offsets=0,1 
     * nodes are specified in tree levelorder, trees must be perfect 
       with 1,3,7 or 15 nodes corresponding to trees of height 0,1,2,3

EOU
}

tboolean-env(){      olocal- ;  }
tboolean-dir(){ echo $(opticks-home)/tests ; }
tboolean-cd(){  cd $(tboolean-dir); }

join(){ local IFS="$1"; shift; echo "$*"; }

tboolean-tag(){  echo 1 ; }
tboolean-det(){  echo boolean ; }
tboolean-src(){  echo torch ; }
tboolean-args(){ echo  --det $(tboolean-det) --src $(tboolean-src) ; }

tboolean--(){

    tboolean-

    local msg="=== $FUNCNAME :"
    local cmdline=$*

    local testconfig
    if [ -n "$TESTCONFIG" ]; then
        testconfig=${TESTCONFIG}
    else
        testconfig=$(tboolean-testconfig)
    fi 

    op.sh  \
            $cmdline \
            --animtimemax 10 \
            --timemax 10 \
            --geocenter \
            --eye 1,0,0 \
            --dbganalytic \
            --test --testconfig "$testconfig" \
            --torch --torchconfig "$(tboolean-torchconfig)" \
            --tag $(tboolean-tag) --cat $(tboolean-det) \
            --save 
}

tboolean-tracetest()
{
    tboolean-- --tracetest $*
}

tboolean-enum(){
   local tmp=$TMP/$FUNCNAME.exe
   clang $OPTICKS_HOME/optixrap/cu/boolean-solid.cc -lstdc++ -I$OPTICKS_HOME/optickscore -o $tmp && $tmp $*
}


tboolean-torchconfig()
{

    local pol=${1:-s}
    local wavelength=500
    local identity=1.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,0.000,1.000

    #local photons=1000000
    local photons=100000
    #local photons=1

    local torch_config_disc=(
                 type=disc
                 photons=$photons
                 mode=fixpol
                 polarization=1,1,0
                 frame=-1
                 transform=$identity
                 source=0,0,599
                 target=0,0,0
                 time=0.1
                 radius=300
                 distance=200
                 zenithazimuth=0,1,0,1
                 material=Vacuum
                 wavelength=$wavelength 
               )


    local discaxial_target=0,0,0
    local torch_config_discaxial=(
                 type=discaxial
                 photons=$photons
                 frame=-1
                 transform=$identity
                 source=$discaxial_target
                 target=0,0,0
                 time=0.1
                 radius=100
                 distance=400
                 zenithazimuth=0,1,0,1
                 material=Vacuum
                 wavelength=$wavelength 
               )

    #echo "$(join _ ${torch_config_discaxial[@]})" 
    echo "$(join _ ${torch_config_disc[@]})" 
}



#tboolean-material(){ echo MainH2OHale ; }
tboolean-material(){ echo GlassSchottF2 ; }
tboolean-container(){ echo Rock//perfectAbsorbSurface/Vacuum ; }
tboolean-testobject(){ echo Vacuum///GlassSchottF2 ; }


tboolean-bib-box()
{
    local test_config=(
                 mode=BoxInBox
                 name=$FUNCNAME
                 analytic=1

                 node=box      parameters=0,0,0,1000               boundary=$(tboolean-container)
                 node=box      parameters=0,0,0,100                boundary=$(tboolean-testobject)

                    )
     echo "$(join _ ${test_config[@]})" 
}


tboolean-bib-box-small-offset-sphere()
{
    local test_config=(
                 mode=BoxInBox
                 name=$FUNCNAME
                 analytic=1

                 node=sphere           parameters=0,0,0,1000          boundary=$(tboolean-container)
 
                 node=${1:-difference} parameters=0,0,0,300           boundary=$(tboolean-testobject)
                 node=box              parameters=0,0,0,200           boundary=$(tboolean-testobject)
                 node=sphere           parameters=0,0,200,100         boundary=$(tboolean-testobject)
               )
     echo "$(join _ ${test_config[@]})" 
}

tboolean-bib-box-sphere()
{
    local operation=${1:-difference}
    local inscribe=$(python -c "import math ; print 1.3*200/math.sqrt(3)")
    local test_config=(
                 mode=BoxInBox
                 name=$FUNCNAME
                 analytic=1

                 node=box          parameters=0,0,0,1000          boundary=$(tboolean-container)
 
                 node=$operation   parameters=0,0,0,300           boundary=$(tboolean-testobject)
                 node=box          parameters=0,0,0,$inscribe     boundary=$(tboolean-testobject)
                 node=sphere       parameters=0,0,0,200           boundary=$(tboolean-testobject)
               )

     echo "$(join _ ${test_config[@]})" 
}







tboolean-box(){ TESTCONFIG=$($FUNCNAME- 2>/dev/null)    tboolean-- ; } 
tboolean-box-(){  $FUNCNAME- | python $* ; }
tboolean-box--(){ cat << EOP 

from opticks.ana.base import opticks_main
from opticks.ana.pmt.polyconfig import PolyConfig
from opticks.dev.csg.csg import CSG  

args = opticks_main()

container = CSG("box")
container.boundary = args.container
container.meta.update(PolyConfig("CONTAINER").meta)


im = dict(poly="IM", resolution="40", verbosity="1", ctrl="0" )

#tr = dict(translate="0,0,100", rotate="1,1,1,45", scale="1,1,2")
#tr = dict(scale="2,2,2", rotate="1,1,1,45")

kwa = {}
kwa.update(im)
#kwa.update(tr)

box_param = [0,0,0,200]
box3_param = [100,200,200,0] 

box = CSG("box3", param=box3_param, boundary="$(tboolean-testobject)", **kwa )
box.dump()


CSG.Serialize([container, box], args.csgpath, outmeta=True )
EOP
}



tboolean-cone-scan(){ SCAN="0,0,100,1,0,0,0,300,10" NCSGScanTest $TMP/tboolean-cone--/1 ; }
tboolean-cone(){ TESTCONFIG=$($FUNCNAME- 2>/dev/null)    tboolean-- ; } 
tboolean-cone-(){  $FUNCNAME- | python $* ; }
tboolean-cone--(){ cat << EOP 

from opticks.ana.base import opticks_main
from opticks.ana.pmt.polyconfig import PolyConfig
from opticks.dev.csg.csg import CSG  

args = opticks_main()

container = CSG("box")
container.boundary = args.container
container.meta.update(PolyConfig("CONTAINER").meta)

im = dict(poly="IM", resolution="40", verbosity="1", ctrl="0" )

r2,r1 = 100,300
#r2,r1 = 300,300    ## with equal radii (a cylinder) polygonization and raytrace both yield nothing 
#r2,r1 = 300,100    ## radii swapped (upside-down cone) works


z2 = 200
z1 = 0

param = [r1,z1,r2,z2]
obj = CSG("cone", param=param, boundary=args.testobject, **im )
obj.dump()


CSG.Serialize([container, obj], "$TMP/$FUNCNAME", outmeta=True )
EOP
}




tboolean-trapezoid(){ TESTCONFIG=$($FUNCNAME- 2>/dev/null)    tboolean-- ; } 
tboolean-trapezoid-deserialize(){ NCSGDeserializeTest $TMP/tboolean-trapezoid-- ; }
tboolean-trapezoid-(){  $FUNCNAME- | python $* ; }
tboolean-trapezoid--(){ cat << EOP 

from opticks.ana.base import opticks_main
from opticks.ana.pmt.polyconfig import PolyConfig
from opticks.dev.csg.csg import CSG  

args = opticks_main()

container = CSG("box")
container.boundary = args.container
container.meta.update(PolyConfig("CONTAINER").meta)

im = dict(poly="IM", resolution="40", verbosity="1", ctrl="0" )

obj = CSG("trapezoid")
obj.boundary = args.testobject
obj.planes = CSG.CubePlanes(200.)
obj.meta.update(im)

obj.dump()

CSG.Serialize([container, obj], "$TMP/$FUNCNAME", outmeta=True )
EOP
}












tboolean-uniontree(){ TESTCONFIG=$($FUNCNAME- 2>/dev/null) &&  tboolean-- || echo $FUNCNAME : ERROR : investigate with : $FUNCNAME-  ; } 
tboolean-uniontree-(){  $FUNCNAME- | python $* ; }
tboolean-uniontree--(){ cat << EOP 

import numpy as np
from opticks.ana.base import opticks_main
from opticks.ana.pmt.polyconfig import PolyConfig
from opticks.dev.csg.csg import CSG  

args = opticks_main()

container = CSG("box")
container.boundary = args.container
container.meta.update(PolyConfig("CONTAINER").meta)

im = dict(poly="IM", resolution="40", verbosity="1", ctrl="0" )


sp = CSG("sphere", param=[0,0,-1,100] )   # zrange -100:100
sp2 = CSG("sphere", param=[0,0,0,200] )   # zrange -200:200

zs = CSG("zsphere", param=[0,0,0,500], param1=[100,200,0,0],param2=[0,0,0,0])  # zrange 100:200
zs.param2.view(np.uint32)[0] = 3 

co = CSG("cone", param=[300,0,100,200])   # zrange 0:200


#prim = [sp,zs,co]    
prim = [sp,co,zs]    

"""
prim = [sp,zs,co]    

            un    
     un          co
 sp      zs        


         __________
        /          \       zs
       +---+-----+--+  
          /_______\        co
             \_/           sp


Looks like improper "shadow" sphere surface inside the union,  
propagation intersects with improper surf between sphere and cone,
nudging sphere upwards makes a hole in the center of improper surface
 ... suspect issue with three way overlapping 
nudging downwards still get the improper surf


Can only see the shadow shape when positioned to look up at cone 
(ie looking into threeway region)

Changing order to a more natural (2-way overlapping) one, gets expected behavior

::

    prim = [sp,co,zs]   

                un    
         un          zs
     sp      co        


Does the order depency of a set of unions indicate a bug, 
or a limitation of the algorithm ... or is it just 
a result of having coincident z-faces ?

NON-CONCLUSION: 

* make sure uniontree primitives are in a sensible order 
* avoid three way overlapping where possible


Thinking about the pairwise CSG algorithm the behaviour is kinda
understandable... sp and zs are initially tangential and then after 
nudging the sp upwards creates a small intersection opening up the hole.

But then the union with the cone thru the middle should get rid of that 
surface, and open up a full cavity betweeb the zs and sp ?


TODO:

* see what happens when all coincidident z-planes are avoided

* automatic z-growing in polycone to avoid coincident surfaces 
  (this would be difficult in a general uniontree, but easy in 
  polycone as just needs 3*epsilon grow in z from the
  smaller radius part into the larger radius part)  
  ... analogous to joins in carpentry
          

"""

#prim = [sp2, zs, co]  # works as expected
#prim = [sp, co]       # works as expected
#prim = [sp, zs ]      #  sp just touches zp, so difficult to say 
#prim = [sp2, zs]      # expected behavior


ut = CSG.uniontree(prim, name="$FUNCNAME")
ut.boundary = args.container
ut.meta.update(im)
ut.dump()

CSG.Serialize([container, ut], "$TMP/$FUNCNAME", outmeta=True )


EOP
}








tboolean-sphere(){ TESTCONFIG=$(tboolean-csg-sphere 2>/dev/null)    tboolean-- ; } 
tboolean-csg-sphere(){ $FUNCNAME- | python $* ; } 
tboolean-csg-sphere-(){ cat << EOP 
from opticks.dev.csg.csg import CSG  

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20" )


im = dict(poly="IM", resolution="50", verbosity="1", ctrl="0" )
#tr = dict(scale="1,1,2")
tr = dict(translate="0,0,100", rotate="1,1,1,45", scale="1,1,2")

kwa = {}
kwa.update(im)
kwa.update(tr)

sphere = CSG("sphere", param=[0,0,0,100], boundary="$(tboolean-testobject)", **kwa )

CSG.Serialize([container, sphere], "$TMP/$FUNCNAME" )
EOP
}


tboolean-zsphere(){ TESTCONFIG=$(tboolean-zsphere- 2>/dev/null)    tboolean-- ; } 
tboolean-zsphere-(){ $FUNCNAME- | python $* ; } 
tboolean-zsphere--(){ cat << EOP 

import numpy as np
from opticks.dev.csg.csg import CSG  

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20" )


im = dict(poly="IM", resolution="50", verbosity="3", ctrl="0" )
#tr = dict(scale="1,1,2")
tr = dict(translate="0,0,100", rotate="1,1,1,45", scale="1,1,2")

kwa = {}
kwa.update(im)
#kwa.update(tr)

#zsphere = CSG("zsphere", param=[0,0,0,500], param1=[-200,200,0,0],param2=[0,0,0,0],  boundary="$(tboolean-testobject)", **kwa )
zsphere = CSG("zsphere", param=[0,0,0,500], param1=[100,200,0,0],param2=[0,0,0,0],  boundary="$(tboolean-testobject)", **kwa )

ZSPHERE_QCAP = 0x1 << 1   # ZMAX
ZSPHERE_PCAP = 0x1 << 0   # ZMIN
#flags = ZSPHERE_QCAP | ZSPHERE_PCAP
flags = ZSPHERE_QCAP | ZSPHERE_PCAP
#flags = ZSPHERE_QCAP
#flags = ZSPHERE_PCAP
#flags = 0 

zsphere.param2.view(np.uint32)[0] = flags 

CSG.Serialize([container, zsphere], "$TMP/$FUNCNAME" )

EOP
}




tboolean-union-zsphere(){ TESTCONFIG=$(tboolean-union-zsphere- 2>/dev/null)    tboolean-- ; } 
tboolean-union-zsphere-(){ $FUNCNAME- | python $* ; } 
tboolean-union-zsphere--(){ cat << EOP 

import numpy as np
from opticks.dev.csg.csg import CSG  

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20" )

im = dict(poly="IM", resolution="50", verbosity="3", ctrl="0" )

kwa = {}
kwa.update(im)


ZSPHERE_QCAP = 0x1 << 1   # ZMAX
ZSPHERE_PCAP = 0x1 << 0   # ZMIN
flags = ZSPHERE_QCAP | ZSPHERE_PCAP

lzs = CSG("zsphere", param=[0,0,0,500], param1=[-200,200,0,0],param2=[0,0,0,0] )
lzs.param2.view(np.uint32)[0] = flags   

rzs = CSG("zsphere", param=[0,0,0,500], param1=[300,400,0,0] ,param2=[0,0,0,0] )
rzs.param2.view(np.uint32)[0] = flags

uzs = CSG("union", left=lzs, right=rzs, boundary="$(tboolean-testobject)", **kwa )

CSG.Serialize([container, uzs], "$TMP/$FUNCNAME" )

"""
Observe wierdness when caps are off:

* wrong sub-object appears in front of other...
* hmm maybe fundamental closed-sub-object-limitation again 

"""

EOP
}






tboolean-difference-zsphere(){ TESTCONFIG=$(tboolean-difference-zsphere- 2>/dev/null)    tboolean-- ; } 
tboolean-difference-zsphere-(){ $FUNCNAME- | python $* ; } 
tboolean-difference-zsphere--(){ cat << EOP 

import numpy as np
from opticks.dev.csg.csg import CSG  

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20" )

im = dict(poly="IM", resolution="50", verbosity="3", ctrl="0", seeds="0,0,0,1,0,0")

kwa = {}
kwa.update(im)

ZSPHERE_QCAP = 0x1 << 1   # ZMAX
ZSPHERE_PCAP = 0x1 << 0   # ZMIN
both = ZSPHERE_QCAP | ZSPHERE_PCAP

lzs = CSG("zsphere", param=[0,0,0,500], param1=[-100,100,0,0],param2=[0,0,0,0] )
lzs.param2.view(np.uint32)[0] = both   

rzs = CSG("zsphere", param=[0,0,0,400], param1=[-101,101,0,0] ,param2=[0,0,0,0] )
rzs.param2.view(np.uint32)[0] = both

dzs = CSG("difference", left=lzs, right=rzs, boundary="$(tboolean-testobject)", **kwa )

CSG.Serialize([container, dzs], "$TMP/$FUNCNAME" )

"""

#. FIXED: Differencing two concentric zspheres with same zmin/zmax does not 
   produce the expected ring like shape, unless you slightly increase the 
   zmin/zmax of the one you are subtracting over the other

   * avoid common/coincident faces between the subtracted solids 


#. FIXED: IM poly: fails to find surface even radii 400 and 500, hmm NZSphere looking in +z, 
   but manual seeding doesnt find surface either, it does after fix  
   bug in the setting of manual seed directions in NImplicitMesher


"""

EOP
}











tboolean-box-small-offset-sphere-py(){ $FUNCNAME- | python $* ; } 
tboolean-box-small-offset-sphere-py-(){ cat << EOP
from opticks.dev.csg.csg import CSG  

container = CSG("sphere",           param=[0,0,0,1000], boundary="$(tboolean-container)", poly="IM", resolution="10" )

box = CSG("box",    param=[0,0,0,200], boundary="$(tboolean-testobject)", rotate="0,0,1,45" )
sph = CSG("sphere", param=[0,0,0,100], boundary="$(tboolean-testobject)", translate="0,0,200", scale="1,1,0.5" )

object = CSG("${1:-difference}", left=box, right=sph, boundary="$(tboolean-testobject)", poly="IM", resolution="50" )

CSG.Serialize([container, object], "$TMP/$FUNCNAME" )
EOP
}




tboolean-bsu(){ TESTCONFIG=$(tboolean-csg-box-sphere-py union)        tboolean-- ; }
tboolean-bsd(){ TESTCONFIG=$(tboolean-csg-box-sphere-py difference)   tboolean-- ; }
tboolean-bsi(){ TESTCONFIG=$(tboolean-csg-box-sphere-py intersection) tboolean-- ; }
tboolean-csg-box-sphere-py(){ $FUNCNAME- $* | python  ; } 
tboolean-csg-box-sphere-py-(){ cat << EOP 
import math
from opticks.dev.csg.csg import CSG  

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20" )
  
radius = 200 
inscribe = 1.3*radius/math.sqrt(3)

box = CSG("box", param=[0,0,0,inscribe])


rtran = dict(translate="100,0,0")
sph = CSG("sphere", param=[0,0,0,radius], **rtran)

object = CSG("${1:-difference}", left=box, right=sph, boundary="$(tboolean-testobject)", poly="IM", resolution="50" )

CSG.Serialize([container, object], "$TMP/$FUNCNAME" )
EOP
}




tboolean-sphere-slab(){ TESTCONFIG=$(tboolean-csg-sphere-slab 2>/dev/null)    tboolean-- ; } 
tboolean-csg-sphere-slab(){  $FUNCNAME- | python $* ; } 
tboolean-csg-sphere-slab-(){ cat << EOP 
import numpy as np
from opticks.ana.base import opticks_main
from opticks.dev.csg.csg import CSG  
args = opticks_main()

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20" )
  
slab   = CSG("slab", param=[0,0,1,0],param1=[-500,100,0,0] )


SLAB_ACAP = 0x1 << 0
SLAB_BCAP = 0x1 << 1
flags = SLAB_ACAP | SLAB_BCAP
slab.param.view(np.uint32)[3] = flags 


sphere = CSG("sphere", param=[0,0,0,500] )

object = CSG("intersection", left=sphere, right=slab, boundary="$(tboolean-testobject)", poly="IM", resolution="50" )

CSG.Serialize([container, object], "$TMP/$FUNCNAME" )

"""

0. Works 

Why tboolean-sphere-slab raytrace is OK but tboolean-sphere-plane has directional visibility issues ?

* suspect due to "sub-objects must be closed" limitation of the  algorithm that 
  my CSG implementation is based upon: "Kensler:Ray Tracing CSG Objects Using Single Hit Intersections"

* http://xrt.wikidot.com/doc:csg

    "The [algorithm] computes intersections with binary CSG objects using the
    [nearest] intersection. Though it may need to do several of these per
    sub-object, the usual number needed is quite low. The only limitation of this
    algorithm is that the sub-objects must be closed, non-self-intersecting and
    have consistently oriented normals."

It appears can get away with infinite slab, which isnt bounded also, 
as only unbounded in "one" direction whereas half-space is much more
unbounded : in half the directions.

"""
EOP
}


tboolean-sphere-plane(){ TESTCONFIG=$(tboolean-csg-sphere-plane 2>/dev/null)    tboolean-- ; }
tboolean-csg-sphere-plane(){  $FUNCNAME- | python $* ; } 
tboolean-csg-sphere-plane-(){ cat << EOP 
from opticks.ana.base import opticks_main
from opticks.dev.csg.csg import CSG  
args = opticks_main()

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20", verbosity="0" )
  
plane  = CSG("plane",  param=[0,0,1,100] )
sphere = CSG("sphere", param=[0,0,0,500] )

object = CSG("intersection", left=sphere, right=plane, boundary="$(tboolean-testobject)", poly="IM", resolution="50", verbosity="1" )

CSG.Serialize([container, object], "$TMP/$FUNCNAME" )

"""
Exibits wierdness, unbounded sub-objects such as planes are not valid CSG sub-objects within OpticksCSG 

0. Polygonization looks correct
1. only see the sphere surface from beneath the plane (ie beneath z=100)
2. only see the plane surface in shape of disc from above the plane 

"""
EOP
}

tboolean-box-plane(){ TESTCONFIG=$(tboolean-csg-box-plane 2>/dev/null)    tboolean-- ; }
tboolean-csg-box-plane(){  $FUNCNAME- | python $* ; } 
tboolean-csg-box-plane-(){ cat << EOP 
from opticks.ana.base import opticks_main
from opticks.dev.csg.csg import CSG  
args = opticks_main()

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20", verbosity="0" )

plane  = CSG("plane",  param=[0,0,1,100] )
box    = CSG("box", param=[0,0,0,200]  )
object = CSG("intersection", left=plane, right=box, boundary="$(tboolean-testobject)", poly="IM", resolution="50", verbosity="1" )

CSG.Serialize([container, object], "$TMP/$FUNCNAME" )

"""
#. Analogous issue to tboolean-sphere-plane
"""
EOP
}



tboolean-plane(){ TESTCONFIG=$(tboolean-csg-plane 2>/dev/null)    tboolean-- ; }
tboolean-csg-plane(){ $FUNCNAME- | python $* ; } 
tboolean-csg-plane-(){ cat << EOP 
from opticks.ana.base import opticks_main
from opticks.dev.csg.csg import CSG  
args = opticks_main()

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20", verbosity="0" )

bigbox = CSG("box", param=[0,0,0,999] )
plane  = CSG("plane",  param=[0,0,1,100] )
object = CSG("intersection", left=plane, right=bigbox, boundary="$(tboolean-testobject)", poly="IM", resolution="50", verbosity="1" )

CSG.Serialize([container, object], "$TMP/$FUNCNAME" )

"""


#. An odd one, it appears OK in polygonization and raytrace : but it is breaking the rules,
   are using an unbounded sub-object (the plane) in intersection with the bigbox.

#. Actually the wierdness is there, just you there is no viewpoint from which you can see it. 
   Reducing the size of the bigbox to 500 allows it to manifest.

#. intersecting the plane with the container, leads to coincident surfaces and a flickery mess when 
   view from beneath the plane, avoided issue by intersecting instead with a bigbox slightly 
   smaller than the container

"""

EOP
}



tboolean-cylinder(){ TESTCONFIG=$(tboolean-csg-cylinder 2>/dev/null)    tboolean-- ; }
tboolean-csg-cylinder(){  $FUNCNAME- | python $* ; } 
tboolean-csg-cylinder-(){ cat << EOP 
import numpy as np
from opticks.ana.base import opticks_main
from opticks.dev.csg.csg import CSG  
args = opticks_main()

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="MC", nx="20", verbosity="0" )



kwa = {}

im = dict(poly="IM", resolution="50")
transform = dict(scale="1,1,0.5", rotate="1,1,1,45", translate="100,100,100" )

kwa["verbosity"] = "1" 
kwa.update(im)
kwa.update(transform)

cylinder = CSG("cylinder", param=[0,0,0,200], param1=[400,0,0,0], boundary="$(tboolean-testobject)", **kwa )

PCAP = 0x1 << 0  # smaller z endcap
QCAP = 0x1 << 1  

flags = PCAP          # bottom-cap(-z) 
#flags = QCAP           # top-cap(+z) 
#flags = PCAP | QCAP   # both-caps
#flags = 0             # no-caps 

cylinder.param1.view(np.uint32)[1] = flags 

CSG.Serialize([container, cylinder], "$TMP/$FUNCNAME" )

"""

Issue:

1. FIXED:seeing endcaps when would expect to see the sides of the cylinder
2. FIXED:Not honouring transforms
3. CONCLUDE_UNFIXABLE_SDF_LIMITATION:polygonization does not honour endcap flags, but raytrace does
4. FIXED:with only one endcap enabled, when look into the open side, can see straight thru the other endcap

Note that endcaps and insides of the cylinder look dark from inside: 
this is correct as normals are rigidly attached to geometry pointing outwards.

"""
EOP
}





tboolean-unbalanced(){   TESTCONFIG=$(tboolean-unbalanced-)  tboolean-- ; }
tboolean-unbalanced-(){ $FUNCNAME- | python $*  ; }
tboolean-unbalanced--()
{
    local material=$(tboolean-material)
    local base=$TMP/$FUNCNAME 
    cat << EOP 
import math, logging
log = logging.getLogger(__name__)
from opticks.ana.base import opticks_main
from opticks.dev.csg.csg import CSG  
args = opticks_main()

 
radius = 200 
inscribe = 1.3*radius/math.sqrt(3)

lbox = CSG("box",    param=[100,100,-100,inscribe])
lsph = CSG("sphere", param=[100,100,-100,radius])
left  = CSG("difference", left=lbox, right=lsph, boundary="$(tboolean-testobject)" )

right = CSG("sphere", param=[0,0,100,radius])

object = CSG("union", left=left, right=right, boundary="$(tboolean-testobject)", poly="IM", resolution="60" )
object.dump()

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="IM", resolution="20")

CSG.Serialize([container, object], "$base" )


EOP
}



tboolean-gds(){  TESTCONFIG=$($FUNCNAME- 2>/dev/null) tboolean--  ; }
tboolean-gds-(){ tboolean-gdml- $TMP/$FUNCNAME --gsel /dd/Geometry/AD/lvGDS0x ; }

tboolean-oav(){  TESTCONFIG=$($FUNCNAME- 2>/dev/null) tboolean--  ; }
tboolean-oav-(){ tboolean-gdml- $TMP/$FUNCNAME --gsel /dd/Geometry/AD/lvOAV0x ; }

tboolean-pmt(){  TESTCONFIG=$($FUNCNAME- 2>/dev/null) tboolean--  ; }
tboolean-pmt-(){ tboolean-gdml- $TMP/$FUNCNAME --gsel /dd/Geometry/PMT/lvPmtHemi0x ; }


tboolean-gdml-()
{      
    local csgpath=$1
    shift
    python $(tboolean-dir)/tboolean_gdml.py \
          --csgpath $csgpath \
          --container $(tboolean-container)  \
          --testobject $(tboolean-testobject) \
          $*
}
tboolean-gdml-check(){ tboolean-gdml- 2> /dev/null ; }
tboolean-gdml-edit(){ vi $(tboolean-dir)/tboolean_gdml.py  ; }
tboolean-gdml-scan(){ SCAN="0,0,127.9,0,0,1,0,0.1,0.01" NCSGScanTest $TMP/tboolean-gdml-/1 ; }
tboolean-gdml-deserialize(){ NCSGDeserializeTest $TMP/tboolean-gdml- ; }
tboolean-gdml-ip(){  tboolean-cd ; ipython tboolean_gdml.py -i ; }



tboolean-dd(){          TESTCONFIG=$(tboolean-dd- 2>/dev/null)     tboolean-- $* ; }
tboolean-dd-()
{       
    python $(tboolean-dir)/tboolean_dd.py \
          --csgpath $TMP/$FUNCNAME \
          --container $(tboolean-container)  \
          --testobject $(tboolean-testobject)  

    # got too long for here-string  so broke out into script
}
tboolean-dd-check(){ tboolean-dd- 2> /dev/null ; }
tboolean-dd-edit(){ vi $(tboolean-dir)/tboolean_dd.py  ; }
tboolean-dd-scan(){ SCAN="0,0,127.9,0,0,1,0,0.1,0.01" NCSGScanTest $TMP/tboolean-dd-/1 ; }




tboolean-interlocked(){  TESTCONFIG=$(tboolean-csg-two-box-minus-sphere-interlocked-py) tboolean-- ; }
tboolean-csg-two-box-minus-sphere-interlocked-py(){ $FUNCNAME- | python $* ; }
tboolean-csg-two-box-minus-sphere-interlocked-py-()
{
    local base=$TMP/$FUNCNAME 
    cat << EOP 
import math
from opticks.dev.csg.csg import CSG  
  
radius = 200 
inscribe = 1.3*radius/math.sqrt(3)

lbox = CSG("box",    param=[100,100,-100,inscribe])
lsph = CSG("sphere", param=[100,100,-100,radius])
left  = CSG("difference", left=lbox, right=lsph, boundary="$(tboolean-testobject)" )

rbox = CSG("box",    param=[0,0,100,inscribe])
rsph = CSG("sphere", param=[0,0,100,radius])


tran = dict(translate="0,0,200", rotate="1,1,1,45", scale="1,1,1.5" )
right = CSG("difference", left=rbox, right=rsph, boundary="$(tboolean-testobject)", **tran)

dcs = dict(poly="DCS", nominal="7", coarse="6", threshold="1", verbosity="0")

#seeds = "100,100,-100,0,0,300"
im = dict(poly="IM", resolution="64", verbosity="0", ctrl="0" )
object = CSG("union", left=left, right=right,  boundary="$(tboolean-testobject)", **im )

mc = dict(poly="MC", nx="20")

container = CSG("box", param=[0,0,0,1000], boundary="$(tboolean-container)", poly="IM", resolution="20" )

CSG.Serialize([container, object], "$base" )
# marching cubes with nx=15 makes a mess with this 

EOP
}



tboolean-testconfig()
{
    # token BoxInBox 
    #tboolean-bib-box
    #tboolean-bib-box-small-offset-sphere
    #tboolean-bib-box-sphere


    #tboolean-box-py
    #tboolean-sphere-py

    #tboolean-box-small-offset-sphere-py difference
    #tboolean-box-small-offset-sphere-py intersection
    #tboolean-box-small-offset-sphere-py union

    #tboolean-box-sphere-py intersection 
    #tboolean-box-sphere-py difference
    tboolean-box-sphere-py union

    #tboolean-csg-unbalanced-py
    #tboolean-csg-pmt-py 2>/dev/null

    #tboolean-csg-two-box-minus-sphere-interlocked-py
}





