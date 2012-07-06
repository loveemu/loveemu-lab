
#ifndef ndsfat_h
#define ndsfat_h

#include <stdio.h>
#include "../cioutil.h"

#define NITROFS_MAXPATHLEN   1024

#ifndef NO_FILE_MAPPING

typedef bool (*NdsEnumFilesCallback) (uint8*, const char*, const char*, uint32, uint32, void*);

int ndsInformFile(uint8 *nds, const char *prefix, const char *entry_name, uint16 file_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);
int ndsEnumFilesDir(uint8 *nds, char *prefix, uint16 dir_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);
int ndsEnumFileCountFromBuf(uint8 *nds, int *errCode);
int ndsEnumFilesFromBuf(uint8 *nds, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);

#else

typedef bool (*NdsEnumFilesCallback) (FILE*, const char*, const char*, uint32, uint32, void*);

int ndsInformFile(FILE *fNDS, const char *prefix, const char *entry_name, uint16 file_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);
int ndsEnumFilesDir(FILE *fNDS, char *prefix, uint16 dir_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);
int ndsEnumFileCountFromPointer(FILE *fNDS, int *errCode);
int ndsEnumFileCount(const char *ndsfilename, int *errCode);
int ndsEnumFilesFromPointer(FILE *fNDS, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);
int ndsEnumFiles(const char *ndsfilename, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData);

#endif // NO_FILE_MAPPING

#endif
