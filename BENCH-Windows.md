# Windows Benchmarks

## Skylake

### single producer single consumer

```
# test_002_pipe_buffer: 1 write thread(s) 1 read thread(s)
# os: Windows cpu: Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz

wsum:010070fe2ba22800 ( 13.59 sec) rsum:010070fe2ba22800 ( 13.59 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     557888984     404.99ns    2469209       9.42
     write   33551616     693969837     404.99ns    2469209       9.42

wsum:010070fe2ba22800 (  3.41 sec) rsum:010070fe2ba22800 (  3.41 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388351     150699493     406.75ns    2458485      37.51
     write    8388096     176650878     406.77ns    2458410      37.51

wsum:010070fe2ba22800 (  0.86 sec) rsum:010070fe2ba22800 (  0.86 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097407      36138458     412.41ns    2424747     147.96
     write    2097152      41667918     412.46ns    2424453     147.96

wsum:010070fe2ba22800 (  0.05 sec) rsum:010070fe2ba22800 (  0.05 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      32768        624744    1525.88ns     655360    2559.79
     write      32768       4142173    1525.88ns     655360    2559.79

wsum:010070fe2ba22800 (  0.04 sec) rsum:010070fe2ba22800 (  0.04 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       4096        277842   10498.05ns      95255    2976.49
     write       4096       5632767   10498.05ns      95255    2976.49
```

### multiple producer multiple consumer

```
# test_003_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Windows cpu: Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz

wsum:010070fe2ba22800 ( 27.16 sec) rsum:010070fe2ba22800 ( 27.16 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616    2723959464     809.59ns    1235195       4.71
     write   33551616    3181772014     809.62ns    1235150       4.71

wsum:010070fe2ba22800 (  6.78 sec) rsum:010070fe2ba22800 (  6.78 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388683     680289409     807.75ns    1237999      18.89
     write    8388667     799216665     807.76ns    1237996      18.89

wsum:010070fe2ba22800 (  1.70 sec) rsum:010070fe2ba22800 (  1.70 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097874     177540535     811.77ns    1231869      75.16
     write    2097858     212257626     811.78ns    1231860      75.16

wsum:010070fe2ba22800 (  0.04 sec) rsum:010070fe2ba22800 (  0.04 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      33962       5655836    1207.23ns     828341    3121.69
     write      33986       8044270    1206.38ns     828926    3121.69

wsum:010070fe2ba22800 (  0.02 sec) rsum:010070fe2ba22800 (  0.02 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       5278       4343345    3978.78ns     251333    6094.73
     write       5368       9108414    3912.07ns     255619    6094.73
```

### multiple producer multiple consumer (zero-copy)

```
# test_004_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Windows cpu: Intel(R) Core(TM) i9-7980XE CPU @ 2.60GHz

wsum:010070fe2ba22800 ( 28.23 sec) rsum:010070fe2ba22800 ( 28.23 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616    1629851883     841.36ns    1188551       4.53
     write   33551616    1676788703     841.36ns    1188551       4.53

wsum:010070fe2ba22800 (  7.06 sec) rsum:010070fe2ba22800 (  7.06 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8389604     412714536     841.99ns    1187656      18.12
     write    8389591     420804695     842.00ns    1187654      18.12

wsum:010070fe2ba22800 (  1.86 sec) rsum:010070fe2ba22800 (  1.86 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2099178     111372248     887.49ns    1126772      68.70
     write    2099196     115966363     887.48ns    1126782      68.70

wsum:010070fe2ba22800 (  0.04 sec) rsum:010070fe2ba22800 (  0.04 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      36079       2853098    1136.40ns     879975    3121.69
     write      36112       3671185    1135.36ns     880780    3121.69

wsum:010070fe2ba22800 (  0.02 sec) rsum:010070fe2ba22800 (  0.02 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       7950       1245107    2264.15ns     441666    7110.51
     write       8084       2780994    2226.62ns     449111    7110.51
```

## Kaby Lake

### single producer single consumer

