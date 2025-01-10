#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt); // Sauvegarde des paramètres du terminal
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Mode non canonique, sans écho
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restauration des paramètres
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif

#define FB 5

typedef struct produit
{
    int id;
    char nomFICHIER[10];
    int supprimé;
} produit;

typedef struct Bloc Bloc;
struct Bloc
{
    produit t[FB];
    int nbenrigstrement;
};

// Structure pour représenter la mémoire secondaire
typedef struct
{
    Bloc *blocs; // Tableau de blocs
    int M;       // Nombre total de blocs
    int b;       // Facteur de blocage (capacité max d'un bloc)
} MS;

// Structure pour les métadonnées
typedef struct MetaDonnees
{
    char Nomfichier[10];
    int Tailleblocs;
    int tailleenreg;
    int Adressebloc;
    enum organisationGLOBAL Mglobale;
    enum organisationINTERNE Minterne;
    int supprime;
} MetaDonnees;

typedef struct TableAlloc
{
    int dispoStockage;     // Boolean pour 0 si plein, 1 si vide
    int nbrEnregistrement; // Nombre de produits
} TableAlloc;

enum organisationGLOBAL
{
    contegue, // 0 par défaut
    chainer   // 1 par défaut
};

enum organisationINTERNE
{
    trier,   // 0
    nontrier // 1
};

void MAJMeta(FILE *ms, char nomfichier[20], int v)
{
    char nom[20];
    Bloc buffer;
    MetaDonnees buffer1;

    ms = fopen("ms.dat", "rb+");
    while (fread(&buffer1, sizeof(MetaDonnees), 1, ms))
    {
        if (strcmp(buffer1.Nomfichier, nomfichier) == 0)
        {
            switch (v)
            {
            case 1:
                printf("choisir le nouveau nom du fichier \n");
                scanf("%s", &nom);
                strcpy(buffer1.Nomfichier, nom);
                fseek(ms, -sizeof(MetaDonnees), SEEK_CUR);
                fwrite(&buffer1, sizeof(MetaDonnees), 1, ms);
                break;
            case 2:
                printf("choisir le nouveau nombre d'enregistrement \n");
                scanf("%d", &buffer1.tailleenreg);
                fseek(ms, -sizeof(MetaDonnees), SEEK_CUR);
                fwrite(&buffer1, sizeof(MetaDonnees), 1, ms);

                break;
            case 3:
                printf("choisir le nouveau nombre de bloc \n");
                scanf("%d", &buffer1.Tailleblocs);
                fseek(ms, -sizeof(MetaDonnees), SEEK_CUR);
                fwrite(&buffer1, sizeof(MetaDonnees), 1, ms);
                break;
            case 4:
                printf("vous aller changer l'adresse du bloc  \n");
                scanf("%d", &buffer1.Adressebloc);
                fseek(ms, -sizeof(MetaDonnees), SEEK_CUR);
                fwrite(&buffer1, sizeof(MetaDonnees), 1, ms);
                break;
            case 5:
                printf("choisir de supprimer logiquement : 1 ou 0   \n");
                scanf("%d", &buffer1.supprime);
                if (buffer1.supprime == 1)
                {
                    printf("le fichier est supprimer logiquement \n");
                    fseek(ms, -sizeof(MetaDonnees), SEEK_CUR);
                    fwrite(&buffer1, sizeof(MetaDonnees), 1, ms);
                }
                else
                {
                    printf("le fichier n'est pas supprimer logiquement \n");
                    fseek(ms, -sizeof(MetaDonnees), SEEK_CUR);
                    fwrite(&buffer1, sizeof(MetaDonnees), 1, ms);
                }

                break;
            default:

                break;
            }
        }
        else
        {
            fread(&buffer, sizeof(Bloc), 1, ms);
        }
    }
    fclose(ms);
}

