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
#ifndef COMMUNISTAPP_H_
#define COMMUNISTAPP_H_

#include <gtkmm.h>
#include "MainWindow.h"

class CommunistApp : public Gtk::Application
{
protected:
  CommunistApp();
public:
  static Glib::RefPtr<CommunistApp>
  create();
  virtual
  ~CommunistApp();
private:
  MainWindow*
  create_appwindow();
protected:
  void
  on_activate() override;
};

#endif /* COMMUNISTAPP_H_ */
