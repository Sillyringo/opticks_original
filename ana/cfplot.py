#!/usr/bin/env python
#
# Copyright (c) 2019 Opticks Team. All Rights Reserved.
#
# This file is part of Opticks
# (see https://bitbucket.org/simoncblyth/opticks).
#
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.  
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
# See the License for the specific language governing permissions and 
# limitations under the License.
#

"""
cfplot.py : Comparison Plotter with Chi2 Underplot 
======================================================

To control this warning, see the rcParam `figure.max_num_figures


"""
import os, logging, numpy as np
from collections import OrderedDict as odict
from opticks.ana.main import opticks_main
from opticks.ana.cfh import CFH 
log = logging.getLogger(__name__)

try:
    import matplotlib.pyplot as plt
    import matplotlib.gridspec as gridspec
    plt.rcParams["figure.max_open_warning"] = 200    # default is 20
except ImportError:
    print("matplotlib missing : you need this to make plots")
    plt = None


def cfplot(fig, gss, h, xline=[]): 
    """
    :param fig:
    :param gss: 2 element gridspec list 
    :param h:  .ahis .bhis .la .lb .ylim .bins .chi2 .c2label .c2_ymax .log 
    """

    ax = fig.add_subplot(gss[0])
    ax0 = ax

    ax.plot( h.bins[:-1], h.ahis , drawstyle="steps", label=h.la  )
    ax.plot( h.bins[:-1], h.bhis , drawstyle="steps", label=h.lb  )

    if h.log:
        ax.set_yscale('log')
    pass

    ylim0 = h.ylim
    if len(xline) > 0:
        for x in xline:
            ax.plot( [x,x], ylim0, linestyle="dashed" )
        pass
    pass

    ax.set_ylim(ylim0)
    ax.legend()

    xlim = ax.get_xlim()

    ax = fig.add_subplot(gss[1])
    ax1 = ax

    ax.plot( h.bins[:-1], h.chi2, drawstyle='steps', label=h.c2label )

    ax.set_xlim(xlim) 
    ax.legend()

    ylim1 = [0,h.c2_ymax*1.05]
    if len(xline) > 0:
        for x in xline:
            ax.plot( [x,x], ylim1, linestyle="dashed" )
        pass
    pass
    ax.set_ylim(ylim1) 

    return [ax0,ax1]



def qwns_plot( ok, hh, suptitle ):
    nhh = len(hh)
    nxm = 4 
    if nhh > nxm:
        # pagination 
        for p in range(nhh/nxm):
            phh = hh[p*nxm:(p+1)*nxm]
            qwns_plot( ok, phh, suptitle + " (%d)" % p )   
        pass
    else:
        fig = plt.figure(figsize=ok.figsize)
        fig.suptitle(suptitle)
        ny = 2 
        nx = len(hh)
        gs = gridspec.GridSpec(ny, nx, height_ratios=[3,1])
        for ix in range(nx):
            gss = [gs[ix], gs[nx+ix]]
            h = hh[ix]
            cfplot(fig, gss, hh[ix] )
        pass

def one_cfplot(ok, h, xline=[]):
    """
    :param ok: used for figsize
    :param h: CFH instance
    """
    fig = plt.figure(figsize=ok.figsize)
    fig.suptitle(h.suptitle)
    ny = 2
    nx = 1
    gs = gridspec.GridSpec(ny, nx, height_ratios=[3,1])

    ix = 0 
    gss = [gs[ix], gs[nx+ix]]
    axs = cfplot(fig, gss, h, xline=xline )
    return fig, axs 


def multiplot(ok, ab, start=0, stop=5, log_=False):
    """
    """
    pages = ok.qwn.split(",")    ## default ok.qwn 'XYZT,ABCW'

    for i,isel in enumerate(range(start, stop)):

        ab.sel = slice(isel, isel+1)
        nrec = ab.nrec      ## number of slots with the selected history 

        for irec in range(nrec):     

            ab.irec = irec           
            suptitle = ab.suptitle   ## irec "slot" gets highlighted in the title eg "... SI [BT] BT AB "

            log.info("multiplot irec %d nrec %d suptitle %s " % (irec, nrec, suptitle))

            for page in pages:
                hh = ab.rhist(page, irec, log_ )
                qwns_plot( ok, hh, suptitle )
            pass
        pass
    pass


def sc_selection_plot(ok, ab, log_=False):
    """
    :param ok:
    :param ab:
    :param log_:

    Plotting quantities at first scatter, needs a single selection.
    """
    pages = ok.qwn.split(",")
    irec = ab.iflg("SC")

    ab.irec = irec
    suptitle = ab.suptitle

    for page in pages:
        hh = ab.rhist(page, irec, log_)
        qwns_plot( ok, hh, suptitle )
    pass



if __name__ == '__main__':
    ok = opticks_main()
    print(ok)
    plt.ion()
    plt.close()

    from opticks.ana.ab import AB
    h = AB.rrandhist()
    assert type(h).__name__  == "CFH"
    one_cfplot(ok, h) 