void creation(FILE *ms, FILE *fichier, struct TableAlloc *Tablealloc, enum organisationGLOBAL org, enum organisationINTERNE orgin, MetaDonnees meta)
{
    char nom[10];
    int nbenre;
    produit *buffer;

    fichier = fopen("fichier.dat", "ab+");
    if (fichier == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    printf("Le nom : \n");
    scanf("%s", nom);
    rename("fichier.dat", nom); // Renommer le fichier

    printf("Le nombre d'enregistrements : \n");
    scanf("%d", &nbenre);

    buffer = malloc(nbenre * sizeof(produit));
    if (buffer == NULL)
    {
        perror("Erreur d'allocation mémoire");
        fclose(fichier);
        return;
    }

    for (int i = 0; i < nbenre; i++)
    {
        printf("Produit %d : \n", i + 1);
        printf("ID : ");
        scanf("%d", &buffer[i].id);

        strcpy(buffer[i].nomFICHIER, nom); // Nom du fichier

        buffer[i].supprimé = 0; // Par défaut, non supprimé
    }

    // Mise à jour des métadonnées
    strcpy(meta.Nomfichier, nom);
    meta.tailleenreg = nbenre;
    meta.Tailleblocs = ceil((double)nbenre / FB);
    meta.Mglobale = org;
    meta.Minterne = orgin;

    fwrite(&meta, sizeof(MetaDonnees), 1, fichier);
    fwrite(buffer, sizeof(produit), nbenre, fichier);

    int combined = org * 2 + orgin;
    switch (combined)
    {
    case 0:                                 // org == 0 && orgin == 0
        insertion_trier(ms, fichier, meta); // Fonction à implémenter
        break;
    case 1: // org == 0 && orgin == 1
        insertion_contigue(ms, fichier, Tablealloc, meta.Tailleblocs);
        break;
    default:
        printf("Organisation non supportée.\n");
        break;
    }

    free(buffer);
    fclose(fichier);
}

int lireCaracteristique(FILE *f, int nc)
{
    rewind(f);
    MetaDonnees caracteristique;
    fread(&caracteristique, sizeof(MetaDonnees), 1, f);
    switch (nc)
    {
    case 1:
        return caracteristique.Tailleblocs;
        break;
    case 2:
        return caracteristique.tailleenreg;
        break;
    case 3:
        return caracteristique.Adressebloc;
        break;
    default:
        printf("");
        break;
    }
}

void rehcrehceCONTIGUE(FILE *MS, int id, char filename[10], int *indice, int *numbloc)
{

    Bloc buffer;
    int trouver;
    trouver = 0;
    // si le fichier n'est pas trier ou trier
    // recherche sequentielle
    MS = fopen("memoireSECONDAIRE.dat", "rb");
    *numbloc = 0;
    // lecture du premier block
    while (fread(&buffer, sizeof(Bloc), 1, MS))
    {
        *numbloc++;
        // lecture des enregistrement avec la boucle for
        for (int i = 0; i < buffer.nbenrigstrement; i++)
        {

            // je teste si je suis arriver au fichier
            if (buffer.t->nomFICHIER == filename)
            {

                // si je suis arriver au fichier donc je vais tester l'id du produit
                if (buffer.t->id == id)
                {
                    trouver = 1;
                    *indice = i;
                    printf("l'enregistrement se trouve dans le block num %d a la position num %d", *numbloc, *indice);
                    return;
                }
            }
        }
    }
    if (trouver == 0)
    {
        printf("le produit que vous cherchez n'existe pas dans le fichier");
    }
}

int insertion_contigue(FILE *Ms, FILE *f, struct TableAlloc *Tablealloc, double maxbloc)
{

    int i, cmpt = 0;
    int nbrBlocNecessaireF = lireCaracteristique(f, 1);
    int nbrBlocLibreContigue = 0;
    int adressEspaceSufF = -1;

    // verif si il ya assez de place dans la MS pour acceuillir ce fichier
    for (i = 0; i < maxbloc; i++)
    {

        if (Tablealloc[i].dispoStockage == 1)
        {
            cmpt++;
            if (nbrBlocNecessaireF == cmpt)
            {                                                  // ida le compteur yalha9 le nombre de bloc li kayen dakhel f alors ya assez de place en contigue
                adressEspaceSufF = i - nbrBlocNecessaireF + 1; // trouve l'adress ou il y a assez de bloc contigue pour caller le ficher
            }
        }
        else
        {
            cmpt = 0;
        }
    }
    if (adressEspaceSufF == -1)
    {
        printf("il n ya pas assez de place pour ce fichier");
        return -1;
    }
    else
    {
        // le cas ou il y a de la place pour notre fichier f
        rewind(Ms);
        rewind(f);
        Bloc buffer;
        produit p;
        fseek(Ms, adressEspaceSufF * sizeof(buffer), SEEK_SET); // ce dirige vers le bloc visée

        for (i = 0; i < nbrBlocNecessaireF; i++)
        { // ajoute les bloc dans le Ms avec le buffer a partir de fichier
            fread(&buffer, sizeof(buffer), 1, f);
            fwrite(&buffer, sizeof(buffer), 1, Ms);
        }
    }

    printf("le fichier est stocker dans la MEMOIRE SECONDAIRE");
    return 0;
}

void initMs(FILE *ms)
{
    rewind(ms);
    ms = fopen("memoiresecondaire.dat", "rb+");
}

void insertenregistrement_nontrier(char filename[10], int id)
{
    // table d'allocation
}

void suppression_Logique_Contigue(FILE *MS, char file_name[20], int id)
{
    Bloc buffer;
    if (!MS)
    {
        printf("Erreur : Impossible d'ouvrir le fichier %s.\n", file_name);
        return 0;
    }
    int position_bloc = 0;
    while (fread(&buffer, sizeof(Bloc), 1, MS))
    {
        for (int i = 0; i < FB; i++)
        {
            if (buffer.t[i].id == id && buffer.t[i].nomFICHIER == file_name && !buffer.t[i].supprimé)
            {
                buffer.t[i].supprimé = 1;
                fseek(MS, -1 * sizeof(Bloc), SEEK_CUR);
                fwrite(&buffer, sizeof(buffer), 1, MS);
                printf("Étudiant supprimé logiquement.\n");
            }
            else
                printf("Cet etudiant n'existe pas dans le fichier!");
        }
    }
}

void suppression_Physique_Contigue(FILE *MS, char file_name[20], int id)
{
    FILE *Ms = fopen(file_name, "rb"); // Ouvrir le fichier en mode lecture
    if (!MS)
    {
        printf("Erreur : Impossible d'ouvrir le fichier %s.\n", file_name);
        return 0;
    }
    FILE *temp = fopen("temp.dat", "wb");
    if (temp == NULL)
    {
        printf("Erreur : Impossible de créer un fichier temporaire.\n");
        fclose(Ms);
        return 0;
    }

    Bloc buffer;
    Bloc buffer1;

    while (fread(&buffer, sizeof(buffer), 1, Ms))
    {
        buffer1.nbenrigstrement = 0;
        int j = 0;
        for (int i = 0; i < FB; i++)
        {

            if (buffer.t[i].id != id)
            {
                buffer1.t[j] = buffer.t[i];
                j++;
                buffer1.nbenrigstrement++;
            }
        }
        fwrite(&buffer1, sizeof(buffer1), 1, temp);
    }
    fclose(Ms);
    fclose(temp);
    remove("MEMOIRESECONDAIRE.dat");
    rename("temp.dat", "mémoire_secondaire.dat");
    printf("Produit supprimé physiquement.\n");
}

// comactage
void compactage_contigue()
{
}

// Fonction pour créer et initialiser la table d'allocation
void creationTableAlloc(FILE *ms, TableAlloc **tableAlloc)
{
    // Rewind pour repositionner le pointeur du fichier au début
    rewind(ms);

    // On pose le nombre de blocs max (20 ici comme exemple)
    int i = 0;
    int maxBlocs = 20;

    // Allocation dynamique pour la table d'allocation
    *tableAlloc = (TableAlloc *)malloc(maxBlocs * sizeof(TableAlloc));
    if (*tableAlloc == NULL)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour la table d'allocation\n");
        exit(1);
    }

    // Initialisation du premier bloc (Table d'allocation elle-même)
    (*tableAlloc)[0].dispoStockage = 1;
    (*tableAlloc)[0].nbrEnregistrement = 0;

    // Initialisation des blocs restants
    for (i = 1; i < maxBlocs; i++)
    {
        (*tableAlloc)[i].dispoStockage = 0;
        (*tableAlloc)[i].nbrEnregistrement = 0;
    }

    // Écriture de la table d'allocation dans le fichier
    if (fwrite(*tableAlloc, sizeof(TableAlloc), maxBlocs, ms) != maxBlocs)
    {
        fprintf(stderr, "Erreur lors de l'écriture dans le fichier\n");
        free(*tableAlloc);
        exit(1);
    }
}

