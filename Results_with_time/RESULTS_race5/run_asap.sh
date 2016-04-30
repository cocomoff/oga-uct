echo -n '' > ASAP/asapNew.txt

for n in 100 300 500 700 900
do
	echo $n
	./../../ASAP/race/race_NEW -f -a 0 -h 0 -s 0 -t 1000 ../../ASAP/race/tracks/ring-5.track random uct $n 50 2 > ASAP/res$n
	
	echo $n $(grep -n "uct(random" ASAP/res$n | awk '{print $2,$3,$5}') >> ASAP/asapNew.txt	# Adding results to merged file too
done

cp ASAP/asapNew.txt ALL/

