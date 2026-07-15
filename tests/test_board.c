/* test_board.c — vérifie la cohérence topologique du plateau (T1).
 *
 * Le moteur reste sans I/O ; c'est le harnais de test qui affiche.
 * Toute incohérence fait échouer un assert().
 */
#include <assert.h>
#include <stdio.h>

#include "../src/board.h"
#include "../src/game.h"

static int count_vertices_with_tiles(const Board *b, int k) {
    int c = 0;
    for (int i = 0; i < b->n_vertices; i++) {
        if (b->vertices[i].n_tiles == k) c++;
    }
    return c;
}

int main(void) {
    Board b;
    board_build(&b);

    /* Cardinalités de la topologie du plateau. */
    assert(b.n_tiles == 19);
    assert(b.n_vertices == 54);
    assert(b.n_edges == 72);

    /* Chaque tuile : 6 coins, 6 arêtes valides, 3..6 voisines. */
    int sum_neighbors = 0;
    for (int i = 0; i < b.n_tiles; i++) {
        const Tile *t = &b.tiles[i];
        assert(t->n_neighbors >= 3 && t->n_neighbors <= 6);
        sum_neighbors += t->n_neighbors;
        for (int d = 0; d < 6; d++) {
            assert(t->vertices[d] >= 0 && t->vertices[d] < b.n_vertices);
            assert(t->edges[d] >= 0 && t->edges[d] < b.n_edges);
        }
    }
    /* Adjacence des tuiles symétrique. */
    for (int i = 0; i < b.n_tiles; i++) {
        const Tile *t = &b.tiles[i];
        for (int d = 0; d < 6; d++) {
            int nb = t->neighbors[d];
            if (nb >= 0) {
                int back = 0;
                for (int e = 0; e < 6; e++) {
                    if (b.tiles[nb].neighbors[e] == i) back = 1;
                }
                assert(back);
            }
        }
    }
    assert(sum_neighbors == 84);   /* 2 × 42 arêtes intérieures */

    /* Arêtes : 2 extrémités distinctes, 1..2 tuiles bordantes. */
    int sum_edge_tiles = 0;
    for (int i = 0; i < b.n_edges; i++) {
        const Edge *e = &b.edges[i];
        assert(e->n_tiles >= 1 && e->n_tiles <= 2);
        sum_edge_tiles += e->n_tiles;
        assert(e->v[0] >= 0 && e->v[0] < b.n_vertices);
        assert(e->v[1] >= 0 && e->v[1] < b.n_vertices);
        assert(e->v[0] != e->v[1]);
    }
    assert(sum_edge_tiles == 114);   /* 2×42 intérieures + 30 de bordure */

    /* Intersections : 1..3 tuiles, 2..3 arêtes/voisines (cohérentes). */
    int sum_vertex_tiles = 0;
    for (int i = 0; i < b.n_vertices; i++) {
        const Vertex *v = &b.vertices[i];
        assert(v->n_tiles >= 1 && v->n_tiles <= 3);
        assert(v->n_edges >= 2 && v->n_edges <= 3);
        assert(v->n_adj == v->n_edges);
        sum_vertex_tiles += v->n_tiles;
    }
    assert(sum_vertex_tiles == 114);   /* 19 tuiles × 6 coins */

    /* Cohérence arête ↔ intersection : chaque extrémité référence l'arête. */
    for (int i = 0; i < b.n_edges; i++) {
        const Edge *e = &b.edges[i];
        for (int s = 0; s < 2; s++) {
            const Vertex *v = &b.vertices[e->v[s]];
            int found = 0;
            for (int k = 0; k < v->n_edges; k++) {
                if (v->edges[k] == i) found = 1;
            }
            assert(found);
        }
    }

    /* Initialisation d'une partie : plateau vierge. */
    GameState g;
    game_init(&g, 4, 12345u, 12345u);
    assert(g.n_players == 4);
    assert(g.board.n_tiles == 19);
    for (int i = 0; i < MAX_VERTICES; i++) {
        assert(g.vertex_building[i] == BUILD_NONE);
        assert(g.vertex_owner[i] == -1);
    }
    for (int i = 0; i < MAX_EDGES; i++) {
        assert(g.edge_owner[i] == -1);
    }

    printf("OK topologie : %d tuiles, %d intersections, %d aretes\n",
           b.n_tiles, b.n_vertices, b.n_edges);
    printf("  intersections par nb de tuiles : 3->%d  2->%d  1->%d\n",
           count_vertices_with_tiles(&b, 3),
           count_vertices_with_tiles(&b, 2),
           count_vertices_with_tiles(&b, 1));
    printf("Tous les asserts topologiques passent.\n");
    return 0;
}
