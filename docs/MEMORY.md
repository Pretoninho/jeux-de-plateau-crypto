# Mémoire projet — jeux-de-plateau-crypto

> Fichier de **mémoire vivante**. À lire en début de chaque session (avant toute action)
> et à mettre à jour dès qu'une décision est prise, qu'un état change, ou qu'une
> question ouverte est tranchée. Voir le protocole dans [`../CLAUDE.md`](../CLAUDE.md).
>
> **Dernière mise à jour : 2026-07-11 (D6 — Pages auto-activé via `enablement: true`)**

---

## 1. Identité du projet

- Reskin crypto d'un jeu à mécaniques Catan, moteur en **C99**, projet d'apprentissage.
- **Ne pas confondre avec TERRITOIRE** (jeu de cartes 5×7, code totalement séparé).
- Spec de référence : [`spec.md`](spec.md). En cas de conflit entre ce fichier et la spec,
  la spec fait foi pour les *règles*, ce fichier fait foi pour les *décisions et l'état courant*.

## 2. État courant

- **Phase 1 — préparation.** Aucun code C écrit à ce jour.
- Repo contient : `README.md`, `docs/spec.md`, `docs/MEMORY.md`, `docs/TASKS.md`, `CLAUDE.md`, hook SessionStart,
  site vitrine `web/` + workflow de déploiement GitHub Pages (`.github/workflows/pages.yml`).
