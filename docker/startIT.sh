EXIST=`docker images | grep raf_compiled`
while [ -z "$EXIST"]
do
  echo "raf_compiled is empty"
  EXIST=`docker images | grep raf_compiled`
  sleep 3
done

#docker build -t raf_it .
docker run -t --rm raf_compiled /home/project/./startIT

