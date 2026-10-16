#include <dirmain/icksession_menu.h>
#include <dirmain/icksession_home.h>
#include <ickstorage.h>
#include <ickutils.h>
#include <gtk/gtk.h>

enum label_menu_toplevel {
  MENU_FILE,
  MENU_EDIT,
  MENU_VIEW,
  TOP_LEVEL
};
enum label_menu_layers {
  MENU_LAYER_INDEX,
  MENU_LAYER_FILE,
  MENU_LAYER_EDIT,
  MENU_LAYER_VIEW,
  MENU_LAYERS
};
enum label_menu_layer_file {
  MENU_FILE_HOME,
  MENU_FILE_EXIT,
  FILE_LEVEL
};
enum label_menu_layer_edit {
  MENU_EDIT_PREF,
  EDIT_LEVEL
};

static const char* icksession_menu_label_get(int label_id, int label_layer) {
  switch (label_layer) {
    case MENU_LAYER_INDEX: { goto label_assign_index; }
    case MENU_LAYER_FILE:  { goto label_assign_file;  }
    case MENU_LAYER_EDIT:  { goto label_assign_edit;  }
    default:               { goto label_assign_err;   }
  }
label_assign_index:
  switch (label_id) {
    case MENU_FILE: { return "File"; }
    case MENU_EDIT: { return "Edit"; }
    case MENU_VIEW: { return "View"; }
    default:        { goto label_assign_err; }
  }
label_assign_file:
  switch (label_id) {
    case MENU_FILE_HOME: { return "Home"; }
    case MENU_FILE_EXIT: { return "Quit"; }
    default:        { goto label_assign_err; }
  }
label_assign_edit:
  switch (label_id) {
    case MENU_EDIT_PREF: { return "Preference"; }
    default:        { goto label_assign_err; }
  }
label_assign_err:
  ickout(ICKOUT_ERROR, "Label[%d:%d] is out of range.\n", label_id, label_layer);
  return "[UNDEFINED]";
}

static void icksession_menu_func(icksession_t *session) {
  if (session == NULL)
  { return; }

  //[[FILE>>HOME - Home
  GtkWidget *fhome = (GtkWidget* )icknode_acquire(
    (icknode_t* )icksession_read(session, "menu_bar_file"),
    MENU_FILE_HOME
  );
  g_signal_connect_swapped(
    fhome,
    "activate",
    G_CALLBACK(icksession_create_home),
    session
  );
  //[[FILE>>QUIT - Exit
  GtkWidget *fexit = (GtkWidget* )icknode_acquire(
    (icknode_t* )icksession_read(session, "menu_bar_file"),
    MENU_FILE_EXIT
  );
  g_signal_connect_swapped(
    fexit,
    "activate",
    G_CALLBACK(gtk_widget_destroy),
    icksession_readw(session, "window_main")
  );
}

void icksession_create_menu(icksession_t *session) {
  if (session == NULL) { return; }

  const ickpoint_t session_menu_pos = ickpoint(0,0);
  const ickpoint_t session_menu_len = ickpoint(1,1);
  icknode_t *session_menu_items = icknode();
  icknode_t *session_menu_tree  = icknode();
  icknode_t *session_sub_file   = icknode();
  icknode_t *session_sub_edit   = icknode();
  icksession_attach(session, "menu_bar", gtk_menu_bar_new());

  for (int i = 0; i < TOP_LEVEL; i++) {
    const char* label = icksession_menu_label_get(i, MENU_LAYER_INDEX);
    GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
    GtkWidget *menu_tree = gtk_menu_new();
    gtk_menu_shell_append(
      GTK_MENU_SHELL(
        icksession_readw(session, "menu_bar")
      ),
      menu_item
    );
    gtk_menu_item_set_submenu(
      GTK_MENU_ITEM(menu_item),
      menu_tree
    );
    gtk_widget_set_hexpand(menu_item, TRUE);
    icknode_append(session_menu_items, menu_item);
    icknode_append(session_menu_tree,  menu_tree);
  }
  for (int i = 0; i < FILE_LEVEL; i++) {
    const char* label = icksession_menu_label_get(i, MENU_LAYER_FILE);
    GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
    gtk_menu_shell_append(
      GTK_MENU_SHELL(
        icknode_acquire(session_menu_tree, MENU_FILE)
      ),
      menu_item
    );
    gtk_widget_set_hexpand(menu_item, TRUE);
    icknode_append(session_sub_file,  menu_item);
  }
  for (int i = 0; i < EDIT_LEVEL; i++) {
    const char* label = icksession_menu_label_get(i, MENU_LAYER_EDIT);
    GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
    gtk_menu_shell_append(
      GTK_MENU_SHELL(
        icknode_acquire(session_menu_tree, MENU_EDIT)
      ),
      menu_item
    );
    gtk_widget_set_hexpand(menu_item, TRUE);
    icknode_append(session_sub_edit,  menu_item);
  }

  icksession_attach(session, "menu_bar_items", session_menu_items);
  icksession_attach(session, "menu_bar_tree",  session_menu_tree);
  icksession_attach(session, "menu_bar_file",  session_sub_file);

  gtk_grid_attach(
    GTK_GRID(icksession_readw(session, "master_grid")),
    icksession_readw(session, "menu_bar"),
    session_menu_pos.x, session_menu_pos.y,
    session_menu_len.x, session_menu_len.y
  );
  gtk_widget_set_hexpand(icksession_readw(session, "menu_bar"), TRUE);
  icksession_menu_func(session);
}

void icksession_close_menu(icksession_t *session) {
  ickout(ICKOUT_LOG, "process[%p] shutdown - requested -> session_menu\n", session);
  if (session == NULL) { return; }
  icknode_smash(icksession_read(session, "menu_bar_items"));
  icknode_smash(icksession_read(session, "menu_bar_tree"));
  icknode_smash(icksession_read(session, "menu_bar_file"));
  ickout(ICKOUT_LOG, "process[%p] shutdown - completed -> session_menu\n", session);
}
