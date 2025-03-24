#!/usr/bin/env bash

##################################################
echo ""
echo "     Date Time: $(date)"
echo "Operate System: $(uname -a)"
##################################################
echo -e "\n>>>>>>>>>>>>>> CPU Info <<<<<<<<<<<<<<"
physical_cpus=$(grep "physical id" /proc/cpuinfo | sort | uniq | wc -l)
logical_cpus=$(grep "processor" /proc/cpuinfo | wc -l)
cpu_kernels=$(grep "cores" /proc/cpuinfo | uniq | awk -F ': ' '{print $2}')
cpu_type=$(grep "model name" /proc/cpuinfo | awk -F ': ' '{print $2}' | sort | uniq)
cpu_arch=$(uname -m)
echo "Physical(s): $physical_cpus"
echo " Logical(s): $logical_cpus"
echo "  Cores/CPU: $cpu_kernels"
echo "       Type: $cpu_type"
echo "       Arch: $cpu_arch"
##################################################
echo -e "\n>>>>>>>>>>>>>> Mem Info <<<<<<<<<<<<<<"
free -h
##################################################
echo -e "\n>>>>>>>>>>>>> Disk Info <<<<<<<<<<<<<<"
df -hiP | sed 's/Mounted on/Mounted/' >/tmp/inode
df -hTP | sed 's/Mounted on/Mounted/' >/tmp/disk
join /tmp/disk /tmp/inode | awk '{print $1,$2,"|",$3,$4,$5,$6,"|",$8,$9,$10,$11,"|",$12}' | column -t
##################################################
echo ""
