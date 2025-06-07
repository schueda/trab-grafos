#include "grafo.h"

#include <stdlib.h>
#include <string.h>

#define TAMANHO_BUFFER_LINHA 2048

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

    aresta *fronteira;
    aresta *ultima_aresta;

    vertice *pai;

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

    char *v_nome = (char *)malloc(TAMANHO_BUFFER_LINHA * sizeof(char));
    if (v_nome == NULL) {
        perror("[cria_vertice] Não foi possível allocar v_nome.\n");

        free(v);
        return NULL;
    }

    memcpy(v_nome, nome, TAMANHO_BUFFER_LINHA * sizeof(char));
    v->nome = v_nome;

    v->estado = 0;
    v->componente = 0;
    v->cor = 0;
    v->nivel = 0;
    v->low_point = 0;
    v->dist = 0;

    v->fronteira = NULL;
    v->ultima_aresta = NULL;

    v->pai = NULL;
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

    g->nome = (char *)malloc(TAMANHO_BUFFER_LINHA * sizeof(char));
    if (g->nome == NULL) {
        perror("[cria_grafo] Não foi possível alocar nome do grafo.\n");
        free(g);
        return NULL;
    }

    memcpy(g->nome, nome, TAMANHO_BUFFER_LINHA * sizeof(char));
    g->nome[strcspn(g->nome, "\n")] = 0;

    g->n_vertices = 0;
    g->n_arestas = 0;

    return g;
}

grafo *le_grafo(FILE *f) {
    grafo *g = NULL;

    char *buffer = (char *)malloc(TAMANHO_BUFFER_LINHA * sizeof(char));
    while (fgets(buffer, TAMANHO_BUFFER_LINHA, f)) {
        char *token = strtok(buffer, " \t\n");
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

        token = strtok(NULL, " ");
        if (token != NULL && !strcmp(token, "--")) {
            token = strtok(NULL, " ");
            token[strcspn(token, "\n")] = 0;
            vertice *v = obtem_vertice(token, g);

            unsigned int peso;
            token = strtok(NULL, " ");
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
        v->estado = 0;
        v = v->prox;
    }

    fila_p *V = cria_fila_p(g->n_vertices);
    r->dist = 0;
    r->estado = 1;
    fila_insere_vertice(V, r);

    while (V->tam != 0) {
        v = fila_obtem_vertice(V);
        if (v == NULL) {
            perror("[dijkstra] Não foi possível obter vértice.\n");
            return 0;
        }

        aresta *a = v->fronteira;
        while (a != NULL) {
            vertice *u = a->dest;
            if (u == NULL) {
                perror("[dijkstra] Aresta apontando para nulo.\n");
                return 0;
            }
            u->componente = r->componente;

            if (u->estado == 1) {
                if (v->dist + a->peso < u->dist) {
                    u->dist = v->dist + a->peso;
                }
            } else if (u->estado == 0) {
                u->dist = v->dist + a->peso;
                u->estado = 1;
                fila_insere_vertice(V, u);
            }
            a = a->prox;
        }
        v->estado = 2;
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

static int compara_ints(const void *a, const void *b) {
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
    unsigned int *diams = (unsigned int *)calloc(g->n_vertices, sizeof(unsigned int));

    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->componente == 0) {
            v->componente = ++c;
        }
        unsigned int max = dijkstra(g, v);
        diams[v->componente - 1] = diams[v->componente - 1] < max ? max : diams[v->componente - 1];

        v = v->prox;
    }

    qsort(diams, c, sizeof(int), compara_ints);

    unsigned int tamanho_buffer = (10 * c) + 1;
    char *s = (char *)malloc(tamanho_buffer * sizeof(char));
    s[0] = '\0';

    for (int i = 0; i < (int)c; i++) {
        char numero[10] = "";
        sprintf(numero, "%u ", diams[i]);
        strncat(s, numero, tamanho_buffer);
    }
    s[strlen(s) - 1] = '\0';

    return s;
}

static unsigned int v_corte(grafo *g, vertice *r, char **cortes, unsigned int *tam_cortes) {
    r->estado = 1;
    aresta *a = r->fronteira;
    while (a != NULL) {
        vertice *w = a->dest;
        if (w == NULL) {
            perror("[v_corte] Aresta apontando para nulo.\n");
            return 0;
        }

        if (w->estado == 1 && w->nivel < r->low_point && w != r->pai) {
            r->low_point = w->nivel;
        } else if (w->estado == 0) {
            w->pai = r;
            w->nivel = r->nivel + 1;
            w->low_point = w->nivel;
            v_corte(g, w, cortes, tam_cortes);
            if (w->low_point < r->low_point) {
                r->low_point = w->low_point;
            }
        }

        a = a->prox;
    }
    r->estado = 2;

    a = r->fronteira;
    int n_de_filhos = 0;
    while (a != NULL) {
        vertice *w = a->dest;
        if (w == NULL) {
            perror("[v_corte] Aresta apontando para nulo.\n");
            return 0;
        }

        if (w->pai == r) {
            n_de_filhos++;
            if (r->pai == NULL) {
                if (n_de_filhos >= 2) {
                    cortes[(*tam_cortes)++] = r->nome;
                }
            } else {
                if (r->nivel <= w->low_point) {
                    cortes[(*tam_cortes)++] = r->nome;
                }
            }
        }

        a = a->prox;
    }
    return 0;
}

static int compara_strings(const void *str1, const void *str2) {
    char *const *pp1 = str1;
    char *const *pp2 = str2;
    return strcmp(*pp1, *pp2);
}

char *vertices_corte(grafo *g) {
    if (g == NULL) {
        perror("[vertices_corte] Grafo nulo.\n");
        return NULL;
    }

    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        v->estado = 0;
        v->pai = NULL;

        v = v->prox;
    }

    unsigned int tam_cortes = 0;
    char **cortes = (char **)malloc(g->n_vertices * sizeof(char *));
    if (cortes == NULL) {
        perror("[vertices_corte] Não foi possível alocar cortes.\n");
        return NULL;
    }

    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->estado == 0) {
            v->low_point = 0;
            v->nivel = 0;
            v_corte(g, v, cortes, &tam_cortes);
        }

        v = v->prox;
    }

    qsort(cortes, tam_cortes, sizeof(char *), compara_strings);

    unsigned int tamanho_buffer = tam_cortes * TAMANHO_BUFFER_LINHA;
    char *cortes_str = (char *)malloc(tamanho_buffer * sizeof(char));
    cortes_str[0] = '\0';

    for (unsigned int i = 0; i < tam_cortes; i++) {
        unsigned int len = (unsigned int)strlen(cortes[i]);
        if (len >= TAMANHO_BUFFER_LINHA - 2) {
            perror("[vertices_corte] Buffer da string cheio.\n");
            return NULL;
        }
        cortes[i][len] = ' ';
        cortes[i][len + 1] = '\0';
        strncat(cortes_str, cortes[i], tamanho_buffer);
        cortes[i][len] = '\0';
    }
    cortes_str[strlen(cortes_str) - 1] = '\0';

    free(cortes);
    return cortes_str;
}

