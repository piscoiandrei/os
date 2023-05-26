output_gcc=$(gcc -Wall "$1" 2>&1)
exit_code=$?

if test $exit_code -ne 0
then
    errors=$(echo "$output_gcc" | grep -c "error" )
    warnings=$(echo "$output_gcc" | grep -c "warning" )

    echo "$errors/$warnings"
else
    echo "OK"
fi
