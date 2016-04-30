echo OGA
for domain in gol gol1 gol2 gol3 gol4 gol5
do
	echo $domain
	cd RESULTS_$domain
	./run_asap.sh
	cd ../
done
