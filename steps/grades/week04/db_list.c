/**
 * @file do_list.h
 * @brief Implementation of the command do_list
 *
 * @date 9 Mar 2016
 */

#include "pictDB.h"

/********************************************************************//**
 * Full database display
 */
void do_list (const struct pictdb_file db_file) {
    const struct pictdb_header header = db_file.header;

    print_header(header);

    if (header.num_files == 0) {
        puts("<< empty database >>");
    } else {
		uint32_t i = 0, num_files = 0;
        while (i < header.max_files && num_files < header.num_files) {
            struct pict_metadata metadata = db_file.metadata[i];
			
            if (metadata.is_valid == NON_EMPTY) {
                print_metadata(metadata);
                num_files++;
			}

            i++;
        }
    }
}
