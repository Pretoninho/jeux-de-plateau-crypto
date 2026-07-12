/* board.c — construction de la topologie du plateau (cf. board.h). */
#include "board.h"

#include <assert.h>
#include <string.h>

/* --- clés canoniques (tri des coordonnées cube) --------------------------- */

static void sort2(Cube *a) {
    if (cube_cmp(a[0], a[1]) > 0) {
        Cube t = a[0]; a[0] = a[1]; a[1] = t;
    }
}

static void sort3(Cube *a) {
    /* tri par insertion sur 3 éléments */
    for (int i = 1; i < 3; i++) {
        Cube k = a[i];
        int j = i - 1;
        while (j >= 0 && cube_cmp(a[j], k) > 0) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = k;
    }
}

/* --- recherche / insertion déduplicquée ----------------------------------- */

static int find_tile(const Board *b, Cube c) {
    for (int i = 0; i < b->n_tiles; i++) {
        if (cube_eq(b->tiles[i].coord, c)) return i;
    }
    return -1;
}

/* Retourne l'id de l'intersection {a0,a1,a2}, en la créant si nécessaire. */
static int find_or_add_vertex(Board *b, Cube a0, Cube a1, Cube a2) {
    Cube key[3] = { a0, a1, a2 };
    sort3(key);
    for (int i = 0; i < b->n_vertices; i++) {
        Vertex *v = &b->vertices[i];
        if (cube_eq(v->tri[0], key[0]) &&
            cube_eq(v->tri[1], key[1]) &&
            cube_eq(v->tri[2], key[2])) {
            return i;
        }
    }
    assert(b->n_vertices < MAX_VERTICES);
    int id = b->n_vertices++;
    Vertex *v = &b->vertices[id];
    memset(v, 0, sizeof(*v));
    v->tri[0] = key[0];
    v->tri[1] = key[1];
    v->tri[2] = key[2];
    return id;
}

/* Retourne l'id de l'arête {a0,a1}, en la créant si nécessaire. */
static int find_or_add_edge(Board *b, Cube a0, Cube a1) {
    Cube key[2] = { a0, a1 };
    sort2(key);
    for (int i = 0; i < b->n_edges; i++) {
        Edge *e = &b->edges[i];
        if (cube_eq(e->pair[0], key[0]) && cube_eq(e->pair[1], key[1])) {
            return i;
        }
    }
    assert(b->n_edges < MAX_EDGES);
    int id = b->n_edges++;
    Edge *e = &b->edges[id];
    memset(e, 0, sizeof(*e));
    e->pair[0] = key[0];
    e->pair[1] = key[1];
    return id;
}

/* --- construction --------------------------------------------------------- */

void board_build(Board *b) {
    memset(b, 0, sizeof(*b));

    /* 1) Tuiles : hexagone plein de rayon BOARD_RADIUS → 19 cases. */
    for (int x = -BOARD_RADIUS; x <= BOARD_RADIUS; x++) {
        int ylo = -x - BOARD_RADIUS;
        int yhi = -x + BOARD_RADIUS;
        if (ylo < -BOARD_RADIUS) ylo = -BOARD_RADIUS;
        if (yhi >  BOARD_RADIUS) yhi =  BOARD_RADIUS;
        for (int y = ylo; y <= yhi; y++) {
            assert(b->n_tiles < MAX_TILES);
            Tile *t = &b->tiles[b->n_tiles++];
            memset(t, 0, sizeof(*t));
            t->coord.x = x;
            t->coord.y = y;
            t->coord.z = -x - y;
            t->resource = RES_NONE;
            t->number = 0;
        }
    }

    /* 2) Intersections (coins) et 3) arêtes, engendrées par tuile.
     *    Coin d : hexes {T, T+dir[d], T+dir[d+1]}. Arête d : {T, T+dir[d]}. */
    for (int i = 0; i < b->n_tiles; i++) {
        Cube c = b->tiles[i].coord;
        for (int d = 0; d < 6; d++) {
            Cube n1 = cube_neighbor(c, d);
            Cube n2 = cube_neighbor(c, (d + 1) % 6);
            b->tiles[i].vertices[d] = find_or_add_vertex(b, c, n1, n2);
            b->tiles[i].edges[d]    = find_or_add_edge(b, c, n1);
        }
    }

    /* 4) Voisinage entre tuiles. */
    for (int i = 0; i < b->n_tiles; i++) {
        Tile *t = &b->tiles[i];
        t->n_neighbors = 0;
        for (int d = 0; d < 6; d++) {
            int id = find_tile(b, cube_neighbor(t->coord, d));
            t->neighbors[d] = id;
            if (id >= 0) t->n_neighbors++;
        }
    }

    /* 5) Arêtes → tuiles bordantes + intersections aux extrémités.
     *    Les 2 extrémités de {A,B} sont les intersections {A,B,C} où C est
     *    l'un des 2 voisins communs de A et B. */
    for (int i = 0; i < b->n_edges; i++) {
        Edge *e = &b->edges[i];
        Cube A = e->pair[0], B = e->pair[1];

        e->n_tiles = 0;
        int ta = find_tile(b, A);
        if (ta >= 0) e->tiles[e->n_tiles++] = ta;
        int tb = find_tile(b, B);
        if (tb >= 0) e->tiles[e->n_tiles++] = tb;

        int vv[2] = { -1, -1 }, found = 0;
        for (int d = 0; d < 6 && found < 2; d++) {
            Cube C = cube_neighbor(A, d);
            if (cube_dist(C, B) == 1) {   /* C voisin commun de A et B */
                vv[found++] = find_or_add_vertex(b, A, B, C);
            }
        }
        assert(found == 2);
        e->v[0] = vv[0];
        e->v[1] = vv[1];
    }

    /* 6) Intersections → tuiles bordantes. */
    for (int i = 0; i < b->n_vertices; i++) {
        Vertex *v = &b->vertices[i];
        v->n_tiles = 0;
        v->n_edges = 0;
        v->n_adj = 0;
        for (int t = 0; t < 3; t++) {
            int id = find_tile(b, v->tri[t]);
            if (id >= 0) v->tiles[v->n_tiles++] = id;
        }
    }

    /* 7) Arêtes incidentes et intersections voisines, dérivées de la table
     *    des arêtes (chaque arête relie ses 2 extrémités). */
    for (int i = 0; i < b->n_edges; i++) {
        Edge *e = &b->edges[i];
        int a = e->v[0], c = e->v[1];
        Vertex *va = &b->vertices[a];
        Vertex *vc = &b->vertices[c];
        assert(va->n_edges < 3 && vc->n_edges < 3);
        va->edges[va->n_edges++] = i;
        va->adj[va->n_adj++] = c;
        vc->edges[vc->n_edges++] = i;
        vc->adj[vc->n_adj++] = a;
    }
}
