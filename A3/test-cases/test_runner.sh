#!/bin/bash

for testfile in *.txt; do
    if [[ $testfile == *_result.txt || $testfile == *_result2.txt ]]; then
        continue
    fi

    resultfile="${testfile%.txt}_result.txt"
    resultfile2="${testfile%.txt}_result2.txt"
    actual_result=""

    if [[ -f "$resultfile" ]]; then
        actual_result="$resultfile"
    elif [[ -f "$resultfile2" ]]; then
        actual_result="$resultfile2"
    else
        echo "$testfile has no result file, skipping."
        continue
    fi

    read framesize varmemsize <<< $(grep -m1 "Frame Store Size" "$actual_result" | awk -F'[^0-9]+' '{print $2, $3}')
    
    if ! [[ "$framesize" =~ ^[0-9]+$ ]] || ! [[ "$varmemsize" =~ ^[0-9]+$ ]]; then
        echo "Error: Invalid sizes in $actual_result"
        continue
    fi

    echo "Compiling for $testfile with FRAMESIZE=$framesize, VARMEMSIZE=$varmemsize"
    (cd ../../project/src/ && make clean && make framesize="$framesize" varmemsize="$varmemsize")

    ../../project/src/mysh < "$testfile" > output.txt

    show_mismatch() {
        echo ""
        echo "===== Expected (from $1): ====="
        cat "$1"
        echo ""
        echo "===== Actual (from output.txt): ====="
        cat output.txt
        echo ""
    }

    if [[ -f "$resultfile" && -f "$resultfile2" ]]; then
        if diff -q -w -B output.txt "$resultfile" > /dev/null; then
            echo "$testfile passed (matched $resultfile)"
        elif diff -q -w -B output.txt "$resultfile2" > /dev/null; then
            echo "$testfile passed (matched $resultfile2)"
        else
            echo "$testfile failed. No match with either result file"
            show_mismatch "$resultfile"
        fi
    elif [[ -f "$resultfile" ]]; then
        if diff -q -w -B output.txt "$resultfile" > /dev/null; then
            echo "$testfile passed"
        else
            echo "$testfile failed"
            show_mismatch "$resultfile"
        fi
    else
        if diff -q -w -B output.txt "$resultfile2" > /dev/null; then
            echo "$testfile passed"
        else
            echo "$testfile failed"
            show_mismatch "$resultfile2"
        fi
    fi

    rm output.txt
done
