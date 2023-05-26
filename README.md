# ScaPSRS
A scalable parallel sorting algorithm by regular sampling

## compile
> make

## run
1. run $p$ processes to ScaPSRS, $localnum$ data per process, `mpirun -n p scapsrs -n localnum`
> e.g. `mpirun -n 128 scapsrs -n 100000`

2. the distribution of the generated data, default is random, `-g rand`
> e.g. uniform distribution, `mpirun -n 128 scapsrs -n 100000 -g uniform`\
> e.g. normal distribution, `mpirun -n 128 scapsrs -n 100000 -g normal`\
> e.g. exponential distribution, `mpirun -n 128 scapsrs -n 100000 -g exp`
