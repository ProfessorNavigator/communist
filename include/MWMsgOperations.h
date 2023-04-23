/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

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

#ifndef SRC_MWMSGOPERATIONS_H_
#define SRC_MWMSGOPERATIONS_H_

#include <gtkmm.h>
#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <sstream>
#include <mutex>
#include "MainWindow.h"

class MainWindow;

class MWMsgOperations
{
  friend class ContactOperations;
public:
  MWMsgOperations(MainWindow *Mw);
  virtual
  ~MWMsgOperations();
  void
  sendMsg(Glib::ustring ustxt);
  void
  msgRcvdSlot(std::string *key, std::filesystem::path *p,
              std::mutex *disp3mtx);
  void
  removeMsg(std::string othkey, std::filesystem::path msgpath,
            Gtk::Widget *widg);
  void
  msgSentSlot(std::string *key, std::filesystem::path *msgind,
              std::mutex *disp2mtx);
private:
  MainWindow *mw = nullptr;
  void
  creatReply(int numcl, double x, double y, Gtk::Frame *fr,
             Gtk::Label *message, Glib::RefPtr<Gtk::GestureClick> clck,
             MainWindow *mwl);
  void
  removeMsgFunc(Gtk::Window *window, MainWindow *mwl,
                std::string othkey,
                std::filesystem::path msgpath, Gtk::Widget *widg);
};

#endif /* SRC_MWMSGOPERATIONS_H_ */
