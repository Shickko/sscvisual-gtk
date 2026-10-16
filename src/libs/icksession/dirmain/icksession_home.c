#include <dirmain/icksession_home.h>
#include <ickstorage.h>
#include <ickclock.h>
#include <ickutils.h>
#include <steamcurl.h>
#include <string.h>

int icksession_home_state = 0;

static void icksession_home_update(gpointer dt) {
  icksession_t *session = (icksession_t* )dt;
  steamcli_t   *client  = (steamcli_t* )icksession_read(session, "session_steamcli");
  ickclock_t   *clk     = (ickclock_t* )icksession_read(session, "clk_home");
  ickdict_t    *cfg     = (ickdict_t* )icksession_read(session, "configuration");
  if (client == NULL || clk == NULL || cfg == NULL)
  { return; }
  steamprofile_t usr_profile = steamcli_getplayer(
    client, steamcli_getauthid(client));
  const char* usr_status_str;
  char usrpicture_dir[512];

  switch (usr_profile.steamstatus) {
    /* funny thing you may notice here but apparently 
     * steamstatus = 2, 4 never triggers!
     * honestly i have no idea what's a difference
     * between an "Away" and a "Snooze"
     * also 5 ~ 6 are not implemented because
     * bare nobody uses those archived status
     *
     * but yes it's impressive that you can
     * literally set steam status manually
     * to those by 
     * g_FriendsUIApp.FriendStore.SetUserPersonaState(s) -> s = 5 | 6
     * in https://steamcommunity.com/chat
     *
     * anyways im just too lazy to make them, and nobody uses that...
     * */
    default : { usr_status_str = " [Unknown]";   break; }
    case (0): { usr_status_str = " [Offline]";   break; }
    case (1): { usr_status_str = " [Online]";    break; }
    case (2): { usr_status_str = " [Busy]";      break; }
    case (3): { usr_status_str = " [Away]";      break; }
    case (4): { usr_status_str = " [Snooze]";    break; }
  }

  gtk_label_set_text(GTK_LABEL(icksession_readw(session, "home_username")), usr_profile.steamname);
  gtk_label_set_text(GTK_LABEL(icksession_readw(session, "home_userstat")), usr_status_str);

  sprintf(usrpicture_dir, "%s.jpg", usr_profile.steamid);
  gtk_image_set_from_file (GTK_IMAGE(icksession_readw(session, "home_userpict")), usrpicture_dir);

  steamcli_putplayer(&usr_profile);
}

