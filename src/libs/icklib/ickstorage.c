#include <ickstorage.h>
#include <ickutils.h>
#include <string.h>
#include <stdlib.h>

struct icknode_t {
  unsigned int npos;
  void       *ncurr;
  icknode_t  *nnext;
};
struct ickdict_t {
  size_t dictcap;
  size_t dictlen;
  icknode_t **dt;
  icknode_t **ch;
};

icknode_t* icknode() {
  icknode_t *node = (icknode_t *)calloc(1, sizeof(icknode_t));
  if (!node) { return NULL; }

  node->npos  = 0;
  node->ncurr = NULL;
  node->nnext = NULL;

  return node;
}

icknode_t* icknode_get(icknode_t *index, int pos) {
  //reminder: this is a relative pos from index.
  if (pos < 0) { return NULL; }
  if (index == NULL) {
    ickout(ICKOUT_ERROR, "invalid node read at pos %d\n", pos);
    return NULL; 
  }
  unsigned int actual_pos = index->npos + pos;
  while (index->npos != actual_pos) {
    if (index->nnext == NULL) { return NULL; }
    index = index->nnext;
  }
  return index;
}

int icknode_smash(icknode_t *node) {
  if (node == NULL) { return ICKERR; }
  icknode_t *node_curr = node;
  while (node->nnext != NULL) {
    node = node->nnext;
    free(node_curr);
    node_curr = node;
  }
  free(node_curr);
  node_curr = NULL;
  node      = NULL;
  return ICKSUCCESS;
}

int icknode_getlen(icknode_t *node) {
  if (node == NULL) { return ICKERR; }
  unsigned int pos_index = node->npos;
  while (node->nnext != NULL) {
    node = node->nnext;
  }
  return node->npos - pos_index + 1;
}

int icknode_append(icknode_t *node, void *tar) {
  if (node == NULL) { return ICKERR; }

  while (node->ncurr != NULL) {
    unsigned int pos_curr = node->npos;
    if (node->nnext == NULL) {
      node->nnext = icknode();
      node = node->nnext;
      if (!node) { return ICKERR; }
      node->npos = pos_curr + 1;
    } else {
      node = node->nnext;
      if (!node) { return ICKERR; }
    }
  }
  node->ncurr = tar;

  return ICKSUCCESS;
}

int icknode_insert(icknode_t *node, int pos, void *tar) {
  if (node == NULL) { return ICKERR; }
  void *reserve, *reserve_next;

  if (icknode_get(node, pos) == NULL) {
    while (node->npos != pos) {
      unsigned int pos_curr = node->npos;
      if (node->nnext == NULL) {
        node->nnext = icknode();
        node = node->nnext;
        if (!node) { return ICKERR; }
        node->npos = pos_curr + 1;
      } else {
        node = node->nnext;
        if (!node) { return ICKERR; }
      }
    }
  } else {
    node = icknode_get(node, pos);
  }

  reserve = node->ncurr;
  node->ncurr = tar;

  icknode_append(node, NULL);

  while (icknode_get(node, 1) != NULL) {
    node = icknode_get(node, 1);
    if (node == NULL) { return ICKUE; }
    reserve_next = node->ncurr;
    node->ncurr = reserve;
    reserve = reserve_next;
  }

  return ICKSUCCESS;
}

void* icknode_replace(icknode_t *node, int pos, void *tar) {
  if (node == NULL) { return NULL; }
  void *reserve;
  if (icknode_get(node, pos) == NULL) {
    while (node->npos != pos) {
      unsigned int pos_curr = node->npos;
      if (node->nnext == NULL) {
        node->nnext = icknode();
        node = node->nnext;
        if (!node) { return NULL; }
        node->npos = pos_curr + 1;
      } else {
        node = node->nnext;
        if (!node) { return NULL; }
      }
    }
  } else {
    node = icknode_get(node, pos);
  }

  reserve = node->ncurr;
  node->ncurr = tar;
  return reserve;
}

void* icknode_acquire(icknode_t *node, int pos) {
  if (icknode_get(node, pos) == NULL) { 
    ickout(ICKOUT_WARN, "node[%p] - failed to read pos %d -> possible missing\n",
      node, pos
    );
    return NULL; 
  }
  return icknode_get(node, pos)->ncurr;
}

void** icknode_dump(icknode_t *node) {
  if (node == NULL) { return NULL; }
  unsigned int nlen = icknode_getlen(node);
  void** dump = malloc(nlen * sizeof(void *));
  for (int i = 0; i < nlen; i++) {
    dump[i] = node->ncurr;
    node = node->nnext;
  }
  return dump; // need manual free
}

