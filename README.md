# MPI_Partix

MPI Partix is an application test suite for user-level threading (ULT) and partitioned communication (PC) support in MPI implementations. A generic tasking interface abstracts from several threading libraries.

## Usage

This software suite uses the CMake build tool. Several CMake options exist:
```
- CMAKE_BUILD_TYPE
- CMAKE_INSTALL_PREFIX            
- Partix_ENABLE_ARGOBOTS
- Partix_ENABLE_OPENMP
- Partix_ENABLE_PTHREADS
- Partix_ENABLE_QTHREADS
```
Enabling the Qthreads or Argobots backends requires to set the library installation path using the cmake configure option ```Qthreads_ROOT``` or ```Argobots_ROOT```.

## Performance Insights

Several run scripts are included for reproducibility. Executing the included run scripts on a two nodes with 2 x 24 core Intel(R) Xeon(R) Platinum 8160 CPU @ 2.10GHz each and connected over Intel OmniPath results in the following performance charts. The following charts show performance variations with a variable compute to communication ratio. In these cases, computation time ranges from zero to 160 ms. Compute phases can overlap with communication times as shown in the flow chart further below.

<img src=https://user-images.githubusercontent.com/755191/160702328-028dc6c0-d311-44d0-965f-dff9b48b7b38.png width=35% height=35% align="left">
<img src=https://user-images.githubusercontent.com/755191/160702337-d611741c-8037-4cb7-b3ae-39cf99a24acc.png width=35% height=35% align="left">
<img src=https://user-images.githubusercontent.com/755191/160702340-21d76ecf-93c8-421c-8911-b72591e03ffe.png width=35% height=35% align="left">
<img src=https://user-images.githubusercontent.com/755191/160702416-52e9c5d7-c4b2-4bbc-b4ac-adab7aadc2ec.png width=35% height=35% align="left">
<img src=https://user-images.githubusercontent.com/755191/160702444-6aa59656-e604-4432-8cd5-ad1dfc2991d3.png width=35% height=35% align="left">
<img src=https://user-images.githubusercontent.com/755191/160702445-1b152a3a-4530-48dc-82cc-6a2b55cde798.png width=35% height=35% align="left">
<img src=https://user-images.githubusercontent.com/755191/160702448-bb932f44-c8ce-4a6a-bad8-7ff2d5386d87.png width=35% height=35% align="left">



