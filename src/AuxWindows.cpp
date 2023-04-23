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

#include "AuxWindows.h"

AuxWindows::AuxWindows(MainWindow *Mw)
{
  mw = Mw;
}

AuxWindows::~AuxWindows()
{
  // TODO Auto-generated destructor stub
}

void
AuxWindows::ownKey()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_transient_for(*mw);
  window->set_name("ownKeyWin");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title(gettext("Key"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  window->set_child(*grid);

  Gtk::Label *label = Gtk::make_managed<Gtk::Label>();
  label->set_selectable(true);
  LibCommunist Lc;
  std::string key = Lc.getKeyFmSeed(mw->seed);
  label->set_text(Glib::ustring(key));
  label->set_margin(5);
  label->set_halign(Gtk::Align::CENTER);
  grid->attach(*label, 0, 0, 2, 1);

  Gtk::Button *copy = Gtk::make_managed<Gtk::Button>();
  copy->set_label(gettext("Copy"));
  copy->set_halign(Gtk::Align::CENTER);
  copy->set_margin(5);
  copy->set_name("applyButton");
  copy->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Glib::RefPtr<Gdk::Display> display = window->get_display();
  Glib::RefPtr<Gdk::Clipboard> clipboard = display->get_clipboard();
  copy->signal_clicked().connect([label, clipboard, window]
  {
    clipboard->set_text(label->get_text());
    window->close();
  });
  grid->attach(*copy, 0, 1, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_label(gettext("Close"));
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_name("rejectButton");
  close->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  close->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*close, 1, 1, 1, 1);
  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);

  window->show();
}

