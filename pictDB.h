/**
 * @file pictDB.h
 * @brief Header principal pour le cœur de pictDB.
 *
 * Définit la structures des données qui seront enregistrées sur le disque
 * et fournit les fonctions servant d'interface.
 *
 * La base de données d'image débute avec exatement une structure header,
 * suivit par exactement pictdb_header.max_files metadata structure. Le
 * contenu actuel n'est pas définit par ces structures car il est stocké
 * sous forme de données brutes à la fin du fichier et addressé par le
 * champ offset de la structure metadata.
 *
 * @author Mia Primorac, Dominique Roduit, Thierry Treyer
 * @date 2 Nov 2015
 */

#ifndef PICTDBPRJ_PICTDB_H
#define PICTDBPRJ_PICTDB_H

#include "error.h" /* Pas nécessaire ici, mais nécessaire à toutes
                    * les fonctions de cette lib.
                    */
#include <stdio.h> // pour FILE
#include <stdint.h> // pour uint32_t, uint64_t
#include <openssl/sha.h> // pour SHA256_DIGEST_LENGTH

#define CAT_TXT "EPFL PictDB binary"
#define NAME_SUFFIX_LENGTH 6 // strlen("_.jpg") + '\0'

/* constraints */
#define MAX_DB_NAME 31  // taille max d'un nom de PictDB
#define MAX_PIC_ID 127  // taille max de l'id d'une image
#define MAX_MAX_FILES 100000
#define MAX_SMALL_RES 512
#define MAX_THUMB_RES 128

/* Pour is_valid dans pictdb_metadata */
#define EMPTY 0
#define NON_EMPTY 1

// codes internes à pictDB pour les différentes résolutions d'images.
#define RES_THUMB 0
#define RES_SMALL 1
#define RES_ORIG  2
#define NB_RES    3

