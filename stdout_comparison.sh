#! /usr/bin/bash

# Compare kernel output with standard output
# standard output is generated, and part of the repository
# This test should be ran by Cmake test

STANDARD_LOG=$1
CURRENT_LOG=$2

if [ ! -f "$CURRENT_LOG" ] || [ ! -f "$STANDARD_LOG" ]; then
  echo "log files does not exist."
  exit 1
fi

FILTERED_STANDARD_LOG=/tmp/staros_standard_output
FILTERED_CURRENT_LOG=/tmp/staros_current_output

# For debugging, kernel will log a timestamp
# So, just remove this line and then compare log files
sed '/DEBUG]: Compiled at:/d' $STANDARD_LOG > $FILTERED_STANDARD_LOG
sed '/DEBUG]: Compiled at:/d' $CURRENT_LOG > $FILTERED_CURRENT_LOG
diff -U 3 $FILTERED_STANDARD_LOG $FILTERED_CURRENT_LOG