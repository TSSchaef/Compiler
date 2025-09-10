make -s
./mycc -2 test/test0/test.c > test/test0/test_out.txt
TEST_DIFF=$(diff test/test0/test_out.txt test/test0/test.txt)
if [ -z "$TEST_DIFF" ]; then
    echo "Test 0 passed!"
    rm -f test/log/test0_diff.log
else
    echo "Test 0 FAILED!"
    echo "$TEST_DIFF" > test/log/test0_diff.log
fi

./mycc -2 test/test1/test1.c > test/test1/test_out.txt
TEST1_DIFF=$(diff test/test1/test_out.txt test/test1/test1.txt)
if [ -z "$TEST1_DIFF" ]; then
    echo "Test 1 passed!"
    rm -f test/log/test1_diff.log
else
    echo "Test 1 FAILED!"
    echo "$TEST1_DIFF" > test/log/test1_diff.log
fi


