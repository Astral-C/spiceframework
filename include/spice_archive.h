#ifndef __SPICE_ARCHIVE_H__
#define __SPICE_ARCHIVE_H__
#include <spice_util.h>
#include <stdint.h>

typedef struct {
    sp_str name;
    uint8_t* data;
    uint32_t size;
} prkArcFile;

typedef struct {
    uint32_t file_count;
    prkArcFile* files;
    uint8_t* _raw_file_data;
} prkArchive;

prkArchive* prkArchiveOpen(char* path);
prkArcFile* prkArchiveGetFile(prkArchive* arc, sp_str name);

void prkArchiveClose(prkArchive* arc);

#endif