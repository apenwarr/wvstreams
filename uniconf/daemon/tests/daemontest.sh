#!/bin/sh

# Shell script file to test some basic functionality of the UniConfDaemon.
# Note:  This uses the files daemontest##.txt and daemonresult##.txt

function do_help
{
    echo ""
    echo "This script tests the UniConfDaemon, via nc"
    echo "usage:  daemontest.sh [-h] [-host <host>] [-port <port>]"
    echo "Options:"
    echo "     -h          :  Display this help page"
    echo "     -host <host>:  Test the UniConfDaemon on the specified server."
    echo "     -port <port>:  Use the specified port number"
    echo "     -noless     :  Don't less the difference files on a failed test." 
    echo ""
    echo "Note:  To add more tests to this script, simply create a new daemontest*.txt"
    echo "file in this directory, and create an appropriate real_res_daemontest*.txt file."
    echo ""
    exit 0
}

if [ "$1" = "-h" ]; then
    do_help
fi

# Set up the basic variables now
testfiles=$(ls daemontest*.txt)
failedfiles=
nl=
host="localhost"
port="4111"
# Now poll any command line arguments to get specified vars

until [ $# -eq 0 ]; do
    case $1 in 
    "-host" )
        shift
        if [ $# -eq 0 ]; then
            do_help
        else
            host="$1"
        fi
    ;;
    "-port" )
        shift
        if [ $# -eq 0 -o ! isnum $1 ]; then
            do_help
        else
            port=$1
        fi
    ;;
    "-noless" )
        nl="-"
    ;;
    * )
        do_help
    ;;
    esac
    shift
done

# Now create our nc command
cmd="nc -q 100 $host $port"

for filename in $testfiles; do
#    echo $filename
    $cmd < $filename > res_$filename
    diff -b real_res_$filename res_$filename > diff_$filename
    if [ -s diff_$filename ]; then
        echo "TEST WITH FILE:  $filename FAILED!"
        failedfiles="$failedfiles $filename"
    else
        rm -f diff_$filename res_$filename
    fi
done

if [ -n "$nl" ]; then
    exit 0
fi

for filename in $failedfiles; do
    less -Psdiff_$filename diff_$filename
done

