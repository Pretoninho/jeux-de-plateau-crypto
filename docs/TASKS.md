# Liste de tâches — jeux-de-plateau-crypto

Ordre indicatif, du plus structurant au plus détaillé. Cocher au fur et à mesure et refléter
l'avancement dans [`MEMORY.md`](MEMORY.md) §2 (État courant).

Légende : ⬜ à faire · 🟡 en cours · ✅ fait

---

## Phase 0 — Cadrage (préalable au code)

- [ ] **T0.1** — Trancher les questions ouvertes (MEMORY §5) :
  - [ ] Q1 : layout fixe pour tests, ou générateur aléatoire d'emblée ?
  - [ ] Q2 : Desk inclus en Phase 1, ou repoussé ?
  - [ ] Q3 : nombre de joueurs pour la 1re version jouable ?
  - [ ] Q4 : nom de projet définitif ?
- [x] **T0.2** — Ancrer la spec, la mémoire et le protocole de session dans le repo.

## Phase 1 — Moteur de règles (cœur, sans I/O)

### T1 — Modélisation du plateau *(le point dur — à faire en premier)*
- [ ] Choisir la représentation : coordonnées axiales/cube pour les hexagones.
- [ ] Représenter les **intersections** (sommets — emplacements de Positions/Desks) et les **arêtes** (Lignes).
- [ ] Table d'adjacence hex ↔ intersection ↔ arête (précalculée, taille fixe).
- [ ] Structures C de base : `Tile`, `Vertex`, `Edge`, `Player`, `GameState` (tous à taille fixe).
- [ ] Tests `assert()` sur la cohérence topologique (nb d'intersections, d'arêtes, adjacences).

### T2 — Génération de plateau
- [ ] Placer les 18 cases ressource + 1 case Rekt selon la distribution (BTC 7 / Alts 4 / ETH 3 / Stable 3 / SOL 1).
- [ ] Assigner les 18 jetons numériques aléatoirement (RNG seedé, `roll` isolé).
- [ ] Mode layout fixe **et/ou** aléatoire selon décision Q1.
- [ ] Test : distribution correcte, jamais de jeton 7, reproductibilité à seed égale.

### T3 — Boucle de tour & production
- [ ] `roll_2d6()` isolé, seed configurable.
- [ ] Production : pour une somme ≠ 7, chaque case du numéro produit vers Positions (1) / Desks (2) adjacents.
- [ ] Somme = 7 → tour sans effet (Margin Call hors scope Phase 1).
- [ ] Comptabilité des ressources par joueur (tableaux à taille fixe).
- [ ] Test : simulation Monte-Carlo par lots (façon `territoire_sol_risk.c`), moyennes de production cohérentes.

### T4 — Construction & validation des règles
- [ ] Coûts : Ligne (1 SOL+1 ETH), Position (1 SOL+1 ETH+1 Stable+1 Alt), Desk (2 Stable+3 BTC).
- [ ] Adjacence Position : aucune intersection adjacente déjà occupée (règle de distance).
- [ ] Adjacence Ligne : connectée à une Ligne ou Position du même joueur.
- [ ] Desk = upgrade d'une Position existante du joueur (selon décision Q2).
- [ ] Débit des ressources uniquement si la construction est valide.
- [ ] Test : constructions valides/invalides couvertes par `assert()`.

### T5 — Valorisation & fin de partie
- [ ] Score : Position = 1, Desk = 2. Logger le score à chaque tour.
- [ ] Ne pas bloquer sur l'atteinte de 10 en Phase 1 (condition réelle en Phase 2).

### T6 — Interface terminal (fine, au-dessus du moteur)
- [ ] Affichage lisible du plateau et de l'état en terminal (aucune logique de règle ici).
- [ ] Boucle hotseat 2-4 joueurs, saisie des actions.
- [ ] CLI : `--seed`, `--players` (et `--layout` si Q1 le justifie).

### T7 — Outillage
- [ ] Makefile minimal (`gcc -Wall -Wextra -std=c99`, zéro warning).
- [ ] Arborescence `src/` (moteur) séparée de l'interface.
- [ ] Cible de simulation par lots pour valider le moteur sans I/O interactive.

## Definition of Done — Phase 1

> Le moteur fait tourner N tours pour P joueurs (2-4), production et construction correctement
> comptabilisées et validées, **sans crash**, et le moteur de règles est testable/appelable
> **sans aucune I/O** (façon `territoire_sol_risk.c`).

## Hors périmètre (Phase 2+)

Trading (Bourse, Courtier) · Signal (cartes développement) + Risk Manager · Margin Call (voleur) ·
réseau/multijoueur distant · rendu graphique (SDL2/raylib) · condition de victoire à 10 points.
