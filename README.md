# jeux-de-plateau-crypto

Reskin crypto d'un **jeu de plateau à ressources** (hexagones, dés, production, construction), écrit en **C99** comme projet d'apprentissage. Pas de deadline, pas de commande client — démarche empirique : valider le moteur de règles avant de sur-spécifier le rendu ou l'outillage.

> ⚠️ **Distinct de TERRITOIRE** (jeu de cartes 5×7, capture en cascade). Aucune logique ni code partagé entre les deux projets — ne pas les confondre.

## État actuel

**Phase 1 — complète.** Le moteur de règles (topologie, génération, tour/production, construction & validation,
score), l'interface terminal et l'outillage de simulation par lots sont implémentés et testés (T1→T7, 7 suites,
zéro warning). Definition of Done atteinte. Voir [`docs/TASKS.md`](docs/TASKS.md). Suite : Phase 2 (trading,
Signal, Margin Call…).

## Le jeu en bref

- **Plateau** : 19 hexagones (18 cases ressource + 1 case Rekt), jetons numériques assignés aléatoirement.
- **Ressources** : BTC (6 cases), Alts (4), ETH (3), Stablecoins (3), SOL (2). *(SOL 1→2 au playtest, cf. MEMORY.)*
- **Boucle de tour** : lancer `2d6` → production → construction.
- **Constructions** : Ligne (route), Position (colonie), Desk (ville).
- **Mode** : hotseat local, 2 à 4 joueurs, terminal uniquement (pas de graphismes en Phase 1).

| Terme du jeu | Mécanique classique |
|---|---|
| Ligne | Route |
| Position | Colonie |
| Desk | Ville |
| Signal | Carte développement |
| Margin Call | Voleur |
| Valorisation | Points de victoire |

Glossaire complet et règles détaillées : [`docs/spec.md`](docs/spec.md).

## Contraintes techniques (figées)

- C99, `gcc -Wall -Wextra -std=c99`, **zéro warning toléré**, aucune dépendance externe en Phase 1.
- **Séparation stricte moteur de règles (logique pure, sans I/O) / interface terminal.** Le moteur doit rester testable et simulable sans aucun affichage.
- État de jeu petit et borné (19 cases, ≤ 4 joueurs) → tableaux à taille fixe, pas d'allocation dynamique dans la boucle de tour.
- RNG : seed configurable en argument CLI, pour des runs reproductibles.

## Frontend — jouable dans le navigateur

Le site est servi par **GitHub Pages** depuis le dossier [`web/`](web/), déployé via GitHub Actions
([`.github/workflows/pages.yml`](.github/workflows/pages.yml)).

- **Jouer** : **https://pretoninho.github.io/jeux-de-plateau-crypto/play.html** (accueil : `.../` )
- Le **moteur C tourne dans le navigateur**, compilé en **WebAssembly** (Emscripten). Le JS ne contient aucune
  règle — il pilote le moteur via l'API `wasm_*` ([`src/wasm_api.c`](src/wasm_api.c)). C'est la séparation
  moteur / I/O (décision D2) qui rend ce portage possible.
- Plateau hexagonal SVG cliquable, dés, ressources, scores, hotseat 2–4 ([`web/play.html`](web/play.html), [`web/game.js`](web/game.js)).

Le workflow installe Emscripten, exécute `make wasm`, puis déploie `web/` (les artefacts `engine.js`/`engine.wasm`
ne sont pas versionnés). Pages est activé automatiquement (`enablement: true`).

### Build WASM en local

```sh
# une fois : installer Emscripten (emsdk), puis dans chaque shell :
source ~/emsdk/emsdk_env.sh
make wasm                     # -> web/engine.js + web/engine.wasm
python3 -m http.server -d web 8099   # puis ouvrir http://localhost:8099/play.html
```

## Documentation du projet

| Fichier | Rôle |
|---|---|
| [`docs/spec.md`](docs/spec.md) | Spécification de référence (règles, coûts, distribution, stack). |
| [`docs/MEMORY.md`](docs/MEMORY.md) | Mémoire vivante : décisions, état, questions ouvertes. **À lire en début de session.** |
| [`docs/TASKS.md`](docs/TASKS.md) | Liste de tâches et jalons. |
| [`CLAUDE.md`](CLAUDE.md) | Protocole de travail (dont consultation/mise à jour de la mémoire). |

## Build & exécution

```sh
make test                 # compile et lance les 7 suites de tests (zéro warning)
make                      # tests + binaire crypto-board
make run                  # démo automatique (40 tours, 3 joueurs)
make sim                  # simulation par lots (1000 parties) + rapport agrégé

# Interface terminal
./crypto-board --seed 42 --players 3            # mode interactif (tapez 'help')
./crypto-board --seed 7  --players 4 --demo 60  # partie automatique de 60 tours
./crypto-board --players 4 --sim 1000 --turns 80  # simulation + stats (production, SOL, scores)
```

**Découpage** : `src/` contient le moteur pur (aucune I/O) — `hex`, `board`, `rng`, `setup`,
`turn`, `build`, `score`, `game` — plus l'interface terminal (`ui.c`, `main.c`), seule couche d'I/O.
Tests dans `tests/`. Avancement détaillé : [`docs/TASKS.md`](docs/TASKS.md).
