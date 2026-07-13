/* ui.h — interface terminal (T6).
 *
 * SEULE couche d'I/O du projet. Ne contient AUCUNE règle : elle affiche l'état
 * et traduit des commandes texte en appels au moteur (turn.h / build.h). Les
 * fonctions écrivent dans un FILE* et lisent depuis un FILE* → testables par
 * script (pas de dépendance implicite à stdin/stdout).
 */
#ifndef UI_H
#define UI_H

#include <stdio.h>
#include "game.h"

/* Affichage. */
void ui_render_board(FILE *out, const GameState *g);
void ui_render_scores(FILE *out, const GameState *g);

/* Boucle interactive hotseat : lit des commandes sur `in`, écrit sur `out`.
 * S'arrête sur « quit » ou fin de flux (EOF). */
void ui_run(GameState *g, FILE *in, FILE *out);

/* Partie automatique (bot simple) : joue `turns` tours et logge le score
 * à chaque tour sur `out`. Sans saisie — utile pour démo et non-régression. */
void ui_demo(GameState *g, int turns, FILE *out);

#endif /* UI_H */
