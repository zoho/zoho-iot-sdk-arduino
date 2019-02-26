clear
rm -rf build/*
cd build
cmake ..
# cmake -I "/mymac/code/sdk/zoho-iot-sdk-arduino/unit_test/sample/fakeit_master/include" -I"/mymac/code/sdk/zoho-iot-sdk-arduino/unit_test/sample/fakeit_master/config/catch" ..
make unit_tests -j 8
./unit_tests -r junit > unit_test_result.xml
cd ..

