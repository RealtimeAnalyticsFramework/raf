

#
# generate a template file
#
gen_base() {
  ALL_RANGES="2 4 6 8 12 16 24 32 48 64 128 256 512 1024 2048 4096 8192 16384"
  rm base.txt >/dev/null 2>&1
  echo "Range" >>base.txt
  for R in $ALL_RANGES ; do
    echo $R >>base.txt
  done
}

#
# format a result file
# $1 input file
#
format_file() {
  FN=$1
  BN=`basename $FN`
  BN=`echo $BN | sed -e 's/.txt//'`
  RF="right.txt"
  echo "Range,$BN" >$RF
  cat $FN | awk -F ,  '{ gsub(/^[ \t]+/, "", $1); gsub(/[ \t]+$/, "", $1); gsub(/^[ \t]+/, "", $2); gsub(/[ \t]+$/, "", $2); print $1 "," $2;}' >>$RF
}

#format_file c120/c120_BlankCodec-map-1-mutex.txt
#join --nocheck-order -t\, base.txt right.txt

#
# join all files
#
join_files() {
  gen_base
  mv base.txt left.txt
  for F in $@; do
    format_file $F
    join --nocheck-order -t\, left.txt right.txt >middle.txt
    mv middle.txt left.txt
  done
  rm right.txt
}

join_files $@

#
# usage:
# find . -name "c120*.txt" | xargs ./tsx_analyze.sh
#
