valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file="val.txt" -s  ./server -p 9090 -o ./Extras/logs.txt -l 4 -d Extras/annual-enterprise-survey-2019-financial-year-provisional-csv.csv