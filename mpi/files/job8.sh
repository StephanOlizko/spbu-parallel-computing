#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 8.cpp -o 8
mpirun ./8




