tds3ip_with_7_evt_concatenated
===================================

* the 7 evt are with 7 different starting input wavelengths 
* problems mostly out in the history tail 
* trauncation zeros 

  * TODO: increase bouncemax to 16, to see what happens 


::

    In [8]: ab.his[:20]                                                                                                                                                                                  
    Out[8]: 
    ab.his
    .       seqhis_ana  cfo:sum  100:g4live:tds3ip   -100:g4live:tds3ip 
    .             TOTALS:   560000   560000                  2231.10     2231.10/751 =  2.97   pvalue:P[C2>]:0.000  1-pvalue:P[C2<]:1.000 
       n             iseq        a        b      a-b     (a-b)^2/(a+b)        a/b                  b/a          [ns]   label
    0000               4d   104250   104391     -141            0.10        0.999 +- 0.003       1.001 +- 0.003 [2 ]   TO AB
    0001           7ccc6d    42518    42407      111            0.15        1.003 +- 0.005       0.997 +- 0.005 [6 ]   TO SC BT BT BT SD
    0002            7cccd    38264    38396     -132            0.23        0.997 +- 0.005       1.003 +- 0.005 [5 ]   TO BT BT BT SD
    0003           7ccc5d    33792    33591      201            0.60        1.006 +- 0.005       0.994 +- 0.005 [6 ]   TO RE BT BT BT SD
    0004           8ccc6d    19899    20603     -704           12.24        0.966 +- 0.007       1.035 +- 0.007 [6 ]   TO SC BT BT BT SA
    0005            8cccd    19762    19652      110            0.31        1.006 +- 0.007       0.994 +- 0.007 [5 ]   TO BT BT BT SA
    0006              45d    19196    19244      -48            0.06        0.998 +- 0.007       1.003 +- 0.007 [3 ]   TO RE AB
    0007              46d    19134    19112       22            0.01        1.001 +- 0.007       0.999 +- 0.007 [3 ]   TO SC AB
    0008          7ccc66d    14492    14387      105            0.38        1.007 +- 0.008       0.993 +- 0.008 [7 ]   TO SC SC BT BT BT SD
    0009           8ccc5d    12784    13178     -394            5.98        0.970 +- 0.009       1.031 +- 0.009 [6 ]   TO RE BT BT BT SA
    0010            8cc6d    12829    12729      100            0.39        1.008 +- 0.009       0.992 +- 0.009 [5 ]   TO SC BT BT SA
    0011       bccbccbc5d    12224    12387     -163            1.08        0.987 +- 0.009       1.013 +- 0.009 [10]   TO RE BT BR BT BT BR BT BT BR
    0012             4c5d    10157    10035      122            0.74        1.012 +- 0.010       0.988 +- 0.010 [4 ]   TO RE BT AB
    0013          8ccc66d     6816     6960     -144            1.51        0.979 +- 0.012       1.021 +- 0.012 [7 ]   TO SC SC BT BT BT SA
    0014          7ccc65d     6865     6856        9            0.01        1.001 +- 0.012       0.999 +- 0.012 [7 ]   TO RE SC BT BT BT SD
    0015          7ccc55d     6840     6730      110            0.89        1.016 +- 0.012       0.984 +- 0.012 [7 ]   TO RE RE BT BT BT SD
    0016             466d     6216     6168       48            0.19        1.008 +- 0.013       0.992 +- 0.013 [4 ]   TO SC SC AB
    0017            8cc5d     6074     5861      213            3.80        1.036 +- 0.013       0.965 +- 0.013 [5 ]   TO RE BT BT SA
    0018         7ccc666d     5045     4960       85            0.72        1.017 +- 0.014       0.983 +- 0.014 [8 ]   TO SC SC SC BT BT BT SD
    .             TOTALS:   560000   560000                  2231.10     2231.10/751 =  2.97   pvalue:P[C2>]:0.000  1-pvalue:P[C2<]:1.000 


