#!/bin/bash

for testfile in *.txt; do
    if [[ $testfile == *_result.txt || $testfile == *_result2.txt ]]; then
        continue
    fi

    ../../project/src/mysh < "$testfile" > output.txt
    
    resultfile="${testfile%.txt}_result.txt"
    resultfile2="${testfile%.txt}_result2.txt"


    if [[ -f "$resultfile" && -f "$resultfile2" ]]; then
        if diff -q -w -B output.txt "$resultfile" > /dev/null; then
            echo "$testfile passed (matched $resultfile"
        elif diff -q -w -B output.txt "$resultfile2" > /dev/null; then
            echo "$testfile passed (matched $resultfile2"
        else
            echo "$testfile failed. No match with $resultfile or $resultfile2 "
        fi
    elif [[ -f "$resultfile" ]]; then
        if diff -q -w -B output.txt "$resultfile" > /dev/null; then
            echo "$testfile passed"
        else
            echo "$testfile failed."
        fi
    elif [[ -f "$resultfile2" ]]; then
        if diff -q -w -B output.txt "$resultfile2" > /dev/null; then
            echo "$testfile passed"
        else
            echo "$testfile failed."
        fi
    else
        echo "$testfile has no result file, skipping."
    fi

    rm output.txt
done
