
export idgs_group="jk_nightly"
export IDGS_HOME=$WORKSPACE/idgs/dist


BUILD_DIR=$WORKSPACE/gcc_debug
export BUILD_DIR

if test ! -d "$BUILD_DIR"  ; then
  mkdir $BUILD_DIR
fi

cd $BUILD_DIR
/bin/bash $WORKSPACE/idgs/build/build.sh

cd $WORKSPACE/idgs
# API docs
/bin/bash $WORKSPACE/idgs/build/gen-apidoc.sh
# static check
/bin/bash $WORKSPACE/idgs/build/check.sh

