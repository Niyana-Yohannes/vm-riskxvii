#!/bin/bash

directory="tests_source_code"
my_array=("itype_operations/itype_operations.mi" "loadstore_operations/loadstore_operations.mi" "rtype_operations/rtype_operations.mi")
test_cases=(tests/*.in)

# Run the tests
for i in "${!my_array[@]}"; do

    string="${my_array[i]}"
    test_case="${test_cases[i]}"

    expected_output="${test_case%.in}.out"

    if ./vm_riskxvii "$directory/$string" < "$test_case" | diff - "$expected_output" > /dev/null; then
        echo "$test_case passed"
    else
        echo "$test_case failed"
    fi
done