void compactage_contigue(FILE *ms, TableAlloc *tableAlloc, int maxBloc)
{
    Bloc buffer, tempBuffer;
    int writePos = 1; // Position d'écriture (en sautant la table d'allocation)
    int readPos;

    // Rewind pour repositionner le fichier au début
    rewind(ms);

    for (readPos = 1; readPos < maxBloc; readPos++)
    {
        fseek(ms, readPos * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, ms);

        if (buffer.nbenrigstrement > 0)
        {
            fseek(ms, writePos * sizeof(Bloc), SEEK_SET);
            fwrite(&buffer, sizeof(Bloc), 1, ms);
            tableAlloc[writePos].dispoStockage = 1; // Bloc rempli
            tableAlloc[readPos].dispoStockage = 0;  // Bloc vidé
            writePos++;
        }
    }

    printf("Compactage terminé : %d blocs remplis.\n", writePos - 1);
}

void defragmentation(FILE *ms, TableAlloc *tableAlloc, int maxBloc)
{
    Bloc buffer;
    int writePos = 1; // Position pour écrire les données défragmentées

    // Rewind pour repositionner le fichier au début
    rewind(ms);

    for (int readPos = 1; readPos < maxBloc; readPos++)
    {
        fseek(ms, readPos * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, ms);

        if (buffer.nbenrigstrement > 0)
        {
            // Déplace les enregistrements d'un bloc fragmenté
            for (int i = 0; i < buffer.nbenrigstrement; i++)
            {
                if (!buffer.t[i].supprimé)
                { // Si l'enregistrement n'est pas supprimé
                    fseek(ms, writePos * sizeof(Bloc), SEEK_SET);
                    fwrite(&buffer.t[i], sizeof(produit), 1, ms);
                }
            }
            tableAlloc[readPos].dispoStockage = 0; // Marque le bloc comme vide
        }
    }

    printf("Défragmentation terminée.\n");
}

void MajTablealloc(FILE *ms, struct TableAlloc *Tablealloc)
{
    int s;               // Compteur pour les suppressions
    Bloc buffer;         // Structure représentant un bloc
    MetaDonnees buffer1; // Métadonnées associées
    rewind(ms);

    // Aller au début des blocs après la TableAlloc et les MetaDonnees
    fseek(ms, sizeof(struct TableAlloc), SEEK_SET);
    fseek(ms, sizeof(MetaDonnees), SEEK_CUR);

    // Parcourir tous les blocs dans le fichier
    while (fread(&buffer, sizeof(Bloc), 1, ms))
    {
        s = 0; // Initialisation du compteur de suppressions

        // Parcourir les enregistrements dans le bloc
        for (size_t j = 0; j < FB; j++)
        {
            if (buffer.t[j].supprimé == 1)
            {
                s++; // Compter les enregistrements supprimés
            }
        }

        // Mise à jour des informations de la table d'allocation
        size_t i = buffer1.Adressebloc; // Adresse ou index du bloc dans la table
        if (s == FB)
        {                                    // Si tous les enregistrements sont supprimés
            Tablealloc[i].dispoStockage = 1; // Le bloc est marqué comme disponible
        }
        else
        {
            Tablealloc[i].dispoStockage = 0; // Le bloc reste occupé
        }
        Tablealloc[i].nbrEnregistrement = FB - s; // Nombre d'enregistrements restants

        // Avancer au prochain bloc en sautant les MetaDonnees
        fseek(ms, sizeof(MetaDonnees), SEEK_CUR);
    }

    // Réécrire la TableAlloc mise à jour au début du fichier
    rewind(ms);
    fwrite(Tablealloc, sizeof(struct TableAlloc), 20, ms); // Écrire 20 blocs
}

void afficherMemoireRectangle(FILE *ms)
{
    rewind(ms);
    Bloc buffer;

    // Positionner après la table d'allocation
    fseek(ms, sizeof(struct TableAlloc), SEEK_SET);

    printf("\n======================== Mémoire secondaire =======================================\n");

    for (size_t i = 0; i < 20; i++)
    {
        printf("Bloc %zu : ", i);

        // Positionnement sur le bloc actuel
        fseek(ms, sizeof(struct TableAlloc) + sizeof(MetaDonnees) + i * sizeof(Bloc), SEEK_SET);

        // Lecture du bloc
        if (fread(&buffer, sizeof(Bloc), 1, ms) == 1)
        {
            for (size_t j = 0; j < FB; j++)
            {
                // Texte en vert
                printf("\033[32m%d \033[0m", buffer.t[j].id);
            }
            printf("\n");
        }
        else
        {
            // Afficher une erreur en couleur neutre
            printf("Erreur de lecture ou bloc vide\n");
        }
    }

    printf("======================================================================================\n");
}
//===================================================================================================================================================================
// les fonction du mose chaine
#define FB 5          // Facteur de blocage: Nombre maximum d'enregistrements par bloc
#define MAX_BLOCKS 15 // Nombre maximum de blocs

