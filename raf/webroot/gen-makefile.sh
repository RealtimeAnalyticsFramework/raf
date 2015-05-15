#!/bin/bash


HS=`find . -type f | sed -e '/akefile/d' -e '1,$s/\.\//  /' -e '1,$s/\(.*\)$/\1 \\\\/' -e '$s/\\\\$//' `

cat <<END >Makefile.am.tmp

nobase_webroot_DATA = \\
$HS

END

if diff Makefile.am.tmp Makefile.am; then
  echo "No change"
  rm Makefile.am.tmp
else
  echo "genertate new Makefile"
  mv Makefile.am.tmp Makefile.am
fi

SLFB_TMP=`tac Makefile.am | sed -n 2p`
echo $SLFB_TMP
SLFB=${SLFB_TMP/\\/""}
echo $SLFB
sed '$d' -i Makefile.am
sed '$d' -i Makefile.am
echo "$SLFB" >> Makefile.am
echo <<END  >> Makefile.am

