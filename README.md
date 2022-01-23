# DV1628_Lab3

## Current Issues:

* Vi har /abc och /a/def. append def ../abc get seg fault

* mv a/abc b ; seg fault



## Solved issues:
* RM relative path
    - använde filepath istället för temp_dir_nånting
* RM ../../a/b ; vill ta bort a inte dir b. Pga dir_path_temp_temp blir root inte a 
    - utkommenterade disk read på fel block rättatade.
* RM på folder seggar i root directory.
    - hade en cout på dir_path_temp_temp.back(), men den är tom eftersom att vi är i rooten.
* cd c/d tar en bara till c, verka vara pga Last item in path is folder.
    - Lade till en inparameter from_cd till funktionen så att vi undviker fallet.

* RM a/def ; Seg fault  
    -dum cout

* Append abc a/def skapar dubbla def 
    - Satte if relative check där vi skriver till disken, orskas av att vi inte refresha mappens innehåll emellan.


* cd ../.. i mappen a dör i str to vector. Borde ge felmeddelande istället.
    - Felmeddelande.

* cp abc a/abc - A file with that name already exists in this directory, try again
    - Kolla för destination 