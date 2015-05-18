#/bin/bash
# this script should be run under the 'src' directory and a new Makefile.am will
# be generated.


HS=`find . -name "*.h" | sed -e '1,$s/\.\//  /' -e '1,$s/h$/h \\\\/' -e '$s/\\\\$//' `

cat <<END >Makefile.am
nobase_include_HEADERS = \
$HS


END
