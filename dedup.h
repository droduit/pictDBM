/**
 * @file dedup.h
 * @brief Dé-duplication des images pour éviter qu'une même image (même contenu)
 * 		  soit présente plusieurs fois dans la base.
 *
 * @author Dominique Roduit, Thierry Treyer
 * @date 29 Avr 2016
 */

#include <stdlib.h>
#include "error.h"

/**
 * @brief Compare les deux hashcode sha1 et sha2 de la même manière que strcmp.
 * @param sha1 Hashcode 1
 * @param sha2 Hashcode 2
 * @return Un nombre négatif, nul ou positif selon le premier hash est
 *         plus petit, égal ou plus grand que le deuxième.
 */
int shacmp(const unsigned char *sha1, const unsigned char *sha2);

/**
 * @brief Dé-duplication des images
 * @param db_file Fichier pictDB précédemment ouvert
 * @param index Spécifie la position d'une image donnée dans le tableau metadata
 */
int do_name_and_content_dedup(struct pictdb_file* db_file, const uint32_t index);
