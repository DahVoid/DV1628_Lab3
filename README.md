# DV1628_Lab3

## Current Issues:
* cd c/d tar en till c, verka vara pga Last item in path is folder.
* cd ../.. i mappen a dör i str to vector.
* RM på folder seggar i root directory.

## Solved issues:
* RM relative path'
    - använde filepath istället för temp_dir_nånting
* RM ../../a/b ; vill ta bort a inte dir b. Pga dir_path_temp_temp blir root inte a 
    - utkommenterade disk read på fel block rättatade.