// Déclarations des structures
typedef struct Produit Produit;
typedef struct Block Block;
typedef struct MetaDonneess MetaDonneess;
typedef struct AllocationEntry AllocationEntry;

typedef enum
{
    contigue, // Organisation contiguë
    chainee   // Organisation chaînée
} organisationGLOBAL;

typedef enum
{
    trier,   // Organisation triée
    nontrier // Organisation non triée
} organisationINTERNE;

struct Produit
{
    int id;
    char nomFICHIER[20];
};

struct Block
{
    Produit t[FB];
    int nbenregistrement;
    int nbrBlock;
    Block *prochainBloc;
};

struct MetaDonneess
{
    char Nomfichier[10];
    int Tailleblocs;
    int tailleenreg;
    Block *AdressePremierBloc;
    organisationGLOBAL Mglobale;
    organisationINTERNE Minterne;
};

struct AllocationEntry
{
    int isFree;    // 1 si libre, 0 si occupé
    int nextBlock; // Pointeur vers le bloc suivant (ou -1 pour aucun)
};

// Table d'allocation
AllocationEntry allocationTable[MAX_BLOCKS];
MetaDonneess fichiersMeta[MAX_BLOCKS];
int fichierMetaCount = 0; // Nombre de fichiers créés

// Initialisation de la table d'allocation
void initializeAllocationTable()
{
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        allocationTable[i].isFree = 1;
        allocationTable[i].nextBlock = -1;
    }
    fichierMetaCount = 0;
    printf("Memoire secondaire initialisee.\n");
}

// Allocation d'un bloc
Block *allocateBlock(int *allocatedIndex)
{
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        if (allocationTable[i].isFree)
        {
            allocationTable[i].isFree = 0;
            *allocatedIndex = i;
            Block *newBlock = (Block *)malloc(sizeof(Block)); // Dynamically allocate a block
            if (!newBlock)
            {
                perror("Erreur d'allocation de memoire.");
                exit(EXIT_FAILURE);
            }
            return newBlock; // Return the dynamically allocated block
        }
    }
    perror("Aucun bloc libre disponible.");
    exit(EXIT_FAILURE);
}

// Création d'un fichier en mode chaîne
void creerFichierChaine(MetaDonneess *meta, int n)
{
    printf("Entrez le nom du fichier : ");
    scanf("%s", meta->Nomfichier);

    printf("Organisation globale (0: contigue, 1: chainee) : ");
    int choixGlobale;
    scanf("%d", &choixGlobale);
    meta->Mglobale = choixGlobale == 1 ? chainee : contigue;

    printf("Organisation interne (0: triee, 1: non triee) : ");
    int choixInterne;
    scanf("%d", &choixInterne);
    meta->Minterne = choixInterne == 1 ? nontrier : trier;

    meta->tailleenreg = n;
    meta->Tailleblocs = (n + FB - 1) / FB; // Calcul du nombre de blocs nécessaires

    Block *premierBloc = NULL;
    Block *blocActuel = NULL;
    int prevIndex = -1; // Index du bloc précédent dans la table d'allocation

    for (int i = 0; i < meta->Tailleblocs; i++)
    {
        // Allouer un nouveau bloc
        Block *nouveauBloc = (Block *)malloc(sizeof(Block));
        if (!nouveauBloc)
        {
            perror("Erreur d'allocation de memoire.");
            exit(EXIT_FAILURE);
        }

        // Initialiser les enregistrements dans le bloc
        nouveauBloc->nbenregistrement = 0;
        nouveauBloc->prochainBloc = NULL;

        // Remplir le bloc avec des produits
        for (int j = 0; j < FB && n > 0; j++, n--)
        {
            printf("Produit %d - ID : ", meta->tailleenreg - n + 1);
            scanf("%d", &nouveauBloc->t[j].id);
            printf("Produit %d - Nom : ", meta->tailleenreg - n + 1);
            scanf("%s", nouveauBloc->t[j].nomFICHIER);
            nouveauBloc->nbenregistrement++;
        }

        // Lier les blocs
        if (!premierBloc)
        {
            premierBloc = nouveauBloc; // Premier bloc dans la chaîne
        }
        else
        {
            blocActuel->prochainBloc = nouveauBloc; // Relier le précédent au nouveau
        }
        blocActuel = nouveauBloc;
        // Mettre à jour la table d'allocation
        for (int j = 0; j < MAX_BLOCKS; j++)
        {
            if (allocationTable[j].isFree)
            {
                allocationTable[j].isFree = 0; // Marquer comme occupé
                if (prevIndex != -1)
                {
                    allocationTable[prevIndex].nextBlock = j; // Relier au précédent
                }
                prevIndex = j; // Mettre à jour le précédent
                break;
            }
        }
    }

    // Terminer la chaîne
    if (prevIndex != -1)
    {
        allocationTable[prevIndex].nextBlock = -1; // Dernier bloc
    }

    meta->AdressePremierBloc = premierBloc;
    fichiersMeta[fichierMetaCount++] = *meta; // Assignation du pointeur directement.


    printf("Fichier cree avec succes.\n");
}

// Affichage de l'état de la mémoire
void afficherEtatMemoire()
{
    printf("Etat de la memoire secondaire:\n");
    printf("Index\t\tLibre\tSuivant\n");

    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        printf("%d\t\t%d\t%d\n", i, allocationTable[i].isFree, allocationTable[i].nextBlock);
    }
}

