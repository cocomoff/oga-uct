cd ../
for n in 50 100 150 200 250 300
do
	echo $n
	./gol -f -a 0 -h 0 -s 0 -t 1000 100 random uct $n 50 1 > gol_latest/$n.txt
done
cd gol_latest