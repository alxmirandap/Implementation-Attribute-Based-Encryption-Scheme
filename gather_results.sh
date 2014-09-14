#  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
#  Code by: Alexandre Miranda Pinto
#
#  This file is a script to run the full gamut of performance tests and collect the data in log files.

red='\x1b[1;31m'
cyan='\x1b[1;36m'
purple='\x1b[1;35m'
yellow='\x1b[1;33m'
none='\x1b[0m'

echo -e "${red} Benchmark BL 1"
./benchmark_bl_1 all | tee "benchmark-results-BL-1.txt"
echo -e "${cyan} Benchmark BL 2"
./benchmark_bl_2 all | tee "benchmark-results-BL-2.txt"
echo -e "${purple} Benchmark SH 1"
./benchmark_sh_1 all | tee "benchmark-results-SH-1.txt"
echo -e "${yellow} Benchmark SH 2"
./benchmark_sh_2 all | tee "benchmark-results-SH-2.txt"
echo -e "${none} Done"