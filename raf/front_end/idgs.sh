rm /tmp/root/*.log
export SQL_ENGINE_HOME=/root/cdt-workspace/idgs/front_end/sql_engine/target/classes/
mvn exec:java -pl sql_engine -Dexec.mainClass="idgs.IdgsCliDriver" -Dexec.args="$@" -Dexec.classpathScope=runtime
