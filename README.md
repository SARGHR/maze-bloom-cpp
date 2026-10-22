# Maze Bloom V3 🌸 — C++ Maze Adventure

Jeu de labyrinthe en C++17 / SDL2 avec programmation orientée objet.

## Fonctionnalités
- Menu de sélection de difficulté : facile, moyen, difficile (touches 1, 2, 3 ou souris)
- Labyrinthes générés aléatoirement, recherche du plus court chemin BFS
- Déplacement du personnage aux flèches ou ZQSD / WASD
- Monstres animés **mobiles** dans les couloirs secondaires, sans bloquer le chemin garanti
- 3 cœurs, collision avec un monstre = perte d'une vie et retour au départ
- Pièces dorées à ramasser : +50 points chacune
- Bonus de +500 points si le niveau est terminé en suivant le chemin le plus court
- Indice avec H, nouveau labyrinthe avec N, menu des niveaux avec M

## Installation Debian / Linux Penguin
```bash
sudo apt update
sudo apt install -y g++ make pkg-config libsdl2-dev libsdl2-ttf-dev fonts-dejavu-core
make
./maze-bloom
```

## Installation macOS (Homebrew)
```bash
brew install sdl2 sdl2_ttf pkg-config
make
./maze-bloom
```

## Commandes
- **Flèches** / **ZQSD** / **WASD** : déplacer
- **1 / 2 / 3** : choisir un niveau
- **H** : indice
- **N** : recommencer
- **M** : menu des niveaux
- **Échap** : quitter

## Remarque
Les monstres ne se déplacent jamais sur le chemin BFS garanti, pour conserver un itinéraire sûr. Les pièces se trouvent sur ce chemin. Le bonus de parcours parfait récompense le nombre de déplacements minimal, même si le joueur a collecté des pièces.

Aucun paiement, réseau ni donnée personnelle. Projet de démonstration académique.
