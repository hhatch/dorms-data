#!/bin/bash
# usage: ./run.sh > run.log 2>&1

# install simulation (run.cpp)
mkdir build
pushd build
  cmake ..
  make
popd

# squeue simulation on HPC node
((num_hours=5*24))
num_procs=32
((num_procs_ext=2*$num_procs))

# Write a SLURM script to file and queue it.
function launch_node {
cat << _EOF_ > launch.cmd
#!/bin/bash
#SBATCH -n ${num_procs_ext}
#SBATCH -N 1
#SBATCH -t ${num_hours}:00:00
#SBATCH -o hostname_%j.out
#SBATCH -e hostname_%j.out
echo "Running on host \$(hostname)"
echo "Time is \$(date)"
echo "Directory is \$PWD"
echo "ID is \$SLURM_JOB_ID"
echo "CPU is `grep "model name" /proc/cpuinfo | sort -u | cut -d : -f 2-`"

cd \$PWD
./build/run --num_procs ${num_procs} --num_hours $num_hours

echo "Time is \$(date)"
_EOF_

sbatch launch.cmd
}

launch_node