void
AuxWindows::friendDetails(Gtk::Button *button)
{
  mw->frvectmtx.lock();
  auto itfr = std::find_if(mw->friendvect.begin(), mw->friendvect.end(),
			   [button]
			   (auto &el)
			     {
			       return std::get<0> (el) == button;
			     });
  if(itfr != mw->friendvect.end())
    {
      std::string key = std::string(std::get<2>(*itfr)->get_text());
      mw->contmtx.lock();
      auto itcont = std::find_if(mw->contacts.begin(), mw->contacts.end(),
				 [&key]
				 (auto &el)
				   {
				     return std::get<1>(el) == key;
				   });
      if(itcont != mw->contacts.end())
	{
	  std::string friendkey;
	  friendkey = std::get<1>(*itcont);
	  std::string homepath;
	  AuxFunc af;
	  af.homePath(&homepath);
	  LibCommunist Lc;
	  std::vector<std::tuple<std::string, std::vector<char>>> frprofvect;
	  frprofvect = Lc.readFriendProfile(homepath, friendkey, mw->Username,
					    mw->Password);
	  std::vector<char> *picture = new std::vector<char>;
	  auto itprv = std::find_if(frprofvect.begin(), frprofvect.end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Avatar";
	    });
	  if(itprv != frprofvect.end())
	    {
	      *picture = std::get<1>(*itprv);
	    }

	  Gtk::Window *window = new Gtk::Window;
	  itprv = std::find_if(frprofvect.begin(), frprofvect.end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Nick";
	    });
	  if(itprv != frprofvect.end())
	    {
	      std::vector<char> val = std::get<1>(*itprv);
	      std::string line;
	      std::copy(val.begin(), val.end(), std::back_inserter(line));
	      window->set_title(Glib::ustring(line));
	    }
	  window->set_application(mw->get_application());
	  window->set_transient_for(*mw);
	  window->set_name("settingsWindow");
	  window->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
	  grid->set_halign(Gtk::Align::CENTER);
	  window->set_child(*grid);

	  Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea>();
	  drar->set_margin(5);
	  drar->set_halign(Gtk::Align::CENTER);
	  drar->set_size_request(200, 200);
	  grid->attach(*drar, 0, 0, 1, 4);
	  MainWindow *mwl = mw;
	  if(picture->size() > 0)
	    {
	      drar->set_draw_func(
		  sigc::bind(sigc::mem_fun(*mwl, &MainWindow::on_draw_frpd),
			     picture));
	    }
	  else
	    {
	      delete picture;
	    }

	  Gtk::Label *lab1 = Gtk::make_managed<Gtk::Label>();
	  lab1->set_text(gettext("Key: "));
	  lab1->set_halign(Gtk::Align::START);
	  lab1->set_margin(5);
	  grid->attach(*lab1, 1, 0, 1, 1);

	  Gtk::Label *lab2 = Gtk::make_managed<Gtk::Label>();
	  lab2->set_text(gettext("Nickname: "));
	  lab2->set_halign(Gtk::Align::START);
	  lab2->set_margin(5);
	  grid->attach(*lab2, 1, 1, 1, 1);

	  Gtk::Label *lab3 = Gtk::make_managed<Gtk::Label>();
	  lab3->set_text(gettext("Name: "));
	  lab3->set_halign(Gtk::Align::START);
	  lab3->set_margin(5);
	  grid->attach(*lab3, 1, 2, 1, 1);

	  Gtk::Label *lab4 = Gtk::make_managed<Gtk::Label>();
	  lab4->set_text(gettext("Surname: "));
	  lab4->set_halign(Gtk::Align::START);
	  lab4->set_margin(5);
	  grid->attach(*lab4, 1, 3, 1, 1);

	  Gtk::Label *keylab = Gtk::make_managed<Gtk::Label>();
	  keylab->set_markup("<i>" + Glib::ustring(key) + "</i>");
	  keylab->set_halign(Gtk::Align::START);
	  keylab->set_margin(5);
	  keylab->set_name("friendDetLab");
	  keylab->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  keylab->set_selectable(true);
	  grid->attach(*keylab, 2, 0, 1, 1);

	  Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label>();
	  itprv = std::find_if(frprofvect.begin(), frprofvect.end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Nick";
	    });
	  if(itprv != frprofvect.end())
	    {
	      std::vector<char> val = std::get<1>(*itprv);
	      std::string line;
	      std::copy(val.begin(), val.end(), std::back_inserter(line));
	      nicklab->set_markup("<i>" + Glib::ustring(line) + "</i>");
	    }
	  nicklab->set_halign(Gtk::Align::START);
	  nicklab->set_margin(5);
	  nicklab->set_name("friendDetLab");
	  nicklab->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  nicklab->set_selectable(true);
	  grid->attach(*nicklab, 2, 1, 1, 1);

	  Gtk::Label *namelab = Gtk::make_managed<Gtk::Label>();
	  itprv = std::find_if(frprofvect.begin(), frprofvect.end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Name";
	    });
	  if(itprv != frprofvect.end())
	    {
	      std::vector<char> val = std::get<1>(*itprv);
	      std::string line;
	      std::copy(val.begin(), val.end(), std::back_inserter(line));
	      namelab->set_markup("<i>" + Glib::ustring(line) + "</i>");
	    }
	  namelab->set_halign(Gtk::Align::START);
	  namelab->set_margin(5);
	  namelab->set_name("friendDetLab");
	  namelab->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  namelab->set_selectable(true);
	  grid->attach(*namelab, 2, 2, 1, 1);

	  Gtk::Label *surnamelab = Gtk::make_managed<Gtk::Label>();
	  itprv = std::find_if(frprofvect.begin(), frprofvect.end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Surname";
	    });
	  if(itprv != frprofvect.end())
	    {
	      std::vector<char> val = std::get<1>(*itprv);
	      std::string line;
	      std::copy(val.begin(), val.end(), std::back_inserter(line));
	      surnamelab->set_markup("<i>" + Glib::ustring(line) + "</i>");
	    }
	  surnamelab->set_halign(Gtk::Align::START);
	  surnamelab->set_margin(5);
	  surnamelab->set_name("friendDetLab");
	  surnamelab->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  surnamelab->set_selectable(true);
	  grid->attach(*surnamelab, 2, 3, 1, 1);

	  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
	  close->set_label(gettext("Close"));
	  close->set_halign(Gtk::Align::CENTER);
	  close->set_margin(5);
	  close->set_name("applyButton");
	  close->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  close->signal_clicked().connect([window]
	  {
	    window->close();
	  });
	  grid->attach_next_to(*close, *drar, Gtk::PositionType::BOTTOM, 3, 1);

	  window->signal_close_request().connect([window]
	  {
	    window->hide();
	    delete window;
	    return true;
	  },
						 false);
	  window->show();
	}
      mw->contmtx.unlock();
    }
  mw->frvectmtx.unlock();
}

void
AuxWindows::attachFileDialog()
{
  if(mw->selectedc != nullptr)
    {
      Glib::RefPtr<Gtk::FileChooserNative> fcd = Gtk::FileChooserNative::create(
	  gettext("File selection"), *mw, Gtk::FileChooser::Action::OPEN,
	  gettext("Open"), gettext("Cancel"));
      std::string filename;
      AuxFunc af;
      af.homePath(&filename);
      Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
      fcd->set_current_folder(fl);
      MainWindow *mwl = mw;
      fcd->signal_response().connect(
	  sigc::bind(sigc::mem_fun(*this, &AuxWindows::attachFileFunc), fcd,
		     mwl));

      fcd->show();
    }
}

