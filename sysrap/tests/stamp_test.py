#!/usr/bin/env python
import numpy as np, os
import matplotlib.pyplot as plt


if __name__ == '__main__':
    t = np.load(os.environ["TTPATH"])

    dt = t - t[0]

    print(np.c_[t.view("datetime64[us]")])  
    i = np.arange(len(t))
    s = slice(None,None,1000)
   

    fig, ax = plt.subplots(figsize=[12.8,7.2])


    ax.scatter( i[s], dt[s] )

    fig.show() 



