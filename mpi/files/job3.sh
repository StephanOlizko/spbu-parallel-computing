#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 3.cpp -o 3
mpirun ./3




