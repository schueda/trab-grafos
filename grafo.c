#include "grafo.h"

#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 2048

typedef struct vertice vertice;
typedef struct aresta aresta;

struct vertice {
    char *nome;

    unsigned int estado;
    unsigned int componente;
    unsigned int cor;
    unsigned int nivel;
    unsigned int low_point;
    unsigned int dist;

    unsigned int fila_pos;

    aresta *fronteira;
    aresta *ultima_aresta;

    vertice *prox;
};

struct aresta {
    unsigned int peso;
    vertice *dest;

    aresta *prox;
};

struct grafo {
    char *nome;

    vertice *lista_de_vertices;
    unsigned int n_vertices;
    unsigned int n_arestas;
};

static vertice *cria_vertice(char *nome) {
    vertice *v = (vertice *)malloc(sizeof(vertice));
    if (v == NULL) {
        perror("[cria_vertice] Não foi possível alocar vértice.\n");
        return NULL;
    }

    char *v_nome = (char *)malloc(LINE_BUFFER_SIZE * sizeof(char));
    if (v_nome == NULL) {
        perror("[cria_vertice] Não foi possível allocar v_nome.\n");

        free(v);
        return NULL;
    }

    memcpy(v_nome, nome, LINE_BUFFER_SIZE * sizeof(char));
    v->nome = v_nome;

    v->estado = 0;
    v->componente = 0;
    v->cor = 0;
    v->nivel = 0;
    v->low_point = 0;
    v->dist = 0;

    v->fronteira = NULL;
    v->prox = NULL;

    return v;
}

static vertice *obtem_vertice(char *nome, grafo *g) {
    vertice *v_ant = NULL;
    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        if (!strcmp(v->nome, nome)) {
            return v;
        }
        v_ant = v;
        v = v->prox;
    }

    v = cria_vertice(nome);
    g->n_vertices++;

    if (v_ant == NULL) {
        g->lista_de_vertices = v;
        return v;
    }

    v_ant->prox = v;
    return v;
}

static aresta *cria_aresta(vertice *dest, unsigned int peso) {
    aresta *a = (aresta *)malloc(sizeof(aresta));
    if (a == NULL) {
        perror("[cria_aresta] Não foi possível alocar aresta.\n");
        return NULL;
    }

    a->dest = dest;
    a->peso = peso;

    return a;
}

static int vertice_recebe_aresta(vertice *v, aresta *a) {
    if (v == NULL) {
        perror("[vertice_recebe_aresta] Vértice nulo.\n");
        return 0;
    }

    if (v->fronteira == NULL) {
        v->fronteira = a;
        v->ultima_aresta = a;
        return 1;
    }

    if (v->ultima_aresta != NULL) {
        v->ultima_aresta->prox = a;
        v->ultima_aresta = a;
        return 1;
    }

    return 0;
}

static int adiciona_aresta(vertice *u, vertice *v, unsigned int peso) {
    if (u == NULL || v == NULL) {
        perror("[adiciona_aresta] Vértice nulo.\n");
        return 0;
    }

    aresta *a = cria_aresta(v, peso);
    vertice_recebe_aresta(u, a);

    a = cria_aresta(u, peso);
    vertice_recebe_aresta(v, a);

    return 1;
}

static void print_grafo(grafo *g) {
    if (g == NULL) {
        perror("[print_grafo] Grafo nulo.\n");
        return;
    }

    printf("nome: %s\n", g->nome);
    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        printf("%s:", v->nome);
        aresta *a = v->fronteira;
        while (a != NULL) {
            if (a->dest != NULL) {
                printf(" %s(%d)", a->dest->nome, a->peso);
            }
            a = a->prox;
        }
        printf("\n");
        v = v->prox;
    }
}

