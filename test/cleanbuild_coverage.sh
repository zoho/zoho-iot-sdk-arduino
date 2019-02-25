clear
rm -rf build/*
cd build
cmake ..
# cmake -I "/mymac/code/sdk/zoho-iot-sdk-arduino/unit_test/sample/fakeit_master/include" -I"/mymac/code/sdk/zoho-iot-sdk-arduino/unit_test/sample/fakeit_master/config/catch" ..
make unit_tests_coverage -j 8
./unit_tests
cd ..

