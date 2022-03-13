Robert Rachtieanu
332CBa

Tema 3 - APD

Partea 1:
	Fiecare proces coordonator isi citeste din fisierul sau workerii carora
le trimit rankul lor. Isi tin minte rangurile proceselor sale si le trimit 
celorlalte procese coordonator partea lor de topologie pe care o afiseaza. 
Acestea trimit topologia la workeri, ce o primesc si o afiseaza.
	
Partea 2:
	Procesul 0 genereaza vectorul de dimensiunea primului parametru si apoi
genereaza pentru fiecare worker inceputul, finalul si vectorul pe care trebuie 
sa il dubleze si le trimite celorlalte procese din cluster-ul lui. Apoi trimite 
celorlalti coordonatori inceput... care la randul lor le trimit acestea 
fiecarui worker din clusterul lor. Workerii dubleaza vectorul pe portiunea 
alocata lor si apoi trimit coordonatorilor lor. Rangul 1 si 2 trimit portiunile
dublate primite la rangul 0, care imbina si afiseaza vectorul finalizat
