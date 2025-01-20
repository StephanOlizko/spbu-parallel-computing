#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 2.cpp -o 2
mpirun ./2




