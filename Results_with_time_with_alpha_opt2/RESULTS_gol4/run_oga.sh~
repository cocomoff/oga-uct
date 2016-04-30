for k in 3
do
	echo K=$k

	echo -n '' > K$k/k$k\New.txt

	for n in 50 100 150 200 250 300
	do
		echo $n
		./../../OGA/GOL4/gol_NEW_AL_OPT2  -f -a 0 -h 0 -s 0 -t 1000 100 random uct $n 50 $k > K$k/res$n
		
		echo $n $(grep -n "uct(random" K$k/res$n | awk '{print $2,$3,$5}') >> K$k/k$k\New.txt	# Adding results to merged file too
	done

	cp K$k/k$k\New.txt ALL/ 
done

