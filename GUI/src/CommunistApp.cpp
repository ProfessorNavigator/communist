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
#include "CommunistApp.h"

CommunistApp::CommunistApp () :
    Gtk::Application ("ru.mail.bobilev_yury.Communist")
{
  // TODO Auto-generated constructor stub

}

CommunistApp::~CommunistApp ()
{
  // TODO Auto-generated destructor stub
}

Glib::RefPtr<CommunistApp>
CommunistApp::create ()
{
  return Glib::make_refptr_for_instance<CommunistApp> (new CommunistApp ());
}

MainWindow*
CommunistApp::create_appwindow ()
{
  MainWindow *mw = new MainWindow;
  this->add_window (*mw);
  mw->signal_hide ().connect ( [mw, this]
  {
    std::vector<Gtk::Window*> winv;
    winv = this->get_windows ();
    for (size_t i = 0; i < winv.size (); i++)
      {
	Gtk::Window *win = winv[i];
	if (win != mw)
	  {
	    win->hide ();
	    delete win;
	  }
      }
    delete mw;
  });

  return mw;
}

void
CommunistApp::on_activate ()
{
  std::vector<Gtk::Window*> chv;
  chv = this->get_windows ();
  if (chv.size () == 0)
    {
      auto appwin = create_appwindow ();
      appwin->present ();
    }
}
