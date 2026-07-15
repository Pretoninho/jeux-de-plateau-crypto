# Spec de démarrage — Jeu de plateau crypto (mécaniques Catan, moteur en C)

## Contexte

Reskin crypto d’un jeu à mécaniques Catan (hexagones, dés, production, construction), conçu comme projet d’apprentissage C — pas de deadline, pas de commande client. **Distinct de TERRITOIRE** (jeu de cartes 5×7, capture en cascade) : aucune logique ni code partagé entre les deux projets, ne pas les confondre.

**Note IP** : ce projet reprend délibérément l’ossature mécanique de Catan à des fins d’apprentissage personnel. Si l’objectif évolue un jour vers une diffusion publique ou commerciale, il faudra faire diverger davantage les règles et l’expression du jeu de l’original (Catan/Asmodee détient des droits sur l’expression spécifique du jeu — plateau, coûts, iconographie — pas sur le concept générique de jeu de plateau à ressources).

## Portée — Phase 1

**Dans le périmètre :**

- Génération de plateau (19 hexagones, distribution ci-dessous, jetons numériques assignés aléatoirement)
- Boucle de tour : lancer de dés → production → construction (Ligne, Position, Desk)
- Validation des règles de construction (coût, adjacence)
- Mode hotseat local, 2 à 4 joueurs, terminal uniquement (pas de graphismes)

**Hors périmètre (phases suivantes) :**

- Trading (joueur-joueur, Bourse, Courtier générique)
- Signal (cartes développement) et tout ce qui en dépend (Risk Manager)
- Margin Call (voleur) — dépend du 7 ; peut attendre que la production de base soit validée
- Réseau / multijoueur distant
- Rendu graphique (SDL2/raylib) — évalué seulement après validation du moteur de règles

**Definition of Done — Phase 1** : le moteur fait tourner N tours pour P joueurs (2-4), production et construction correctement comptabilisées et validées, sans crash. Le moteur de règles doit être testable/appelable sans aucune I/O, à la manière de `territoire_sol_risk.c` (cf. Références).

## Règles du jeu

### Plateau

19 hexagones : 18 cases ressource + 1 case Rekt (ne produit jamais rien, point de départ de Margin Call).

Distribution des cases ressource (**révisée au playtest, 2026-07-14** ; à l’origine BTC 7 / SOL 1) :

|Actif      |Cases|
|-----------|-----|
|BTC        |6    |
|Alts       |4    |
|ETH        |3    |
|Stablecoins|3    |
|SOL        |2    |

Jetons numériques (18, assignés aléatoirement aux 18 cases ressource) : `2, 3, 3, 4, 4, 5, 5, 6, 6, 8, 8, 9, 9, 10, 10, 11, 11, 12`. Le 7 n’est jamais un jeton de case — c’est le seuil qui déclenche Margin Call.

⚠️ **Rareté de SOL — rééquilibrée au playtest (2026-07-14).** À l’origine SOL n’avait qu’**une** case ; comme SOL est exigé par la Ligne *et* la Position, ~27 % des parties simulées ne produisaient jamais de SOL et ne pouvaient donc construire ni Ligne ni Position (le playtest a confirmé la frustration). SOL est passé à **2 cases** (BTC 7→6) : le taux de parties sans SOL retombe à ~7 %, tout en gardant SOL nettement le plus rare (dominance de marché préservée). Les coûts sont inchangés.

### Coûts de construction

- **Ligne** (route) = 1 SOL + 1 ETH
- **Position** (colonie) = 1 SOL + 1 ETH + 1 Stablecoin + 1 Alt
- **Desk** (ville, upgrade d’une Position existante) = 2 Stablecoins + 3 BTC

### Boucle de tour