// Affichage des métadonnées des fichiers
void afficherMetadonnees()
{
    printf("\n=== Metadonnees des fichiers ===\n");
    printf("| Nom       | Blocs | Enregistrements | Organisation Globale | Organisation Interne |\n");
    printf("------------------------------------------------------------------------------------\n");
    for (int i = 0; i < fichierMetaCount; i++)
    {
        printf("| %-9s | %-5d | %-16d | %-19s | %-17s |\n",
               fichiersMeta[i].Nomfichier,
               fichiersMeta[i].Tailleblocs,
               fichiersMeta[i].tailleenreg,
               fichiersMeta[i].Mglobale == chainee ? "Chainee" : "Contigue",
               fichiersMeta[i].Minterne == trier ? "Triee" : "Non triee");
    }
}

// Fonction de recherche d'un enregistrement en mode "chaîne"

void rechercherEnregistrementChaine(MetaDonnees *meta, const char *nomFichier, int id)
{
    FILE *fichierMemoire;
    fichierMemoire = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode binaire

    if (!fichierMemoire)
    {
        printf("Erreur d'ouverture du fichier %s\n", nomFichier);
        return; // Quitter la fonction si le fichier ne peut pas être ouvert
    }

    Produit produit;
    int found = 0;

    // Si les enregistrements sont triés
    if (meta->Minterne == 0)
    {
        // Lecture des enregistrements dans l'ordre trié
        while (fread(&produit, sizeof(Produit), 1, fichierMemoire))
        {
            if (produit.id == id)
            {
                printf("Enregistrement trouve dans fichier trie:\n");
                printf("ID: %d, Nom: %s\n", produit.id, produit.nomFICHIER);
                found = 1;
                break; // Arrêter la recherche dès que l'enregistrement est trouvé
            }
        }
    }
    // Si les enregistrements ne sont pas triés
    else
    {
        // Lecture des enregistrements dans un fichier non trié
        while (fread(&produit, sizeof(Produit), 1, fichierMemoire))
        {
            if (produit.id == id)
            {
                printf("Enregistrement trouve dans fichier non trie:\n");
                printf("ID: %d, Nom: %s\n", produit.id, produit.nomFICHIER);
                found = 1;
                break; // Arrêter la recherche dès que l'enregistrement est trouvé
            }
        }
    }

    if (!found)
    {
        printf("Enregistrement avec ID %d introuvable dans %s.\n", id, nomFichier);
    }

    fclose(fichierMemoire); // Fermer le fichier après la lecture
}

// Mise à jour de la table d'allocation après l'insertion
void updateAllocationTableAfterInsertion(int blockNumber)
{
    if (blockNumber >= 0 && blockNumber < MAX_BLOCKS)
    {
        allocationTable[blockNumber].isFree = 0;     // Le bloc est maintenant occupé
        allocationTable[blockNumber].nextBlock = -1; // Pas de lien vers un autre bloc
        printf("Bloc %d marque comme occupe dans la table d'allocation.\n", blockNumber);
    }
}
// Fonction d'insertion dans un fichier non trié
void insertionProduitNonTrie(FILE *ms, Produit nouveauProduit)
{
    MetaDonneess meta;
    meta.Tailleblocs = 0;
    meta.tailleenreg = 0;
    meta.AdressePremierBloc = NULL;
    meta.Mglobale = chainee;
    meta.Minterne = nontrier;

    Block *current = meta.AdressePremierBloc;
    Block buffer;
    int produitInsere = 0;

    // Recherche du bon bloc pour l'insertion
    while (current != NULL)
    {
        fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        if (buffer.nbenregistrement < FB)
        {
            // Ajouter le produit dans le bloc trouvé
            buffer.t[buffer.nbenregistrement] = nouveauProduit;
            buffer.nbenregistrement++;

            // Sauvegarder le bloc mis à jour
            fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
            fwrite(&buffer, sizeof(Block), 1, ms);
            produitInsere = 1;
            break;
        }

        current = current->prochainBloc;
    }

    // Si aucun bloc n'a d'espace, créer un nouveau bloc
    if (!produitInsere)
    {
        Block *nouveauBloc = (Block *)malloc(sizeof(Block));
        if (!nouveauBloc)
        {
            perror("Erreur d'allocation memoire pour un nouveau bloc");
            exit(EXIT_FAILURE);
        }
        // Ajouter le produit dans le nouveau bloc
        nouveauBloc->t[0] = nouveauProduit;
        nouveauBloc->nbenregistrement = 1;
        nouveauBloc->nbrBlock = meta.Tailleblocs; // Nouveau numéro de bloc
        nouveauBloc->prochainBloc = NULL;

        // Chaîner ce nouveau bloc à la fin de la liste existante
        if (meta.AdressePremierBloc == NULL)
        {
            meta.AdressePremierBloc = nouveauBloc; // Premier bloc
        }
        else
        {
            Block *last = meta.AdressePremierBloc;
            while (last->prochainBloc != NULL)
            {
                last = last->prochainBloc;
            }
            last->prochainBloc = nouveauBloc; // Ajouter à la fin de la chaîne
        }

        // Sauvegarder le nouveau bloc dans le fichier
        fseek(ms, nouveauBloc->nbrBlock * sizeof(Block), SEEK_SET);
        fwrite(nouveauBloc, sizeof(Block), 1, ms);

        // Mise à jour des métadonnées
        meta.Tailleblocs++;

        // Mise à jour de la table d'allocation après l'insertion du nouveau bloc
        updateAllocationTableAfterInsertion(nouveauBloc->nbrBlock);
    }

    printf("Produit insere avec succes dans un bloc non trie.\n");
}