#ifdef __cplusplus
extern "C" {
#endif

enum do_list_mode {
    STDOUT, JSON
};

// header qui contient les informations de configuration de la pictDB
struct pictdb_header {
    // Nom de la base d'images
    char db_name[MAX_DB_NAME + 1];
    // Version de la base de données
    uint32_t db_version;
    // Nombre d'images présentes dans la base
    uint32_t num_files;
    // Nombre maximal d'images possibles dans la base
    uint32_t max_files;
    // Tableaux des résolutions des images (thumb X, thumb Y, small X, small Y)
    uint16_t res_resized[2 * (NB_RES - 1)];

    // Prévu pour des évolutions futures ou des informations temporaires
    uint32_t unused_32;
    uint64_t unused_64;
};

// Métadonnées d'une image
struct pict_metadata {
    // Identificateur unique (nom) de l'image
    char pict_id[MAX_PIC_ID + 1];
    // Hash code de l'image
    unsigned char SHA[SHA256_DIGEST_LENGTH];
    // Résolution de l'image d'origine
    uint32_t res_orig[2];
    // Tailles mémoire (en octets) des images aux différentes résolutions
    uint32_t size[NB_RES];
    // positions dans le fichier des images aux différentes résolutions
    uint64_t offset[NB_RES];
    // Indique si l'image est encore utilisée (NON_EMPTY) ou effacée (EMPTY)
    uint16_t is_valid;

    // Prévu pour des évolutions futures ou informations temporaires
    uint16_t unused_16;
};

struct pictdb_file {
    // Indique le fichier contenant tout (sur le disque)
    FILE* fpdb;
    // Informations générales de la base d'images
    struct pictdb_header header;
    // Métadata des images dans la base
    struct pict_metadata* metadata;
};

/**
 * @brief Affiche l'en-tête de la base de données.
 *
 * @param header L'en-tête à afficher.
 */
void print_header (const struct pictdb_header* header);

/**
 * @brief Affiche les informations d'une metadata.
 *
 * @param metadata Les metadata d'une image.
 */
void print_metadata (const struct pict_metadata* metadata);

/**
 * @brief Affiche (sur stdout) les informations de la pictDB.
 *
 * @param db_file Structure contenant l'en-tête et les metadatas.
 */
const char* do_list (const struct pictdb_file* db_file, enum do_list_mode mode);

/**
 * @brief Crée une base de données nommée filename. Écrit l'en-tête et
 *        pré-alloue un tableau de metadatas vide dans le fichier.
 *
 * @param db_file Structure contenant l'en-tête et les metadatas.
 */
int do_create(const char* filename, struct pictdb_file* db_file);

/**
 * @brief Ouvre le fichier db_filename, lis le contenu du header et les metadonnées.
 * @param db_filename Nom de fichier de la base d'image
 * @param mode Mode d'ouverture du fichier
 * @param db_file Structure dans laquelle stocker les données lues
 * @return 0 si pas d'erreur, sinon le code d'erreur approprié (cf. error.h)
 */
int do_open(const char* db_filename, const char* mode, struct pictdb_file* db_file);

/**
 * @brief Ferme le fichier contenu par la structure db_file
 * @param db_file Structure contenant le fichier à fermer
 */
void do_close(struct pictdb_file* db_file);

/**
 * @brief Écrit le contenu de la structure db_file sur le disque
 * @param db_file Structure à écrire
 */
int do_write(const struct pictdb_file* db_file, size_t *items_written);

/**
 * @brief Supprime l'image spécifiée par son identifiant id dans db_file
 * @param id Identifiant de l'image à supprimer
 * @param db_file Structure de notre la base de donnée d'image
 * @return 0 si pas d'erreur, sinon le code d'erreur approprié (cf. error.h)
 */
int do_delete(const char* id, struct pictdb_file* db_file);

/**
 * @brief Transforme une chaine de caractères spécifiant une résolution d'image
 * en une des énumérations spécifiant un type de résolution.
 * @param res Résolution d'image sous forme de chaine de caractères
 * @return Enumération spécifiant le type de résolution correspondant à res
 */
int resolution_atoi(const char* res);

/**
 * @brief Lis une image dans la pictDB
 * @param pict_id Identifiant d'image
 * @param res Code d'une résolution d'image
 * @param image_buffer Adresse de l'image en mémoire
 * @param image_size Taille de l'image
 * @param db_file Structure de laquelle on lira l'image
 * @return Code d'erreur approprié
 */
int do_read(const char* pict_id, uint32_t res, char** image_buffer, uint32_t* image_size, struct pictdb_file* db_file);

/**
 * @brief Ajoute une image dans la pictDB
 * @param img Image sous forme d'un pointeur (tableau) de caractères (utilisés en tant qu'octets)
 * @param size Taille de l'image
 * @param pict_id Identifiant d'image
 * @param db_file structure dans laquelle on ajoutera l'image
 * @return Code d'erreur approprié
 */
int do_insert(const char* img, size_t size, const char* pict_id, struct pictdb_file* db_file);

/**
 * @brief Nettoye la base d'images en collectant l'espace libre d'une pictDB.
 * @param src  La structure pictdb_file source
 * @param src_name Nom original du fichier pictdb (deja ouvert et correspond au fichier pointé dans le pictdb_file)
 * @param tmp_name Nom du nouveau fichier, utilisé temporairement pour la nouvelle version "nettoyée"
 * @return Code d'erreur approprié (0 en cas de succès)
 */
int do_gbcollect(struct pictdb_file* src, const char* src_name, const char* tmp_name);

/**
 * @brief Créé un nom suivant les conventions de nommages
 * original_prefix + resolution_suffix + '.jpg'
 * @param pict_id Identifiant de l'image (original_prefix)
 * @param res Resolution de l'image
 * @return Nom de l'image suivant la convention
 */
const char* create_name(const char* pict_id, uint32_t res);

/**
 * @brief Lis l'image spécifie par filename et la met dans image
 * @param filename Le nom du fichier image à lire
 * @param image Adresse a laquelle on va stocker le contenu de l'image
 * @param image_size Taille de l'image
 */
int read_disk_image(const char *filename, void **image, size_t *image_size);

/**
 * @brief Ecris l'image spécifiée par l'adresse image dans le fichier filename
 * @param filename Nom de l'image sur le disque
 * @param image Adresse vers le contenu de l'image en mémoire
 * @param image_size Taille de l'image
 */
int write_disk_image(const char *filename, const void *image, const size_t image_size);

#ifdef __cplusplus
}
#endif
#endif
