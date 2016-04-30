echo OGA
for domain in sysadmin sysadmin1
do
	echo $domain
	cd RESULTS_$domain
	./run_oga.sh
	cd ../
done
