#!/bin/bash

# Boucle pour executer le script plusieurs fois

# Définir le nom du fichier
file='test.cc'

for i in {1..10..1}
do
   echo "Execution numéro : $i"
   val=$i
   # Utiliser sed pour chercher et remplacer la chaîne pour la nouvelle valeur de DataRate
   sed -i -e "s/[0-9]*kbps/${val}kbps/g" $file
   ./../ns3 run "test --rate=${val}" # execution du code simplement
done



