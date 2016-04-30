echo OGA
for domain in race race1 race2 race3 race4 race5
do
	echo $domain
	cd RESULTS_$domain
	./run_oga.sh
	cd ../
done
