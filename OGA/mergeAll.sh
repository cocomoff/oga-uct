for domain in ctp gol navigation race sailing sysadmin
do
	echo $domain
	cd RESULTS_$domain
	#./merge.sh ASAP_NEW asapNew.txt
	#./merge.sh BASE_NEW baseNew.txt
	./merge.sh K1_NEW k1New.txt
	./merge.sh K3_NEW k3New.txt
	./merge.sh K6_NEW k6New.txt
	./merge.sh K10_NEW k10New.txt
	./merge.sh K20_NEW k20New.txt
	cd ../
done
