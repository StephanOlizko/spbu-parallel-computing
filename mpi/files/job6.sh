#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 6.cpp -o 6
mpirun ./6




