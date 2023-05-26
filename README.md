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

3. set the acceptable imbalance ratio $\in(0,0.5)$, default is 0.1, `-i 0.1`
> e.g. `mpirun -n 128 scapsrs -n 100000 -i 0.1`
