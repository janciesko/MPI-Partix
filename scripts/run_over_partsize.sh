binary=$1
hosts=$2
if [ x"$binary" = x ]; then
  echo "Usage: sh run.sh binary hostlist"
  echo "hostlist : host1,host2"
  exit 1
fi

# date
# mpirun -n 2 --host ${hosts} hostname

start_total_length_kb=16 #16KB total
let total_partlen_elems_start=16*1024/8 #MPI_DOUBLE
num_tasks=1
num_threads=$num_tasks
num_part=$num_tasks

export QTHREAD_STACK_SIZE=8192
#export OMPI_MCA_pml=ob1

echo "tasks,threads,partitions,partsize(KB),totalsize(KB),time,bw(MB)"
for t in {1..10..1}; do 
  total_partlen_elems=$total_partlen_elems_start
  for size in {1..18..1}; do 
    let num_partlen=$total_partlen_elems/$num_tasks
     mpirun --map-by ppr:1:node --host  $hosts -x LD_PRELOAD=/home/projects/x86-64/gcc/10.2.0/lib64/libstdc++.so.6 -x QTHREAD_STACK_SIZE=8196 $binary $num_tasks $num_threads $num_part $num_partlen
    let total_partlen_elems=$total_partlen_elems*2
  done
  let num_tasks=$num_tasks*2
  num_threads=$num_tasks
  num_part=$num_tasks
done
