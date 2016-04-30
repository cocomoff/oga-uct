for k in 3
do
	echo K=$k

	echo -n '' > K$k/k$k\New.txt

	for n in 30 60 100 300 500 700 900
	do
		echo $n
		./../../OGA/race/race_NEW_AL_OPT2 -f -a 0 -h 0 -s 0 -t 1000 ../../OGA/race/tracks/barto-big.track random uct $n 50 $k > K$k/res$n
		
		echo $n $(grep -n "uct(random" K$k/res$n | awk '{print $2,$3,$5}') >> K$k/k$k\New.txt	# Adding results to merged file too
	done

	cp K$k/k$k\New.txt ALL/ 
done

