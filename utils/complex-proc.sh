#!/bin/bash
# A program that generates some subprograms.  For testing WvSubProc.
MAIN="$1-pid$$"

go()
{
	SUB="$1"
	
	trap "echo Exiting $SUB on TERM...; exit 0" SIGTERM
	trap "echo Exiting $SUB on INT...; exit 0" SIGINT

	for a in 1 2 3 4 5 6 7 8 9 10; do
		echo "   $SUB.$a"
		sleep 1
	done
	
	echo "Done $SUB."
}

trap "echo Exiting $MAIN on TERM...; exit 0" SIGTERM
trap "echo Exiting $MAIN on INT...; exit 0" SIGINT

for d in 1 2 3 4 5 6 7 8 9 10; do
	echo "Starting $MAIN.$d..."
	go $MAIN.$d &
	sleep 3
done

echo "Done $MAIN."