// Fonction d'insertion dans un fichier trié
void insertionProduitTrie(FILE *ms, Produit nouveauProduit)
{
    MetaDonneess meta;
    meta.Tailleblocs = 0;
    meta.tailleenreg = 0;
    meta.AdressePremierBloc = NULL;
    meta.Mglobale = chainee;
    meta.Minterne = trier;

    Block *current = meta.AdressePremierBloc;
    Block buffer;
    int produitInsere = 0;

    while (current != NULL)
    {
        fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        // Insertion dans un bloc trié
        for (int i = 0; i < buffer.nbenregistrement; i++)
        {
            if (nouveauProduit.id < buffer.t[i].id)
            {
                // Décaler les éléments pour insérer le produit
                for (int j = buffer.nbenregistrement; j > i; j--)
                {
                    buffer.t[j] = buffer.t[j - 1];
                }
                buffer.t[i] = nouveauProduit;
                buffer.nbenregistrement++;

                // Sauvegarder le bloc mis à jour
                fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
                fwrite(&buffer, sizeof(Block), 1, ms);
                produitInsere = 1;
                break;
            }
        }

        if (produitInsere)
            break;

        current = current->prochainBloc;
    }
    // Si aucun bloc n'a d'espace ou pas de bon endroit, créer un nouveau bloc
    if (!produitInsere)
    {
        Block *nouveauBloc = (Block *)malloc(sizeof(Block));
        if (!nouveauBloc)
        {
            perror("Erreur d'allocation memoire pour un nouveau bloc");
            exit(EXIT_FAILURE);
        }

        // Ajouter le produit dans le nouveau bloc, trié
        nouveauBloc->t[0] = nouveauProduit;
        nouveauBloc->nbenregistrement = 1;
        nouveauBloc->nbrBlock = meta.Tailleblocs; // Nouveau numéro de bloc
        nouveauBloc->prochainBloc = NULL;

        // Chaîner ce nouveau bloc à la fin de la liste existante
        if (meta.AdressePremierBloc == NULL)
        {
            meta.AdressePremierBloc = nouveauBloc; // Premier bloc
        }
        else
        {
            Block *last = meta.AdressePremierBloc;
            while (last->prochainBloc != NULL)
            {
                last = last->prochainBloc;
            }
            last->prochainBloc = nouveauBloc; // Ajouter à la fin de la chaîne
        }

        // Sauvegarder le nouveau bloc dans le fichier
        fseek(ms, nouveauBloc->nbrBlock * sizeof(Block), SEEK_SET);
        fwrite(nouveauBloc, sizeof(Block), 1, ms);

        // Mise à jour des métadonnées
        meta.Tailleblocs++;

        // Mise à jour de la table d'allocation après l'insertion du nouveau bloc
        updateAllocationTableAfterInsertion(nouveauBloc->nbrBlock);
    }

    printf("Produit insere avec succes dans un bloc trie.\n");
}

// Mise à jour de la table d'allocation après suppression logique
void updateAllocationTableLogical(int blockNumber, int recordIndex)
{
    printf("Bloc %d, enregistrement %d marque comme supprimé dans la table d'allocation.\n", blockNumber, recordIndex);
}

// Mise à jour de la table d'allocation après suppression physique
void updateAllocationTablePhysical(int blockNumber)
{
    if (allocationTable[blockNumber].isFree == 0)
    {
        allocationTable[blockNumber].isFree = 1; // Le bloc est maintenant libre
        allocationTable[blockNumber].nextBlock = -1;
        printf("Bloc %d marque comme libre dans la table d'allocation.\n", blockNumber);
    }
}

void SuppressionLogique(FILE *ms, MetaDonneess *meta, int id)
{
    Block buffer;
    int found = 0;
    Block *current = meta->AdressePremierBloc;

    while (current != NULL && !found)
    {
        // Lire le bloc courant depuis le fichier
        fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        // Rechercher l'ID dans le bloc
        for (int i = 0; i < buffer.nbenregistrement; i++)
        {
            if (buffer.t[i].id == id)
            {
                buffer.t[i].id = -1; // Marquer comme supprimé
                found = 1;

                // Écrire le bloc modifié dans le fichier
                fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
                fwrite(&buffer, sizeof(Block), 1, ms);

                updateAllocationTableLogical(current->nbrBlock, i);
                break;
            }
        }

        // Passer au bloc suivant (via la table d'allocation)
        if (allocationTable[current->nbrBlock].nextBlock != -1)
        {
            current->nbrBlock = allocationTable[current->nbrBlock].nextBlock;
        }
        else
        {
            current = NULL;
        }
    }

    if (!found)
    {
        printf("Produit avec ID %d non trouve.\n", id);
    }
    else
    {
        printf("Produit avec ID %d marque comme supprime.\n", id);
    }
}

void suppressionPhysique(FILE *ms, MetaDonneess *meta, int id)
{
    Block buffer;
    int found = 0;
    Block *current = meta->AdressePremierBloc;

    while (current != NULL && !found)
    {
        // Lire le bloc courant depuis le fichier
        fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        // Rechercher l'ID dans le bloc
        for (int i = 0; i < buffer.nbenregistrement; i++)
        {
            if (buffer.t[i].id == id)
            {
                found = 1;
                // Décaler les enregistrements pour combler le vide
                for (int j = i; j < buffer.nbenregistrement - 1; j++)
                {
                    buffer.t[j] = buffer.t[j + 1];
                }
                buffer.nbenregistrement--;
                // Si le bloc est vide, le libérer
                if (buffer.nbenregistrement == 0)
                {
                    updateAllocationTablePhysical(current->nbrBlock);
                }

                // Écrire le bloc modifié dans le fichier
                fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
                fwrite(&buffer, sizeof(Block), 1, ms);

                break;
            }
        }

        // Passer au bloc suivant (via la table d'allocation)
        if (allocationTable[current->nbrBlock].nextBlock != -1)
        {
            current->nbrBlock = allocationTable[current->nbrBlock].nextBlock;
        }
        else
        {
            current = NULL;
        }
    }

    if (!found)
    {
        printf("Produit avec ID %d non trouve.\n", id);
    }
    else
    {
        printf("Produit avec ID %d supprime physiquement.\n", id);
    }
}

// defragmentation ici

