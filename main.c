/*
 * @author Patrice Plouvin
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char *mot;
    int *nb_occurence;
    struct node *suiv;
} node;
typedef struct Liste {
    node *tete;
} Liste;
typedef struct Hash {
    Liste **buckets;
    int size;
} Hash;

typedef struct Fichier {
    char *nom;
    int nb_mots;
} Fichier;

// Créer une liste chainée
Liste *init_liste() {
    Liste *l = malloc(sizeof(Liste));
    l->tete = NULL;
    return l;
}

// Créer une nouvelle trable de hash
Hash *init_hash_table(int size) {
    Hash *h = malloc(sizeof(Hash));
    h->size = size;
    h->buckets = malloc(size * sizeof(Liste *));
    for (int i = 0; i < size; i++) h->buckets[i] = init_liste();
    return h;
}

// Créer un nouveau noeud
node *init_node(char *mot, int nbDoc, int indexDoc) {
    node *n = malloc(sizeof(node));
    n->mot = mot;
    n->nb_occurence = calloc(nbDoc, sizeof(int));
    n->nb_occurence[indexDoc] = 1;
    n->suiv = NULL;
    return n;
}

// Calculer la fonction de hashage
int hashKey(char *mot, int max) {
    int i = 0;
    int h = 0;
    while (mot[i] != '\0') {
        h += mot[i];
        i++;
    }
    return h % max;
}

// tester si un caractère est une lettre minuscule
int test_min(char c) { return (c >= 'a' && c <= 'z'); }
// tester si un caractère est une lettre majuscule
int test_maj(char c) { return (c >= 'A' && c <= 'Z'); }
// tester si un caractère est une lettre
int test_lettre(char c) { return (test_min(c) || test_maj(c)); }
// convertir en majuscule une lettre minuscule
char maj(char c) {
    if (test_min(c))
        return c - 32;
    else
        return c;
}
// ouvrir un fichier en lecture grâce à son nom (indice : stdio.h ou fcntl.h)
FILE *ouvrir(char *nom) {
    FILE *f = fopen(nom, "r");
    return f;
}
// lire un mot dans un fichier
char *lire_mot(FILE *f) {
    char *mot = malloc(25 * sizeof(char));
    int i = 0;
    char c;
    while (!test_lettre(c = fgetc(f)) && c != EOF)
        ;                       // on saute les séparateurs
    if (c == EOF) return NULL;  // fin du fichier
    mot[i++] = maj(c);  // On s'est arreté a une lettre donc on l'ajoute
    while (test_lettre(c = fgetc(f)))
        mot[i++] = maj(c);  // On ajoute les lettre suivantes
    mot[i] = '\0';          // Fin mot
    return mot;
}

// Ajouter un mot à la liste
void ajouter(Liste *l, char *mot, int nbDoc, int indexDoc) {
    node *n = init_node(mot, nbDoc, indexDoc);

    // Si la liste est vide
    if (l->tete == NULL) {
        l->tete = n;  // On ajoute le mot en tete
        return;
    }

    // On parcours la liste
    node *tmp = l->tete;

    // Si le mot est le meme que la tete
    if (strcmp(tmp->mot, mot) == 0) {
        tmp->nb_occurence[indexDoc]++;
        free(n);
        return;
    }

    // Si le mot est plus petit que le premier mot de la liste
    if (strcmp(tmp->mot, mot) > 0) {
        n->suiv = tmp;  // On l'ajoute en tete
        l->tete = n;    // On met a jour la tete
        return;
    }

    // On cherche la position du mot dans la liste
    while (tmp->suiv != NULL && strcmp(tmp->suiv->mot, mot) < 0)
        tmp = tmp->suiv;

    // Si le mot est deja present
    if (tmp->suiv != NULL && strcmp(tmp->suiv->mot, mot) == 0) {
        tmp->suiv->nb_occurence[indexDoc]++;
        free(n->nb_occurence);
        free(n);
        return;
    }

    // On ajoute le mot
    n->suiv = tmp->suiv;
    tmp->suiv = n;
}

// Ajouter un mot à la table de hash
void ajouter_hash(Hash *h, char *mot, int nbDoc, int indexDoc) {
    assert(h);
    int i = hashKey(mot, h->size);
    ajouter(h->buckets[i], mot, nbDoc, indexDoc);
}

// frequence d'un mot dans un fichier
float tf(Hash *h, Fichier *mesFichiers, char *mot, int idDoc) {
    int i = hashKey(mot, h->size);  // On recupere le bucket ou se trouve le mot
    node *tmp =
        h->buckets[i]->tete;  // On parcours la liste a la recherche du mot
    while (tmp != NULL) {
        if (strcmp(tmp->mot, mot) == 0) {
            return (float)tmp->nb_occurence[idDoc] /
                   (float)mesFichiers[idDoc]
                       .nb_mots;  // On retourne la frequence du mot dans le
                                  // document
        }
        tmp = tmp->suiv;  // Sinon on passe au mot suivant
    }
    return 0;
}
// idf(mot) qui est le nombre de documents du corpus divisé par le nombre de
// document où le mot apparaît
float idf(Hash *h, int nbDoc, char *mot) {
    int i = hashKey(mot, h->size);  // On recupere le bucket ou se trouve le mot
    node *tmp =
        h->buckets[i]->tete;  // On parcours la liste a la recherche du mot
    int nbDocMot = 0;
    while (tmp != NULL) {
        if (strcmp(tmp->mot, mot) == 0) {
            for (int i = 0; i < nbDoc; i++) {
                if (tmp->nb_occurence[i] > 0) nbDocMot++;
            }
            return (float)nbDoc / (float)nbDocMot;  // On retourne l'idf du mot
        }
        tmp = tmp->suiv;  // Sinon on passe au mot suivant
    }
    return 0;
}
// tf_idf(mot,document) qui est la multiplication des deux rapports précédents
float tf_idf(Hash *h, Fichier *mesFichiers, int nbDoc, char *mot, int idDoc) {
    return tf(h, mesFichiers, mot, idDoc) * idf(h, nbDoc, mot);
}

// Afficher la liste
void printListe(Liste *l, int nbDoc) {
    node *tmp = l->tete;
    while (tmp != NULL) {
        printf("%s : ", tmp->mot);
        for (int i = 0; i < nbDoc; i++)
            printf("DOC[%d]->%doccurences ", i, tmp->nb_occurence[i]);
        printf("\n");
        tmp = tmp->suiv;
    }
}

// Afficher la table de hash
void print_hash(Hash *h, int nbDoc) {
    assert(h);
    for (int i = 0; i < h->size; i++) {
        printf("-------------Bucket %d----------------\n", i);
        printListe(h->buckets[i], nbDoc);
    }
}

void print_fichier(Fichier *ff, int len) {
    for (int i = 0; i < len; i++) {
        printf("Fichier [%d] : Nom %s, nombre de mot %d\n", i, ff[i].nom,
               ff[i].nb_mots);
    }
}

char *getMot() {
    char *mot = malloc(sizeof(char) * 30);
    printf("Mot : ");
    scanf("%s", mot);
    return mot;
}

// le top 3 des documents les plus pertinents.
int minimum(float *tab, int len, float plgr) {
    int min = 0;
    for (int i = 0; i < len; i++) {
        if (tab[i] < tab[min] && tab[i] > plgr) min = i;
    }
    return min;
}
void top3(Hash *h, Fichier *mesFichiers, int nbDoc, char *mot) {
    int min = 0;
    float *tab = malloc(sizeof(float) * nbDoc);
    for (int i = 0; i < nbDoc; i++) {
        tab[i] = tf_idf(h, mesFichiers, nbDoc, mot, i);
    }
    min = minimum(tab, nbDoc, 0);

    printf("Top 3 des documents les plus pertinents pour le mot %s :\n", mot);
    for (int i = 0; i < 3; i++) {
        printf("Fichier [%d] : Nom %s, nombre de mot %d, tf_idf : %f\n", min,
               mesFichiers[min].nom, mesFichiers[min].nb_mots, tab[min]);
        min = minimum(tab, nbDoc, tab[min]);
    }
}

void clearHash(Hash *h, int nbDoc) {
    for (int i = 0; i < h->size; i++) {
        node *tmp = h->buckets[i]->tete;
        while (tmp != NULL) {
            free(tmp->nb_occurence);
            node *tmp2 = tmp;
            tmp = tmp->suiv;
            free(tmp2);
        }
        free(h->buckets[i]);
    }
    free(h->buckets);
    free(h);
}

/*
Objectif
Ce TP va vous permettre de mettre en pratique les algorithmes de construction et
de parcours d'un arbre binaire de recherche, en construisant une nouvelle
structure pour représenter le corpus. On souhaite maintenant pouvoir accéder
rapidement à des mots en fonction de leur nombre d'occurrences dans le corpus.
*/
/*
Arbres binaires de recherche

Dans un premier temps, on considérera de simples arbres binaires de recherche,
non équilibrés. Le principe est que à partir de chaque noeud, les éléments dont
la clé a une valeur plus petite que le noeud courant se trouvent dans le
sous-arbre gauche du noeud, et les éléments dont la clé a une valeur plus grande
sont dans le sous-arbre droit du noeud.

L'objectif ici est de construire la structure qui va vous permettre de
représenter votre arbre. Dans chaque noeud, la clé est un nombre d'occurrences
dans le corpus. On ajoute dans chaque noeud la liste (chaînée) de tous les mots
qui ont ce même nombre d'occurrences. On y trouve également un lien vers son
fils gauche et un lien vers son fils droit. Définissez votre structure d'arbre
binaire.
*/
typedef struct _NListe {
    char *mot;
    struct _NListe *suiv;
} NListe;
typedef struct _ListeABR {
    NListe *tete;
} ListeABR;

