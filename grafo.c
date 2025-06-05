#include "grafo.h"

#include <stdlib.h>
#include  <string.h>

#define LINE_BUFFER_SIZE 2048

typedef struct vertice vertice;
typedef struct aresta aresta;

struct vertice {
    char *nome;

    unsigned int estado;
    unsigned int componente;

    aresta *fronteira;
    aresta *ultima_aresta;

    vertice *prox;
};

struct aresta {
    int peso;
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
    vertice *v = (vertice *) malloc(sizeof(vertice));
    if (v == NULL) 
        return NULL;

    char *v_nome = (char *) malloc(LINE_BUFFER_SIZE * sizeof(char));
    if (v_nome == NULL)
        return NULL;

    memcpy(v_nome, nome, LINE_BUFFER_SIZE * sizeof(char));
    v->nome = v_nome;

    v->estado = 0;
    v->componente = 0;

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

static aresta *cria_aresta(vertice *dest, int peso) {
    aresta *a = (aresta *) malloc(sizeof(aresta));
    if (a == NULL)
        return NULL;

    a->dest = dest;
    a->peso = peso;

    return a;
}

static int vertice_recebe_aresta(vertice *v, aresta *a) {
    if (v == NULL)
        return 0;
    
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

static int adiciona_aresta(vertice *u, vertice *v, int peso) {
    if (u == NULL || v == NULL) 
        return 0;

    aresta *a = cria_aresta(v, peso);
    vertice_recebe_aresta(u, a);

    a = cria_aresta(u, peso);
    vertice_recebe_aresta(v, a);

    return 1;
}

static void print_grafo(grafo *g) {
    if (g == NULL)
        return;

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
    if (nome == NULL)
        return NULL;

    grafo *g = (grafo *) malloc(sizeof(grafo));
    if (g == NULL)
        return NULL;

    g->n_vertices = 0;
    g->n_arestas = 0;

    g->nome = (char *) malloc(LINE_BUFFER_SIZE * sizeof(char));
    if (g->nome == NULL) {
        return NULL;
    }

    memcpy(g->nome, nome, LINE_BUFFER_SIZE * sizeof(char));
    g->nome[strcspn(g->nome, "\n")] = 0;
    
    return g;
}

grafo *le_grafo(FILE *f) {
    grafo *g = NULL;

    char *buffer = (char *) malloc(LINE_BUFFER_SIZE * sizeof(char));
    while (fgets(buffer, LINE_BUFFER_SIZE, f)) {
        char *str_resto;

        char *token = strtok_r(buffer, " \t\n", &str_resto);
        if (token == NULL || !strncmp(token, "//", 2))
            continue;
        token[strcspn(token, "\n")] = 0;

        if (g == NULL) {
            g = cria_grafo(buffer);
            if (g == NULL)
                return NULL;
            continue;
        }

        vertice *u = obtem_vertice(token, g);

        token = strtok_r(NULL, " ", &str_resto);
        if (token != NULL && !strcmp(token, "--")) {
            token = strtok_r(NULL, " ", &str_resto);
            token[strcspn(token, "\n")] = 0;
            vertice *v = obtem_vertice(token, g);

            int peso;
            token = strtok_r(NULL, " ", &str_resto);
            if (token == NULL) {
                peso = 1;
            } else {
                peso = atoi(token);
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
    if (g == NULL) 
        return 0;

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
    return g->nome;
}

unsigned int bipartido(grafo *g) {
    return 0;
}

unsigned int n_vertices(grafo *g) {
    return g->n_vertices;
}

unsigned int n_arestas(grafo *g) {
    return g->n_arestas;
}

static int componentes(grafo *g, vertice *r) {
    r->estado = 1;

    vertice **V = (vertice **) malloc(g->n_vertices * sizeof(vertice *));
    if (V == NULL)
        return 0;
    
    int começo_V = 0;
    int final_V = 0;
    V[final_V++] = r;
    while (começo_V < final_V) {
        vertice *v = V[começo_V++];

        aresta *a = v->fronteira;
        while (a != NULL) {
            vertice *w = a->dest;
            if (w == NULL)
                return 0;

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
    if (g == NULL)
        return 0;

    unsigned int c = 0;
    vertice *v = g->lista_de_vertices;
    while (v != NULL) {
        if (v->estado == 0) {
            v->componente = ++c;
            componentes(g, v);
        }

        v = v->prox;
    }
    return c;
}

char *diametros(grafo *g) {
    return NULL;
}

char *vertices_corte(grafo *g) {
    return NULL;
}

char *arestas_corte(grafo *g) {
    return NULL;
}
