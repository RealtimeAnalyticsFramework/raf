merge() {
  path=front_end/idgs-sql
  param=$1
  head=
  value=
  for filename in `ls $path/$param*.txt`
  do
      head=$head`cat $filename | awk -F ' ' '{print $1}'`,
      value=$value`cat $filename | awk -F ' ' '{print $2}'`,
  done

  txtfile=ssb$param.txt
  if [ -f $txtfile ]; then
    rm $txtfile
  fi 

  echo ${head%?} >> $txtfile
  echo ${value%?} >> $txtfile

  cat $txtfile

  rm $path/$param*.txt
}

merge Q1
merge Q2
merge Q3
merge Q4
