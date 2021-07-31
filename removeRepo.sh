#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
CWD=$PWD
echo -e "Eliminando repo"
cd /home/utnso/TPCUATRI
rm -rf tp-2021-1c-Pascusa/
git clone https://github.com/sisoputnfrba/tp-2021-1c-Pascusa
