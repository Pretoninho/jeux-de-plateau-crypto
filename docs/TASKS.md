# Liste de tâches — jeux-de-plateau-crypto

Ordre indicatif, du plus structurant au plus détaillé. Cocher au fur et à mesure et refléter
l'avancement dans [`MEMORY.md`](MEMORY.md) §2 (État courant).

Légende : ⬜ à faire · 🟡 en cours · ✅ fait

---

## Phase 0 — Cadrage (préalable au code)

- [x] **T0.1** — Trancher les questions ouvertes (MEMORY §5) :
  - [x] Q1 : **aléatoire seedé** (D7).
  - [x] Q2 : **Desk inclus** en Phase 1 (D8).
  - [x] Q3 : **générique 2–4**, tester à 2 (D9).
  - [ ] Q4 : nom de projet — **reporté** (identifiants C neutres en attendant).
- [x] **T0.2** — Ancrer la spec, la mémoire et le protocole de session dans le repo.
- [ ] **T0.3** — **Q5 — mécanique « signature » qui distingue le jeu de Catan** (MEMORY §5, pistes listées).
  Décision empirique, **après** que la boucle de base tourne (T1–T5). Ne pas sur-concevoir avant.

## Phase 1 — Moteur de règles (cœur, sans I/O)

### T1 — Modélisation du plateau ✅ *(le point dur — fait)*
- [x] Représentation : coordonnées **cube** (`src/hex.{h,c}`).
- [x] **Intersections** (clé = 3 hexes) et **arêtes** (clé = 2 hexes) — `src/board.{h,c}`, 100 % entier.
- [x] Tables d'adjacence hex ↔ intersection ↔ arête (précalculées, taille fixe).
- [x] Structures C de base : `Tile`, `Vertex`, `Edge` (board.h), `Player`, `GameState` (game.h) — toutes à taille fixe.
- [x] Tests `assert()` de cohérence topologique (`tests/test_board.c`) : 19/54/72 + invariantes. `make test`, zéro warning.

### T2 — Génération de plateau ✅
- [x] 18 cases ressource + 1 Rekt selon la distribution (BTC 7 / Alts 4 / ETH 3 / Stable 3 / SOL 1) — `setup.c`.
- [x] 18 jetons numériques assignés aléatoirement (Fisher-Yates sur RNG seedé isolé `rng.{h,c}`).
- [x] Génération **aléatoire seedée** (D7) : `game_init(seed)` → plateau reproductible (pas de mode « fixe »).
- [x] Test (`tests/test_setup.c`) : distribution, jamais de 7, Rekt=0, reproductibilité à seed égale — 300 graines.

### T3 — Boucle de tour & production ✅
- [x] `roll_2d6()` isolé sur `g->rng` (`turn.c`).
- [x] Production : somme ≠ 7 → chaque tuile du numéro verse aux Positions (1) / Desks (2) adjacents (`game_produce`).
- [x] Somme = 7 → tour sans effet (Margin Call hors scope Phase 1).
- [x] Comptabilité des ressources par joueur (tableaux à taille fixe dans `Player`).
- [x] Test (`tests/test_turn.c`) : recoupement dual, Desk=2×, 7 sans effet, simulation 2000 tours reproductible.

### T4 — Construction & validation des règles ✅
- [x] Coûts : Ligne (1 SOL+1 ETH), Position (1 SOL+1 ETH+1 Stable+1 Alt), Desk (2 Stable+3 BTC) — `build.c`.
- [x] Adjacence Position : règle de distance (aucune intersection voisine occupée).
- [x] Adjacence Ligne : connectée à une Ligne ou Position/Desk du même joueur.
- [x] Desk = upgrade d'une Position existante du joueur (D8).
- [x] Débit des ressources uniquement si la construction est valide (`can_build_*` séparé de `build_*`).
- [x] Test (`tests/test_build.c`) : cas valides/invalides couverts par `assert()`, ids dérivés de la topologie.

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

## Volet Web — GitHub Pages (transverse, cf. MEMORY D6)

### T8 — Frontend web
- [x] **T8.1** — Scaffolding : `web/index.html` (vitrine placeholder) + workflow `.github/workflows/pages.yml`.
- [x] **T8.2** — Auto-activation de Pages par le workflow (`configure-pages` avec `enablement: true`) : plus d'action manuelle dans Settings. Le déploiement se fait au merge sur `main`. URL : `https://pretoninho.github.io/jeux-de-plateau-crypto/`.
- [ ] **T8.3** — Une fois le moteur écrit : cibler une build **WebAssembly** (Emscripten `emcc`) exposant l'API du moteur.
- [ ] **T8.4** — Pilote JS minimal : charger le WASM, rendre le plateau (hexagones/intersections/arêtes), boucler les tours.
- [ ] **T8.5** — Brancher l'étape de build WASM dans `pages.yml` (émettre `.wasm`/`.js` dans `web/` avant l'upload).
- [ ] **T8.6** — Vérifier que le moteur reste **pur/sans I/O** : la couche WASM ne fait qu'exposer des fonctions, aucune règle côté JS.

## Definition of Done — Phase 1

> Le moteur fait tourner N tours pour P joueurs (2-4), production et construction correctement
> comptabilisées et validées, **sans crash**, et le moteur de règles est testable/appelable
> **sans aucune I/O** (façon `territoire_sol_risk.c`).

## Hors périmètre (Phase 2+)

Trading (Bourse, Courtier) · Signal (cartes développement) + Risk Manager · Margin Call (voleur) ·
réseau/multijoueur distant · rendu graphique (SDL2/raylib) · condition de victoire à 10 points.