static grafo *cria_grafo(char *nome) {
    if (nome == NULL) {
        perror("[cria_grafo] nome nulo.\n");
        return NULL;
    }

    grafo *g = (grafo *)malloc(sizeof(grafo));
    if (g == NULL) {
        perror("[cria_grafo] Não foi possível alocar grafo.\n");
        return NULL;
    }

    g->nome = (char *)malloc(LINE_BUFFER_SIZE * sizeof(char));
    if (g->nome == NULL) {
        perror("[cria_grafo] Não foi possível alocar nome do grafo.\n");
        free(g);
        return NULL;
    }

    memcpy(g->nome, nome, LINE_BUFFER_SIZE * sizeof(char));
    g->nome[strcspn(g->nome, "\n")] = 0;

    g->n_vertices = 0;
    g->n_arestas = 0;

    return g;
}

grafo *le_grafo(FILE *f) {
    grafo *g = NULL;

    char *buffer = (char *)malloc(LINE_BUFFER_SIZE * sizeof(char));
    while (fgets(buffer, LINE_BUFFER_SIZE, f)) {
        char *str_resto;

        char *token = strtok_r(buffer, " \t\n", &str_resto);
        if (token == NULL || !strncmp(token, "//", 2))
            continue;
        token[strcspn(token, "\n")] = 0;

        if (g == NULL) {
            g = cria_grafo(buffer);
            if (g == NULL) {
                perror("[le_grafo] Não foi possível criar o grafo.\n");
                return NULL;
            }
            continue;
        }

        vertice *u = obtem_vertice(token, g);

        token = strtok_r(NULL, " ", &str_resto);
        if (token != NULL && !strcmp(token, "--")) {
            token = strtok_r(NULL, " ", &str_resto);
            token[strcspn(token, "\n")] = 0;
            vertice *v = obtem_vertice(token, g);

            unsigned int peso;
            token = strtok_r(NULL, " ", &str_resto);
            if (token == NULL) {
                peso = 1;
            } else {
                peso = (unsigned int)atoi(token);
                if (!peso)
                    peso = 1;
            }

            adiciona_aresta(u, v, peso);
            g->n_arestas++;
        }
    }

    free(buffer);
    return g;
}

unsigned int destroi_grafo(grafo *g) {
    if (g == NULL) {
        perror("[destroi_grafo] Grafo nulo.\n");
        return 0;
    }

    if (g->nome != NULL) {
        free(g->nome);
        g->nome = NULL;
    }

    vertice *v = g->lista_de_vertices;
    vertice *v_aux = NULL;
    while (v != NULL) {
        v_aux = v->prox;

        if (v->nome != NULL) {
            free(v->nome);
            v->nome = NULL;
        }

        aresta *a = v->fronteira;
        aresta *a_aux = NULL;
        while (a != NULL) {
            a_aux = a->prox;
            free(a);
            a = a_aux;
        }

        free(v);
        v = v_aux;
    }

    return 1;
}

char *nome(grafo *g) {
    if (g == NULL) {
        perror("[nome] Grafo nulo.\n");
        return NULL;
    }

    return g->nome;
}

static unsigned int comp_bipartido(grafo *g, vertice *r) {
    r->estado = 1;

    vertice **V = (vertice **)malloc(g->n_vertices * sizeof(vertice *));
    if (V == NULL) {
        perror("[comp_bipartido] Não foi possível alocar V.\n");
        return 0;
    }

    int começo_V = 0;
    int final_V = 0;
    V[final_V++] = r;

    while (começo_V < final_V) {
        vertice *v = V[começo_V++];

        aresta *a = v->fronteira;
        while (a != NULL) {
            vertice *w = a->dest;
            if (w == NULL) {
                perror("[comp_bipartido] Aresta apontando para nulo.\n");
                return 0;
            }

            if (w->estado == 0) {
                w->cor = v->cor == 1 ? 0 : 1;
                w->estado = 1;
                V[final_V++] = w;
            } else if (w->cor == v->cor) {
                free(V);
                return 0;
            }
            a = a->prox;
        }
        v->estado = 2;
    }

    free(V);
    return 1;
}

unsigned int bipartido(grafo *g) {
    if (g == NULL) {
        perror("[bipartido] Grafo nulo.\n");
        return 0;
    }

    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        v->estado = 0;
        v->cor = 0;

        v = v->prox;
    }

    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->estado == 0) {
            if (!comp_bipartido(g, v)) {
                return 0;
            }
        }

        v = v->prox;
    }

    return 1;
}

