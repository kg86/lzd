#!/bin/sh

now=`date +%Y-%m-%d-%H-%M-%S`
num_repeat=1
flist=$1
purge="--purge"
purge=""
verbose="-v"
verbose=""
expdir="exp/$now/"
pydir="py/"
python="python2"

# expdir="./"
# pydir="../../py/"

echo "run compression and decompression"
$python ${pydir}run_test.py --act=compdecomp --num_repeat=$num_repeat --date=$now $flist $verbose $purge --fname=$flist

# measure several points from the log of the above

# measure compression ratio
echo "measure compression ratio"
$python ${pydir}comp_ratio.py $expdir $flist True > ${expdir}res.ratio.csv

# measure compression time
echo "measure compression time"
$python ${pydir}analyze_output.py comp ${expdir}run_test.comp.txt > ${expdir}res.comp.time.csv
echo "$python ${pydir}analyze_output.py comp ${expdir}run_test.comp.txt > ${expdir}res.comp.time.csv"
$python ${pydir}make_gp_single.py ${expdir}res.ratio.csv ${expdir}res.comp.time.csv

# measure compression speed (chars / seconds)
echo "measure compression speed (chars / seconds)"
$python ${pydir}analyze_output.py speed ${expdir}run_test.comp.txt > ${expdir}res.comp.speed.csv $flist
$python ${pydir}make_gp_single.py ${expdir}res.ratio.csv ${expdir}res.comp.speed.csv

# measure decompression time
echo "measure decompression time"
$python ${pydir}analyze_output.py comp ${expdir}run_test.decomp.txt > ${expdir}res.decomp.time.csv
$python ${pydir}make_gp_single.py ${expdir}res.ratio.csv ${expdir}res.decomp.time.csv

# measure decompression speed
echo "measure decompression speed"
$python ${pydir}analyze_output.py speed ${expdir}run_test.decomp.txt > ${expdir}res.decomp.speed.csv $flist
$python ${pydir}make_gp_single.py ${expdir}res.ratio.csv ${expdir}res.decomp.speed.csv

