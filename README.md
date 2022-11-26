# ipc
L'objectif de ce mini projet est de tester la fonction random de la bibliothèque C.
Notamment son équilibrage, c'est à dire la répartition des valeurs sur l'intervalle [0, RAND_MAX].
La fonction rand() étant non réentrante, il faut utiliser les processus qui garantissent ainsi son bon fonctionnement.
L'idée est de remplir un tableau de répartition [0, 1000000] dans une mémoire partagée entre des processus qui utilisent la fonction rand(). Ce tableau pourrait être mis à jour toutes les 1000000000 valeurs générées par chaque processus, en gérant bien évidemment tous les problèmes inhérents à l'aspect écriture de valeurs dans une zone commune...

Pour ce faire, vous utiliserez les processus (nombre paramétrable) et les IPC (mémoire partagée, sémaphores et plus si affinité). Processus sur des PC distants. À la fin de la génération des nombres, le processus maître devra vérifier si la fonction random du C est équilibrée ou non (bande de 5% par exemple).