typedef struct noeud {
    int cle;
    ListeABR *liste;
    struct noeud *fg;
    struct noeud *fd;
} Noeud;

typedef Noeud *Arbre;

Noeud *initNoeud(int cle) {
    Noeud *n = malloc(sizeof(Noeud));
    n->cle = cle;
    n->liste = malloc(sizeof(ListeABR));
    n->liste->tete = NULL;
    n->fg = NULL;
    n->fd = NULL;
    return n;
}

//Écrivez la fonction d'insertion d'un élément dans l'arbre qui prend en
//paramètre un arbre, un mot, et un entier
void insererABR(Arbre *a, char *mot, int nb_occurence) {
    if (*a == NULL) {
        *a = initNoeud(nb_occurence);
        NListe *n = malloc(sizeof(NListe));
        n->mot = mot;
        n->suiv = NULL;
        (*a)->liste->tete = n;
    } else {
        if (nb_occurence < (*a)->cle)
            insererABR(&(*a)->fg, mot, nb_occurence);
        else if (nb_occurence > (*a)->cle)
            insererABR(&(*a)->fd, mot, nb_occurence);
        else {
            NListe *n = malloc(sizeof(NListe));
            n->mot = mot;
            n->suiv = (*a)->liste->tete;
            // sans l'odre alphabetique
            (*a)->liste->tete = n;
        }
    }
}

