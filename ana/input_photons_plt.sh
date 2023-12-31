#!/bin/bash -l 

usage(){ cat << EOU
input_photons_plt.sh
=====================

::

   cd ~/opticks/ana
   ./input_photons.sh    # generate the input photons 
   ./input_photons_plt.sh 

EOU
}


#stem=RandomSpherical100
#stem=RandomDisc100
#stem=UniformDisc_R500_10k
#stem=GridXY_X700_Z230_10k
stem=GridXY_X1000_Z1000_40k
#stem=UpXZ1000
#stem=DownXZ1000
#stem=RainXZ1000
#stem=RainXZ_Z230_1000

#sufx=_f4
sufx=_f8

path=${stem}${sufx}.npy

mode=3
export MODE=${MODE:-$mode}

export OPTICKS_INPUT_PHOTON=${OPTICKS_INPUT_PHOTON:-$path}

${IPYTHON:-ipython} --pdb -i input_photons_plt.py -- $*




