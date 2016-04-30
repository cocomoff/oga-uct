cd $1
echo -n '' > $2
for n in 30 60 100 150 200 300 500
do
	echo $n $(grep -n "uct(random" res$n | awk '{print $2}') $(grep -n "uct(random" res$n | awk '{print $3}') $(grep -n "uct(random" res$n | awk '{print $5}') >> $2
done
cd ../
cp $1/$2 ALL/
