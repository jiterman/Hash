#define _GNU_SOURCE 1
#include "hash.h"
#include "lista.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define TAM_INICIAL 7
#define CRIT_AGRANDAR 3
#define CRIT_ACHICAR 4
#define FACTOR_AMPLIACION 2
#define FACTOR_REDUCCION 2

struct hash{
    lista_t** listas;
    size_t cantidad;
    size_t capacidad;
    void (*hash_destruir_dato_t)(void *);
};

typedef struct campo{
    char* clave;
    void* valor;
} campo_t;

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if (!hash) return NULL;
    
    hash->listas = calloc(TAM_INICIAL, sizeof(lista_t*));
    if (!hash->listas){
        free(hash);
        return NULL;
    }

    hash->cantidad = 0;
    hash->capacidad = TAM_INICIAL;
    hash->hash_destruir_dato_t = destruir_dato;
    return hash;
}

//Función de hash
size_t f_hash(hash_t* hash, char *str){
    size_t valor = 5381;
    int c;
    while ((c = *str++)){
        valor = ((valor << 5) + valor) + c;
    }

    return valor % hash->capacidad;
}

lista_iter_t* iter_buscar_clave(lista_t* lista, char* clave){
    lista_iter_t* iter = lista_iter_crear(lista);
    if (!iter) return NULL; 

    while (!lista_iter_al_final(iter)){
        char* clave_act = ((campo_t*)lista_iter_ver_actual(iter))->clave;
        if (!strcmp(clave_act, clave)) break;
        lista_iter_avanzar(iter);
    }

    return iter;
}

campo_t* buscar_campo(hash_t* hash, char* clave){
    size_t i = f_hash(hash, clave);

    if (!hash->cantidad || !hash->listas[i]) return NULL;

    lista_iter_t* iter_clave = iter_buscar_clave(hash->listas[i], clave); 
    campo_t* campo = (campo_t*)lista_iter_ver_actual(iter_clave);
    if (!iter_clave || !campo) return NULL;

    lista_iter_destruir(iter_clave);

    return campo;
}

bool redimensionar(hash_t* hash, size_t capacidad_nueva){
    void* datos_nuevos = calloc(capacidad_nueva, sizeof(lista_t*));
    if (!datos_nuevos) return false;   

}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    char* _clave = strdup(clave);
    size_t i = f_hash(hash, _clave);
    if (!hash->listas[i]){
        lista_t* lista = lista_crear();
        hash->listas[i] = lista;
    }
    lista_iter_t* iterador = iter_buscar_clave(listas[i], _clave);
    if (!iterador) {
        free(_clave);
        return false;
    }
    campo_t* campo;
    hash->cantidad++;
    else if (lista_iter_ver_actual(iterador)){
        campo = lista_iter_ver_actual(iterador);
        void* valor_a_borrar = campo->valor;
        campo->valor = dato;
        free(valor_a_borrar);
        return true;
    }
    campo->clave = _clave;
    campo->valor = dato;
    lista_iter_insertar(iterador, campo);    
    return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
    size_t i = f_hash(hash, clave);

    if (!hash->cantidad || !hash->listas[i]) return NULL;

    lista_iter_t* iter_clave = buscar_clave(hash->listas[i], clave); 
    campo_t* campo = (campo_t*)lista_iter_ver_actual(iter_clave);
    if (!iter_clave || !campo) return NULL;

    void* valor = campo->valor;
    free(campo->clave);
    lista_iter_borrar(iter_clave);
    lista_iter_destruir(iter_clave);
    hash->cantidad--;

    if (hash->cantidad >= hash->capacidad * FACTOR_AMPLIACION){
        redimensionar(hash, hash->capacidad * CRIT_AGRANDAR);
    }

    return valor;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    campo_t* campo = buscar_campo(hash, clave);

    if (!campo) return NULL;

    return campo->valor;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    campo_t* campo = buscar_campo(hash, clave);

    if (!campo) return false;

    return true;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}

void hash_destruir(hash_t *hash){
    size_t i = (hash->cantidad)-1;
    while (i>=0){
        lista_t* lista = hash->listas[i];
        while (lista && !lista_esta_vacia(lista)){
            campo_t* campo = lista_ver_primero(lista);
            hash->hash_destruir_dato_t(campo->valor);
            free(campo->clave);
            lista_borrar_primero(lista);
        }
        if(lista) lista_destruir(lista, NULL);
        else free(lista);
        hash->cantidad--;
        i--;
    }
    free(hash);
}

struct hash_iter{
    size_t pos;
    lista_iter_t* iter_lista;
    const hash_t* hash;
};

/* Busca la próxima posición con una lista no vacía.
 * Si el valor devuelto es igual a la capacidad del hash, 
 * entonces no hay más listas por recorrer */
size_t encontrar_prox_lista(const hash_t* hash, size_t n){
    size_t i = n;

    while (i < hash->capacidad && (!hash->listas[i] || lista_esta_vacia(hash->listas[i])){
        i++;
    }

    return i;
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if (!iter) return NULL;

    size_t i = encontrar_prox_lista(hash, 0);

    if (i == hash->capacidad){
        iter->iter_lista = NULL;
    }
    
    else{
        lista_iter_t* lista_iter = lista_iter_crear(hash->listas[i]);
        if (!iter_lista){
            free(iter);
            return NULL;
        }
        iter->iter_lista = lista_iter;
    }
    
    iter->pos = i;
    iter->hash = hash;

    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
    if (lista_iter_avanzar(iter->iter_lista) return true;

    size_t i = encontrar_prox_lista(hash, iter->pos + 1);
    if (i == iter->hash->capacidad) return false;

    *lista_iter_t lista_iter = lista_iter_crear(hash->listas[i]);
    if (!lista_iter) return false;

    lista_iter_destruir(iter->iter_lista);
    iter->pos = i;
    iter->iter_lista = lista_iter;

    return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter);

bool hash_iter_al_final(const hash_iter_t *iter){
    if (iter->pos == hash->capacidad) return true;
    if (!lista_iter_al_final(iter->iter_lista)) return false;
    return encontrar_prox_lista(iter->hash, iter->pos + 1) == iter->hash->capacidad;
}

void hash_iter_destruir(hash_iter_t* iter);