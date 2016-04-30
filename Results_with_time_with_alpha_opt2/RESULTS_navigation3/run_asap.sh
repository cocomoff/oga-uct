echo -n '' > ASAP/asapNew.txt

for n in 30 60 100 150 200 250 300 350 400 450 500
do
	echo $n
	./../../ASAP/navigation3/navigation_NEW_AL  -f -a 0 -h 0 -s 0 -t 1000 100 random uct $n 50 2 > ASAP/res$n
	
	echo $n $(grep -n "uct(random" ASAP/res$n | awk '{print $2,$3,$5}') >> ASAP/asapNew.txt	# Adding results to merged file too
done

cp ASAP/asapNew.txt ALL/