::

    In [5]: ab.his.ll[np.logical_and(ab.his.c2 > 10, ab.his.idif<0)]                                                                                                                                     
    Out[5]: 
    array(['0004           8ccc6d    19899    20603     -704           12.24        0.966 +- 0.007       1.035 +- 0.007 [6 ]   TO SC BT BT BT SA',
           '0028          8ccc55d     2560     2825     -265           13.04        0.906 +- 0.018       1.104 +- 0.021 [7 ]   TO RE RE BT BT BT SA',
           '0043         8caccc5d     1237     1520     -283           29.05        0.814 +- 0.023       1.229 +- 0.032 [8 ]   TO RE BT BT BT SR BT SA',
           '0048         8caccc6d     1131     1496     -365           50.71        0.756 +- 0.022       1.323 +- 0.034 [8 ]   TO SC BT BT BT SR BT SA',
           '0086        8caccc66d      471      580     -109           11.30        0.812 +- 0.037       1.231 +- 0.051 [9 ]   TO SC SC BT BT BT SR BT SA',
           '0114        7ccccbc5d      265      344      -79           10.25        0.770 +- 0.047       1.298 +- 0.070 [9 ]   TO RE BT BR BT BT BT BT SD',
           '0226          8cbcc6d       88      140      -52           11.86        0.629 +- 0.067       1.591 +- 0.134 [7 ]   TO SC BT BT BR BT SA',
           '0373       8cacaccc6d        0       98      -98           98.00        0.000 +- 0.000       0.000 +- 0.000 [10]   TO SC BT BT BT SR BT SR BT SA',
           '0404       8cacaccc5d        0       86      -86           86.00        0.000 +- 0.000       0.000 +- 0.000 [10]   TO RE BT BT BT SR BT SR BT SA',
           '0415       c666cbc55d       26       56      -30           10.98        0.464 +- 0.091       2.154 +- 0.288 [10]   TO RE RE BT BR BT SC SC SC BT',
           '0453         8ccccc6d       20       55      -35           16.33        0.364 +- 0.081       2.750 +- 0.371 [8 ]   TO SC BT BT BT BT BT SA',
           '0519       cacaccc66d        4       53      -49           42.12        0.075 +- 0.038      13.250 +- 1.820 [10]   TO SC SC BT BT BT SR BT SR BT',
           '0566       acacaccc6d        7       42      -35           25.00        0.167 +- 0.063       6.000 +- 0.926 [10]   TO SC BT BT BT SR BT SR BT SR',
           '0654         45cbc65d        8       30      -22           12.74        0.267 +- 0.094       3.750 +- 0.685 [8 ]   TO RE SC BT BR BT RE AB',
           '0720       c6cbc6555d        7       25      -18           10.12        0.280 +- 0.106       3.571 +- 0.714 [10]   TO RE RE RE SC BT BR BT SC BT'], dtype='<U144')


    In [6]: ab.his.ll[np.logical_and(ab.his.c2 > 10, ab.his.idif>0)]                                                                                                                                     
    Out[6]: 
    array(['0104           4ccc6d      487      251      236           75.47        1.940 +- 0.088       0.515 +- 0.033 [6 ]   TO SC BT BT BT AB',
           '0105           4ccc5d      490      230      260           93.89        2.130 +- 0.096       0.469 +- 0.031 [6 ]   TO RE BT BT BT AB',
           '0171          8bcc66d      220      139       81           18.28        1.583 +- 0.107       0.632 +- 0.054 [7 ]   TO SC SC BT BT BR SA',
           '0214          4ccc66d      161       89       72           20.74        1.809 +- 0.143       0.553 +- 0.059 [7 ]   TO SC SC BT BT BT AB',
           '0316          8bcc55d       80       43       37           11.13        1.860 +- 0.208       0.537 +- 0.082 [7 ]   TO RE RE BT BT BR SA',
           '0330          4ccc55d       79       38       41           14.37        2.079 +- 0.234       0.481 +- 0.078 [7 ]   TO RE RE BT BT BT AB',
           '0407         4ccc666d       67       19       48           26.79        3.526 +- 0.431       0.284 +- 0.065 [8 ]   TO SC SC SC BT BT BT AB',
           '0428       7ccc65665d       57       21       36           16.62        2.714 +- 0.360       0.368 +- 0.080 [10]   TO RE SC SC RE SC BT BT BT SD',
           '0431       8caccccc5d       56       21       35           15.91        2.667 +- 0.356       0.375 +- 0.082 [10]   TO RE BT BT BT BT BT SR BT SA',
           '0464       8cabaccc6d       71        0       71           71.00        0.000 +- 0.000       0.000 +- 0.000 [10]   TO SC BT BT BT SR BR SR BT SA',
           '0473       8caccccc6d       54       14       40           23.53        3.857 +- 0.525       0.259 +- 0.069 [10]   TO SC BT BT BT BT BT SR BT SA',
           '0569          4cccc5d       45        4       41           34.31       11.250 +- 1.677       0.089 +- 0.044 [7 ]   TO RE BT BT BT BT AB',
           '0593       ababaccc6d       44        0       44           44.00        0.000 +- 0.000       0.000 +- 0.000 [10]   TO SC BT BT BT SR BR SR BR SR',
           '0620       8cabaccc5d       40        0       40           40.00        0.000 +- 0.000       0.000 +- 0.000 [10]   TO RE BT BT BT SR BR SR BT SA',
           '0649        8cacccc6d       38        0       38           38.00        0.000 +- 0.000       0.000 +- 0.000 [9 ]   TO SC BT BT BT BT SR BT SA',
           '0660          4cbcc6d       33        5       28           20.63        6.600 +- 1.149       0.152 +- 0.068 [7 ]   TO SC BT BT BR BT AB',
           '0744         4ccccc6d       30        1       29           27.13       30.000 +- 5.477       0.033 +- 0.033 [8 ]   TO SC BT BT BT BT BT AB'], dtype='<U144')





