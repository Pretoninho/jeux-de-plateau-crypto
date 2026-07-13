/* ui.c — interface terminal (cf. ui.h). Aucune règle ici (D2). */
#include "ui.h"

#include <string.h>

#include "turn.h"
#include "build.h"
#include "score.h"

/* --- libellés ------------------------------------------------------------- */

static const char *res_name(Resource r) {
    switch (r) {
        case RES_BTC:    return "BTC";
        case RES_ETH:    return "ETH";
        case RES_ALT:    return "Alt";
        case RES_STABLE: return "Stable";
        case RES_SOL:    return "SOL";
        case RES_NONE:   return "Rekt";
        default:         return "?";
    }
}

static const char *build_name(Building b) {
    switch (b) {
        case BUILD_POSITION: return "Position";
        case BUILD_DESK:     return "Desk";
        default:             return "-";
    }
}

/* Message associé à un résultat de construction. */
static const char *build_message(BuildResult r) {
    switch (r) {
        case BUILD_OK:              return "construit";
        case BUILD_ERR_PLAYER:      return "joueur invalide";
        case BUILD_ERR_BOUNDS:      return "identifiant hors bornes";
        case BUILD_ERR_OCCUPIED:    return "emplacement deja pris";
        case BUILD_ERR_ADJACENT:    return "trop proche d'une autre Position";
        case BUILD_ERR_CONNECT:     return "Ligne non connectee a votre reseau";
        case BUILD_ERR_NOT_POSITION:return "pas de Position a vous a upgrader ici";
        case BUILD_ERR_COST:        return "ressources insuffisantes";
        default:                    return "erreur";
    }
}

/* --- affichage ------------------------------------------------------------ */

void ui_render_board(FILE *out, const GameState *g) {
    const Board *b = &g->board;
    fprintf(out, "=== Plateau (%d tuiles) ===\n", b->n_tiles);
    fprintf(out, " id  (x, y, z)   actif    jeton\n");
    for (int i = 0; i < b->n_tiles; i++) {
        const Tile *t = &b->tiles[i];
        fprintf(out, " %2d  (%2d,%2d,%2d)  %-7s ", i,
                t->coord.x, t->coord.y, t->coord.z, res_name(t->resource));
        if (t->number > 0) {
            fprintf(out, " %2d\n", t->number);
        } else {
            fprintf(out, "  -\n");
        }
    }

    /* Constructions posées. */
    int any = 0;
    for (int v = 0; v < b->n_vertices; v++) {
        if (g->vertex_building[v] != BUILD_NONE) {
            if (!any) { fprintf(out, "--- Constructions ---\n"); any = 1; }
            fprintf(out, " intersection %2d : %-8s joueur %d\n",
                    v, build_name(g->vertex_building[v]), g->vertex_owner[v]);
        }
    }
    for (int e = 0; e < b->n_edges; e++) {
        if (g->edge_owner[e] != -1) {
            if (!any) { fprintf(out, "--- Constructions ---\n"); any = 1; }
            fprintf(out, " arete %2d        : Ligne    joueur %d\n",
                    e, g->edge_owner[e]);
        }
    }
    if (!any) fprintf(out, "(aucune construction)\n");
}

void ui_render_scores(FILE *out, const GameState *g) {
    fprintf(out, "--- Scores ---\n");
    for (int p = 0; p < g->n_players; p++) {
        fprintf(out, " joueur %d : %d point(s) | ", p, game_score(g, p));
        for (int k = RES_BTC; k < RES_COUNT; k++) {
            fprintf(out, "%s:%d ", res_name((Resource)k), g->players[p].resources[k]);
        }
        fprintf(out, "\n");
    }
    fprintf(out, " (en tete : joueur %d)\n", game_leader(g));
}

static void ui_help(FILE *out) {
    fprintf(out,
        "Commandes : roll | pos <v> | line <e> | desk <v> | board | score | end | help | quit\n");
}

/* --- boucle interactive --------------------------------------------------- */

void ui_run(GameState *g, FILE *in, FILE *out) {
    char line[128];
    int rolled = 0;

    fprintf(out, "Partie : %d joueurs, graine %u.\n", g->n_players, g->seed);
    ui_help(out);

    for (;;) {
        fprintf(out, "\n[Joueur %d%s] > ", g->current, rolled ? "" : ", a lancer");
        if (fgets(line, sizeof(line), in) == NULL) break;

        char cmd[32];
        int arg = -1;
        int n = sscanf(line, "%31s %d", cmd, &arg);
        if (n < 1) continue;

        if (strcmp(cmd, "quit") == 0) {
            break;
        } else if (strcmp(cmd, "help") == 0) {
            ui_help(out);
        } else if (strcmp(cmd, "board") == 0) {
            ui_render_board(out, g);
        } else if (strcmp(cmd, "score") == 0) {
            ui_render_scores(out, g);
        } else if (strcmp(cmd, "roll") == 0) {
            if (rolled) {
                fprintf(out, "Deja lance ce tour ('end' pour passer).\n");
            } else {
                int d = roll_2d6(&g->rng);
                game_produce(g, d);
                rolled = 1;
                fprintf(out, "Des : %d%s\n", d,
                        d == 7 ? " -> Margin Call (aucun effet en Phase 1)" : " -> production");
            }
        } else if (strcmp(cmd, "end") == 0) {
            rolled = 0;
            g->current = (g->current + 1) % g->n_players;
            fprintf(out, "Au tour du joueur %d.\n", g->current);
        } else if (strcmp(cmd, "pos") == 0 || strcmp(cmd, "line") == 0 ||
                   strcmp(cmd, "desk") == 0) {
            if (n < 2) {
                fprintf(out, "Usage : %s <id>\n", cmd);
                continue;
            }
            BuildResult r;
            if (strcmp(cmd, "pos") == 0)       r = build_position(g, g->current, arg);
            else if (strcmp(cmd, "line") == 0) r = build_line(g, g->current, arg);
            else                               r = build_desk(g, g->current, arg);
            fprintf(out, "%s %d : %s\n", cmd, arg, build_message(r));
        } else {
            fprintf(out, "Commande inconnue : %s\n", cmd);
        }
    }
}

/* --- partie automatique --------------------------------------------------- */

/* Tente une construction utile pour `p` : nouvelle Position, sinon upgrade
 * Desk, sinon Ligne. (Politique de bot volontairement simple.) */
static void ui_auto_build(GameState *g, int p) {
    for (int v = 0; v < g->board.n_vertices; v++) {
        if (can_build_position(g, p, v) == BUILD_OK) {
            build_position(g, p, v);
            return;
        }
    }
    for (int v = 0; v < g->board.n_vertices; v++) {
        if (can_build_desk(g, p, v) == BUILD_OK) {
            build_desk(g, p, v);
            return;
        }
    }
    for (int e = 0; e < g->board.n_edges; e++) {
        if (can_build_line(g, p, e) == BUILD_OK) {
            build_line(g, p, e);
            return;
        }
    }
}

void ui_demo(GameState *g, int turns, FILE *out) {
    fprintf(out, "=== Demo automatique : %d tours, %d joueurs, graine %u ===\n",
            turns, g->n_players, g->seed);
    for (int t = 0; t < turns; t++) {
        int p = g->current;
        int d = roll_2d6(&g->rng);
        game_produce(g, d);
        ui_auto_build(g, p);
        g->current = (p + 1) % g->n_players;

        fprintf(out, "tour %3d | joueur %d | des %2d | scores", t + 1, p, d);
        for (int q = 0; q < g->n_players; q++) {
            fprintf(out, " %d", game_score(g, q));
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\n");
    ui_render_scores(out, g);
}
