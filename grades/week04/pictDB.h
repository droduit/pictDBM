/**
 * @file pictDB.h
 * @brief Main header file for pictDB core library.
 *
 * Defines the format of the data structures that will be stored on the disk
 * and provides interface functions.
 *
 * The picture database starts with exactly one header structure
 * followed by exactly pictdb_header.max_files metadata
 * structures. The actual content is not defined by these structures
 * because it should be stored as raw bytes appended at the end of the
 * database file and addressed by offsets in the metadata structure.
 *
 * @date 2 Nov 2015
 */

#ifndef PICTDBPRJ_PICTDB_H
#define PICTDBPRJ_PICTDB_H

#include "error.h" /* not needed here, but provides it as required by
                    * all functions of this lib.
                    */
#include <stdio.h> // for FILE
#include <stdint.h> // for uint32_t, uint64_t
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH

#define CAT_TXT "EPFL PictDB binary"

/* constraints */
#define MAX_DB_NAME 31  // max. size of a PictDB name
#define MAX_PIC_ID 127  // max. size of a picture id
#define MAX_MAX_FILES 10  // will be increased later in the project

/* For is_valid in pictdb_metadata */
#define EMPTY 0
#define NON_EMPTY 1

// pictDB library internal codes for different picture resolutions.
#define RES_THUMB 0
#define RES_SMALL 1
#define RES_ORIG  2
#define NB_RES    3

#ifdef __cplusplus
extern "C" {
#endif

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
	struct pict_metadata metadata[MAX_MAX_FILES];
};

/**
 * @brief Prints database header informations.
 *
 * @param header The header to be displayed.
 */
void print_header (const struct pictdb_header header);

/**
 * @brief Prints picture metadata informations.
 *
 * @param metadata The metadata of one picture.
 */
void print_metadata (const struct pict_metadata metadata);

/**
 * @brief Displays (on stdout) pictDB metadata.
 *
 * @param db_file In memory structure with header and metadata.
 */
void do_list (const struct pictdb_file db_file);

/**
 * @brief Creates the database called db_filename. Writes the header and the
 *        preallocated empty metadata array to database file.
 *
 * @param db_file In memory structure with header and metadata.
 */
/* **********************************************************************
 * TODO WEEK 05: ADD THE PROTOTYPE OF do_create HERE.
 * **********************************************************************
 */
int do_create(const char* filename, struct pictdb_file db_file);

/* **********************************************************************
 * TODO WEEK 06: ADD THE PROTOTYPE OF do_delete HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF resolution_atoi HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF do_read HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF do_insert HERE.
 * **********************************************************************
 */

#ifdef __cplusplus
}
#endif
#endif
