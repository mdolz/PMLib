#vtcc vt_test.c -o vt_test
mpicc-vt mpi_vt_test.c -o mpi_vt_test
#vtcc test.c -o test -fopenmp

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../

#export VT_PLUGIN_CNTR_METRICS=PmlibPlugin_127.0.0.1_LMG450-0_0:PmlibPlugin_127.0.0.1_LMG450-1_1:PmlibPlugin_127.0.0.1_LMG450-1_2

export VT_PLUGIN_CNTR_METRICS=PmlibPlugin_10.0.0.100_LMG450-1_2_CLUSTER:PmlibPlugin_10.0.0.100_LMG450-1_3
#export VT_PLUGIN_CNTR_METRICS=PmlibPlugin_10.0.0.100_LMG450-1_2_CLUSTER
#export VT_PLUGIN_CNTR_METRICS=PmlibPlugin_10.0.0.100_LMG450-1_2

#export VT_MODE=STAT

#./vt_test
mpirun -np 10 ./mpi_vt_test 
#./test

export SCOREP_METRIC_RUSAGE=all
export SCOREP_METRIC_PLUGINS=PmlibPlugin
export SCOREP_METRIC_PMLIBPLUGIN=127.0.0.1_LMG450-0_0
export SCOREP_ENABLE_TRACING=true
export SCOREP_ENABLE_PROFILING=false
export SCOREP_VERBOSE=true
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../

scorep icc vt_test.c -o vt_test_scorep
scorep mpicc mpi_vt_test.c -o mpi_vt_test_scorep

./vt_test_scorep
# mpirun -np 2 ./mpi_vt_test_scorep

