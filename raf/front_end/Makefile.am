SUBDIRS = . install


all-local:
	if test ! ${BUILD_FRONT}; then \
	if which mvn; then \
	  mvn ${MVN_OPTIONS} -f @srcdir@/pom.xml install -Dmaven.test.skip=true; \
	fi \
	fi

clean:
	if which mvn; then \
	  mvn ${MVN_OPTIONS} -f @srcdir@/pom.xml clean; \
	fi

install-exec-hook:
	if test ! ${BUILD_FRONT}; then \
	if which mvn; then \
	  mvn ${MVN_OPTIONS} -f @srcdir@/pom.xml install; \
          cd @srcdir@/install; ./gen-makefile.sh; cd - ; \
	fi \
	fi