void
AuxWindows::attachFileFunc(int rid, Glib::RefPtr<Gtk::FileChooserNative> fcd,
			   MainWindow *mwl)
{
  if(rid == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fcd->get_file();
      std::string filename = fl->get_path();
      mwl->frvectmtx.lock();
      auto itfr = std::find_if(mwl->friendvect.begin(), mwl->friendvect.end(),
			       [mwl]
			       (auto &el)
				 {
				   return std::get<0>(el) == mwl->selectedc;
				 });
      if(itfr != mwl->friendvect.end())
	{
	  std::string key(std::get<2>(*itfr)->get_text());
	  if(mwl->Rightgrid != nullptr && mwl->attachfbutton != nullptr
	      && mwl->attachedfile == nullptr)
	    {
	      mwl->attach = Gtk::make_managed<Gtk::Label>();
	      mwl->attach->set_text(gettext(" File: "));
	      mwl->attach->set_halign(Gtk::Align::START);
	      mwl->attach->set_margin(5);
	      mwl->attach->set_name("highlightText");
	      mwl->attach->get_style_context()->add_provider(
		  mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      mwl->Rightgrid->insert_next_to(*(mwl->attachfbutton),
					     Gtk::PositionType::BOTTOM);
	      mwl->Rightgrid->attach_next_to(*(mwl->attach),
					     *(mwl->attachfbutton),
					     Gtk::PositionType::BOTTOM, 1, 1);

	      mwl->attachedfile = Gtk::make_managed<Gtk::Label>();
	      mwl->attachedfile->set_text(Glib::ustring(filename));
	      mwl->attachedfile->set_halign(Gtk::Align::START);
	      mwl->attachedfile->set_margin(5);
	      mwl->attachedfile->set_ellipsize(Pango::EllipsizeMode::START);
	      mwl->attachedfile->set_name("fileAddress");
	      mwl->attachedfile->get_style_context()->add_provider(
		  mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      mwl->Rightgrid->insert_next_to(*(mwl->attachfbutton),
					     Gtk::PositionType::BOTTOM);
	      mwl->Rightgrid->attach_next_to(*(mwl->attachedfile),
					     *(mwl->attach),
					     Gtk::PositionType::RIGHT, 1, 1);

	      mwl->attachcancel = Gtk::make_managed<Gtk::Button>();
	      mwl->attachcancel->set_valign(Gtk::Align::CENTER);
	      mwl->attachcancel->set_halign(Gtk::Align::END);
	      mwl->attachcancel->set_margin(5);
	      mwl->attachcancel->set_name("cancelRepl");
	      mwl->attachcancel->get_style_context()->add_provider(
		  mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      mwl->attachcancel->set_icon_name("window-close-symbolic");
	      mwl->attachcancel->signal_clicked().connect([mwl]
	      {
		mwl->Rightgrid->remove(*(mwl->attachedfile));
		mwl->Rightgrid->remove(*(mwl->attachcancel));
		mwl->Rightgrid->remove(*(mwl->attach));
		mwl->attachedfile = nullptr;
		mwl->attachcancel = nullptr;
		mwl->attach = nullptr;
	      });
	      mwl->Rightgrid->attach_next_to(*(mwl->attachcancel),
					     *(mwl->attachedfile),
					     Gtk::PositionType::RIGHT, 1, 1);
	    }
	}
      mwl->frvectmtx.unlock();
    }
}

void
AuxWindows::ipv6Window()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_modal(true);
  window->set_transient_for(*mw);
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title(gettext("Choose IPv6"));

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_margin(5);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_justify(Gtk::Justification::FILL);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_text(gettext("Program cannot automatically identify machine's IPv6. "
			"Please choose manually. "
			"Available variants are: "));
  lab->set_max_width_chars(40);
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::ComboBoxText *cmbtxt = Gtk::make_managed<Gtk::ComboBoxText>();
  cmbtxt->set_halign(Gtk::Align::CENTER);
  cmbtxt->set_margin(5);
  mw->ipv6vectmtx.lock();
  for(size_t i = 0; i < mw->ipv6vect.size(); i++)
    {
      cmbtxt->append(Glib::ustring(mw->ipv6vect[i]));
    }
  mw->ipv6vectmtx.unlock();
  cmbtxt->set_active(0);
  grid->attach(*cmbtxt, 0, 1, 1, 1);

  Gtk::Button *choose = Gtk::make_managed<Gtk::Button>();
  choose->set_halign(Gtk::Align::CENTER);
  choose->set_margin(5);
  choose->set_name("applyButton");
  choose->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  choose->set_label(gettext("Choose"));
  grid->attach(*choose, 0, 2, 1, 1);

  choose->signal_clicked().connect([window]
  {
    window->close();
  });

  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size(rq1, rq2);
  window->set_size_request(rq2.get_width(), rq2.get_height());

  MainWindow *mwl = mw;

  window->signal_close_request().connect([window, cmbtxt, mwl]
  {
    std::string value(cmbtxt->get_active_text());
    value.erase(0, value.find(" ") + std::string(" ").size());
    mwl->oper->setIPv6(value);
    mwl->ipv6vectmtx.lock();
    mwl->ipv6vect.clear();
    mwl->ipv6vectmtx.unlock();
    window->hide();
    delete window;
    return true;
  },
					 false);

  window->show();
}

void
AuxWindows::ipv4Window()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_modal(true);
  window->set_transient_for(*mw);
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title(gettext("Choose IPv4"));

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_justify(Gtk::Justification::FILL);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_text(gettext("Program cannot automatically identify machine's IPv4. "
			"Please choose manually. "
			"Available variants are: "));
  lab->set_max_width_chars(40);
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::ComboBoxText *cmbtxt = Gtk::make_managed<Gtk::ComboBoxText>();
  cmbtxt->set_halign(Gtk::Align::CENTER);
  cmbtxt->set_margin(5);
  mw->ipv6vectmtx.lock();
  for(size_t i = 0; i < mw->ipv4vect.size(); i++)
    {
      cmbtxt->append(Glib::ustring(mw->ipv4vect[i]));
    }
  mw->ipv6vectmtx.unlock();
  cmbtxt->set_active(0);
  grid->attach(*cmbtxt, 0, 1, 1, 1);

  Gtk::Button *choose = Gtk::make_managed<Gtk::Button>();
  choose->set_halign(Gtk::Align::CENTER);
  choose->set_margin(5);
  choose->set_name("applyButton");
  choose->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  choose->set_label(gettext("Choose"));
  grid->attach(*choose, 0, 2, 1, 1);

  choose->signal_clicked().connect([window]
  {
    window->close();
  });

  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size(rq1, rq2);
  window->set_size_request(rq2.get_width(), rq2.get_height());

  MainWindow *mwl = mw;

  window->signal_close_request().connect([window, cmbtxt, mwl]
  {
    std::string value(cmbtxt->get_active_text());
    value.erase(0, value.find(" ") + std::string(" ").size());
    mwl->oper->setIPv4(value);
    mwl->ipv4vectmtx.lock();
    mwl->ipv4vect.clear();
    mwl->ipv4vectmtx.unlock();
    window->hide();
    delete window;
    return true;
  },
					 false);

  window->show();
}

void
AuxWindows::editAddFriends()
{
  AuxFunc af;
  std::string homepath;
  af.homePath(&homepath);
  LibCommunist Lc;
  mw->proffopmtx.lock();
  std::vector<std::tuple<std::string, std::vector<char>>> profvect;
  profvect = Lc.readProfile(homepath, mw->Username, mw->Password);
  mw->addfrmtx.lock();
  mw->Addfriends = Lc.readRequestList(homepath, mw->Username, mw->Password);
  mw->addfrmtx.unlock();
  mw->proffopmtx.unlock();
  Gtk::Window *window = new Gtk::Window;
  window->set_title(gettext("Friends request edit"));
  window->set_application(mw->get_application());
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::ScrolledWindow *scrll = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrll->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
  scrll->set_min_content_height(200);
  Gtk::Label l;
  std::array<char, 32> seed;
  seed = Lc.seedGenerate();
  std::string txt = Lc.getKeyFmSeed(seed);
  l.set_text(Glib::ustring(txt));
  Gtk::Requisition rq1, rq2;
  l.get_preferred_size(rq1, rq2);
  Gtk::Grid *contgr = Gtk::make_managed<Gtk::Grid>();
  scrll->set_child(*contgr);
  scrll->set_size_request(rq2.get_width(), -1);
  grid->attach(*scrll, 0, 0, 2, 1);

  std::vector<Gtk::Label*> *labvect = new std::vector<Gtk::Label*>;
  MainWindow *mwl = mw;
  mw->addfrmtx.lock();
  for(size_t i = 0; i < mw->Addfriends.size(); i++)
    {

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_text(Glib::ustring(mw->Addfriends[i]));
      lab->set_margin(5);
      lab->set_name("blklistInactive");
      lab->set_halign(Gtk::Align::START);
      lab->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
      clck->set_button(0);
      clck->signal_pressed().connect(
	  sigc::bind(sigc::mem_fun(*this, &AuxWindows::addFriendsSelected),
		     labvect, lab, clck, contgr, mwl));
      lab->add_controller(clck);
      contgr->attach(*lab, 0, i, 1, 1);
      labvect->push_back(lab);
    }
  mw->addfrmtx.unlock();

  Gtk::Button *remove = Gtk::make_managed<Gtk::Button>();
  remove->set_label(gettext("Remove from list"));
  remove->set_halign(Gtk::Align::CENTER);
  remove->set_margin(5);
  remove->set_name("clearButton");
  remove->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  remove->signal_clicked().connect([mwl, contgr, labvect]
  {
    if(mwl->activereq != nullptr)
      {
	AuxFunc af;
	std::string homepath;
	af.homePath(&homepath);
	std::string key = std::string(mwl->activereq->get_text());

	mwl->addfrmtx.lock();
	mwl->Addfriends.erase(std::remove(mwl->Addfriends.begin(), mwl->Addfriends.end(), key), mwl->Addfriends.end());
	if(mwl->oper != nullptr)
	  {
	    mwl->oper->removeFriend(key);
	  }
	mwl->addfrmtx.unlock();
	labvect->erase(std::remove_if(labvect->begin(), labvect->end(), [&key]
	(auto &el)
	  {
	    std::string elt = el->get_text();
	    if(elt == key)
	      {
		return true;
	      }
	    else
	      {
		return false;
	      }
	  }),
		       labvect->end());
	contgr->remove(*(mwl->activereq));
	mwl->proffopmtx.unlock();
	mwl->activereq = nullptr;
      }
  });
  grid->attach(*remove, 0, 1, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_label(gettext("Close"));
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_name("rejectButton");
  cancel->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 1, 1, 1, 1);

  window->signal_close_request().connect([window, labvect]
  {
    window->hide();
    delete window;
    delete labvect;
    return true;
  },
					 false);

  window->show();
}

void
AuxWindows::addFriendsSelected(int n_press, double x, double y,
			       std::vector<Gtk::Label*> *labvect,
			       Gtk::Label *lab,
			       Glib::RefPtr<Gtk::GestureClick> clck,
			       Gtk::Grid *contgr, MainWindow *mwl)
{
  std::string filename;
  std::filesystem::path filepath;
  AuxFunc af;
  mwl->activereq = lab;
  if(clck->get_current_button() == 1)
    {
      for(size_t i = 0; i < labvect->size(); i++)
	{
	  labvect->at(i)->set_name("blklistInactive");
	}
      lab->set_name("blklistActive");
    }
  if(clck->get_current_button() == 3)
    {
      for(size_t i = 0; i < labvect->size(); i++)
	{
	  labvect->at(i)->set_name("blklistInactive");
	}
      lab->set_name("blklistActive");
      Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
      menu->append(gettext("Remove from list"), "popup.remove");
      Gtk::PopoverMenu *Menu = Gtk::make_managed<Gtk::PopoverMenu>();
      Menu->set_parent(*lab);
      Menu->set_menu_model(menu);
      Menu->set_has_arrow(false);
      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
	  Gio::SimpleActionGroup::create();
      acgroup->add_action("remove", [contgr, labvect, lab, mwl]
      {
	AuxFunc af;
	std::string homepath;
	af.homePath(&homepath);
	std::string key(lab->get_text());
	mwl->addfrmtx.lock();
	mwl->Addfriends.erase(std::remove(mwl->Addfriends.begin(), mwl->Addfriends.end(), key), mwl->Addfriends.end());
	mwl->addfrmtx.unlock();
	labvect->erase(std::remove_if(labvect->begin(), labvect->end(), [lab]
	(auto &el)
	  {
	    return lab == el;
	  }),
		       labvect->end());
	contgr->remove(*lab);
	mwl->activereq = nullptr;
	if(mwl->oper != nullptr)
	  {
	    mwl->oper->removeFriend(key);
	  }
	mwl->proffopmtx.unlock();
      }
      );
      lab->insert_action_group("popup", acgroup);
      const Gdk::Rectangle rect(x, y, 1, 1);
      Menu->set_pointing_to(rect);
      Menu->popup();
    }
}

void
AuxWindows::addFriends()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title(gettext("Add contact"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Public key:"));
  lab->set_halign(Gtk::Align::START);
  lab->set_margin(5);
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Entry *otherkey = Gtk::make_managed<Gtk::Entry>();
  otherkey->set_margin(5);
  otherkey->set_halign(Gtk::Align::CENTER);
  otherkey->set_width_chars(64);
  grid->attach(*otherkey, 0, 1, 2, 1);

  Gtk::Button *accept = Gtk::make_managed<Gtk::Button>();
  accept->set_label(gettext("Add"));
  accept->set_halign(Gtk::Align::CENTER);
  accept->set_margin(5);
  accept->set_name("applyButton");
  accept->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  accept->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &AuxWindows::addFriendsFunc), window,
		 otherkey, mw));
  grid->attach(*accept, 0, 2, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_label(gettext("Cancel"));
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_name("rejectButton");
  cancel->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 1, 2, 1, 1);

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);

  window->show();
}

