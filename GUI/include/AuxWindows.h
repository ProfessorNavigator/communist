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

#ifndef SRC_AUXWINDOWS_H_
#define SRC_AUXWINDOWS_H_

#include <gtkmm.h>
#include <filesystem>
#include <vector>
#include "MainWindow.h"

class MainWindow;

class AuxWindows
{
public:
  AuxWindows (MainWindow *Mw);
  virtual
  ~AuxWindows ();
  void
  ownKey ();
  void
  friendDetails (Gtk::Button *button);
  void
  attachFileDialog ();
  void
  ipv6Window ();
  void
  ipv4Window ();
  void
  editAddFriends ();
  void
  addFriends ();
  void
  resendWindow (std::string othkey, std::filesystem::path msgpath);
  void
  editRelayList ();
  void
  infoMessage (Glib::ustring username, Glib::ustring password,
	       Glib::ustring reppassword);
private:
  MainWindow *mw = nullptr;
  void
  attachFileFunc (int rid, Gtk::FileChooserDialog *fcd, MainWindow *mwl);
  void
  addFriendsSelected (int n_press, double x, double y,
		      std::vector<Gtk::Label*> *labvect, Gtk::Label *lab,
		      Glib::RefPtr<Gtk::GestureClick> clck, Gtk::Grid *contgr,
		      MainWindow *mwl);
  void
  addFriendsFunc (Gtk::Window *window, Gtk::Entry *entry, MainWindow *mwl);
  void
  confResend (Gtk::Window *window, std::string othkey,
	      std::filesystem::path msgpath, MainWindow *mwl);
  void
  editRelayListFunc (
      MainWindow *mwl,
      std::vector<std::tuple<Gtk::Label*, Gtk::CheckButton*>> *listv,
      Gtk::Window *window);
};

#endif /* SRC_AUXWINDOWS_H_ */
