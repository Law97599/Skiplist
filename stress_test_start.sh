#########################################################################
# > File Name:     stress_test_start.sh
# > Author:        JasonLaw
# > Created Time:  2022-01-02
# > Description:   压力测试脚本
#########################################################################
#!/bin/bash
g++ stress_test.cpp -o ./bin/stress  --std=c++11 -pthread  
./bin/stress