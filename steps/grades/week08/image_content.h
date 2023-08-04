/**
 * @file image_content.h
 * @brief Créé une nouvelle variante d'une image, avec la resolution spécifiée
 * 		  si cette résolution n'existe pas déjà.
 *
 * @date 11 Avr 2016
 */

#include <stdlib.h>
#include <vips/vips.h>

#include "error.h"
 
/**
 * @brief Vérifie la légitimité des arguments, et retourne si nécessaire une valeur d'erreur
 * appropriée.
 * 
 * Si l'image demandée existe déjà dans la résolution correspondante, ne fait rien.
 * Sinon, créé une nouvelle variante de l'image spécifiée, dans la résolution spécifiée.
 * 
 * Copie le contenu de cette nouvelle image à la fin du fichier pictDB.
 * 
 * Met à jour le contenu de la metadata en mémoire et sur le disque
 * 
 * @param res Code de la résolution (RES_THUMB, RES_SMALL) cf. pictDB.h
 * @param db_file structure avec laquelle on travaille
 * @param index position de l'image à traiter
 * @return 0 si pas d'erreur, sinon le code d'erreur approprié
 */
int lazily_resize(struct pictdb_file* db_file, const size_t index, const uint32_t res);
/**
 * @brief Computes the shrinking factor (keeping aspect ratio)
 * @param image The image to be resized.
 * @param max_width The maximum width allowed for thumbnail creation.
 * @param max_height The maximum height allowed for thumbnail creation.
 */
double resize_ratio(const VipsImage *image, const uint16_t max_width, const uint16_t max_height);
/**
 * @brief Lis l'image à la résolution donnée res dans le fichier de base de donnée
 * @param db_file Structure sur laquelle on travaille
 * @param index Position de l'image à récupérer
 * @param res Résolution de l'image
 * @param buf Buffer dans lequel on met l'image
 **/
int fetch_image(const struct pictdb_file* db_file, const size_t index, const uint32_t res, void **buf);
/**
 * @brief Stock le contenu du buffer contenant l'image dans le fichier de base de donnée
 * @param db_file Structure sur laquelle on travaille
 * @param index Position de l'image à stoquer
 * @param res Résolution de l'image à stoquer
 * @param buf Buffer contenant l'image à stoquer
 * @param len Taille de l'image
 */
int store_image(struct pictdb_file* db_file, const size_t index, const uint32_t res, const void *buf, const uint32_t len);
/**
 * @brief Vérifie qu'une image à une résolution donnée existe dans le fichier de base de donnée
 * @param db_file Structure avec laquelle on travaille
 * @param index Position de l'image dans le fichier de base de donnée
 * @param res Résolution de l'image
 */
int check_image_exists(const struct pictdb_file* db_file, const size_t index, const uint32_t res);
