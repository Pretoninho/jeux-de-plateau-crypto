/* test_ui.c — interface terminal (T6).
 *
 * L'UI lit/écrit des FILE* : on la pilote par script (tmpfile ISO C) et on
 * inspecte la sortie, sans jamais bloquer sur une saisie interactive.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/game.h"
#include "../src/score.h"
#include "../src/ui.h"

static long file_len(FILE *f) {
    fflush(f);
    fseek(f, 0, SEEK_END);
    return ftell(f);
}

static int file_contains(FILE *f, const char *needle) {
    fflush(f);
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)n + 1);
    assert(buf);
    size_t rd = fread(buf, 1, (size_t)n, f);
    buf[rd] = '\0';
    int found = (strstr(buf, needle) != NULL);
    free(buf);
    return found;
}

int main(void) {
    /* 1) Rendu du plateau et des scores. */
    GameState g;
    game_init(&g, 3, 42u, 42u);

    FILE *fb = tmpfile();
    assert(fb);
    ui_render_board(fb, &g);
    assert(file_len(fb) > 0);
    assert(file_contains(fb, "Plateau"));
    fclose(fb);

    FILE *fs = tmpfile();
    assert(fs);
    ui_render_scores(fs, &g);
    assert(file_contains(fs, "Scores"));
    fclose(fs);

    /* 2) Boucle interactive pilotée par script : une construction réussit. */
    GameState g2;
    game_init(&g2, 2, 7u, 7u);
    game_place_initial(&g2, INITIAL_POSITIONS);   /* joueur 0 a des Positions */
    for (int k = 0; k < RES_COUNT; k++) g2.players[0].resources[k] = 10;

    /* Arête reliée à une Position du joueur 0 → Ligne constructible (règle A :
     * une Ligne est valide si reliée à une construction du joueur). */
    int p0v = -1;
    for (int v = 0; v < g2.board.n_vertices; v++) {
        if (g2.vertex_owner[v] == 0 && g2.vertex_building[v] != BUILD_NONE) { p0v = v; break; }
    }
    assert(p0v >= 0);
    int e0 = g2.board.vertices[p0v].edges[0];
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "line %d\n", e0);

    FILE *in = tmpfile();
    assert(in);
    fputs("board\n", in);
    fputs(cmd, in);            /* joueur 0 : Ligne reliée à sa Position */
    fputs("score\n", in);
    fputs("quit\n", in);
    rewind(in);

    FILE *out = tmpfile();
    assert(out);
    ui_run(&g2, in, out);

    assert(g2.edge_owner[e0] == 0);
    assert(file_contains(out, "construit"));
    fclose(in);
    fclose(out);

    /* 3) Commande invalide et hors-bornes : pas de crash, message d'erreur. */
    GameState g3;
    game_init(&g3, 2, 1u, 1u);
    FILE *in3 = tmpfile();
    fputs("blabla\n", in3);
    fputs("pos 9999\n", in3);   /* hors bornes */
    fputs("line\n", in3);       /* argument manquant */
    fputs("quit\n", in3);
    rewind(in3);
    FILE *out3 = tmpfile();
    ui_run(&g3, in3, out3);
    assert(file_contains(out3, "inconnue"));
    assert(file_contains(out3, "hors bornes"));
    fclose(in3);
    fclose(out3);

    /* 4) Démo automatique : joue sans saisie, logge le score par tour.
     * Le score final ne descend jamais sous les Positions initiales, et sur
     * plusieurs graines la démo construit au-delà de l'amorçage (robuste à la
     * variance d'une graine unique). */
    int base = 4 * INITIAL_POSITIONS;
    int built_beyond = 0, sum_shown = 0;
    FILE *fd = tmpfile();
    for (unsigned int s = 0; s < 8u; s++) {
        GameState g4;
        game_init(&g4, 4, s, s);
        game_place_initial(&g4, INITIAL_POSITIONS);
        ui_demo(&g4, 80, fd);
        int sum = 0;
        for (int p = 0; p < g4.n_players; p++) sum += game_score(&g4, p);
        assert(sum >= base);            /* jamais sous l'amorçage */
        if (sum > base) built_beyond = 1;
        if (s == 0) sum_shown = sum;
    }
    assert(file_contains(fd, "Demo"));
    assert(file_contains(fd, "tour"));
    assert(built_beyond);               /* construit au-delà de l'amorçage */
    fclose(fd);

    printf("OK interface : rendu plateau/scores, boucle scriptee, erreurs,\n");
    printf("  demo automatique (construit au-dela de l'amorcage ; score graine 0 = %d).\n", sum_shown);
    return 0;
}