void* icknode_remove(icknode_t *node, int pos) {
  if (!icknode_get(node, pos)) { return NULL; }
  void *reserve = 
    icknode_get(node, pos)->ncurr;
    icknode_get(node, pos)->ncurr = NULL;
  return reserve;
}

ickdict_t* ickdict() {
  size_t dictcap = STORAGE_BUFFER;
  size_t dictlen = 0;
  ickdict_t *dict;
  icknode_t **dt;
  icknode_t **ch;

  dict = (ickdict_t *)malloc(sizeof(ickdict_t));
  dt   = (icknode_t **)malloc(dictcap * sizeof(icknode_t *));
  ch   = (icknode_t **)malloc(dictcap * sizeof(icknode_t *));
  for (int i = 0; i < dictcap; i ++) {
    dt[i] = icknode();
    ch[i] = icknode();
  }

  dict->dictcap = dictcap;
  dict->dictlen = dictlen;
  dict->dt = dt;
  dict->ch = ch;

  ickout(ICKOUT_LOG, "ickdict[%p] - initialized\n", dict);
  return dict;
}

int ickdict_smash(ickdict_t *dict) {
  if (dict == NULL)
  { return ICKERR; }
  for (int i = 0; i < dict->dictcap; i++) {
    icknode_smash(dict->dt[i]);
    icknode_smash(dict->ch[i]);
  }

  ickout(ICKOUT_LOG, "ickdict[%p] - removed \n", dict);
  /* here ickdict has no responsiblity of
   * cleaning stored data. Any manually
   * allocated data shall be also freed
   * manually at this point, in order to
   * avoid leakage. */
  free(dict);

  dict = NULL;

  return ICKSUCCESS;
}

static unsigned int ickdict_genkey(ickdict_t *dict, char* key) {
  if (dict == NULL) { return ICKERR; }
  unsigned int keyvar = 0;

  for (int i = 0; key[i] != '\0'; i ++) {
    keyvar += key[i] * 33 + key[i];
  } 
  //ickout(ICKOUT_LOG, "from %s = %d\n", key, keyvar);
  keyvar %= dict->dictcap;
  //ickout(ICKOUT_LOG, "to %s = %d\n", key, keyvar);

  return keyvar; 
}

int ickdict_attach(ickdict_t *dict, char* label, void *tar) {
  if (dict == NULL)                   { return ICKUE;  }
  if (dict->dictlen == dict->dictcap) { return ICKERR; }
  dict->dictlen += 1;

  unsigned int keyvar = ickdict_genkey(dict, label);
  icknode_t *index = dict->dt[keyvar];
  icknode_t *key   = dict->ch[keyvar];

  int s1 = icknode_append(index, tar);
  int s2 = icknode_append(key, label);

  if (s1 != ICKSUCCESS || s2 != ICKSUCCESS) { 
    ickout(ICKOUT_ERROR, "ickdict[%p] append failure - "
      "%s[%p] -> node[%p]: %d, node[%p]: %d\n",
      dict, label, tar, index, s1, key, s2
    ); 
    return ICKERR;
  }
  ickout(ICKOUT_LOG, "ickdict[%p] append success - "
    "%s[%p] -> node[%p]: %d, node[%p]: %d\n",
    dict, label, tar, index, s1, key, s2
  ); 

  return ICKSUCCESS;
}

void* ickdict_detach(ickdict_t *dict, char* label) {
  if (dict == NULL) { return NULL; }
  unsigned int keyvar = ickdict_genkey(dict, label);
  int offset = 0;
  icknode_t *index = dict->dt[keyvar];
  icknode_t *key   = dict->ch[keyvar];

  while (icknode_get(key, offset) != NULL) {
    int varcheck = strcmp(
      (char* )icknode_acquire(key, offset),
      label
    );

    if (varcheck == 0) {
      void *tar = icknode_remove(index, offset);
      icknode_remove(key, offset);
      return tar;
    } else {
      offset += 1;
    }
  }
}

void* ickdict_read(ickdict_t *dict, char* label) {
  if (dict == NULL || label == NULL) 
  { return NULL; }
  unsigned int keyvar = ickdict_genkey(dict, label);
  int offset = 0;
  icknode_t *index = dict->dt[keyvar];
  icknode_t *key   = dict->ch[keyvar];

  while (icknode_get(key, offset) != NULL) {
    int varcheck = strcmp(
      (char* )icknode_acquire(key, offset),
      label
    );

    if (varcheck == 0) {
      void *tar = icknode_acquire(index, offset);
      return tar;
    } else {
      offset += 1;
    }
  }
  return NULL;
}
