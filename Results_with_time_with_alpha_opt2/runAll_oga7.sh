echo OGA
for domain in sysadmin2 sysadmin3
do
	echo $domain
	cd RESULTS_$domain
	./run_oga.sh
	cd ../
done
