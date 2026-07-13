# jeux-de-plateau-crypto

Reskin crypto d'un jeu à mécaniques **Catan** (hexagones, dés, production, construction), écrit en **C99** comme projet d'apprentissage. Pas de deadline, pas de commande client — démarche empirique : valider le moteur de règles avant de sur-spécifier le rendu ou l'outillage.

> ⚠️ **Distinct de TERRITOIRE** (jeu de cartes 5×7, capture en cascade). Aucune logique ni code partagé entre les deux projets — ne pas les confondre.

## État actuel

**Phase 1 — moteur jouable.** Le cœur des règles est implémenté et testé (T1→T6) : topologie du plateau,
génération, boucle de tour/production, construction & validation, score, et interface terminal. Reste
l'outillage de simulation par lots (T7). Voir [`docs/TASKS.md`](docs/TASKS.md).

## Le jeu en bref

- **Plateau** : 19 hexagones (18 cases ressource + 1 case Rekt), jetons numériques assignés aléatoirement.
- **Ressources** : BTC (7 cases), Alts (4), ETH (3), Stablecoins (3), SOL (1).
- **Boucle de tour** : lancer `2d6` → production → construction.
- **Constructions** : Ligne (route), Position (colonie), Desk (ville).
- **Mode** : hotseat local, 2 à 4 joueurs, terminal uniquement (pas de graphismes en Phase 1).

| Terme du jeu | Équivalent Catan |
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

## Frontend — GitHub Pages

Le site est servi par **GitHub Pages** depuis le dossier [`web/`](web/), déployé via GitHub Actions
([`.github/workflows/pages.yml`](.github/workflows/pages.yml)).

- URL (après activation) : **https://pretoninho.github.io/jeux-de-plateau-crypto/**
- Aujourd'hui : page vitrine (règles, distribution, coûts). Le moteur C sera rendu **jouable dans le navigateur**
  en le compilant en **WebAssembly** (Emscripten) — c'est la séparation moteur / I/O (décision D2) qui rend ce portage possible.

> Le workflow **active Pages automatiquement** (`configure-pages` avec `enablement: true`) — aucune manip dans les Settings.
> Le déploiement se déclenche à chaque push sur `main` touchant `web/`.

## Documentation du projet

| Fichier | Rôle |
|---|---|
| [`docs/spec.md`](docs/spec.md) | Spécification de référence (règles, coûts, distribution, stack). |
| [`docs/MEMORY.md`](docs/MEMORY.md) | Mémoire vivante : décisions, état, questions ouvertes. **À lire en début de session.** |
| [`docs/TASKS.md`](docs/TASKS.md) | Liste de tâches et jalons. |
| [`CLAUDE.md`](CLAUDE.md) | Protocole de travail (dont consultation/mise à jour de la mémoire). |

## Build & exécution

```sh
make test                 # compile et lance les 6 suites de tests (zéro warning)
make                      # tests + binaire crypto-catan
make run                  # démo automatique (40 tours, 3 joueurs)

# Interface terminal
./crypto-catan --seed 42 --players 3            # mode interactif (tapez 'help')
./crypto-catan --seed 7  --players 4 --demo 60  # partie automatique de 60 tours
```

**Découpage** : `src/` contient le moteur pur (aucune I/O) — `hex`, `board`, `rng`, `setup`,
`turn`, `build`, `score`, `game` — plus l'interface terminal (`ui.c`, `main.c`), seule couche d'I/O.
Tests dans `tests/`. Avancement détaillé : [`docs/TASKS.md`](docs/TASKS.md).
