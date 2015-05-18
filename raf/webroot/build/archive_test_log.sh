#
# Archive logs for test cases.
# Caller SHOULD set sys env IT_CASE_NAME
#

LOG_DIR=it_log

#
# Convert ANSI color code to html.
#
ansiescape2html() {
  sed -e 's/ /\&nbsp;/g' -e 'a \ <br>' -e 's/\x1b\[0;32m/\<span style="color: green" \>/g' -e 's/\x1b\[0;31m/\<span style="color: red" \>/g' -e 's/\x1b\[m/\<\/span\>/g' -e 's/^\(W.*\)$/\<span style="color: brown" \>\1<\/span>/g' -e 's/^\(E.*\)$/\<span style="color: red" \>\1<\/span>/g'
}

#
# Generate index html.
#
index_log() {
  echo "<html><head><title>Log of Test Cases</title></head><body>"
 
  # skipped IT case
  echo "<h2>Cases Skipped</h2>"
  echo "<pre>"
  grep "^#case" build/it.sh
  echo "</pre>"

  echo "<hr>"
  echo "<h2>Last Case: $IT_CASE_NAME</h2>"

  # logs
  echo "<h2>Logs</h2>"

  # ls *.log 2>/dev/null | sed -e '1,$s/^\(.*\)$/<a href="\1.html">\1.html<\/a><br>/'
  for LF in `ls *.log 2>/dev/null`; do
    FILE_SIZE=`du -sh $LF | awk '{print $1;}'`
    echo $LF | sed -e '1,$s/^\(.*\)$/<a href="\1.html">\1.html<\/a>/'
    echo $FILE_SIZE
    echo "<br/>"
  done

  # core dump
  echo "<h2>Core Dump</h2>"
  echo "<pre>"
  for CF in `ls core* 2>/dev/null`; do
    echo "<h3>"
    file $CF
    echo "</h3>"
    EXE=`file $CF | sed 's/.*from \(.*\)/\1/' | sed "s/[\']//g" | awk '{print $1;}'`
    EXE=`basename $EXE`
    EXE=`find ./dist -name "$EXE"`
    gdb -ex "thread apply all bt" -ex quit $EXE $CF
    echo "<hr>"
  done
  echo "</pre>"

  echo "</body> </html>"
}

#
# Generate index html and archive all log files
#
gen_log_report() {
  rm -rf $LOG_DIR 2>/dev/null
  mkdir $LOG_DIR
  
  find . -name "*.log*" -exec mv {} . \; 2>/dev/null
  for LF in `ls hive_job_log*.txt 2>/dev/null`; do
    mv $LF $LF.log 
  done
  
  index_log >index.html
  for LF in `ls *.log 2>/dev/null`; do
    cat $LF | ansiescape2html >$LOG_DIR/$LF.html 
  done
  
  mv index.html $LOG_DIR
}

#
# kill all child processes and archive logs.
#
atexit_func() {
  echo "build terminates"
  jobs -p | xargs kill -9  >/dev/null 2>&1
  cd $WORKSPACE/idgs
  gen_log_report
}

#
# Register "EXIT" trap function
#
trap "atexit_func" EXIT

TBB_LIB=`find $TBB_HOME/build -name "*_release" | sed -e '/preview/d'`
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$TBB_LIB"
echo $LD_LIBRARY_PATH