void
AuxWindows::addFriendsFunc(Gtk::Window *window, Gtk::Entry *entry,
			   MainWindow *mwl)
{
  std::string key(entry->get_text());
  window->close();
  if(key.size() == 64)
    {
      AuxFunc af;
      std::string homepath;
      af.homePath(&homepath);
      mwl->proffopmtx.lock();
      std::vector<std::tuple<std::string, std::vector<char>>> profvect;
      LibCommunist Lc;
      profvect = Lc.readProfile(homepath, mwl->Username, mwl->Password);
      mwl->addfrmtx.lock();
      mwl->Addfriends = Lc.readRequestList(homepath, mwl->Username,
					   mwl->Password);
      auto it = std::find(mwl->Addfriends.begin(), mwl->Addfriends.end(), key);
      if(it == mwl->Addfriends.end())
	{
	  mwl->Addfriends.push_back(key);
	  mwl->oper->getNewFriends(key);
	}
      mwl->addfrmtx.unlock();
      mwl->proffopmtx.unlock();
    }
}

void
AuxWindows::resendWindow(std::string othkey, std::filesystem::path msgpath)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Select contact"));
  window->set_transient_for(*mw);
  window->set_modal(true);
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  for(size_t i = 0; i < mw->friendvect.size(); i++)
    {
      if(std::get<0>(mw->friendvect[i]) != mw->selectedc)
	{
	  Gtk::Label *lab = std::get<4>(mw->friendvect[i]);
	  Glib::ustring labt = lab->get_text();
	  lab = Gtk::make_managed<Gtk::Label>();
	  lab->set_halign(Gtk::Align::CENTER);
	  lab->set_margin(5);
	  lab->set_text(labt);
	  grid->attach(*lab, 0, i, 1, 1);

	  Gtk::CheckButton *tgbtn = Gtk::make_managed<Gtk::CheckButton>();
	  tgbtn->set_halign(Gtk::Align::CENTER);
	  tgbtn->set_valign(Gtk::Align::CENTER);
	  tgbtn->set_active(false);
	  grid->attach(*tgbtn, 1, i, 1, 1);

	  std::tuple<std::string, Gtk::CheckButton*> ttup;
	  lab = std::get<2>(mw->friendvect[i]);
	  std::get<0>(ttup) = std::string(lab->get_text());
	  std::get<1>(ttup) = tgbtn;
	  mw->resendactive.push_back(ttup);
	}
    }
  int last = mw->friendvect.size();

  Gtk::Button *conf = Gtk::make_managed<Gtk::Button>();
  conf->set_halign(Gtk::Align::CENTER);
  conf->set_margin(5);
  conf->set_label(gettext("Confirm"));
  conf->set_name("applyButton");
  conf->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  MainWindow *mwl = mw;
  conf->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &AuxWindows::confResend), window, othkey,
		 msgpath, mwl));
  grid->attach(*conf, 0, last, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_name("rejectButton");
  cancel->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect([window]
  {
    window->close();
  });
  grid->attach(*cancel, 1, last, 1, 1);

  window->signal_close_request().connect([window, mwl]
  {
    mwl->resendactive.clear();
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->show();
}

void
AuxWindows::confResend(Gtk::Window *window, std::string othkey,
		       std::filesystem::path msgpath, MainWindow *mwl)
{
  if(mwl->resendactive.size() > 0)
    {
      LibCommunist Lc;
      std::string filename = Lc.openMessage(msgpath, othkey, mwl->Username,
					    mwl->Password);
      std::filesystem::path outpath = std::filesystem::u8path(filename);
      std::string chstr;
      std::string nick = "";
      std::fstream f;
      std::vector<std::string> msgv;
      f.open(outpath, std::ios_base::in);
      while(!f.eof())
	{
	  getline(f, chstr);
	  if(chstr != "")
	    {
	      msgv.push_back(chstr);
	    }
	}
      f.close();
      std::filesystem::remove_all(outpath);
      if(msgv.size() > 7)
	{
	  chstr = msgv[1];
	  std::string::size_type n;
	  n = chstr.find("Resend from: ");
	  if(n != std::string::npos)
	    {
	      chstr.erase(0, n + std::string("Resend from: ").size());
	      nick = chstr;
	    }
	  else
	    {
	      mwl->frvectmtx.lock();
	      auto itfrv = std::find_if(
		  mwl->friendvect.begin(), mwl->friendvect.end(), [othkey]
		  (auto &el)
		    {
		      Gtk::Label *lab = std::get<2>(el);
		      return std::string(lab->get_text()) == othkey;
		    });
	      if(itfrv != mwl->friendvect.end())
		{
		  nick = std::string(std::get<4>(*itfrv)->get_text());
		}
	      mwl->frvectmtx.unlock();
	    }
	  chstr = msgv[4];
	  n = chstr.find("Type: ");
	  if(n != std::string::npos)
	    {
	      chstr.erase(n, std::string("Type: ").size());
	    }
	  if(chstr == "text message")
	    {
	      chstr = "0";
	    }
	  if(chstr == "file message")
	    {
	      chstr = "1";
	    }
	  for(size_t k = 0; k < mwl->resendactive.size(); k++)
	    {
	      std::string selkey = std::get<0>(mwl->resendactive[k]);
	      Gtk::CheckButton *chbtn = std::get<1>(mwl->resendactive[k]);
	      if(chbtn->get_active())
		{
		  if(chstr == "0")
		    {
		      mwl->oper->sendMessage(selkey, nick, "", msgv[7]);
		    }
		  if(chstr == "1")
		    {
		      chstr = msgv[7];
		      std::filesystem::path p = std::filesystem::u8path(chstr);
		      if(!std::filesystem::exists(p))
			{
			  Gtk::Window *errwin = new Gtk::Window;
			  errwin->set_application(mwl->get_application());
			  errwin->set_name("settingsWindow");
			  errwin->get_style_context()->add_provider(
			      mwl->css_provider,
			      GTK_STYLE_PROVIDER_PRIORITY_USER);
			  errwin->set_title(gettext("Error!"));
			  errwin->set_transient_for(*mwl);
			  errwin->set_modal(true);
			  Gtk::Grid *gr = Gtk::make_managed<Gtk::Grid>();
			  gr->set_halign(Gtk::Align::CENTER);
			  errwin->set_child(*gr);

			  Gtk::Label *errl = Gtk::make_managed<Gtk::Label>();
			  errl->set_halign(Gtk::Align::CENTER);
			  errl->set_margin(5);
			  errl->set_text(
			      gettext("File you want to send does not exists"));
			  gr->attach(*errl, 0, 0, 1, 1);

			  Gtk::Button *clb = Gtk::make_managed<Gtk::Button>();
			  clb->set_halign(Gtk::Align::CENTER);
			  clb->set_margin(5);
			  clb->set_name("clearButton");
			  clb->get_style_context()->add_provider(
			      mwl->css_provider,
			      GTK_STYLE_PROVIDER_PRIORITY_USER);
			  clb->set_label(gettext("Close"));
			  clb->signal_clicked().connect([errwin]
			  {
			    errwin->close();
			  });
			  gr->attach(*clb, 0, 1, 1, 1);

			  errwin->signal_close_request().connect([errwin]
			  {
			    errwin->hide();
			    delete errwin;
			    return true;
			  },
								 false);
			  errwin->show();
			  break;
			}
		      else
			{
			  mwl->oper->sendFile(selkey, nick, "", p.u8string());
			}
		      chstr = "1";
		    }
		}
	    }
	}
      window->close();
    }
}

void
AuxWindows::editRelayList()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Contacts to send by relay"));
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_transient_for(*mw);

  Gtk::Grid *wingr = Gtk::make_managed<Gtk::Grid>();
  wingr->set_halign(Gtk::Align::CENTER);
  wingr->set_valign(Gtk::Align::CENTER);
  window->set_child(*wingr);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_expand(true);
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);

  LibCommunist Lc;
  AuxFunc af;
  std::string homepath;
  af.homePath(&homepath);
  mw->proffopmtx.lock();
  std::vector<std::string> rellist;
  rellist = Lc.readRelayContacts(homepath, mw->Username, mw->Password);
  mw->proffopmtx.unlock();

  std::vector<std::tuple<Gtk::Label*, Gtk::CheckButton*>> *listv =
      new std::vector<std::tuple<Gtk::Label*, Gtk::CheckButton*>>;
  std::tuple<Gtk::Label*, Gtk::CheckButton*> ttup;

  Gtk::Label *nickh = Gtk::make_managed<Gtk::Label>();
  nickh->set_halign(Gtk::Align::CENTER);
  nickh->set_margin(5);
  nickh->set_text(gettext("Nick"));
  nickh->set_name("headerLab");
  nickh->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*nickh, 0, 0, 1, 1);

  Gtk::Label *keylh = Gtk::make_managed<Gtk::Label>();
  keylh->set_halign(Gtk::Align::CENTER);
  keylh->set_margin(5);
  keylh->set_text(gettext("Key"));
  keylh->set_name("headerLab");
  keylh->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*keylh, 1, 0, 1, 1);

  Gtk::Label *status = Gtk::make_managed<Gtk::Label>();
  status->set_halign(Gtk::Align::CENTER);
  status->set_margin(5);
  status->set_text(gettext("Use relay"));
  status->set_name("headerLab");
  status->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*status, 2, 0, 1, 1);

  int finished_on = 0;
  mw->frvectmtx.lock();
  for(size_t i = 0; i < mw->friendvect.size(); i++)
    {
      std::tuple<Gtk::Label*, Gtk::CheckButton*> ttup;
      Gtk::Label *nick = Gtk::make_managed<Gtk::Label>();
      nick->set_halign(Gtk::Align::CENTER);
      nick->set_margin(3);
      nick->set_text(std::get<4>(mw->friendvect[i])->get_text());
      grid->attach(*nick, 0, i + 1, 1, 1);

      Gtk::Label *keyl = Gtk::make_managed<Gtk::Label>();
      keyl->set_halign(Gtk::Align::CENTER);
      keyl->set_margin(3);
      keyl->set_text(std::get<2>(mw->friendvect[i])->get_text());
      keyl->set_ellipsize(Pango::EllipsizeMode::END);
      keyl->set_max_width_chars(20);
      keyl->set_width_chars(20);
      grid->attach(*keyl, 1, i + 1, 1, 1);
      std::get<0>(ttup) = keyl;

      std::string key = std::string(keyl->get_text());
      Gtk::CheckButton *chb = Gtk::make_managed<Gtk::CheckButton>();
      chb->set_halign(Gtk::Align::CENTER);
      chb->set_valign(Gtk::Align::CENTER);
      auto it = std::find(rellist.begin(), rellist.end(), key);
      if(it != rellist.end())
	{
	  chb->set_active(true);
	}
      else
	{
	  chb->set_active(false);
	}
      grid->attach(*chb, 2, i + 1, 1, 1);
      std::get<1>(ttup) = chb;
      listv->push_back(ttup);
      finished_on = i + 1;
    }
  mw->frvectmtx.unlock();

  mw->addfrmtx.lock();
  for(size_t i = 0; i < mw->Addfriends.size(); i++)
    {
      finished_on++;
      std::tuple<Gtk::Label*, Gtk::CheckButton*> ttup;
      Gtk::Label *nick = Gtk::make_managed<Gtk::Label>();
      nick->set_halign(Gtk::Align::CENTER);
      nick->set_margin(3);
      nick->set_text("");
      grid->attach(*nick, 0, finished_on, 1, 1);

      Gtk::Label *keyl = Gtk::make_managed<Gtk::Label>();
      keyl->set_halign(Gtk::Align::CENTER);
      keyl->set_margin(3);
      keyl->set_text(Glib::ustring(mw->Addfriends[i]));
      keyl->set_ellipsize(Pango::EllipsizeMode::END);
      keyl->set_max_width_chars(20);
      keyl->set_width_chars(20);
      grid->attach(*keyl, 1, finished_on, 1, 1);
      std::get<0>(ttup) = keyl;

      std::string key = std::string(keyl->get_text());
      Gtk::CheckButton *chb = Gtk::make_managed<Gtk::CheckButton>();
      chb->set_halign(Gtk::Align::CENTER);
      chb->set_valign(Gtk::Align::CENTER);
      auto it = std::find(rellist.begin(), rellist.end(), key);
      if(it != rellist.end())
	{
	  chb->set_active(true);
	}
      else
	{
	  chb->set_active(false);
	}
      grid->attach(*chb, 2, finished_on, 1, 1);
      std::get<1>(ttup) = chb;
      listv->push_back(ttup);
    }
  mw->addfrmtx.unlock();
  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size(rq1, rq2);
  Gdk::Rectangle winsz = mw->screenRes();
  if(rq2.get_width() > winsz.get_width() * 0.75)
    {
      scrl->set_min_content_width(winsz.get_width() * 0.75);
    }
  else
    {
      scrl->set_min_content_width(rq2.get_width());
    }
  if(rq2.get_height() > winsz.get_height() * 0.75)
    {
      scrl->set_min_content_height(winsz.get_height() * 0.75);
    }
  else
    {
      scrl->set_min_content_height(rq2.get_height());
    }
  scrl->set_child(*grid);

  wingr->attach(*scrl, 0, 0, 2, 1);

  Gtk::Button *apply = Gtk::make_managed<Gtk::Button>();
  apply->set_margin(5);
  apply->set_halign(Gtk::Align::CENTER);
  apply->set_label(gettext("Apply"));
  apply->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  apply->set_name("applyButton");
  apply->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &AuxWindows::editRelayListFunc), mw,
		 listv, window));
  wingr->attach(*apply, 0, 1, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_label(gettext("Close"));
  close->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  close->set_name("rejectButton");
  close->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  wingr->attach(*close, 1, 1, 1, 1);

  window->signal_close_request().connect([window, listv]
  {
    listv->clear();
    delete listv;
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->show();
}

void
AuxWindows::editRelayListFunc(
    MainWindow *mwl,
    std::vector<std::tuple<Gtk::Label*, Gtk::CheckButton*>> *listv,
    Gtk::Window *window)
{
  std::vector<std::string> relvect;
  for(size_t i = 0; i < listv->size(); i++)
    {
      Gtk::CheckButton *chb = std::get<1>(listv->at(i));
      if(chb->get_active())
	{
	  Gtk::Label *lab = std::get<0>(listv->at(i));
	  std::string key = std::string(lab->get_text());
	  relvect.push_back(key);
	}
    }
  if(mwl->oper)
    {
      mwl->oper->editContByRelay(relvect);
    }
  window->close();
}

void
AuxWindows::infoMessage(Glib::ustring username, Glib::ustring password,
			Glib::ustring reppassword)
{
  Gtk::Window *window = new Gtk::Window;
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);

  Gtk::Label *label = Gtk::make_managed<Gtk::Label>();
  if(username == "")
    {
      label->set_text(gettext("Input user name!"));
    }
  if(password == "" && username != "")
    {
      label->set_text(gettext("Input password!"));
    }
  if(password != reppassword && username != "" && password != "")
    {
      label->set_text(gettext("Passwords do not match!"));
    }
  label->set_margin(5);
  label->set_halign(Gtk::Align::CENTER);
  grid->attach(*label, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_label(gettext("Close"));
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("applyButton");
  close->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  close->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*close, 0, 1, 1, 1);

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);
  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size(rq1, rq2);
  window->set_default_size(rq2.get_width(), -1);
  Glib::RefPtr<Gtk::Application> app = mw->get_application();
  app->add_window(*window);
  window->show();
}