unsigned int n_vertices(grafo *g) {
    if (g == NULL) {
        perror("[n_vertices] Grafo nulo.\n");
        return 0;
    }

    return g->n_vertices;
}

unsigned int n_arestas(grafo *g) {
    if (g == NULL) {
        perror("[n_arestas] Grafo nulo.\n");
        return 0;
    }

    return g->n_arestas;
}

static int componentes(grafo *g, vertice *r) {
    r->estado = 1;

    vertice **V = (vertice **)malloc(g->n_vertices * sizeof(vertice *));
    if (V == NULL) {
        perror("[componentes] Não foi possível alocar V.\n");
        return 0;
    }

    int começo_V = 0;
    int final_V = 0;
    V[final_V++] = r;
    while (começo_V < final_V) {
        vertice *v = V[começo_V++];

        aresta *a = v->fronteira;
        while (a != NULL) {
            vertice *w = a->dest;
            if (w == NULL) {
                perror("[componentes] Aresta apontando para nulo.\n");
                return 0;
            }

            if (w->estado == 0) {
                w->componente = v->componente;
                w->estado = 1;
                V[final_V++] = w;
            }
            a = a->prox;
        }

        v->estado = 2;
    }

    return 1;
}

unsigned int n_componentes(grafo *g) {
    if (g == NULL) {
        perror("[n_componentes] Grafo nulo.\n");
        return 0;
    }

    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        v->estado = 0;
        v->componente = 0;

        v = v->prox;
    }

    unsigned int c = 0;
    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->estado == 0) {
            v->componente = ++c;
            componentes(g, v);
        }

        v = v->prox;
    }
    return c;
}

typedef struct fila_p fila_p;
struct fila_p {
    vertice **vertices;
    unsigned int tam;
    unsigned int tam_max;
};

static fila_p *cria_fila_p(unsigned int tam) {
    fila_p *fila = (fila_p *)malloc(sizeof(fila_p));
    if (fila == NULL) {
        perror("[cria_fila_p] Não foi possível alocar fila de prioridade.\n");
        return NULL;
    }

    fila->vertices = (vertice **)malloc(tam * sizeof(vertice *));
    if (fila->vertices == NULL) {
        perror("[cria_fila_p] Não foi possível alocar lista de vértices da fila de prioridade.\n");
        free(fila);
    }

    fila->tam = 0;
    fila->tam_max = tam;

    return fila;
}

static int destroi_fila_p(fila_p *fila) {
    if (fila == NULL) {
        perror("[destroi_fila_p] Fila nula.\n");
        return 0;
    }

    if (fila->vertices != NULL) {
        free(fila->vertices);
    }

    free(fila);
    return 1;
}

static void print_fila(fila_p *fila) {
    if (fila == NULL) {
        perror("[print_fila] Fila nula.\n");
        return;
    }

    for (int i = 0; i < (int)fila->tam; i++) {
        printf("[%u]%s:(dist: %u)\n", fila->vertices[i]->fila_pos, fila->vertices[i]->nome, fila->vertices[i]->dist);
    }
    printf("\n");
}

static unsigned int pai(unsigned int i) {
    return (i - 1) / 2;
}

static unsigned int filho_esq(unsigned int i) {
    return (2 * i) + 1;
}

static unsigned int filho_dir(unsigned int i) {
    return (2 * i) + 2;
}

static void troca(vertice **u, vertice **v) {
    vertice *v_aux = *u;
    *u = *v;
    *v = v_aux;
}

static void sobe(fila_p *fila, unsigned int i) {
    while (i > 0 && fila->vertices[pai(i)]->dist > fila->vertices[i]->dist) {
        fila->vertices[i]->fila_pos = pai(i);
        fila->vertices[pai(i)]->fila_pos = i;
        troca(&fila->vertices[pai(i)], &fila->vertices[i]);
        i = pai(i);
    }
}

