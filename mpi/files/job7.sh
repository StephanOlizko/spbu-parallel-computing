#!/bin/bash

module load gcc/9
module load openmpi
mpic++ 7.cpp -o 7
mpirun ./7




