echo ASAP
for domain in sailing sailing1 sailing2 sailing3 sailing4 
do
	echo $domain
	cd RESULTS_$domain
	./run_asap.sh
	cd ../
done
