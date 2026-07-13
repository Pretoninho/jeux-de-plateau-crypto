/* main.c — point d'entrée de l'interface terminal (T6).
 *
 * Assemble le moteur et l'UI. Deux modes :
 *   - interactif (défaut)  : boucle hotseat lisant les commandes sur stdin ;
 *   - démo (--demo N)      : partie automatique de N tours (sans saisie).
 *
 * CLI : --seed <n> --players <2..4> [--demo <tours>]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "ui.h"

static void usage(const char *prog, FILE *out) {
    fprintf(out,
        "Usage : %s [--seed <n>] [--players <2..4>] [--demo <tours>]\n"
        "  --seed n      graine RNG (defaut 1)\n"
        "  --players n   nombre de joueurs 2..4 (defaut 3)\n"
        "  --demo t      partie automatique de t tours puis sortie\n"
        "  --help        cette aide\n"
        "Sans --demo : mode interactif (tapez 'help' une fois lance).\n",
        prog);
}

/* Lit la valeur entière de l'argument i+1 ; renvoie 0 si absente/invalide. */
static int arg_int(int argc, char **argv, int i, int *ok) {
    if (i + 1 >= argc) { *ok = 0; return 0; }
    char *end = NULL;
    long v = strtol(argv[i + 1], &end, 10);
    *ok = (end != argv[i + 1] && *end == '\0');
    return (int)v;
}

int main(int argc, char **argv) {
    unsigned int seed = 1u;
    int players = 3;
    int demo_turns = -1;   /* < 0 : mode interactif */

    for (int i = 1; i < argc; i++) {
        int ok = 0;
        if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0], stdout);
            return 0;
        } else if (strcmp(argv[i], "--seed") == 0) {
            seed = (unsigned int)arg_int(argc, argv, i, &ok);
            if (!ok) { usage(argv[0], stderr); return 1; }
            i++;
        } else if (strcmp(argv[i], "--players") == 0) {
            players = arg_int(argc, argv, i, &ok);
            if (!ok) { usage(argv[0], stderr); return 1; }
            i++;
        } else if (strcmp(argv[i], "--demo") == 0) {
            demo_turns = arg_int(argc, argv, i, &ok);
            if (!ok) { usage(argv[0], stderr); return 1; }
            i++;
        } else {
            fprintf(stderr, "Argument inconnu : %s\n", argv[i]);
            usage(argv[0], stderr);
            return 1;
        }
    }

    if (players < 2 || players > MAX_PLAYERS) {
        fprintf(stderr, "Nombre de joueurs invalide : %d (attendu 2..%d)\n",
                players, MAX_PLAYERS);
        return 1;
    }

    GameState g;
    game_init(&g, players, seed);
    game_place_initial(&g, INITIAL_POSITIONS);   /* amorçage de la production */

    if (demo_turns >= 0) {
        ui_demo(&g, demo_turns, stdout);
    } else {
        ui_render_board(stdout, &g);
        ui_run(&g, stdin, stdout);
    }
    return 0;
}
