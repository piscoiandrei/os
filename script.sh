if test $# -ne 1 
then
    echo "the wrong nr of arguments"
    exit 1
fi

if ! test -f "$1" || test -h "$1" 
then
    echo "$1 is not a c file"
    exit 1
fi
 
output_gcc=$(gcc -Wall -Wextra "$1" 2>&1)
exit_code=$?

if test $exit_code -ne 0
then
    errors=$(echo "$output_gcc" | grep -c "error" )
    warnings=$(echo "$output_gcc" | grep -c "warning" )

    echo "Number of errors $errors"
    echo "Number of warnings $warnings"
else
    echo "OK"
fi