void supprimerFichier(const char *filename, const char *fileToDelete, MetaDonneess *meta)
{
    FILE *ms = fopen(filename, "r+b");
    if (!ms)
    {
        perror("Erreur lors de l'ouverture du fichier binaire");
        return;
    }

    // Étape 1 : Vérifier si le fichier existe dans les métadonnées
    Block buffer;
    Block *prev = NULL;
    Block *current = meta->AdressePremierBloc;
    int found = 0;

    while (current != NULL)
    {
        // Lire le bloc courant depuis le fichier
        fseek(ms, current->nbrBlock * sizeof(Block), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        // Vérifier si le bloc appartient au fichier à supprimer
        if (strcmp(buffer.t[0].nomFICHIER, fileToDelete) == 0)
        {
            found = 1;

            // Étape 2 : Libérer les blocs dans la table d'allocation
            printf("Suppression des blocs occupes par le fichier '%s'.\n", fileToDelete);
            Block *temp = current;
            while (temp != NULL)
            {
                fseek(ms, temp->nbrBlock * sizeof(Block), SEEK_SET);
                fread(&buffer, sizeof(Block), 1, ms);

                allocationTable[buffer.nbrBlock].isFree = 1; // Marquer le bloc comme libre
                allocationTable[buffer.nbrBlock].nextBlock = -1;

                printf("Bloc %d libere.\n", buffer.nbrBlock);

                temp = buffer.prochainBloc;
            }

            // Étape 3 : Mise à jour des pointeurs dans les métadonnées
            if (prev == NULL)
            {
                meta->AdressePremierBloc = current->prochainBloc;
            }
            else
            {
                prev->prochainBloc = current->prochainBloc;
            }

            // Libération de la mémoire associée au bloc courant
            free(current);
            break;
        }

        prev = current;
        current = current->prochainBloc;
    }

    // Étape 4 : Mise à jour des métadonnées dans le fichier
    if (found)
    {
        printf("Fichier '%s' supprime avec succes.\n", fileToDelete);
        fseek(ms, 0, SEEK_SET); // Revenir au début pour écrire les métadonnées mises à jour
        fwrite(meta, sizeof(MetaDonnees), 1, ms);
    }
    else
    {
        printf("Fichier '%s' introuvable.\n", fileToDelete);
    }

    fclose(ms);
}

void renommerFichier(const char *ancienNom, const char *nouveauNom)
{
    // Appeler la fonction rename() pour renommer le fichier
    if (rename(ancienNom, nouveauNom) == 0)
    {
        printf("Le fichier a ete renommé avec succes de %s a %s.\n", ancienNom, nouveauNom);
    }
    else
    {
        perror("Erreur lors du renommage du fichier");
    }
}

// compactage

void viderMemoireSecondaire()
{
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        allocationTable[i].isFree = 1;
        allocationTable[i].nextBlock = -1;
    }
    fichierMetaCount = 0;
    printf("Memoire secondaire vide.\n");
}
int main()
{

    FILE *ms;
    FILE *fichier;
    struct MetaDonnees meta;
    struct TableAlloc *TableAlloc;
    int choice, id, choixsup;
    char filename[10];
    char input;
    int *indice;
    int *numbloc;
    char nomfochier[10];
    int v;
    int chcn;
    bool running = true;
    int totalBlocks = MAX_BLOCKS;
    MetaDonnees meta;
    char filename[50], nomFichier[20];
    FILE *fichierMemoire;
    int choix, n, id;
    Produit nouveauProduit;

    // initialisation de la memoire secondaire
    initMs(ms);
    printf("Bonjour cher utilisateur bienvenue dans une similation d'un SGF \n");
    creation(ms, fichier, TableAlloc, 0, 1, meta);
    while (running)
    {
        printf("==>chooisissez le mode d'organisation :");
        printf("entrer 1 si vous voulez  avoire un mode d'organisation contigue");
        printf("entrer 2 si vous voulez  avoire un mode d'organisation chaine");
        scanf("%d", &chcn);
        switch (chcn)
        {
        case 1:

            printf("==> choisissez l'action que vous voulez effectuer :\n");
            printf("==> tapez 1 si vous voulez isserez un enregistrement \n");
            printf("==> tapez 2 si vous vouler suprimer un enregistrement \n");
            printf("==> tapez 3 si vus voulez faire un compactage \n");
            printf("==> tapez 4 si vus voulez faire un une defragmentation \n");
            printf("==> tapez 5 si vous voulez faire une recherche");
            printf(" ==> tapez 6 si vous voulez renomer n'importe quel fichier");

            printf("==> inserer une valeure : ");
            scanf("%d", &choice);

            switch (choice)
            {
            case 1:
                printf("donnez l'id du produit a insserer ainsi que le nom du fichier");
                scanf("%d", &id);
                scanf("%s", &filename);
                insertenregistrement_nontrier(filename, id);
                v = 2;
                MAJMeta(ms, filename, v);
                v = 3;
                MAJMeta(ms, filename, v);
                afficherMemoireRectangle(ms);
                break;
            case 2:
                printf("==> tapez 1 si vous voulez une supression logique \n");
                printf("==> tapez 2 si vous voulez une supression physique \n");
                printf("==> inserer une valeure : ");
                scanf("%d", &choixsup);
                switch (choixsup)
                {
                case 1:
                    printf("donnez l'id du produit a suprimer ainsi que le nom du fichier \n");
                    scanf("%d", &id);
                    scanf("%s", &filename);
                    suppression_Logique_Contigue(ms, filename, id);
                    v = 5;
                    MAJMeta(ms, filename, v);
                    afficherMemoireRectangle(ms);
                    break;
                case 2:
                    printf("donnez l'id du produit a suprimer ainsi que le nom du fichier \n");
                    scanf("%d", &id);
                    scanf("%s", &filename);
                    suppression_Physique_Contigue(ms, filename, id);
                    afficherMemoireRectangle(ms);
                    break;
                default:
                    break;
                }
                break;
            case 3:
                compactage_contigue(ms, TableAlloc, 20);
                afficherMemoireRectangle(ms);
                break;
            case 4:
                defragmentation(ms, TableAlloc, 20);
                afficherMemoireRectangle(ms);
                break;
            case 5:
                printf("donnez l'id du produit ainsi que le fichier la ou il se retrouve");
                scanf("%d", &id);
                scanf("%s", &filename);
                rehcrehceCONTIGUE(ms, filename, id, indice, numbloc);
                printf("l'enregistrement se trouve au block %d et a la position %d", *numbloc, *indice);
                afficherMemoireRectangle(ms);
                break;
            case 6:
                printf("entrer l'encien nom du fichier :");
                scanf("%s", &filename);
                printf("entrer le nouveaux nom du fichier :");
                scanf("%s", &nomfochier);
                rename(filename, nomfochier);
                afficherMemoireRectangle(ms);
                break;
            default:
                break;
            }
            if (kbhit())
            {                      // Vérifie si une touche est pressée
                input = getchar(); // Récupère la touche
                if (input == 'q')
                {                    // Si l'utilisateur appuie sur 'q'
                    running = false; // Arrête la boucle
                }
            }

            break;
        case 2:
            // Initialisation de la mémoire secondaire
            initializeAllocationTable(totalBlocks);

            while (1)
            {
                printf("\n--- Menu Principal ---\n");
                printf("1. Creer un fichier\n");
                printf("2. Afficher l'etat de la memoire secondaire\n");
                printf("3. Afficher les metadonnees des fichiers\n");
                printf("4. Rechercher un enregistrement\n");
                printf("5. Inserer un nouvel enregistrement\n");
                printf("6. Supprimer un enregistrement (logique/physique)\n");
                printf("7. Supprimer un fichier\n");
                printf("8. Renommer un fichier\n");
                printf("9. Compactage de la memoire secondaire\n");
                printf("10. Vider la memoire secondaire\n");
                printf("11. Quitter\n");
                printf("Choix: ");
                scanf("%d", &choix);

                switch (choix)
                {
                case 1:
                    // Créer un fichier
                    printf("Entrez le nombre d'enregistrements: ");
                    scanf("%d", &n);
                    creerFichierChaine(&meta, n);
                    break;

                case 2:
                    // Afficher l'état de la mémoire secondaire
                    afficherEtatMemoire();
                    break;

                case 3:
                    // Afficher les métadonnées des fichiers
                    afficherMetadonnees();
                    break;

                case 4:
                    // Rechercher un enregistrement
                    printf("Entrez l'ID de l'enregistrement a rechercher: ");
                    scanf("%d", &id);
                    printf("Entrez le nom du fichier: ");
                    scanf("%s", nomFichier);

                    // Appel de la fonction de recherche avec les paramètres appropriés
                    rechercherEnregistrementChaine(&meta, nomFichier, id);
                    fclose(fichierMemoire);

                    break;

                case 5:
                    // Insérer un nouvel enregistrement
                    printf("Entrez l'ID du nouveau produit: ");
                    scanf("%d", &nouveauProduit.id);
                    printf("Entrez le nom du produit: ");
                    scanf("%s", nouveauProduit.nomFICHIER);
                    printf("Entrez le nom du fichier: ");
                    scanf("%s", nomFichier);
                    fichierMemoire = fopen(nomFichier, "r+b");
                    if (fichierMemoire)
                    {
                        insertionProduitNonTrie(fichierMemoire, nouveauProduit);
                        fclose(fichierMemoire);
                    }
                    else
                    {
                        printf("Erreur d'ouverture du fichier.\n");
                    }
                    break;

                case 6:
                    // Supprimer un enregistrement
                    printf("Entrez l'ID de l'enregistrement a supprimer: ");
                    scanf("%d", &id);
                    printf("Voulez-vous supprimer logiquement (1) ou physiquement (2)? ");
                    scanf("%d", &choix);
                    fichierMemoire = fopen(meta.Nomfichier, "r+b");
                    if (fichierMemoire)
                    {
                        if (choix == 1)
                        {
                            SuppressionLogique(fichierMemoire, &meta, id);
                        }
                        else if (choix == 2)
                        {
                            suppressionPhysique(fichierMemoire, &meta, id);
                        }
                        else
                        {
                            printf("Choix invalide.\n");
                        }
                        fclose(fichierMemoire);
                    }
                    else
                    {
                        printf("Erreur d'ouverture du fichier.\n");
                    }
                    break;

                case 7:
                    // Supprimer un fichier
                    printf("Entrez le nom du fichier e supprimer: ");
                    scanf("%s", nomFichier);
                    supprimerFichier("fichierMemoire.dat", nomFichier, &meta);
                    break;
                case 8:
                    // Renommer un fichier
                    printf("Entrez le nom actuel du fichier: ");
                    scanf("%s", nomFichier);
                    printf("Entrez le nouveau nom du fichier: ");
                    scanf("%s", filename);
                    renommerFichier(nomFichier, filename);
                    break;

                case 9:
                    // Compactage de la mémoire secondaire
                    printf("Defragmentation en cours...\n");
                    // Ajouter votre logique de compactage ici si nécessaire
                    break;

                case 10:
                    // Vider la mémoire secondaire
                    viderMemoireSecondaire();
                    break;

                case 11:
                    // Quitter le programme
                    printf("Quitter le programme...\n");
                    return 0;

                default:
                    printf("Choix invalide, veuillez essayer a nouveau.\n");
                }
            }

            break;

        default:
            break;
        }
        if (kbhit())
        {                      // Vérifie si une touche est pressée
            input = getchar(); // Récupère la touche
            if (input == 'q')
            {                    // Si l'utilisateur appuie sur 'q'
                running = false; // Arrête la boucle
            }
        }
    }
    return 0;
}