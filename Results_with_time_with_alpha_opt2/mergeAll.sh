for domain in navigation acadadvising race sysadmin #gol sailing
do
	echo $domain
	cd RESULTS_$domain
	./merge.sh ASAP asapNew.txt
	./merge.sh BASE baseNew.txt
	./merge.sh K1 k1New.txt
	./merge.sh K3 k3New.txt
	cd ../
done
