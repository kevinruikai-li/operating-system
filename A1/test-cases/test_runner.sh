#!/bin/bash

# Loop through all the test files in the current directory
for testfile in *.txt; do
    # Check if the file does not contain _result in its name
    if [[ $testfile != *_result.txt ]]; then
        # Run the test file with the specified command
        ../../project/src/mysh < "$testfile" > output.txt
        
        # Get the corresponding result file name
        resultfile="${testfile%.txt}_result.txt"
        
        # Compare the output with the result file
        if diff -q output.txt "$resultfile" > /dev/null; then
            echo "$testfile passed."
        else
            echo "$testfile failed."
        fi
        
        # Clean up the output file
        rm output.txt
    fi
done