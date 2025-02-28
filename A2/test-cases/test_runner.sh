#!/bin/bash

for testfile in *.txt; do
    if [[ $testfile == *_result.txt || $testfile == *_result2.txt ]]; then
        continue
    fi

    ../../project/src/mysh < "$testfile" > output.txt
    
    resultfile="${testfile%.txt}_result.txt"
    resultfile2="${testfile%.txt}_result2.txt"

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
            echo "$testfile passed (matched $resultfile ignoring blank lines/whitespace)."
        elif diff -q -w -B output.txt "$resultfile2" > /dev/null; then
            echo "$testfile passed (matched $resultfile2 ignoring blank lines/whitespace)."
        else
            echo "$testfile failed. No match with $resultfile or $resultfile2 (even ignoring blank lines/whitespace)."
            # show_mismatch "$resultfile"
            # echo "===== (Alternate expected from $resultfile2) ====="
            # cat "$resultfile2"
        fi
    elif [[ -f "$resultfile" ]]; then
        if diff -q -w -B output.txt "$resultfile" > /dev/null; then
            echo "$testfile passed (ignoring blank lines/whitespace)."
        else
            echo "$testfile failed."
            # show_mismatch "$resultfile"
        fi
    elif [[ -f "$resultfile2" ]]; then
        if diff -q -w -B output.txt "$resultfile2" > /dev/null; then
            echo "$testfile passed (ignoring blank lines/whitespace)."
        else
            echo "$testfile failed."
            # show_mismatch "$resultfile2"
        fi
    else
        echo "$testfile has no result file, skipping."
    fi

    rm output.txt
done
