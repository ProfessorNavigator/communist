/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of Communist.
 Communist is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 Communist is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with Communist. If not,
 see <https://www.gnu.org/licenses/>.
 */
#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <gtkmm.h>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include <functional>
#include <sigc++/sigc++.h>
#include <ctime>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <tuple>
#include <hunspell.hxx>
#include <libintl.h>
#include <LibCommunist.h>
#include "SettingsWindow.h"
#include "ContactOperations.h"
#include "AuxWindows.h"
#include "MWMsgOperations.h"
#include "MWFileOperations.h"

class SettingsWindow;
class ContactOperations;
class AuxWindows;
class MWMsgOperations;

class MainWindow : public Gtk::ApplicationWindow
{
  friend class SettingsWindow;
  friend class ContactOperations;
  friend class AuxWindows;
  friend class MWMsgOperations;
  friend class MWFileOperations;
public:
  MainWindow();
  virtual
  ~MainWindow();
private:
  void
  userCheck();
  void
  mainWindow();
  void
  infoMessage(Gtk::Entry *usname, Gtk::Entry *passwd,
              Gtk::Entry *reppasswd);
  void
  userCheckFun(Gtk::Entry *usname, Gtk::Entry *passwd);
  void
  createProfile();
  void
  keyGenerate(Gtk::Entry *key);
  void
  openImage();
  void
  onopenImage(int respons_id,
              Glib::RefPtr<Gtk::FileChooserNative> dialog);
  void
  on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
          int height,
          Glib::ustring file);
  void
  on_draw_ep(const Cairo::RefPtr<Cairo::Context> &cr,
             int width, int height,
             std::vector<char> *picture);
  void
  on_draw_sb(const Cairo::RefPtr<Cairo::Context> &cr,
             int width, int height,
             Glib::ustring file);
  void
  on_draw_contactlist(const Cairo::RefPtr<Cairo::Context> &cr,
                      int width,
                      int height, Gtk::Label *keylab);
  void
  clearAvatar(Gtk::Grid *leftgrid);
  void
  saveProfile();
  void
  saveProfileEP(Gtk::Window *window);
  void
  ownKey();
  void
  selectContact(Gtk::ScrolledWindow *scr, std::string selkey,
                std::string selnick);
  void
  deleteContact();
  void
  networkOp();
  void
  addFriends();
  void
  editAddFriends();
  void
  addFriendSlot(std::string *key, int *conind,
                std::mutex *disp1mtx);
  void
  sendMsg(Gtk::TextView *tv);
  void
  sendMsgFile(std::string *txt);
  void
  editProfile();
  void
  attachFileDialog();
  void
  fileRequestSlot(std::string *key, uint64_t *tm,
                  uint64_t *filesize,
                  std::string *filename, std::mutex *disp4mtx);
  void
  fileRejectedSlot(std::string *keyr,
                   std::filesystem::path *filer,
                   std::mutex *disp5mtx);
  void
  fileRcvdStatus(std::string *key,
                 std::filesystem::path *filename,
                 std::mutex *dispNmtx, int variant);
  void
  friendDetails(Gtk::Button *button);
  void
  contMenu(int numcl, double x, double y,
           Glib::RefPtr<Gtk::GestureClick> clck,
           Gtk::Button *button);
  void
  on_draw_frpd(const Cairo::RefPtr<Cairo::Context> &cr,
               int width, int height,
               std::vector<char> *picture);
  void
  tempBlockCont();
  void
  settingsWindow();
  Gdk::Rectangle
  screenRes();
  void
  formIPv6vect(std::string ip);
  void
  formIPv4vect(std::string ip);
  void
  formMsgWinGrid(std::vector<std::filesystem::path> &msg,
                 size_t begin,
                 size_t end, Gtk::Grid *grid, std::string key,
                 std::string nick, int index, int varform);
  void
  removeMsg(std::string othkey, std::filesystem::path msgpath,
            Gtk::Widget *widg);
  void
  fileDownloadProg(std::string *key,
                   std::filesystem::path *filepath,
                   uint64_t *cursize, std::mutex *mtx);
  void
  fileSendProg(std::string *key,
               std::filesystem::path *filepath,
               uint64_t *cursize, std::mutex *mtx);
  void
  autoRemoveMsg();
  void
  aboutProg();
  void
  checkIfConnected(std::string *key, uint64_t *tm,
                   std::mutex *mtx);
  void
  editRelayList();
  bool
  editProfileClose(Gtk::Window *window);
  bool
  sendMsgByKeyBoard(guint keyval, guint keycode,
                    Gdk::ModifierType state);
  void
  spellCheck(Glib::RefPtr<Gtk::TextBuffer> textb);
  bool
  mainWindowClose();
  std::string Username;
  std::string Password;
  std::array<char, 32> seed;
  Gtk::DrawingArea *avatar = nullptr;
  Glib::RefPtr<Gdk::Pixbuf> image = nullptr;
  std::vector<std::pair<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>>>
  conavatars;
  std::mutex conavmtx;
  int Deleteprof = 1;
  int Image = 0;
  int msg_width_chars = 30;
  std::vector<Gtk::Entry*> profilevector;
  std::vector<std::tuple<int, std::string>> contacts;
  std::mutex contmtx;
  std::vector <
  std::tuple<Gtk::Button*, Gtk::Grid*, Gtk::Label*, Gtk::Label*,
      Gtk::Label*, Gtk::Label*, Gtk::Label*, Gtk::PopoverMenu* >>
      friendvect; //0-contact button, 1-buttongrid, 2-key label, 3-rcvd msg quantity label, 4-nick label, 5-name label, 6-blocked label, 7-contact menu
  std::mutex frvectmtx;
  std::vector<std::string> Addfriends;
  std::mutex addfrmtx;
  Gtk::Button *selectedc = nullptr;
  int contact_button_width = 0;
  Glib::RefPtr<Gtk::CssProvider> css_provider;
  Gtk::Grid *fordelgrid = nullptr;
  std::string fordelkey = "";
  Gtk::Label *activeblck = nullptr;
  Gtk::Label *activereq = nullptr;
  Gtk::ScrolledWindow *Winright = nullptr;
  Gtk::ScrolledWindow *Winleft = nullptr;
  Gtk::Paned *Mwpaned = nullptr;
  Gtk::Grid *msg_win_gr = nullptr;
  std::vector<std::tuple<Gtk::Widget*, std::filesystem::path, Gtk::PopoverMenu*>>
      msg_grid_vect;
  std::mutex msg_grid_vectmtx;
  NetOperationsComm *oper = nullptr;
  Gtk::Label *repllabe = nullptr;
  Gtk::Button *replcancel = nullptr;
  Gtk::Grid *replgrid = nullptr;
  Gtk::Grid *Rightgrid = nullptr;
  Gtk::TextView *msgfrm = nullptr;
  std::vector<std::tuple<std::string, std::filesystem::path, Gtk::Label*>>
      sentstatus;
  std::mutex sentstatmtx;
  std::vector<std::tuple<std::string, uint64_t, uint16_t, std::filesystem::path>>
      blockfreq; //key, time, status (waiting, rejected, accepted), path to file
  std::mutex blockfreqmtx;
  std::mutex yesmtx; //Contacts log mutex
  std::mutex proffopmtx; //Profile mutex
  Gtk::PopoverMenu *rightmenfordel = nullptr;
  Glib::Dispatcher msgwinadjdsp;
  double usermsgadj = -1;
  Gtk::Overlay *msgovl = nullptr;
  Gtk::Label *msgovllab = nullptr;
  Gtk::Label *attachedfile = nullptr;
  Gtk::Button *attachfbutton = nullptr;
  Gtk::Button *attachcancel = nullptr;
  Gtk::Label *attach = nullptr;
  std::vector<std::tuple<std::string, std::string>>
      prefvect; //0-key, 1-value
  std::mutex prefvectmtx;
  std::vector<std::string> ipv6vect;
  std::mutex ipv6vectmtx;
  std::vector<std::string> ipv4vect;
  std::mutex ipv4vectmtx;
  std::string Sharepath;
  std::string Userthemepath;
  sigc::connection Scrwinovershot;
  double msgwinadj = -1;
  std::vector<std::tuple<std::string, Gtk::CheckButton*>>
      resendactive;
  Gtk::Grid *dld_grid = nullptr;
  Gtk::Window *dld_win = nullptr;
  std::vector <
  std::tuple<std::string, std::filesystem::path, uint64_t,
      Gtk::ProgressBar*, Gtk::Label*, Gtk::Button* >>
      fileprogrvect; //0-key, 1-filepath, 2-file size, 3-Progress, 4-filename lab, 5-cancel button
  std::mutex fileprogrvectmtx;
  Glib::Dispatcher disp1;
  Glib::Dispatcher disp2;
  Glib::Dispatcher disp3;
  Glib::Dispatcher disp4;
  Glib::Dispatcher disp5;
  Glib::Dispatcher disp6;
  Glib::Dispatcher disp7;
  Glib::Dispatcher disp8;
  Glib::Dispatcher disp9;
  Glib::Dispatcher disp10;
  Glib::Dispatcher disp11;
  Glib::Dispatcher disp12;
  Glib::Dispatcher disp13;
  Glib::Dispatcher disp14;
  Glib::Dispatcher disp15;
  Glib::Dispatcher disp16;
  Glib::Dispatcher disp17;
  Glib::Dispatcher dispclose;

  Gtk::ProgressBar *friend_block_bar = nullptr;

  std::vector<std::tuple<std::string, int, int>>
      winszs; //0-Win code name, 1-width, 2-height
  std::vector<std::tuple<std::string, uint64_t, Gtk::Label*>>
      chifc; //0-key, 1-time, 2-indicator
  std::mutex chifcmtx;
  Glib::RefPtr<Gtk::MediaFile> mf;
  Hunspell *spch = nullptr;
  std::string Theme;
  Gtk::Window *Contdelwin = nullptr;
  SettingsWindow *Set_Win = nullptr;

  bool grnotific = true;
};

#endif /* MAINWINDOW_H_ */