static unsigned int a_corte(grafo *g, vertice *r, char **cortes, unsigned int *tam_cortes) {
    r->estado = 1;
    aresta *a = r->fronteira;
    while (a != NULL) {
        vertice *w = a->dest;
        if (w == NULL) {
            perror("[v_corte] Aresta apontando para nulo.\n");
            return 0;
        }

        if (w->estado == 1 && w->nivel < r->low_point && w != r->pai) {
            r->low_point = w->nivel;
        } else if (w->estado == 0) {
            w->pai = r;
            w->nivel = r->nivel + 1;
            w->low_point = w->nivel;
            a_corte(g, w, cortes, tam_cortes);
            if (w->low_point < r->low_point) {
                r->low_point = w->low_point;
            }
        }

        a = a->prox;
    }
    r->estado = 2;

    a = r->fronteira;
    while (a != NULL) {
        vertice *v = a->dest;
        if (v == NULL) {
            perror("[v_corte] Aresta apontando para nulo.\n");
            return 0;
        }

        if (v->pai == r && r->nivel < v->low_point) {
            char *a_str = (char *)malloc(2 * TAMANHO_BUFFER_LINHA * sizeof(char));
            a_str[0] = '\0';
            if (strcmp(r->nome, v->nome) < 0) {
                sprintf(a_str, "%s %s", r->nome, v->nome);
            } else {
                sprintf(a_str, "%s %s", v->nome, r->nome);
            }
            cortes[(*tam_cortes)++] = a_str;
        }

        a = a->prox;
    }

    return 0;
}

char *arestas_corte(grafo *g) {
    if (g == NULL) {
        perror("[arestas_corte] Grafo nulo.\n");
        return NULL;
    }

    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        v->estado = 0;
        v->pai = NULL;

        v = v->prox;
    }

    unsigned int tam_cortes = 0;
    char **cortes = (char **)malloc(g->n_vertices * sizeof(char *));
    if (cortes == NULL) {
        perror("[arestas_corte] Não foi possível alocar cortes.\n");
        return NULL;
    }

    v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->estado == 0) {
            v->low_point = 0;
            v->nivel = 0;
            a_corte(g, v, cortes, &tam_cortes);
        }

        v = v->prox;
    }

    qsort(cortes, tam_cortes, sizeof(char *), compara_strings);

    unsigned int tamanho_buffer = tam_cortes * TAMANHO_BUFFER_LINHA;
    char *cortes_str = (char *)malloc(tamanho_buffer * sizeof(char));
    cortes_str[0] = '\0';

    for (unsigned int i = 0; i < tam_cortes; i++) {
        unsigned int len = (unsigned int)strlen(cortes[i]);
        if (len >= (2 * TAMANHO_BUFFER_LINHA) - 2) {
            perror("[arestas_corte] Buffer da string cheio.\n");
            return NULL;
        }
        cortes[i][len] = ' ';
        cortes[i][len + 1] = '\0';
        strncat(cortes_str, cortes[i], tamanho_buffer);
        free(cortes[i]);
    }
    cortes_str[strlen(cortes_str) - 1] = '\0';

    free(cortes);
    return cortes_str;
}
