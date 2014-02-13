#!/bin/bash
# prune include files one at a time, recompile, and put them back if it doesn't compile
# arguments are list of files to check
# e.g. find . -name "*.h" | xargs cleaninclude.sh

#
# remove header file from source file
# $1 source file
# $2 header
removenamespace() {
    local file=$1
    local namespace=$2
    perl -i -p -e 's+([ \t]*using[ \t][ \t]*namespace[ \t]'$namespace'[ \t]*;)+//REMOVENAMESPACE $1+' $file
}

#
# revert the source file
# $1 source file
replacenamespace() {
   local file=$1
   perl -i -p -e 's+//REMOVENAMESPACE ++' $file
}

#
# main loop
#
for file in $*
do
    namespaces=`grep "^[ \t]*using[ \t][ \t]*namespace" $file | awk '{print $3;}' | sed 's/;//g'`
    for i in $namespaces
    do
        touch $file # just to be sure it recompiles
        removenamespace $file $i

	make -j8 -k >/dev/null 2>&1
        if make >/dev/null  2>&1;
        then
            grep -v REMOVENAMESPACE $file > tmp && mv tmp $file
            echo removed $i from $file
        else
            replacenamespace $file
            echo $i was needed in $file
        fi
    done
done