- Branche de travail : `claude/file-consultation-4m5duo` (PR #1 mergée ; branche repartie de `main` pour le correctif Pages).
- **Frontend décidé** : site statique sur **GitHub Pages**, moteur C → **WebAssembly** pour la version jouable (voir D6).
  Aujourd'hui : page vitrine placeholder déployable ; le WASM viendra une fois le moteur écrit.
- **CI Pages** : run #1 a échoué (Pages pas activé) → correctif `enablement: true` pour que le workflow active Pages seul.
- Prochaine étape concrète : trancher les questions ouvertes §5, puis modéliser le plateau (§ TASKS T1).

## 3. Décisions figées (structurantes, coûteuses à changer)

| # | Décision | Note |
|---|---|---|
| D1 | C99, `gcc -Wall -Wextra -std=c99`, zéro warning, zéro dépendance externe en Phase 1 | — |
| D2 | Séparation stricte **moteur (logique pure, sans I/O)** / interface terminal | Frontière qui rend tout le reste déférable. Modèle : `territoire_sol_risk.c`. |
| D3 | Plateau en **coordonnées axiales/cube** + représentation explicite des intersections (Positions/Desks) et des arêtes (Lignes) | Le vrai point dur — à concevoir avant la boucle de jeu. |
| D4 | État borné → **tableaux à taille fixe**, pas d'allocation dynamique dans la boucle de tour | 19 cases, ≤ 4 joueurs. |
| D5 | RNG : **seed configurable en argument CLI** (runs reproductibles) | `rand()` pour commencer, isolé dans `roll_2d6()`. |
| D6 | **Frontend web sur GitHub Pages** ; moteur C compilé en **WebAssembly (Emscripten)** pour la version jouable | Rend D2 encore plus critique : le moteur DOIT rester pur/sans I/O pour être embarquable en WASM. Interface terminal et interface web partagent le même moteur. |

### Détail D6 — Frontend GitHub Pages

- **Hébergement** : GitHub Pages, source « GitHub Actions » (workflow `.github/workflows/pages.yml`).
  Le workflow **active Pages lui-même** (`configure-pages` avec `enablement: true`) → plus d'action manuelle dans les Settings.
- **URL attendue** : `https://pretoninho.github.io/jeux-de-plateau-crypto/`.
- **Source du site** : dossier `web/` (statique, self-contained, aucune dépendance externe — cohérent avec l'éthos zéro-dépendance).
- **Aujourd'hui** : `web/index.html` = page vitrine placeholder (règles, distribution, coûts, statut « moteur à venir »).
- **Cible jouable** : compiler le moteur C en WASM via Emscripten (`emcc`), pilote JS minimal au-dessus.
  Le build WASM sera ajouté au workflow *une fois le moteur écrit* (aujourd'hui le workflow ne déploie que le statique).
- Le terminal (spec Phase 1) reste la 1re interface de validation ; le web est une **seconde interface** sur le même moteur,
  pas un remplacement. N'introduit aucune I/O dans le moteur.

## 4. Décisions déférées (peu coûteuses à trancher plus tard)

- **Rendu natif** (hors web) : terminal brut / ncurses / SDL2 / raylib — sans impact sur le moteur si D2 tient.
- **Tests** : `assert()` + simulation par lots (façon `territoire_sol_risk.c`) pour démarrer ; framework dédié seulement si besoin.
- **Algorithme RNG** : migration `rand()` → PCG32/xorshift plus tard, ne touche que `roll_2d6()`.
- **Build** : Makefile minimal ; CMake seulement le jour où une dépendance externe (SDL2…) l'exige.
- **Toolchain WASM** : Emscripten pressenti ; version/flags précis à fixer quand le moteur existera.
- **UI web** : HTML/CSS/JS « vanilla » pour commencer (self-contained) ; framework seulement si le besoin se fait sentir.

## 5. Questions ouvertes (à trancher)

| # | Question | Statut |
|---|---|---|
| Q1 | Layout de plateau fixe pour les premiers tests, ou générateur aléatoire dès le départ ? | ⬜ ouvert |
| Q2 | Desk (ville) inclus en Phase 1, ou repoussé en Phase 2 avec Signal/trading ? | ⬜ ouvert |
| Q3 | Nombre de joueurs cible pour la 1re version jouable (2 pour la boucle ; 3-4 pour l'adjacence) ? | ⬜ ouvert |
| Q4 | Nom de projet définitif (sans nom pour l'instant) ? | ⬜ ouvert |

## 6. Points connus & acceptés (ne pas rouvrir sans raison)

- **SOL n'a qu'une case** → ~6,8 % des parties simulées ont zéro production de SOL. Ce n'est **pas un bug** :
  distribution volontaire (pondération √dominance). À rouvrir seulement si le playtest confirme la frustration.
- **Somme = 7** en Phase 1 : Margin Call hors scope → traiter comme un tour sans effet (pas de production, pas de vol).
- **Condition de victoire** : ne pas bloquer le moteur sur l'atteinte de 10 points en Phase 1 (pas de trading).
  Logger le score à chaque tour ; condition réelle à valider en Phase 2.

## 7. Données de référence (extraites de la spec — pour rappel rapide)

- **Distribution ressources** : BTC 7 · Alts 4 · ETH 3 · Stablecoins 3 · SOL 1 (+ 1 case Rekt) = 19.
- **Jetons** : `2,3,3,4,4,5,5,6,6,8,8,9,9,10,10,11,11,12` (18 jetons ; jamais de 7).
- **Coûts** : Ligne = 1 SOL + 1 ETH · Position = 1 SOL + 1 ETH + 1 Stable + 1 Alt · Desk = 2 Stable + 3 BTC.
- **Production** : Position adjacente = 1 unité ; Desk adjacent = 2 unités.
- **Points** : Position = 1, Desk = 2.

## 8. Journal des décisions (append-only)

- **2026-07-11** — Création du repo documentaire : spec ancrée dans `docs/spec.md`, mise en place de la
  mémoire projet, du protocole de session (CLAUDE.md + hook SessionStart) et de la liste de tâches. Aucun code C encore.
- **2026-07-11** — **D6** : choix d'un frontend web sur **GitHub Pages**, moteur C → **WebAssembly** pour la version jouable.
  Scaffolding posé : `web/index.html` (vitrine placeholder) + `.github/workflows/pages.yml` (déploiement Pages via Actions).
  Le build WASM sera branché plus tard, une fois le moteur écrit. Décision « Rendu » retirée des déférées → tranchée (D6).
- **2026-07-11** — Run Pages #1 (post-merge PR #1) en échec : `configure-pages` → « Get Pages site failed / Not Found »
  car Pages n'était pas activé. Correctif : ajout de `enablement: true` à `configure-pages` pour que le workflow
  active Pages automatiquement au prochain run. Plus aucune manip dans Settings. Poussé sur branche repartie de `main`.
