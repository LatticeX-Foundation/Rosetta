#! /bin/bash
function run() {
	name1=$1
	name2=$2
	fullname=${name1}-${name2}.py
	logname=log/${name1}-${name2}
	echo "run $fullname ..."
	python3 $fullname --node_id=p0 > ${logname}-p0.log 2>&1 &
	python3 $fullname --node_id=p1 > ${logname}-p1.log 2>&1 &
	python3 $fullname --node_id=p2 > ${logname}-p2.log 2>&1 &
	python3 $fullname --node_id=p9 > ${logname}-p9.log 2>&1

	wait
	# sleep 3
}
mkdir -p log
date +"%Y-%m-%d %H:%M:%S"
run mt linear_regression_reveal
run mt matmul2
run mt matmul

run rtt matmul
run rtt ds-lr
run rtt linear_regression_feature_aligned
run rtt linear_regression_saver
run rtt linear_regression_restore
run rtt linear_regression_reveal
run rtt logistic_regression_feature_aligned
run rtt logistic_regression_saver
run rtt logistic_regression_restore
run rtt logistic_regression_reveal

run st ds-lr
run st linear_regression_feature_aligned
run st linear_regression_saver
run st linear_regression_restore
run st linear_regression_reveal
run st logistic_regression_feature_aligned
run st logistic_regression_saver
run st logistic_regression_restore
run st logistic_regression_reveal
date +"%Y-%m-%d %H:%M:%S"
