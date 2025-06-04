#include "grafo.h"

#include <stdlib.h>
#include  <string.h>

#define LINE_BUFFER_SIZE 2048

typedef struct vertice vertice;
typedef struct aresta aresta;

struct vertice {
    char *nome;
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
    int n_vertices;
    int n_arestas;
};

vertice *cria_vertice(char *nome) {
    vertice *v = (vertice *) malloc(sizeof(vertice));
    if (v == NULL) 
        return NULL;

    char *v_nome = (char *) malloc(LINE_BUFFER_SIZE * sizeof(char));
    if (v_nome == NULL)
        return NULL;

    memcpy(v_nome, nome, LINE_BUFFER_SIZE * sizeof(char));
    v->nome = v_nome;

    v->fronteira = NULL;
    v->prox = NULL;

    return v;
}

vertice *obtem_vertice(char *nome, grafo *g) {
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

aresta *cria_aresta(vertice *dest, int peso) {
    aresta *a = (aresta *) malloc(sizeof(aresta));
    if (a == NULL)
        return NULL;

    a->dest = dest;
    a->peso = peso;

    return a;
}

int vertice_recebe_aresta(vertice *v, aresta *a) {
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

int adiciona_aresta(vertice *u, vertice *v, int peso) {
    if (u == NULL || v == NULL) 
        return 0;

    aresta *a = cria_aresta(v, peso);
    vertice_recebe_aresta(u, a);

    a = cria_aresta(u, peso);
    vertice_recebe_aresta(v, a);

    return 1;
}

void print_grafo(grafo *g) {
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

grafo *le_grafo(FILE *f) {
    grafo *g = (grafo *) malloc(sizeof(grafo));
    if (g == NULL)
        return NULL;

    g->n_vertices = 0;
    g->n_arestas = 0;

    char *g_nome = (char *) malloc(LINE_BUFFER_SIZE * sizeof(char));
    fgets(g_nome, LINE_BUFFER_SIZE, f);
    g_nome[strcspn(g_nome, "\n")] = 0;
    g->nome = g_nome;

    char *buffer = (char *) malloc(LINE_BUFFER_SIZE * sizeof(char));
    while (fgets(buffer, LINE_BUFFER_SIZE, f)) {
        char *save_ptr;

        char *token = strtok_r(buffer, " ", &save_ptr);
        token[strcspn(token, "\n")] = 0;
        vertice *u = obtem_vertice(token, g);

        token = strtok_r(NULL, " ", &save_ptr);
        if (token != NULL && !strcmp(token, "--")) {
            token = strtok_r(NULL, " ", &save_ptr);
            token[strcspn(token, "\n")] = 0;
            vertice *v = obtem_vertice(token, g);

            int peso;
            token = strtok_r(NULL, " ", &save_ptr);
            if (token == NULL) {
                peso = 1;
            } else {
                peso = atoi(token);
                if (!peso)
                    peso = 1;
            }

            printf("adicionando aresta: (%s)----%d----(%s)\n", u->nome, peso, v->nome);
            
            adiciona_aresta(u, v, peso);
            g->n_arestas++;
        }
    }
    
    print_grafo(g);
    exit(0);

    return g;
}

unsigned int destroi_grafo(grafo *g) {
    return 0;
}

char *nome(grafo *g) {
    return NULL;
}

unsigned int bipartido(grafo *g) {
    return 0;
}

unsigned int n_vertices(grafo *g) {
    return 0;
}

unsigned int n_arestas(grafo *g) {
    return 0;
}

unsigned int n_componentes(grafo *g) {
    return 0;
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
