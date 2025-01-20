#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 10.cpp -o 10
mpirun ./10




