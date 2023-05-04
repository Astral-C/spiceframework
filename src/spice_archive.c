#include <spice_archive.h>

/// .paprika spec
/// u32 magic
/// u32 file count
/// u32 file data offset
/// u32 padding, maybe compression flag later
/// file_header[file_count] file headers
///
/// file header
/// u8 nameLen
/// string[nameLen] filename;
/// u32 file start
/// u32 file size
///
/// u8* raw file data

prkArchive* prkArchiveOpen(char* path){
    FILE* archive = fopen(path, "rb");

    prkArchive* arc = malloc(sizeof(prkArchive));

    if(archive == NULL || arc == NULL){
        spice_error("Couldn't Init Archive %s\n", path);
        return NULL;
    }

    uint32_t magic, file_count, file_data_offset;

    fread(&magic, sizeof(uint32_t), 1, archive);
    if(magic != 0x414B5250) return NULL;

    fread(&file_count, sizeof(uint32_t), 1, archive);
    fread(&file_data_offset, sizeof(uint32_t), 1, archive);
    spice_info("Loading archive with %d files\n", file_count);

    size_t position = ftell(archive);
    fseek(archive, 0, SEEK_END);
    size_t file_data_size = ftell(archive) - file_data_offset;
    fseek(archive, position+4, SEEK_SET);

    arc->files = malloc(sizeof(prkArcFile) * file_count);
    arc->_raw_file_data = malloc(file_data_size);

    memset(arc->files, 0, sizeof(prkArcFile) * file_count);
    memset(arc->_raw_file_data, 0, file_data_size);

    for(int i = 0; i < file_count; i++){
        uint8_t name_length;
        uint32_t start;

        fread(&name_length, sizeof(uint8_t), 1, archive);

        fread(arc->files[i].name, 1, name_length, archive);
        fread(&start, sizeof(uint32_t), 1, archive);
        fread(&arc->files[i].size, sizeof(uint32_t), 1, archive);

        spice_info("Loadad File %s with size %d from archive\n", arc->files[i].name, arc->files[i].size);

        arc->files[i].data = arc->_raw_file_data + start;
    }

    fseek(archive, file_data_offset, SEEK_SET); //skip padding
    fread(arc->_raw_file_data, file_data_size, 1, archive);

    fclose(archive);

    return arc;

}

prkArcFile* prkArchiveGetFile(prkArchive* arc, sp_str name){
    for(int f = 0; f < arc->file_count; f++){
        if(sp_strcmp(arc->files[f].name, name)){
            spice_info("Checking files %s for %s...\n", arc->files[f].name, name);
            return &arc->files[f];
        }
    }
    return NULL;
}

void prkArchiveClose(prkArchive* arc){
    spice_info("Cleaning up archive...\n");
    free(arc->files);
    free(arc->_raw_file_data);
    free(arc);
}