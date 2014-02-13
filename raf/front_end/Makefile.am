all:
	if test ! ${BUILD_FRONT}; then \
	if which mvn; then \
	  mvn -f @srcdir@/pom.xml test; \
	  mvn -f @srcdir@/pom.xml install -Dmaven.test.skip=true; \
	fi \
	fi

clean:
	if which mvn; then \
	  mvn -f @srcdir@/pom.xml clean; \
	fi

install:all