/*
Nous nous intéressons maintenant à la fonction creer-arbre.
Vous devez construire l'arbre à partir de votre table de hachage.
Pour chaque mot dans la table de hachage, calculez son nombre d'occurrences dans
le corpus (nombre d'occurrences dans tous les documents), puis insérez ce mot et
son nombre d'occurrences dans l'arbre. La fonction doit retourner l'arbre ainsi
créé.
*/
Arbre creer_arbre(Hash *h, Fichier *mesFichiers, int nbDoc) {
    Arbre a = NULL;
    for (int i = 0; i < h->size; i++) {
        node *tmp = h->buckets[i]->tete;
        while (tmp != NULL) {
            int nb_occurence = 0;
            for (int j = 0; j < nbDoc; j++) {
                nb_occurence += tmp->nb_occurence[j];
            }
            insererABR(&a, tmp->mot, nb_occurence);
            tmp = tmp->suiv;
        }
    }
    return a;
}

//Écrivez la fonction qui parcourt l'arbre et affiche tous les mots du corpus
//dans l'ordre de leur fréquence croissante.
void parcoursABR(Arbre a) {
    if (a != NULL) {
        parcoursABR(a->fg);
        NListe *tmp = a->liste->tete;
        printf("[%d] iteration(s) : ", a->cle);
        while (tmp != NULL) {
            printf("%s ", tmp->mot);
            tmp = tmp->suiv;
        }
        printf("\n");
        parcoursABR(a->fd);
    }
}

//Écrivez la fonction qui parcourt l'arbre et retourne la liste des mots les
//moins fréquents du corpus (on considérera les n nombres d'occurrences les plus
//petits, avec n donné en paramètre).
// Affichez cette liste. Faites de même avec les mots les plus fréquents du
// corpus.
void moinsFreqABR(Arbre a, int n) {
    if (a != NULL) {
        while (a->fg != NULL) a = a->fg;

        printf("plus petite frequence : %d\n", a->cle);
        NListe *tmp = a->liste->tete;
        while (tmp != NULL && n > 0) {
            printf("%s\n", tmp->mot);
            tmp = tmp->suiv;
            n--;
        }
    }
}

void clearABR(Arbre a) {
    if (a != NULL) {
        clearABR(a->fg);
        clearABR(a->fd);
        free(a->liste);
        free(a);
    }
}

int main(int argc, char **argv) {
    Hash *h = init_hash_table(10);
    char *mot;
    int nb_fichier = (argc - 1) / 2;
    printf("Nombre de fichier : %d\n", nb_fichier);

    Fichier *mesFichiers = malloc(nb_fichier * sizeof(Fichier));
    for (int i = 0; i < nb_fichier; i++) {
        mesFichiers[i].nom = argv[i + i + 1];
        mesFichiers[i].nb_mots = atoi(argv[i + i + 2]);
    }
    print_fichier(mesFichiers, nb_fichier);

    for (int i = 0; i < nb_fichier; i++) {
        FILE *f = ouvrir(mesFichiers[i].nom);
        while ((mot = lire_mot(f)) != NULL) {
            ajouter_hash(h, mot, nb_fichier, i);
        }
        fclose(f);
    }
    /*
    Teste les print
    print_hash(h, nb_fichier);
    printf("TF : %f\n", tf(h, mesFichiers, "NETUS", 1));
    printf("%f\n", idf(h, nb_fichier, "NETUS"));
    printf("%f\n", tf_idf(h, mesFichiers, nb_fichier, "VOLUTPAT", 0));

    mot = getMot();
    top3(h, mesFichiers, nb_fichier, mot);
    */

    Arbre a = creer_arbre(h, mesFichiers, nb_fichier);
    /* Teste les print de l'ABR
     */
    parcoursABR(a);
    printf("\n");
    moinsFreqABR(a, 3);
    printf("\n");

    // Pour pas avoir le petit chat qui danse
    clearABR(a);
    clearHash(h, nb_fichier);
    free(mesFichiers);
    free(mot);
    return 0;
}