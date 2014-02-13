#!/bin/bash
#
# generate GCH (precompiled header)
#

GCH_HEAD=`cat -E <<END
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(_GNUC_) || defined(__clang__) 
END`

echo $GCH_HEAD >gch_head.txt
sed -i -e '1,1s/\$ /\n/g' -c gch_head.txt 

#
# Parameters:
#   $1: source file to be enhanced 
#
enhance_gch() {
  F=$1
  # check whether has headers
  TMP=`grep "idgs_gch.h" $F`
  if [ "$TMP" = "" ] ; then
    echo "Enhancing $F for gch"
    cat gch_head.txt $F >$F.tmp
    mv $F.tmp -f $F
  fi
}

for SF in `find . -name "*.cpp"` ; do
  enhance_gch $SF
done

rm -f gch_head.txt
