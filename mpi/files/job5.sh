#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 5.cpp -o 5
mpirun ./5




