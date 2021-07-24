#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "\n\nInstalling nivel-gui libraries...\n\n"
sudo apt-get install libncurses5-dev
NIVEL="so-nivel-gui-library"
git clone "https://github.com/sisoputnfrba/${NIVEL}.git" $NIVEL
cd $NIVEL
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "Trayendo tareas...\n\n"
cd ~
TAREAS="a-mongos-pruebas"
git clone "https://github.com/sisoputnfrba/${TAREAS}.git" $TAREAS
mv ${TAREAS}/Finales ..
cd $CWD
#echo -e "\n\nBuilding projects...\n\n"
#make -C ./Discordiador
#make -C ./I-MongoStore
#make -C ./Mi-RAM
#echo -e "\n\nDeploy done!\n\n"