```
# test_002_pipe_buffer: 1 write thread(s) 1 read thread(s)
# os: Windows cpu: Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz

wsum:010070fe2ba22800 (  7.37 sec) rsum:010070fe2ba22800 (  7.37 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     262021905     219.63ns    4553075      17.37
     write   33551616     289699785     219.63ns    4553075      17.37

wsum:010070fe2ba22800 (  1.92 sec) rsum:010070fe2ba22800 (  1.92 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388328      77375702     228.65ns    4373476      66.73
     write    8388096      78543454     228.66ns    4373355      66.73

wsum:010070fe2ba22800 (  0.51 sec) rsum:010070fe2ba22800 (  0.51 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097380      19262831     244.59ns    4088460     249.49
     write    2097152      19752379     244.62ns    4088015     249.49

wsum:010070fe2ba22800 (  0.06 sec) rsum:010070fe2ba22800 (  0.06 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      32769       1022223    1678.42ns     595800    2327.08
     write      32768       2930111    1678.47ns     595781    2327.08

wsum:010070fe2ba22800 (  0.04 sec) rsum:010070fe2ba22800 (  0.04 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       4096        279537   10742.19ns      93090    2908.85
     write       4096       4038730   10742.19ns      93090    2908.85
```

### multiple producer multiple consumer

```
# test_003_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Windows cpu: Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz

wsum:010070fe2ba22800 ( 13.91 sec) rsum:010070fe2ba22800 ( 13.91 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     738627189     414.53ns    2412396       9.20
     write   33551616     797140777     414.53ns    2412396       9.20

wsum:010070fe2ba22800 (  3.60 sec) rsum:010070fe2ba22800 (  3.60 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8388650     182529363     429.51ns    2328240      35.52
     write    8388621     199232158     429.51ns    2328232      35.52

wsum:010070fe2ba22800 (  0.96 sec) rsum:010070fe2ba22800 (  0.96 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2097902      46489691     455.69ns    2194458     133.88
     write    2097913      53766182     455.69ns    2194469     133.88

wsum:010070fe2ba22800 (  0.04 sec) rsum:010070fe2ba22800 (  0.04 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      34142       4133580    1230.16ns     812904    3047.36
     write      34139       4169942    1230.26ns     812833    3047.36

wsum:010070fe2ba22800 (  0.02 sec) rsum:010070fe2ba22800 (  0.02 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       4957       1576658    4639.90ns     215521    5564.75
     write       5076       2529370    4531.13ns     220695    5564.75
```

### multiple producer multiple consumer (zero-copy)

```
# test_004_pipe_buffer: 4 write thread(s) 4 read thread(s)
# os: Windows cpu: Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz

wsum:010070fe2ba22800 ( 16.11 sec) rsum:010070fe2ba22800 ( 16.11 sec)

    4-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read   33551616     570784854     480.22ns    2082399       7.94
     write   33551616     590255987     480.22ns    2082399       7.94

wsum:010070fe2ba22800 (  4.09 sec) rsum:010070fe2ba22800 (  4.09 sec)

   16-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    8389309     135685253     487.53ns    2051175      31.29
     write    8389303     148623259     487.64ns    2050672      31.29

wsum:010070fe2ba22800 (  1.08 sec) rsum:010070fe2ba22800 (  1.08 sec)

   64-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read    2098680      35501975     512.70ns    1950446     118.95
     write    2098707      38277757     512.70ns    1950471     118.95

wsum:010070fe2ba22800 (  0.04 sec) rsum:010070fe2ba22800 (  0.04 sec)

 4096-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read      44738       1454071     916.45ns    1091170    3121.69
     write      44705       1592244     917.12ns    1090365    3121.69

wsum:010070fe2ba22800 (  0.02 sec) rsum:010070fe2ba22800 (  0.02 sec)

32768-byte        ops        errors         time    msg/sec     MB/sec
           ---------- ------------- ------------ ---------- ----------
      read       5851        507754    3760.04ns     265954    5817.69
     write       6037       1325073    3644.19ns     274409    5817.69
```
