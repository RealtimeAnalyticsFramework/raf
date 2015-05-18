#!/bin/bash


HS=`find . -name "*.jar" | sed -e '1,$s/\.\//  /' -e '1,$s/\(.*\)$/\1 \\\\/' -e '$s/\\\\$//' `

cat <<END >Makefile.am.tmp
JARFILES = \\
$HS

javalib_DATA = \$(JARFILES)

\$(JARFILES):
	mvn ${MVN_OPTIONS} -f @srcdir@/pom.xml install -Dmaven.test.skip=true;

bin_SCRIPTS = \
        src/main/scripts/idgs-sql-cli.sh \
        src/main/scripts/idgs-sql-cli.cmd

END

if diff Makefile.am.tmp Makefile.am; then
  echo "No change"
  rm Makefile.am.tmp
else
  echo "genertate new Makefile"
  mv Makefile.am.tmp Makefile.am
fi
