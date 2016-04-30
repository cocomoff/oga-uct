echo OGA
for domain in sysadmin4 sysadmin5
do
	echo $domain
	cd RESULTS_$domain
	./run_oga.sh
	cd ../
done
