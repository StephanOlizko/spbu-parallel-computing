#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 9.cpp -o 9
mpirun ./9




