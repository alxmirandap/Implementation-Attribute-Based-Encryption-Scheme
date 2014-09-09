#bbench all | tee -a "benchmark-results.txt"
red='\x1b[1;31m'
cyan='\x1b[1;36m'
purple='\x1b[1;35m'
yellow='\x1b[1;33m'
none='\x1b[0m'
#echo -e "${red}Hello Stackoverflow${NC}"


echo -e "${red} Benchmark BL 1"
./benchmark_bl_1 dxi | tee -a "fast_benchmark-results-BL-1.txt"
echo -e "${cyan} Benchmark BL 2"
./benchmark_bl_2 dxi | tee -a "fast_benchmark-results-BL-2.txt"
echo -e "${purple} Benchmark SH 1"
./benchmark_sh_1 dxi | tee -a "fast_benchmark-results-SH-1.txt"
echo -e "${yellow} Benchmark SH 2"
./benchmark_sh_2 dxi | tee -a "fast_benchmark-results-SH-2.txt"
echo -e "${none} Done"