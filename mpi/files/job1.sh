#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 1.cpp -o 1
mpirun ./1