static void icksession_home_populate(icksession_t *session) {
  const char* home_var_placeholder        = "[...]";
  const char* home_label_title_str        = "User Profile";
  const ickpoint_t home_label_title_pos   = ickpoint(0,0);
  const ickpoint_t home_label_title_len   = ickpoint(3,1);
  GtkWidget *home_title = gtk_label_new(home_label_title_str);

  const char* home_label_usrname_str      = "Username: ";
  const ickpoint_t home_label_usrname_pos = ickpoint(0,1);
  const ickpoint_t home_label_usrname_len = ickpoint(1,1);
  const ickpoint_t home_var_usrname_pos   = ickpoint(1,1);
  const ickpoint_t home_var_usrname_len   = ickpoint(1,1);
  const ickpoint_t home_var_usrstat_pos   = ickpoint(2,1);
  const ickpoint_t home_var_usrstat_len   = ickpoint(1,1);
  const ickpoint_t home_var_usrpict_pos   = ickpoint(4,1);
  const ickpoint_t home_var_usrpict_len   = ickpoint(1,1);
  const ickpoint_t home_grid_pos          = ickpoint(0,1);
  const ickpoint_t home_grid_len          = ickpoint(1,1);
  const ickpoint_t home_grid_len_sub_top  = ickpoint(1,1);
  const ickpoint_t home_grid_len_sub_btm  = ickpoint(1,1);
  const ickpoint_t home_grid_len_sub_lft  = ickpoint(1,1);
  const ickpoint_t home_grid_len_sub_rgt  = ickpoint(1,1);
  const ickpoint_t home_grid_pos_sub_top  = ickpoint(0,0);
  const ickpoint_t home_grid_pos_sub_btm  = ickpoint(0,1);
  const ickpoint_t home_grid_pos_sub_lft  = ickpoint(0,0);
  const ickpoint_t home_grid_pos_sub_rgt  = ickpoint(1,0);
  GtkWidget *home_usrname_label = gtk_label_new(home_label_usrname_str);
  GtkWidget *home_usrname_var   = gtk_label_new(home_var_placeholder);
  GtkWidget *home_usrstat_var   = gtk_label_new(home_var_placeholder);
  GtkWidget *home_usrpict_var   = gtk_image_new();
  GtkWidget *home_grid          = gtk_grid_new();
  GtkWidget *home_grid_sub_top  = gtk_grid_new();
  GtkWidget *home_grid_sub_btm  = gtk_grid_new();
  GtkWidget *home_grid_sub_rgt  = gtk_grid_new();
  GtkWidget *home_grid_sub_lft  = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(home_grid),    2);
  gtk_grid_set_column_spacing(GTK_GRID(home_grid), 5);
  gtk_widget_set_hexpand(home_grid_sub_top, TRUE);
  gtk_widget_set_hexpand(home_grid_sub_btm, TRUE);

  icksession_attachgrid(session, home_grid_sub_top, home_title, home_label_title_pos, home_label_title_len);
  icksession_attachgrid(session, home_grid_sub_btm, home_usrname_label, home_label_usrname_pos, home_label_usrname_len);
  icksession_attachgrid(session, home_grid_sub_btm, home_usrname_var, home_var_usrname_pos, home_var_usrstat_len);
  icksession_attachgrid(session, home_grid_sub_btm, home_usrstat_var, home_var_usrstat_pos, home_var_usrstat_len);
  icksession_attachgrid(session, home_grid_sub_rgt, home_usrpict_var, home_var_usrpict_pos, home_var_usrpict_len);
  icksession_attachgrid(session, home_grid_sub_lft, home_grid_sub_btm, home_grid_pos_sub_btm, home_grid_len_sub_btm);
  icksession_attachgrid(session, home_grid_sub_lft, home_grid_sub_top, home_grid_pos_sub_top, home_grid_len_sub_top);
  icksession_attachgrid(session, home_grid, home_grid_sub_top, home_grid_pos_sub_top, home_grid_len_sub_top);
  icksession_attachgrid(session, home_grid, home_grid_sub_btm, home_grid_pos_sub_btm, home_grid_len_sub_btm);
  icksession_attachgrid(session, home_grid, home_grid_sub_lft, home_grid_pos_sub_lft, home_grid_len_sub_lft);
  icksession_attachgrid(session, home_grid, home_grid_sub_rgt, home_grid_pos_sub_rgt, home_grid_len_sub_rgt);
  icksession_attachgrid(session, icksession_readw(session, "master_grid"), home_grid, home_grid_pos, home_grid_len);

  icksession_attach(session, "home_username", home_usrname_var);
  icksession_attach(session, "home_userstat", home_usrstat_var);
  icksession_attach(session, "home_userpict", home_usrpict_var);
  gtk_widget_show_all(icksession_readw(session, "master_grid"));
}

void icksession_create_home(gpointer dt) {
  ickout(ICKOUT_LOG, "session: %p\n", dt);
  icksession_t *session = (icksession_t* )dt;
  if (icksession_home_state == 1) 
  { return; }
  icksession_home_state = 1;

  ickclock_t *clk = ickclock();
  ickclock_setmode(clk, CLOCK_TICK);
  ickclock_setinterval(clk, 2);
  if (ickclock_boot(clk) == ICKSUCCESS) {
    ickclock_listen(
        clk, 
        icksession_home_update, 
        session
    );
  }
  icksession_attach(session, "clk_home", clk);

  icksession_home_populate(session);
}

void icksession_close_home(gpointer dt) {
  if (icksession_home_state == 0) 
  { return; }
  icksession_home_state = 0;
  icksession_t *session = (icksession_t* )dt;
  ickclock_t *clk = (ickclock_t* )icksession_detach(session, "clk_home");
  if (clk) { 
    ickclock_smash(clk); 
  }
}
