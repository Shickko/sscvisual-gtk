#ifndef ICKLIB_H_SESSION
#define ICKLIB_H_SESSION

#include <gtk/gtk.h>
#include <ickutils.h>

typedef struct icksession_t icksession_t;

void  icksession_attachgrid(icksession_t *session, GtkWidget *grid, GtkWidget *widget, ickpoint_t pos, ickpoint_t len);
void  icksession_attach(icksession_t *session, char* label, void *dt);
void* icksession_detach(icksession_t *session, char* label);
GtkWidget* icksession_readw(icksession_t *session, char* label);
void*      icksession_read (icksession_t *session, char* label);

int icksession_init(int argc, char* argv[]);
int icksession_exit(icksession_t *session);
int icksession_steamcli_start(icksession_t *session);
int icksession_steamcli_end  (icksession_t *session);

#endif
