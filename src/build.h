/* build.h — construction et validation des règles (T4).
 *
 * Coûts (cf. spec) :
 *   - Ligne    (arête)        : 1 SOL + 1 ETH
 *   - Position (intersection) : 1 SOL + 1 ETH + 1 Stable + 1 Alt
 *   - Desk     (upgrade)      : 2 Stable + 3 BTC
 *
 * Adjacence (règles d'adjacence standard, telles que précisées par la spec) :
 *   - Position : intersection libre ET aucune intersection voisine occupée
 *                (règle de distance). NB : la spec Phase 1 n'exige pas de
 *                connexion routière pour la Position — voir MEMORY (point ouvert).
 *   - Ligne    : arête libre ET connectée à une Ligne ou une Position/Desk
 *                du même joueur.
 *   - Desk     : upgrade d'une Position existante appartenant au joueur.
 *
 * La validation (can_*) ne mute rien. La construction (build_*) débite les
 * ressources et pose l'ouvrage seulement si tout est valide. Moteur pur (D2).
 */
#ifndef BUILD_H
#define BUILD_H

#include "game.h"

typedef enum {
    BUILD_OK = 0,
    BUILD_ERR_PLAYER,        /* identifiant de joueur invalide */
    BUILD_ERR_BOUNDS,        /* intersection / arête hors bornes */
    BUILD_ERR_OCCUPIED,      /* emplacement déjà pris */
    BUILD_ERR_ADJACENT,      /* règle de distance violée (Position) */
    BUILD_ERR_CONNECT,       /* Ligne non connectée au réseau du joueur */
    BUILD_ERR_NOT_POSITION,  /* Desk : pas de Position à upgrader ici */
    BUILD_ERR_COST           /* ressources insuffisantes */
} BuildResult;

/* Validation pure (aucune mutation). */
BuildResult can_build_line(const GameState *g, int player, int edge);
BuildResult can_build_position(const GameState *g, int player, int vertex);
BuildResult can_build_desk(const GameState *g, int player, int vertex);

/* Construction : débite et pose si valide, sinon aucun effet.
 * Met à jour les points de victoire (Position +1, upgrade Desk +1). */
BuildResult build_line(GameState *g, int player, int edge);
BuildResult build_position(GameState *g, int player, int vertex);
BuildResult build_desk(GameState *g, int player, int vertex);

#endif /* BUILD_H */
