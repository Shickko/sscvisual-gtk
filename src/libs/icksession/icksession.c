#include <dirmain/icksession_menu.h>
#include <dirmain/icksession_home.h>
#include <dirconf/ickconf.h>
#include <icksession.h>
#include <ickstorage.h>
#include <steamcurl.h>

struct icksession_t {
  ickdict_t       *session_dt;
  GtkApplication  *src;
};


static int icksession_create_window(icksession_t *session) {
  const ickpoint_t window_size = ickpoint (400, 600);
  const char* window_title     = "Shickko Session";

  GtkWidget *mwin  = gtk_application_window_new(session->src);
  GtkWidget *mgrid = gtk_grid_new();

  icksession_attach(session, "window_main", mwin);
  icksession_attach(session, "master_grid", mgrid);

  if (icksession_read(session, "window_main") == NULL || 
      icksession_read(session, "master_grid") == NULL) 
  { return ICKERR; }

  gtk_window_set_title(
    GTK_WINDOW(icksession_readw(session, "window_main")), 
    window_title
  );
  gtk_window_set_default_size(
    GTK_WINDOW(icksession_readw(session, "window_main")),
    window_size.x,
    window_size.y
  );
  gtk_grid_set_baseline_row(
      GTK_GRID(icksession_readw(session, "master_grid")), 0);
  gtk_container_add(
      GTK_CONTAINER(icksession_readw(session, "window_main")), 
                    icksession_readw(session, "master_grid"));
  return ICKSUCCESS;
}

static void icksession_main(GtkApplication *src, gpointer p) {
  icksession_t *session = p;
  //[[ Configuration
  icksession_attach(
    session,
    "configuration", 
    ickdict());
  ickdict_t *config = (ickdict_t* )icksession_read(
    session,
    "configuration");
  if (ickconf_read(config) != ICKSUCCESS) { 
    ickconf_create(); 
    ickconf_read(config);
  }
  //[[ Steam Client
  icksession_steamcli_start(session);
  //[[ Session Populate
  if (icksession_create_window(session) != ICKSUCCESS)
  { return g_application_quit(G_APPLICATION(src)); }
  icksession_create_menu(session);
  //icksession_create_home(session);
  gtk_widget_show_all(icksession_readw(session, "window_main"));

  return;
}

void icksession_attachgrid(icksession_t *session, GtkWidget *grid, GtkWidget *widget, ickpoint_t pos, ickpoint_t len) {
  if (session == NULL || widget == NULL)
  { return; }
  gtk_grid_attach(
    GTK_GRID(grid), widget,
    pos.x, pos.y,
    len.x, len.y
  );
}

void icksession_attach(icksession_t *session, char* label, void *dt) {
  if (session == NULL || session->src == NULL)
  { return; }
  if (ickdict_attach(session->session_dt, label, dt) == ICKSUCCESS) {
    ickout(ICKOUT_LOG, "process[%p] populate - success -> %s[%p]\n",
      session,
      label,
      dt
    );
  } else {
    ickout(ICKOUT_ERROR, "process[%p] populate - failure -> %s[%p]\n",
      session,
      label,
      dt
    );
  }
}
void* icksession_detach(icksession_t *session, char* label) {
  if (session == NULL || session->src == NULL)
  { return NULL; }
  return ickdict_detach(session->session_dt, label);
}

GtkWidget* icksession_readw(icksession_t *session, char* label)
{ return (GtkWidget* )ickdict_read(session->session_dt, label); }
void*      icksession_read (icksession_t *session, char* label)
{ return ickdict_read(session->session_dt, label); }

int icksession_init(int argc, char* argv[]) {
  icksession_t *session;
  ickdict_t* dt;

  session = calloc(1, sizeof(icksession_t));
  dt = ickdict();

  if (session == NULL || dt == NULL)
  { return ICKERR; }

  session->session_dt = dt;
  session->src = gtk_application_new("org.gtk.icksession", G_APPLICATION_FLAGS_NONE);

  ickout(ICKOUT_LOG, "process[%p] initialize - success -> session_index\n", session);
  g_signal_connect(session->src, "activate", G_CALLBACK(icksession_main), session);
  g_application_run(G_APPLICATION(session->src), argc, argv);

  return icksession_exit(session);
}

int icksession_exit(icksession_t *session) {
  ickout(ICKOUT_LOG, "process[%p] shutdown - requested -> session_index\n", session);

  if (session == NULL || session->src == NULL)
  { return ICKUE; }
  g_object_unref(session->src);

  ickdict_t *config = (ickdict_t* )icksession_detach(session, "configuration");
  if (config) { ickconf_close(config); }

  icksession_close_menu(session);
  icksession_close_home(session);
  icksession_steamcli_end(session);
  ickdict_smash(session->session_dt);

  ickout(ICKOUT_LOG, "process[%p] shutdown - completed -> session_index\n", session);
  free(session);
  return 0;
}

int icksession_steamcli_start(icksession_t *session) {
  ickdict_t  *config;
  steamcli_t *client;

  config = (ickdict_t*)icksession_read(session, "configuration");
  if (ickconf_readv(config, "usr_authkey") == NULL || 
      ickconf_readv(config, "usr_authid")  == NULL) 
  { return ICKERR; }

  client = steamcli_create(
    ickconf_readv(config, "usr_authkey"),
    ickconf_readv(config, "usr_authid")
  );

  if (client == NULL) { 
    ickout(ICKOUT_ERROR, "process[%p] steamcurl -> failed to start (unexpected)\n");
    return ICKERR; 
  }
  icksession_attach(session, "session_steamcli", client);

  return 0;
}

int icksession_steamcli_end(icksession_t *session) {
  steamcli_t *client = (steamcli_t* )icksession_read(session, "session_steamcli");
  if (client == NULL) { return ICKERR; }
  return steamcli_close(client);
}

