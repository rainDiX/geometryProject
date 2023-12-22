# Geometry Assignment/Project
![Smoothed Stanford Bunny with a horn](/.readme/bunicorn.png?raw=true "Smoothed Stanford Bunny with a horn")

## Building the project
### Dependencies
- cmake
- a C++20 compiler
- SDL2
- VTK (with SDL2 enabled)
- Eigen

### Building the dependencies locally
You have to install the dependencies of SDL2, VTK and Eigen beforehand
```sh
cd deps
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=../prefix
cmake --build build --config RelWithDebInfo --parallel
```

### Building the project
```sh
cd ..
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=./prefix
cmake --build build --parallel
```


# ToDo (French)
## À réaliser pour le TP :

- Choix d'une bibliothèque de manipulation et d'affichage de maillages.
- Selection du voisinage d'un point par le nombre d'anneaux topologiques.
- Calcul et affichage (par variation de couleur) du Laplacien par diffusion.
- Calcul et affichage du Laplacien par résolution d'un système linéaire. 

## À réaliser pour le Projet :
- Lissage Laplacien. 
- Selection d'un voisinage par une zone. 
- Déformation du maillage par translation pondérée par le Laplacien calculé entre 0 (bord) et 1 (poignée) sur le maillage.
- Modification des valeurs du Laplacien par une fonction de transfert pour contrôler le profil de la déformation.
