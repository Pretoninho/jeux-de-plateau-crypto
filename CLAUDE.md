# CLAUDE.md — Protocole de travail

Guide pour tout agent (Claude Code ou humain) travaillant sur ce dépôt.

## Projet

Reskin crypto d'un jeu de plateau à ressources, moteur en **C99**, projet d'apprentissage.
**Distinct de TERRITOIRE** (jeu de cartes) — aucun code partagé, ne pas confondre.

Spec de référence : [`docs/spec.md`](docs/spec.md).

## 🧠 Protocole mémoire (obligatoire)

Ce dépôt maintient une **mémoire vivante** dans [`docs/MEMORY.md`](docs/MEMORY.md).

### En début de session — TOUJOURS

1. **Lire `docs/MEMORY.md` en entier** avant toute autre action. Il contient l'état courant,
   les décisions figées, les décisions déférées, les questions ouvertes et les points connus/acceptés.
2. Lire `docs/TASKS.md` pour savoir où en est le travail.
3. Ne pas rouvrir un point listé en §6 « Points connus & acceptés » sans raison explicite.

Un hook `SessionStart` (`.claude/settings.json`) rappelle automatiquement ces fichiers au démarrage.

### Pendant / fin de session — mettre à jour si nécessaire

Mettre à jour `docs/MEMORY.md` **dès que** l'un de ces événements se produit :

- Une **question ouverte (§5)** est tranchée → déplacer la décision en §3 (figée) ou §4 (déférée), marquer Q✅.
- Une **décision structurante** est prise ou révisée → §3 + entrée datée dans le **§8 Journal**.
- L'**état courant (§2)** change (premier fichier C écrit, jalon atteint, branche, etc.).
- Un **nouveau point connu/accepté** émerge → §6.
- Toujours actualiser la date « Dernière mise à jour » en tête de fichier.

Le **§8 Journal est append-only** : on ajoute, on ne réécrit pas l'historique.

Règle de préséance : pour les **règles du jeu**, `docs/spec.md` fait foi ; pour les
**décisions et l'état courant**, `docs/MEMORY.md` fait foi.

## Contraintes techniques (rappel — détail en §3 de MEMORY.md)

- C99 : `gcc -Wall -Wextra -std=c99`, **zéro warning toléré**, aucune dépendance externe en Phase 1.
- **Séparation stricte moteur (logique pure, sans I/O) / interface terminal.** Le moteur doit être
  testable et simulable sans aucun affichage. Ne jamais faire d'`printf`/`scanf` dans le moteur.
- État borné → **tableaux à taille fixe**, pas d'allocation dynamique dans la boucle de tour.
- RNG isolé dans `roll_2d6()`, seed via argument CLI.

## Style & conventions

- Code, commentaires et noms de symboles : cohérents avec l'existant (français pour la doc, anglais
  pour les identifiants C sauf convention contraire déjà en place).
- Chaque changement non trivial : vérifier que ça compile sans warning avant de committer.
- Messages de commit clairs et descriptifs.

## Git

- Branche de développement : `claude/file-consultation-4m5duo` (voir consignes de session).
- Committer avec des messages explicites ; ne pas ouvrir de Pull Request sans demande explicite.
