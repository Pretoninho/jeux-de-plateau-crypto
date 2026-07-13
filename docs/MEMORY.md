# Mémoire projet — jeux-de-plateau-crypto

> Fichier de **mémoire vivante**. À lire en début de chaque session (avant toute action)
> et à mettre à jour dès qu'une décision est prise, qu'un état change, ou qu'une
> question ouverte est tranchée. Voir le protocole dans [`../CLAUDE.md`](../CLAUDE.md).
>
> **Dernière mise à jour : 2026-07-12 (T6 fait — interface terminal + placement initial d'amorçage)**

---

## 1. Identité du projet

- Reskin crypto d'un jeu à mécaniques Catan, moteur en **C99**, projet d'apprentissage.
- **Ne pas confondre avec TERRITOIRE** (jeu de cartes 5×7, code totalement séparé).
- Spec de référence : [`spec.md`](spec.md). En cas de conflit entre ce fichier et la spec,
  la spec fait foi pour les *règles*, ce fichier fait foi pour les *décisions et l'état courant*.

## 2. État courant

- **Phase 1 — cœur (T1→T5) + interface (T6) faits.** Moteur pur complet ; interface terminal jouable par-dessus.
  Moteur `src/` : `hex.{h,c}`, `types.h`, `board.{h,c}`, `rng.{h,c}`, `setup.{h,c}`, `turn.{h,c}`,
  `build.{h,c}`, `score.{h,c}`, `game.{h,c}` (+ `game_place_initial`). Interface : `ui.{h,c}`, `main.c` (binaire `crypto-catan`).
  Tests : `test_board/setup/turn/build/score/ui.c` (6 suites, `make test`). CLI : `--seed`, `--players`, `--demo`.
  Build **zéro warning** (`-Wall -Wextra -Werror -std=c99`). **Reste T7 (outillage/simulation par lots).**
- Repo contient aussi : `README.md`, `docs/spec.md`, `docs/MEMORY.md`, `docs/TASKS.md`, `CLAUDE.md`, hook SessionStart,
  site vitrine `web/` + workflow de déploiement GitHub Pages (`.github/workflows/pages.yml`), `.gitignore`.
- **Choix de modélisation T1 (implémente D3)** : représentation 100 % entière, sans flottant — une intersection = clé
  canonique des 3 hexes qui s'y rencontrent, une arête = les 2 hexes partagés. Déduplication et adjacence exactes.
- Branche de travail : `claude/file-consultation-4m5duo` (PR #1 mergée ; branche repartie de `main` pour le correctif Pages).
- **Frontend décidé** : site statique sur **GitHub Pages**, moteur C → **WebAssembly** pour la version jouable (voir D6).
  Aujourd'hui : page vitrine placeholder déployable ; le WASM viendra une fois le moteur écrit.
- **CI Pages** : ✅ résolu. PR #2 mergée, run #3 vert, **site en ligne** (HTTP 200) : https://pretoninho.github.io/jeux-de-plateau-crypto/
- **Cadrage tranché** : Q1→aléatoire seedé (D7), Q2→Desk inclus (D8), Q3→générique 2–4 (D9). Q4 (nom) reportée.
  Nouveau point ouvert **Q5** : trouver une mécanique « signature » qui distingue le jeu de Catan.
- Prochaine étape concrète : **T7 — outillage/simulation par lots** (cible façon `territoire_sol_risk.c` : lancer P
  parties sur N graines, agréger stats — production moyenne, taux de blocage SOL, scores). Dernière tâche Phase 1.

## 3. Décisions figées (structurantes, coûteuses à changer)

| # | Décision | Note |
|---|---|---|
| D1 | C99, `gcc -Wall -Wextra -std=c99`, zéro warning, zéro dépendance externe en Phase 1 | — |
| D2 | Séparation stricte **moteur (logique pure, sans I/O)** / interface terminal | Frontière qui rend tout le reste déférable. Modèle : `territoire_sol_risk.c`. |
| D3 | Plateau en **coordonnées axiales/cube** + représentation explicite des intersections (Positions/Desks) et des arêtes (Lignes) | Le vrai point dur — à concevoir avant la boucle de jeu. |
| D4 | État borné → **tableaux à taille fixe**, pas d'allocation dynamique dans la boucle de tour | 19 cases, ≤ 4 joueurs. |
| D5 | RNG : **seed configurable en argument CLI** (runs reproductibles) | `rand()` pour commencer, isolé dans `roll_2d6()`. |
| D6 | **Frontend web sur GitHub Pages** ; moteur C compilé en **WebAssembly (Emscripten)** pour la version jouable | Rend D2 encore plus critique : le moteur DOIT rester pur/sans I/O pour être embarquable en WASM. Interface terminal et interface web partagent le même moteur. |
| D10 | **Placement initial gratuit** de `INITIAL_POSITIONS`=2 Positions/joueur (`game_place_initial`) pour amorcer la production | Sans amorçage, personne ne produit → personne ne peut construire (blocage). Placement auto réparti pour l'instant ; **choix interactif du placement = point ouvert** (cf. §5 Q6). Standard Catan. |
| D7 | **Génération de plateau aléatoire seedée** dès le départ (pas de mode « layout fixe » séparé) | `--seed` rend chaque partie reproductible → tests déterministes ET spec respectée. Tranche Q1. |
| D8 | **Desk (ville) inclus dès la Phase 1** | Déjà dans le périmètre spec (coût 2 Stables + 3 BTC), upgrade d'une Position. Boucle de jeu complète. Tranche Q2. |
| D9 | **Moteur générique 2–4 joueurs** dès le début (tableaux dimensionnés à 4) ; valider d'abord à 2, puis l'adjacence à 3–4 | Évite un refactor ultérieur. Tranche Q3. |

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
- **Algorithme RNG** : ✅ tranché à l'implémentation → **xorshift32 à état explicite** dans `src/rng.{h,c}` (pas de
  `rand()` global), pour une vraie reproductibilité. Reste un point unique à migrer (PCG32…) si besoin.
- **Build** : Makefile minimal ; CMake seulement le jour où une dépendance externe (SDL2…) l'exige.
- **Toolchain WASM** : Emscripten pressenti ; version/flags précis à fixer quand le moteur existera.
- **UI web** : HTML/CSS/JS « vanilla » pour commencer (self-contained) ; framework seulement si le besoin se fait sentir.

## 5. Questions ouvertes (à trancher)

| # | Question | Statut |
|---|---|---|
| Q1 | Layout de plateau fixe ou générateur aléatoire ? | ✅ tranchée → **aléatoire seedé** (D7) |
| Q2 | Desk inclus en Phase 1 ? | ✅ tranchée → **oui, inclus** (D8) |
| Q3 | Nombre de joueurs cible pour la 1re version ? | ✅ tranchée → **générique 2–4, test à 2** (D9) |
| Q4 | Nom de projet définitif ? | ⏸️ **reportée** — identifiants C neutres en attendant ; à trancher plus tard |
| Q5 | **Divergence vs Catan** : quel(s) mécanisme(s)/expression propre(s) pour que le jeu ne soit pas un calque trait pour trait de Catan ? | ⬜ **ouvert** — à mûrir (pistes ci-dessous) |
| Q6 | **Placement initial** : automatique réparti (actuel) ou choix interactif par les joueurs (règle Catan standard) ? | ⬜ **ouvert** — auto suffit pour la boucle ; interactif = confort de jeu |

### Pistes pour Q5 — divergence vs Catan (brainstorm, RIEN de tranché)

But : une identité mécanique propre, pas juste un reskin cosmétique. Idées à évaluer (empiriquement, une fois le
moteur de base tournant — ne pas sur-concevoir avant) :

- **Volatilité des jetons** : les numéros/valeurs de production fluctuent au fil des tours (cycles de marché),
  là où Catan fige les numéros à la mise en place. Mécanique 100 % crypto, absente de Catan.
- **Halving** : événement périodique qui divise (ou modifie) la production de BTC — rareté programmée dans le temps.
- **Staking / rendement** : bloquer des ressources pour un revenu passif, au lieu de tout dépenser en construction.
- **Gas fees** : un petit coût variable à la construction (tension supplémentaire selon la « congestion »).
- **Margin Call enrichi** : au lieu du simple voleur, une liquidation qui dépend de l'exposition du joueur
  (plus tu es « leveraged », plus tu risques gros au 7).
- **Airdrops** : gains aléatoires ponctuels récompensant une position ou un comportement.

⚠️ Contrainte de cohérence : toute divergence doit **respecter D2** (logique dans le moteur pur, pas d'I/O) et rester
bornée (pas d'alloc dynamique dans la boucle). Choisir **1 mécanique signature** d'abord plutôt que tout empiler.
Lien IP : cf. spec §Note IP — diverger davantage est justement ce qui protège si le projet devient public un jour.

## 6. Points connus & acceptés (ne pas rouvrir sans raison)

- **SOL n'a qu'une case** → ~6,8 % des parties simulées ont zéro production de SOL. Ce n'est **pas un bug** :
  distribution volontaire (pondération √dominance). À rouvrir seulement si le playtest confirme la frustration.
- **Somme = 7** en Phase 1 : Margin Call hors scope → traiter comme un tour sans effet (pas de production, pas de vol).
- **Condition de victoire** : ne pas bloquer le moteur sur l'atteinte de 10 points en Phase 1 (pas de trading).
  Logger le score à chaque tour ; condition réelle à valider en Phase 2.
- **Position sans connexion routière (T4)** : la spec Phase 1 n'exige, pour une Position, que la *règle de distance*
  (aucune intersection voisine occupée) — pas de connexion à une Ligne du joueur. Implémenté ainsi. À rouvrir si l'on
  veut la règle Catan complète « colonie reliée à une route » (placement libre facilite les tests de la boucle).

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
- **2026-07-12** — PR #2 mergée (`aa757bf`). Run Pages #3 **vert** ; site **en ligne** et vérifié (HTTP 200) :
  https://pretoninho.github.io/jeux-de-plateau-crypto/ . Volet frontend/Pages bouclé. Prochain jalon : trancher Q1–Q4 puis T1.
- **2026-07-12** — Cadrage Phase 0 tranché : **D7** (plateau aléatoire seedé), **D8** (Desk inclus Phase 1),
  **D9** (moteur générique 2–4, test à 2). Q4 (nom) reportée. Ouverture de **Q5** : divergence mécanique vs Catan
  (le jeu ne doit pas être un calque trait pour trait) — pistes listées en §5, décision empirique différée après T1.
- **2026-07-12** — **T1 terminé** : premier code C. Topologie du plateau en coords cube (`src/hex,board,game` + types),
  clé canonique entière (intersection = 3 hexes, arête = 2 hexes) → dédup/adjacence exactes. `tests/test_board.c`
  valide 19/54/72 et toutes les invariantes (symétrie voisinage, sommes 84/114/114, cohérence arête↔intersection).
  Build zéro warning sous `-Wall -Wextra -Werror -std=c99`. Makefile (`make test`) + `.gitignore`. Prochain : T2.
- **2026-07-12** — **T2 terminé** : génération de plateau. `src/rng.{h,c}` (xorshift32 seedé, `rng_range` sans biais
  modulo) + `src/setup.{h,c}` (`board_generate` : distribution BTC7/Alt4/ETH3/Stable3/SOL1 + Rekt, 18 jetons mélangés
  Fisher-Yates). `game_init` enchaîne build+generate sur le même flux RNG. `tests/test_setup.c` valide distribution,
  jetons (jamais de 7, Rekt=0), reproductibilité à graine égale, effet de la graine — sur 300 graines. Zéro warning.
  Choix RNG : xorshift explicite plutôt que `rand()` global (précision de la décision déférée sur l'algo). Prochain : T3.
- **2026-07-12** — **T3 terminé** : boucle de tour & production. `src/turn.{h,c}` — `roll_2d6()` isolé sur `g->rng`,
  `game_produce()` (tuile du numéro → +1 par Position / +2 par Desk adjacent ; 7 = no-op), `game_turn()` (lancer +
  production + joueur suivant). `tests/test_turn.c` : 2d6 (forme triangulaire), production recoupée par un calcul
  **dual** intersection→tuile (valide aussi l'inverse des adjacences T1), Desk=2×Position, 7 sans effet,
  reproductibilité sur 2000 tours. Zéro warning. Prochain : T4 (construction & validation).
- **2026-07-12** — **T4 terminé** : construction & validation. `src/build.{h,c}` — coûts (initialiseurs désignés C99),
  `can_build_*` (validation pure) / `build_*` (débit + pose + points). Règles : Position = libre + règle de distance ;
  Ligne = libre + connectée (construction OU route du joueur) ; Desk = upgrade de sa propre Position. Points de
  victoire maintenus à la construction. `tests/test_build.c` (ids dérivés de la topologie) : distance, coût, occupé,
  connexion route→route, upgrade, refus Desk sur vide / Position adverse, débits exacts. Zéro warning. Prochain : T5.
  Point acté (§6) : en Phase 1 la Position n'exige pas de connexion routière (règle de distance seule, cf. spec).
- **2026-07-12** — **T5 terminé** : valorisation. `src/score.{h,c}` — `game_score()` recalcule depuis le plateau
  (Position 1, Desk 2), `game_leader()` (égalité → plus petit indice). `tests/test_score.c` : score, **cohérence avec
  le compteur `victory_points`** maintenu à la construction, classement, et non-blocage au-delà de 10 (Phase 1).
  **Cœur des règles Phase 1 complet (T1→T5)** : DoD cœur atteinte (N tours, P joueurs, sans crash, sans I/O).
  Zéro warning. Reste l'habillage : T6 (interface terminal) + T7 (outillage). Push branche + `main` sur demande.
- **2026-07-12** — **T6 terminé** : interface terminal. `src/ui.{h,c}` (rendu plateau/scores, boucle de commandes
  pilotable par `FILE*`, démo bot) + `src/main.c` (binaire `crypto-catan`, CLI `--seed`/`--players`/`--demo`).
  Découverte à l'exécution : sans **placement initial**, aucune production → blocage ; ajout de `game_place_initial`
  (**D10**, 2 Positions gratuites/joueur, réparties). Bot de démo : Position → Desk → Ligne. `tests/test_ui.c`
  (pilotage scripté via `tmpfile`, rendu, erreurs, démo qui construit au-delà de l'amorçage). Zéro warning.
  Constat conforme spec : sans trading, réunir les 4 actifs d'une Position est rare → progression surtout via Desk.
  Nouveau point ouvert **Q6** (placement auto vs interactif). Prochain : T7.
