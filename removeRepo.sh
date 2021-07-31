#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
CWD=$PWD
echo -e "\n\nEliminando repo\n\n"
cd /home/utnso/TPCUATRI
rm -rf tp-2021-1c-Pascusa/
git clone https://github.com/sisoputnfrba/tp-2021-1c-Pascusa
cd /home/utnso/TPCUATRI/tp-2021-1c-Pascusa
echo -e "\n\nBuilding projects...\n\n"
make -C ./Discordiador
make -C ./I-MongoStore
make -C ./Mi-RAM
echo -e "\n\nFletaste el repo exitosamente FIUUUMBA\n\n"
