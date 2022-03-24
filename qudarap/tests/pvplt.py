#!/usr/bin/env python

import os 
import numpy as np
import pyvista as pv

GUI = not "NOGUI" in os.environ
SIZE = np.array([1280, 720])

def pvplt_simple(pl, xyz, label):
    """  
    :param pl: pyvista plotter 
    :param xyz: (n,3) shaped array of positions
    :param label: to place on plot 
    """
    pl.add_text( "pvplt_simple %s " % label, position="upper_left")
    pl.add_points( xyz, color="white" )     


def pvplt_photon( pl, p   ):
    assert p.shape == (1,4,4)
    pos = p[:,0,:3]   
    mom = p[:,1,:3]   
    pol = p[:,2,:3]   

    pl.add_points( pos, color="magenta", point_size=16.0 )

    lmom = np.zeros( (2, 3), dtype=np.float32 )
    lmom[0] = pos[0]
    lmom[1] = pos[0] + mom[0]

    lpol = np.zeros( (2, 3), dtype=np.float32 )
    lpol[0] = pos[0]
    lpol[1] = pos[0] + pol[0]

    pl.add_lines( lmom, color="red" ) 
    pl.add_lines( lpol, color="blue" ) 



def pvplt_plotter(label="pvplt_plotter"):
    pl = pv.Plotter(window_size=SIZE*2 )  
    pl.show_grid()
    TEST = os.environ["TEST"]
    pl.add_text( "%s %s " % (label,TEST), position="upper_left")
    return pl 


def pvplt_arrows( pl, pos, vec, color='yellow' ):
    """
     
    glyph.orient
        Use the active vectors array to orient the glyphs
    glyph.scale
        Use the active scalars to scale the glyphs
    glyph.factor
        Scale factor applied to sclaing array
    glyph.geom
        The geometry to use for the glyph

    """
    init_pl = pl == None 
    if init_pl:
        pl = pvplt_plotter(label="pvplt_arrows")   
    pass
    pos_cloud = pv.PolyData(pos)
    pos_cloud['vec'] = vec
    vec_arrows = pos_cloud.glyph(orient='vec', scale=False, factor=0.15,)

    pl.add_mesh(pos_cloud, render_points_as_spheres=True, show_scalar_bar=False)
    pl.add_mesh(vec_arrows, color=color, show_scalar_bar=False)


def pvplt_lines( pl, pos, vec, color='white' ):
    init_pl = pl == None 
    if init_pl:
        pl = pvplt_plotter(label="pvplt_line")   
    pass
    pos_cloud = pv.PolyData(pos)
    pos_cloud['vec'] = vec
    geom = pv.Line(pointa=(0.0, 0., 0.), pointb=(1.0, 0., 0.),)
    vec_lines = pos_cloud.glyph(orient='vec', scale=False, factor=1.0, geom=geom)
    pl.add_mesh(pos_cloud, render_points_as_spheres=True, show_scalar_bar=False)
    pl.add_mesh(vec_lines, color=color, show_scalar_bar=False)


def pvplt_polarized( pl, pos, mom, pol ):
    """
    https://docs.pyvista.org/examples/00-load/create-point-cloud.html
    https://docs.pyvista.org/examples/01-filter/glyphs.html
    """
    mom_pol_transverse = np.abs(np.sum( mom*pol , axis=1 )).max() 
    assert mom_pol_transverse < 1e-5 

    init_pl = pl == None 
    if init_pl:
        pl = pvplt_plotter(label="pvplt_polarized")   
    pass

    pos_cloud = pv.PolyData(pos)
    pos_cloud['mom'] = mom
    pos_cloud['pol'] = pol
    mom_arrows = pos_cloud.glyph(orient='mom', scale=False, factor=0.15,)
    pol_arrows = pos_cloud.glyph(orient='pol', scale=False, factor=0.15,)
    pl.add_mesh(pos_cloud, render_points_as_spheres=True, show_scalar_bar=False)
    pl.add_mesh(pol_arrows, color='lightblue', show_scalar_bar=False)
    pl.add_mesh(mom_arrows, color='red', show_scalar_bar=False)

    if init_pl:
        cp = pl.show() if GUI else None 
    else:
        cp = None 
    pass    
    return cp 

