/* wasm_api.c — couche d'exposition du moteur pour WebAssembly (T8).
 *
 * Fine surface d'appel pour le JS : elle NE contient aucune règle, elle relaie
 * vers le moteur pur (game/turn/build/score). Compilée par emcc avec le reste
 * du moteur ; sans __EMSCRIPTEN__, le fichier compile aussi en natif (macro
 * neutralisée) pour rester vérifiable par le compilateur habituel.
 *
 * Un unique état de partie global (le navigateur est mono-partie, mono-thread).
 */
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "game.h"
#include "turn.h"
#include "build.h"
#include "score.h"

static GameState G;

/* Racine 3 (flat-top) pour le placement pixel des hexagones. */
static double px_of(Cube c) { return 1.5 * (double)c.x; }
static double py_of(Cube c) { return 1.7320508075688772 * ((double)c.z + (double)c.x / 2.0); }

/* --- cycle de vie -------------------------------------------------------- */

/* Démarre une partie SANS placement initial : la mise en place (choix des
 * Positions de départ) est pilotée par l'UI via wasm_place_free (phase setup). */
/* board_seed : plateau (reproductible depuis la « graine » saisie).
 * dice_seed  : dés, flux indépendant. Le JS y passe de l'entropie
 * (crypto.getRandomValues) → la partie n'est PAS scriptée. */
EMSCRIPTEN_KEEPALIVE
void wasm_new_game(int players, unsigned int board_seed, unsigned int dice_seed) {
    if (players < 2) players = 2;
    if (players > MAX_PLAYERS) players = MAX_PLAYERS;
    game_init(&G, players, board_seed, dice_seed);
}

EMSCRIPTEN_KEEPALIVE int wasm_n_players(void)        { return G.n_players; }
EMSCRIPTEN_KEEPALIVE int wasm_current(void)          { return G.current; }
EMSCRIPTEN_KEEPALIVE int wasm_initial_positions(void){ return INITIAL_POSITIONS; }

/* Placement initial gratuit (mise en place), piloté par joueur explicite. */
EMSCRIPTEN_KEEPALIVE int wasm_place_free(int player, int vertex) {
    return (int)place_position_free(&G, player, vertex);
}
EMSCRIPTEN_KEEPALIVE int wasm_can_place_free(int player, int vertex) {
    return (int)can_place_position_free(&G, player, vertex);
}

/* --- géométrie & état du plateau ----------------------------------------- */

EMSCRIPTEN_KEEPALIVE int wasm_n_tiles(void)    { return G.board.n_tiles; }
EMSCRIPTEN_KEEPALIVE int wasm_n_vertices(void) { return G.board.n_vertices; }
EMSCRIPTEN_KEEPALIVE int wasm_n_edges(void)    { return G.board.n_edges; }

EMSCRIPTEN_KEEPALIVE int wasm_tile_resource(int i) { return (int)G.board.tiles[i].resource; }
EMSCRIPTEN_KEEPALIVE int wasm_tile_number(int i)   { return G.board.tiles[i].number; }
EMSCRIPTEN_KEEPALIVE double wasm_tile_px(int i)    { return px_of(G.board.tiles[i].coord); }
EMSCRIPTEN_KEEPALIVE double wasm_tile_py(int i)    { return py_of(G.board.tiles[i].coord); }

/* Un sommet est le barycentre des 3 hexagones qui s'y rencontrent. */
EMSCRIPTEN_KEEPALIVE double wasm_vertex_px(int v) {
    Cube *t = G.board.vertices[v].tri;
    return (px_of(t[0]) + px_of(t[1]) + px_of(t[2])) / 3.0;
}
EMSCRIPTEN_KEEPALIVE double wasm_vertex_py(int v) {
    Cube *t = G.board.vertices[v].tri;
    return (py_of(t[0]) + py_of(t[1]) + py_of(t[2])) / 3.0;
}
EMSCRIPTEN_KEEPALIVE int wasm_vertex_building(int v) { return (int)G.vertex_building[v]; }
EMSCRIPTEN_KEEPALIVE int wasm_vertex_owner(int v)    { return G.vertex_owner[v]; }

EMSCRIPTEN_KEEPALIVE int wasm_edge_v0(int e)    { return G.board.edges[e].v[0]; }
EMSCRIPTEN_KEEPALIVE int wasm_edge_v1(int e)    { return G.board.edges[e].v[1]; }
EMSCRIPTEN_KEEPALIVE int wasm_edge_owner(int e) { return G.edge_owner[e]; }

/* --- joueurs -------------------------------------------------------------- */

EMSCRIPTEN_KEEPALIVE int wasm_player_resource(int p, int k) { return G.players[p].resources[k]; }
EMSCRIPTEN_KEEPALIVE int wasm_score(int p) { return game_score(&G, p); }

/* --- actions -------------------------------------------------------------- */

EMSCRIPTEN_KEEPALIVE int  wasm_roll(void)     { int d = roll_2d6(&G.rng); game_produce(&G, d); return d; }
EMSCRIPTEN_KEEPALIVE void wasm_end_turn(void) { G.current = (G.current + 1) % G.n_players; }

EMSCRIPTEN_KEEPALIVE int wasm_build_position(int v) { return (int)build_position(&G, G.current, v); }
EMSCRIPTEN_KEEPALIVE int wasm_build_line(int e)     { return (int)build_line(&G, G.current, e); }
EMSCRIPTEN_KEEPALIVE int wasm_build_desk(int v)     { return (int)build_desk(&G, G.current, v); }

EMSCRIPTEN_KEEPALIVE int wasm_can_build_position(int v) { return (int)can_build_position(&G, G.current, v); }
EMSCRIPTEN_KEEPALIVE int wasm_can_build_line(int e)     { return (int)can_build_line(&G, G.current, e); }
EMSCRIPTEN_KEEPALIVE int wasm_can_build_desk(int v)     { return (int)can_build_desk(&G, G.current, v); }