static void desce(fila_p *fila, unsigned int i) {
    unsigned int i_min = i;
    unsigned int e = filho_esq(i);
    unsigned int d = filho_dir(i);

    if (e < fila->tam && fila->vertices[e]->dist < fila->vertices[i_min]->dist) {
        i_min = e;
    }

    if (d < fila->tam && fila->vertices[d]->dist < fila->vertices[i_min]->dist) {
        i_min = d;
    }

    if (i != i_min) {
        fila->vertices[i]->fila_pos = i_min;
        fila->vertices[i_min]->fila_pos = i;
        troca(&fila->vertices[i], &fila->vertices[i_min]);
        desce(fila, i_min);
    }
}

static unsigned int fila_insere_vertice(fila_p *fila, vertice *v) {
    if (fila == NULL || v == NULL) {
        perror("[fila_insere_vertice] Função recebeu argumento nulo.\n");
        return 0;
    }

    if (fila->tam >= fila->tam_max) {
        perror("[fila_insere_vertice] Fila cheia.\n");
        return 0;
    }

    fila->vertices[fila->tam++] = v;
    v->fila_pos = fila->tam - 1;
    sobe(fila, fila->tam - 1);

    return 1;
}

static vertice *fila_obtem_vertice(fila_p *fila) {
    if (fila == NULL || fila->vertices == NULL) {
        perror("[fila_obtem_vertice] Função recebeu argumento nulo.\n");
        return NULL;
    }

    if (fila->tam == 0) {
        perror("[fila_obtem_vertice] Fila vazia.\n");
        return NULL;
    }

    vertice *v = fila->vertices[0];
    fila->vertices[0] = fila->vertices[--fila->tam];
    fila->vertices[0]->fila_pos = 0;
    desce(fila, 0);

    return v;
}

static unsigned int dijkstra(grafo *g, vertice *r) {
    if (g == NULL || r == NULL) {
        perror("[dijkstra] Valor nulo passado como parâmetro.\n");
        return 0;
    }

    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        v->dist = UINT32_MAX;
        v = v->prox;
    }

    fila_p *V = cria_fila_p(g->n_vertices);
    r->dist = 0;

    v = g->lista_de_vertices;
    while (v != NULL) {
        fila_insere_vertice(V, v);
        v = v->prox;
    }

    while (V->tam != 0) {
        v = fila_obtem_vertice(V);
        v->fila_pos = V->tam_max;
        if (v == NULL) {
            perror("[dijkstra] Não foi possível obter vértice.\n");
            return 0;
        }

        if (v->componente == r->componente) {
            aresta *a = v->fronteira;
            while (a != NULL) {
                vertice *u = a->dest;
                if (u == NULL) {
                    perror("[dijkstra] Aresta apontando para nulo.\n");
                    return 0;
                }
                u->componente = r->componente;

                if (v->dist + a->peso < u->dist) {
                    u->dist = v->dist + a->peso;
                    if (u->fila_pos < V->tam_max) {
                        sobe(V, u->fila_pos);
                    }
                }

                a = a->prox;
            }
        }
    }

    destroi_fila_p(V);

    unsigned int max = 0;
    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->componente == r->componente && v->dist > max) {
            max = v->dist;
        }
        v = v->prox;
    }

    return max;
}

static int compare_ints(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;

    return *ia - *ib;
}

char *diametros(grafo *g) {
    if (g == NULL) {
        perror("[diametros] Grafo nulo.\n");
        return NULL;
    }

    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        v->componente = 0;
        v = v->prox;
    }

    unsigned int c = 0;
    unsigned int *diams = (unsigned int *)malloc(g->n_vertices * sizeof(unsigned int));

    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->componente == 0) {
            v->componente = ++c;
        }
        unsigned int max = dijkstra(g, v);
        diams[v->componente - 1] = diams[v->componente - 1] < max ? max : diams[v->componente - 1];

        v = v->prox;
    }

    qsort(diams, c, sizeof(int), compare_ints);

    unsigned int buffer_size = (10 * c) + 1;
    char *s = (char *)malloc(buffer_size * sizeof(char));

    for (int i = 0; i < (int)c; i++) {
        char number[10] = "";
        sprintf(number, "%d ", diams[i]);
        strncat(s, number, buffer_size);
    }
    s[strlen(s) - 1] = '\0';

    return s;
}

char *vertices_corte(grafo *g) {
    return NULL;
}

char *arestas_corte(grafo *g) {
    return NULL;
}
