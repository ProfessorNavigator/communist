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

#ifndef SRC_SETTINGSWINDOW_H_
#define SRC_SETTINGSWINDOW_H_

#include <gtkmm.h>
#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <filesystem>
#include <sigc++/sigc++.h>
#include "MainWindow.h"
#include "AuxFunc.h"

class MainWindow;

class SettingsWindow
{
public:
  SettingsWindow (MainWindow *Mw);
  virtual
  ~SettingsWindow ();
  void
  settingsWindow ();
private:
  void
  saveSettings (Gtk::Window *window, Gtk::Entry *themepathval,
		Gtk::Entry *lifcsval, Gtk::Entry *locip6val,
		Gtk::Entry *locip4val, Gtk::Entry *bootstre, Gtk::Entry *msgsze,
		Gtk::Entry *partsze, Gtk::CheckButton *winszch,
		Gtk::Entry *msgwchre, Gtk::ComboBoxText *sendkeycmb,
		Gtk::CheckButton *soundch, Gtk::CheckButton *grnotificch,
		Gtk::Entry *soundpe, Gtk::Entry *shutmte, Gtk::Entry *tmtteare,
		Gtk::CheckButton *langchenbch, Gtk::CheckButton *hole_punchch,
		Gtk::CheckButton *enstunch, Gtk::Entry *stunpe,
		Gtk::CheckButton *enrelch, Gtk::Entry *relaype,
		Gtk::Entry *relaypathe, Gtk::CheckButton *directinetch,
		MainWindow *mwl);
  MainWindow *mw = nullptr;
};

#endif /* SRC_SETTINGSWINDOW_H_ */
