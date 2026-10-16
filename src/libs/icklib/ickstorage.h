#ifndef ICKLIB_H_STORAGE
#define ICKLIB_H_STORAGE
#define STORAGE_BUFFER (1024)
typedef struct icknode_t icknode_t;
typedef struct ickdict_t ickdict_t;

icknode_t* icknode();
icknode_t* icknode_get(icknode_t *index, int pos);
int icknode_smash(icknode_t *node);
int     icknode_getlen(icknode_t *node);
int     icknode_append(icknode_t *node, void *tar);
int     icknode_insert(icknode_t *node, int pos, void *tar);
void*   icknode_replace(icknode_t *node, int pos, void *tar);
void*   icknode_acquire(icknode_t *node, int pos);
void**  icknode_dump(icknode_t *node);
void*   icknode_remove(icknode_t *node, int pos);

ickdict_t* ickdict();
int ickdict_smash(ickdict_t *dict);
int   ickdict_attach(ickdict_t *dict, char* label, void* tar);
void* ickdict_detach(ickdict_t *dict, char* label);
void* ickdict_read(ickdict_t *dict, char* label);
#endif
