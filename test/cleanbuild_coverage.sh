cd test
rm -rf build

set -e

mkdir build
cd build

cmake ..
make -j 8

./unit_tests -r junit > unit_test_result.xml
./unit_tests

lcov -c -d . -o coverage_swp.info
lcov --REMOVE coverage_swp.info 'tests/*' '/usr/*' '/zoho-iot-sdk-arduino/test/*' --OUTPUT-FILE coverage.info

genhtml coverage.info -o coverage

gcovr -x -r ../../ -e ../../test/  -o cobertura.xml