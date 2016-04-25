#!/bin/bash
#
#SBATCH -J vt_test
#SBATCH --ntasks=1
#SBATCH -p compute

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.:..
export VT_PLUGIN_CNTR_METRICS=PmlibPlugin_sandy1_CPU_0-7:PmlibPlugin_10.0.0.100_LMG450-0_0

./vt_test