1. Le joueur actif lance 2d6.
1. Si la somme ≠ 7 : chaque case portant ce numéro produit l’actif correspondant pour chaque joueur y ayant une Position (1 unité) ou un Desk (2 unités) adjacent.
1. Si la somme = 7 : Margin Call — hors scope Phase 1 ; pour l’instant traiter comme un tour sans effet (pas de production, pas de vol).
1. Le joueur peut construire (Ligne / Position / Desk) s’il a les ressources requises, sous réserve des règles d’adjacence standard (Position : aucune intersection adjacente déjà occupée **et** reliée à une Ligne du joueur — sauf les Positions de mise en place, gratuites et exemptes ; Ligne : connectée à une Ligne ou Position existante du même joueur). *(Règle « Position reliée à une Ligne » ajoutée au playtest, 2026-07-14, pour donner un rôle à la Ligne.)*
1. Tour suivant.

### Valorisation (points de victoire)

Position = 1 point, Desk = 2 points. Cible standard = 10, mais en Phase 1 — sans trading ni financement facile d’un Desk — ne pas bloquer le moteur sur l’atteinte de 10 : logger le score à chaque tour, condition de victoire réelle à valider une fois le trading en place (Phase 2).

## Glossaire (mécaniques renommées)

|Terme du jeu      |Équivalent Catan    |
|------------------|--------------------|
|Ligne             |Route               |
|Position          |Colonie             |
|Desk              |Ville               |
|Signal            |Carte développement |
|Margin Call       |Voleur              |
|Market Maker      |Route la plus longue|
|Risk Manager      |Plus grande armée   |
|Bourse            |Port 2:1            |
|Courtier générique|Port 3:1            |
|Valorisation      |Points de victoire  |

## Stack technique

**Décidé — structurant, coûteux à changer après coup :**

- C99, compilation `gcc -Wall -Wextra -std=c99`, zéro warning toléré, aucune dépendance externe en Phase 1.
- Séparation stricte moteur de règles (logique pure, sans I/O) / interface terminal — même principe que `territoire_sol_risk.c` : le moteur doit rester testable et simulable sans aucun affichage. C’est cette frontière qui rend tout le reste ci-dessous déférable sans risque.
- Modélisation du plateau : coordonnées axiales/cube pour les hexagones (standard, bien documenté), plus une représentation des intersections (Positions/Desks) et des arêtes (Lignes). C’est le vrai point dur du projet — à concevoir avant d’écrire la boucle de jeu, pas après.
- État de jeu petit et borné (19 cases, ≤4 joueurs) → tableaux à taille fixe, pas d’allocation dynamique dans la boucle de tour.
- RNG : seed configurable en argument CLI, pour des runs reproductibles en test.

**Volontairement non tranché — peu coûteux à décider plus tard, précisément grâce à la séparation ci-dessus :**

- Rendu : terminal brut / ncurses / SDL2 / raylib — sans impact sur le moteur si la séparation tient.
- Tests : `assert()` + simulation par lots façon `territoire_sol_risk.c` suffisent pour démarrer ; un framework dédié seulement si le besoin se fait sentir.
- Algorithme RNG : `rand()` standard pour commencer (biais modulo mineur déjà connu) ; migrer vers PCG32/xorshift plus tard ne touche qu’un seul point (`roll_2d6()`), pas une décision structurante.
- Build : Makefile minimal ; CMake seulement le jour où une dépendance externe (SDL2 par ex.) l’exige.

Sur-spécifier le rendu ou le framework de test maintenant reviendrait à trancher par théorie avant d’avoir testé le moteur — exactement le biais qu’une démarche empirique évite.

## Questions ouvertes (à trancher avant ou pendant l’implémentation)

1. Layout de plateau fixe pour les premiers tests, ou générateur aléatoire dès le départ ?
1. Desk (ville) inclus dans la Phase 1, ou repoussé avec Signal/trading en Phase 2 ?
1. Nombre de joueurs cible pour la toute première version jouable (2 suffit pour valider la boucle ; 3-4 pour valider les règles d’adjacence complexes) ?
1. Nom de projet définitif (actuellement sans nom — à ne pas confondre avec TERRITOIRE) ?

## Références

- `territoire_sol_risk.c` — simulation Monte Carlo ayant validé la distribution de cases ci-dessus (moyenne de production, taux de blocage SOL).
- `TERRITOIRE_specification.md` — projet distinct (jeu de cartes 5×7), aucune logique partagée, ne pas confondre.