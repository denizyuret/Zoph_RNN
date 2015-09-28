source /usr/usc/cuda/7.0/setup.sh
source /usr/usc/boost/1.55.0/setup.sh
source /usr/usc/gnu/gcc/4.8.1/setup.sh

nvcc -O3 -Xcompiler -fopenmp -I /usr/usc/cuda/7.0/include/ -I /usr/usc/boost/1.55.0/include/     /usr/usc/boost/1.55.0/lib/libboost_system.a       /usr/usc/boost/1.55.0/lib/libboost_filesystem.a   /usr/usc/boost/1.55.0/lib/libboost_program_options.a -I /home/nlg-05/zoph/eigen/ -std=c++11 -lcublas main.cu