# Mémoire projet — jeux-de-plateau-crypto

> Fichier de **mémoire vivante**. À lire en début de chaque session (avant toute action)
> et à mettre à jour dès qu'une décision est prise, qu'un état change, ou qu'une
> question ouverte est tranchée. Voir le protocole dans [`../CLAUDE.md`](../CLAUDE.md).
>
> **Dernière mise à jour : 2026-07-11**

---

## 1. Identité du projet

- Reskin crypto d'un jeu à mécaniques Catan, moteur en **C99**, projet d'apprentissage.
- **Ne pas confondre avec TERRITOIRE** (jeu de cartes 5×7, code totalement séparé).
- Spec de référence : [`spec.md`](spec.md). En cas de conflit entre ce fichier et la spec,
  la spec fait foi pour les *règles*, ce fichier fait foi pour les *décisions et l'état courant*.

## 2. État courant

- **Phase 1 — préparation.** Aucun code C écrit à ce jour.
- Repo contient : `README.md`, `docs/spec.md`, `docs/MEMORY.md`, `docs/TASKS.md`, `CLAUDE.md`, hook SessionStart.
- Branche de travail : `claude/file-consultation-4m5duo`.
- Prochaine étape concrète : trancher les questions ouvertes §5, puis modéliser le plateau (§ TASKS T1).

## 3. Décisions figées (structurantes, coûteuses à changer)

| # | Décision | Note |
|---|---|---|
| D1 | C99, `gcc -Wall -Wextra -std=c99`, zéro warning, zéro dépendance externe en Phase 1 | — |
| D2 | Séparation stricte **moteur (logique pure, sans I/O)** / interface terminal | Frontière qui rend tout le reste déférable. Modèle : `territoire_sol_risk.c`. |
| D3 | Plateau en **coordonnées axiales/cube** + représentation explicite des intersections (Positions/Desks) et des arêtes (Lignes) | Le vrai point dur — à concevoir avant la boucle de jeu. |
| D4 | État borné → **tableaux à taille fixe**, pas d'allocation dynamique dans la boucle de tour | 19 cases, ≤ 4 joueurs. |
| D5 | RNG : **seed configurable en argument CLI** (runs reproductibles) | `rand()` pour commencer, isolé dans `roll_2d6()`. |

## 4. Décisions déférées (peu coûteuses à trancher plus tard)

- **Rendu** : terminal brut / ncurses / SDL2 / raylib — sans impact sur le moteur si D2 tient.
- **Tests** : `assert()` + simulation par lots (façon `territoire_sol_risk.c`) pour démarrer ; framework dédié seulement si besoin.
- **Algorithme RNG** : migration `rand()` → PCG32/xorshift plus tard, ne touche que `roll_2d6()`.
- **Build** : Makefile minimal ; CMake seulement le jour où une dépendance externe (SDL2…) l'exige.

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
