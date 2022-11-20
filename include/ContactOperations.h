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

#ifndef SRC_CONTACTOPERATIONS_H_
#define SRC_CONTACTOPERATIONS_H_

#include <gtkmm.h>
#include <string>
#include <vector>
#include <tuple>
#include <mutex>
#include "MainWindow.h"
#include "MWMsgOperations.h"

class MainWindow;

class ContactOperations
{
public:
  ContactOperations(MainWindow *Mw);
  virtual
  ~ContactOperations();
  void
  selectContact(Gtk::ScrolledWindow *scr, std::string selkey,
                std::string selnick);
  void
  formMsgWinGrid(std::vector<std::filesystem::path> &msg,
                 size_t begin,
                 size_t end, Gtk::Grid *grid, std::string key,
                 std::string nick, int index, int varform);
  void
  deleteContact();
  void
  friendRemoved(std::string *key, std::mutex *mtx);
private:
  MainWindow *mw = nullptr;
  void
  deleteContactFunc(MainWindow *mwl);
};

#endif /* SRC_CONTACTOPERATIONS_H_ */
