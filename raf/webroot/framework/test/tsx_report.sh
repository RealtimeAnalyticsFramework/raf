find . -name "*Blank*.txt" | xargs ./tsx_analyze.sh
mv left.txt all_Blank.csv

find . -name "*Exp*.txt" | xargs ./tsx_analyze.sh
mv left.txt all_Exp.csv

find . -name "c120_Blank*.txt" | xargs ./tsx_analyze.sh
mv left.txt c120_Blank.csv

find . -name "c120_Exp*.txt" | xargs ./tsx_analyze.sh
mv left.txt c120_Exp.csv

find . -name "*c30_Blank*.txt" | xargs ./tsx_analyze.sh
mv left.txt c30_Blank.csv

find . -name "*c30_Blank*.txt" | xargs ./tsx_analyze.sh
mv left.txt c30_Exp.csv
