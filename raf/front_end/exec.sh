if [ $# -eq 0 ]; then
  ./idgs.sh
elif [ $# -eq 1 ]; then
  echo "$1;" | ./idgs.sh
else
  echo "run command : exec or exec "sql""
fi
