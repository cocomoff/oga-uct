for k in 1 3
do
	echo K=$k

	echo -n '' > K$k/k$k\New.txt

	for n in 30 60 100 150 200 300 350 400 450 500 600
	do
		echo $n
		./../../OGA/Sysadmin1/sysadmin_NEW -f -a 0 -h 0 -s 0 -t 1000 100 random uct $n 50 $k > K$k/res$n
		
		echo $n $(grep -n "uct(random" K$k/res$n | awk '{print $2,$3,$5}') >> K$k/k$k\New.txt	# Adding results to merged file too
	done

	cp K$k/k$k\New.txt ALL/ 
done

