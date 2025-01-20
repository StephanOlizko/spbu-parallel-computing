#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 11.cpp -o 11
mpirun ./11




