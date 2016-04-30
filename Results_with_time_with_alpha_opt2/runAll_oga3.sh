echo OGA
for domain in gol gol1
do
	echo $domain
	cd RESULTS_$domain
	./run_oga.sh
	cd ../
done
