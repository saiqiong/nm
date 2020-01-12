#!/bin/zsh

GRN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

for filename in /bin/* /sbin/* /usr/lib/* /usr/bin/* /usr/sbin/*; do
    RESULT=$(diff <(nm $filename 2>&-) <(~/nm/ft_nm $filename 2>&-))
    if [[ $? -ne 0 ]]
    then
	echo "[${RED}ko${NC}] -- $filename"
	echo "$filename" >> nm_fail.txt
	sleep 1
    else
	echo "[${GRN}ok${NC}] -- $filename"
    fi
done
