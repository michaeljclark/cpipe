# Linux Benchmarks

## Skylake

### single producer single consumer

```
# test_002_pipe_buffer: 1 write thread(s) 1 read thread(s)
# os: Linux cpu: Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz

wsum:010070fe2ba22800 ( 14.29 sec) rsum:010070fe2ba22800 ( 14.29 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     441103914     425.98ns    2347509       8.96
     write   33551616     539668704     426.03ns    2347261       8.95

wsum:010070fe2ba22800 (  4.62 sec) rsum:010070fe2ba22800 (  4.62 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388351     129903524     550.77ns    1815644      27.70
     write    8388096     163427952     550.86ns    1815335      27.70

wsum:010070fe2ba22800 (  1.25 sec) rsum:010070fe2ba22800 (  1.25 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097372      35175822     595.04ns    1680558     102.55
     write    2097152      41264061     595.20ns    1680101     102.54

wsum:010070fe2ba22800 (  0.11 sec) rsum:010070fe2ba22800 (  0.11 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      32768       2118795    3384.64ns     295452    1154.01
     write      32768       5795545    3389.95ns     294989    1152.21

wsum:010070fe2ba22800 (  0.09 sec) rsum:010070fe2ba22800 (  0.09 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       4096        706167   21646.24ns      46197    1443.55
     write       4096       5669357   21843.02ns      45781    1430.54
```

### multiple producer multiple consumer

```
# test_003_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Linux cpu: Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz

wsum:010070fe2ba22800 (207.62 sec) rsum:010070fe2ba22800 (207.62 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616    3616167806    6188.22ns     161597       0.62
     write   33551616    4850781234    6188.22ns     161597       0.62

wsum:010070fe2ba22800 ( 51.82 sec) rsum:010070fe2ba22800 ( 51.82 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388489     918639481    6177.12ns     161887       2.47
     write    8388477    1190443398    6177.22ns     161885       2.47

wsum:010070fe2ba22800 ( 13.35 sec) rsum:010070fe2ba22800 ( 13.35 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097708     242906499    6364.98ns     157109       9.59
     write    2097693     320318859    6365.06ns     157107       9.59

wsum:010070fe2ba22800 (  0.33 sec) rsum:010070fe2ba22800 (  0.33 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      33767       8285115    9785.32ns     102193     387.35
     write      33803      12363178    9763.04ns     102427     387.82

wsum:010070fe2ba22800 (  0.16 sec) rsum:010070fe2ba22800 (  0.16 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       5371       5146199   29150.62ns      34304     817.47
     write       5446      10970401   28890.01ns      34614     813.48
```

### multiple producer multiple consumer (zero-copy)

```
# test_004_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Linux cpu: Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz

wsum:010070fe2ba22800 (206.56 sec) rsum:010070fe2ba22800 (206.56 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616    1028561324    6156.42ns     162432       0.62
     write   33551616    1115254539    6156.44ns     162431       0.62

wsum:010070fe2ba22800 ( 51.10 sec) rsum:010070fe2ba22800 ( 51.10 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8389178     244318274    6091.03ns     164175       2.50
     write    8389171     263778923    6091.14ns     164172       2.50

wsum:010070fe2ba22800 ( 12.98 sec) rsum:010070fe2ba22800 ( 12.98 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2098695      64410725    6185.27ns     161674       9.86
     write    2098683      71148516    6185.57ns     161666       9.86

wsum:010070fe2ba22800 (  0.31 sec) rsum:010070fe2ba22800 (  0.31 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      38664       2243494    7981.82ns     125284     414.73
     write      38688       3001481    7989.58ns     125162     414.07

wsum:010070fe2ba22800 (  0.14 sec) rsum:010070fe2ba22800 (  0.14 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       7046       1654073   20267.24ns      49340     896.26
     write       7149       3412050   20064.34ns      49839     892.28
```

## Kaby Lake

### single producer single consumer

```
# test_002_pipe_buffer: 1 write thread(s) 1 read thread(s)
# os: Linux cpu: Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz

wsum:010070fe2ba22800 ( 12.39 sec) rsum:010070fe2ba22800 ( 12.39 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     184654598     369.27ns    2708069      10.33
     write   33551616     280388807     369.35ns    2707440      10.33

wsum:010070fe2ba22800 (  3.04 sec) rsum:010070fe2ba22800 (  3.05 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388304      52400515     362.76ns    2756634      42.06
     write    8388096      59846988     363.18ns    2753444      42.01

wsum:010070fe2ba22800 (  0.83 sec) rsum:010070fe2ba22800 (  0.83 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097196      14471389     393.76ns    2539630     154.99
     write    2097152      14150822     393.99ns    2538135     154.90

wsum:010070fe2ba22800 (  0.11 sec) rsum:010070fe2ba22800 (  0.11 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      32768       2160053    3385.74ns     295356    1153.64
     write      32768       5176867    3410.13ns     293244    1145.39

wsum:010070fe2ba22800 (  0.09 sec) rsum:010070fe2ba22800 (  0.09 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       4096        726810   22892.33ns      43682    1364.97
     write       4096       5005279   22992.68ns      43492    1359.01
```

### multiple producer multiple consumer

```
# test_003_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Linux cpu: Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz

wsum:010070fe2ba22800 ( 95.95 sec) rsum:010070fe2ba22800 ( 95.95 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     671993243    2859.67ns     349690       1.33
     write   33551616     596391923    2859.79ns     349676       1.33

wsum:010070fe2ba22800 ( 29.42 sec) rsum:010070fe2ba22800 ( 29.42 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388551     158128402    3507.32ns     285117       4.35
     write    8388493     170034544    3507.65ns     285090       4.35

wsum:010070fe2ba22800 (  8.24 sec) rsum:010070fe2ba22800 (  8.24 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097357      59985249    3928.63ns     254541      15.53
     write    2097385      75587448    3930.44ns     254424      15.53

wsum:010070fe2ba22800 (  0.31 sec) rsum:010070fe2ba22800 (  0.31 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      33607       4900595    9274.88ns     107818     410.62
     write      33543       5640489    9350.68ns     106944     408.06

wsum:010070fe2ba22800 (  0.17 sec) rsum:010070fe2ba22800 (  0.17 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       4545       1272977   36760.84ns      27202     766.04
     write       4577       3471455   37348.04ns      26775     748.73
```

### multiple producer multiple consumer (zero-copy)

```
# test_004_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Linux cpu: Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz

wsum:010070fe2ba22800 (102.87 sec) rsum:010070fe2ba22800 (102.87 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     425455352    3065.97ns     326160       1.24
     write   33551616     471538200    3066.02ns     326155       1.24

wsum:010070fe2ba22800 ( 25.69 sec) rsum:010070fe2ba22800 ( 25.69 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388938     119423150    3062.45ns     326535       4.98
     write    8388927     120913834    3062.87ns     326491       4.98

wsum:010070fe2ba22800 (  6.37 sec) rsum:010070fe2ba22800 (  6.38 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2098420      25063747    3036.94ns     329278      20.08
     write    2098397      30687019    3038.33ns     329128      20.07

wsum:010070fe2ba22800 (  0.31 sec) rsum:010070fe2ba22800 (  0.32 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      46098       1297210    6768.10ns     147751     410.23
     write      46190       2633173    6839.34ns     146212     405.15

wsum:010070fe2ba22800 (  0.16 sec) rsum:010070fe2ba22800 (  0.17 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       5718        761377   28348.55ns      35275     789.58
     write       5771       1088266   28801.25ns      34720     770.04
```
