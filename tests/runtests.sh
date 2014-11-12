echo "Running integration tests:"

-rm tests/tests.log
for i in tests/*_itest
do
    if test -f $i
    then
        build/jozabad &
        if $VALGRIND ./$i 2>> tests/tests.log
        then
            echo $i PASS
        else
            echo "ERROR in test $i: here's tests/tests.log"
            echo "------"
            tail tests/tests.log
            exit 1
        fi
        killall -SIGHUP jozabad
    fi
done

echo ""
