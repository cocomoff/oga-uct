#echo Vanilla
for domain in acadadvising acadadvising1 acadadvising2 acadadvising3 navigation navigation1 navigation2 navigation3 navigation4 sysadmin sysadmin1 sysadmin2 sysadmin3 sysadmin4 sysadmin5 race race1 race2 race3 race4 race5 gol gol1 gol2 gol3 gol4 gol5 sailing sailing1 sailing2 sailing3 sailing4
do
	#echo $domain
	rm RESULTS_$domain/K3/*
	rm RESULTS_$domain/ALL/k3New.txt
	#cp ../Results_with_time_with_alpha/RESULTS_$domain/ALL/k3New.txt RESULTS_$domain/ALPHA1/
done
