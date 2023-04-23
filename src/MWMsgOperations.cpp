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

#include "MWMsgOperations.h"

MWMsgOperations::MWMsgOperations(MainWindow *Mw)
{
  mw = Mw;
}

MWMsgOperations::~MWMsgOperations()
{
  // TODO Auto-generated destructor stub
}

void
MWMsgOperations::sendMsg(Glib::ustring ustxt)
{
  Glib::ustring utxt = ustxt;
  if(utxt != "" || mw->attachedfile != nullptr)
    {
      Gtk::Requisition rq1, rq2;
      if(mw->msg_win_gr != nullptr)
	{
	  int rowcount = 0;
	  Gtk::Grid *grmsg = Gtk::make_managed<Gtk::Grid>();
	  Gtk::Frame *fr = Gtk::make_managed<Gtk::Frame>();
	  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
	  clck->set_button(0);
	  fr->set_name("myMsg");
	  fr->get_style_context()->add_provider(mw->css_provider,
	  GTK_STYLE_PROVIDER_PRIORITY_USER);
	  fr->set_child(*grmsg);
	  fr->set_margin(2);
	  fr->set_halign(Gtk::Align::FILL);
	  fr->add_controller(clck);

	  Gtk::Label *date = Gtk::make_managed<Gtk::Label>();
	  date->set_halign(Gtk::Align::START);
	  date->set_margin(2);
	  time_t curtm = time(NULL);
	  tm *gmtm = localtime(&curtm);
	  std::string datestr;
	  std::string day;
	  std::string month;
	  std::string year;
	  std::string hour;
	  std::string minut;
	  std::string second;

	  std::stringstream strm;
	  std::locale loc("C");
	  strm.imbue(loc);
	  strm << gmtm->tm_hour;
	  hour = strm.str();
	  if(gmtm->tm_hour < 10)
	    {
	      hour = "0" + hour;
	    }

	  strm.str("");
	  strm.clear();
	  strm.imbue(loc);
	  strm << gmtm->tm_min;
	  minut = strm.str();
	  if(gmtm->tm_min < 10)
	    {
	      minut = "0" + minut;
	    }

	  strm.str("");
	  strm.clear();
	  strm.imbue(loc);
	  strm << gmtm->tm_sec;
	  second = strm.str();
	  if(gmtm->tm_sec < 10)
	    {
	      second = "0" + second;
	    }

	  strm.str("");
	  strm.clear();
	  strm.imbue(loc);
	  strm << gmtm->tm_mday;
	  day = strm.str();
	  if(gmtm->tm_mday < 10)
	    {
	      day = "0" + day;
	    }

	  strm.str("");
	  strm.clear();
	  strm.imbue(loc);
	  strm << gmtm->tm_mon + 1;
	  month = strm.str();
	  if(gmtm->tm_mon < 10)
	    {
	      month = "0" + month;
	    }

	  strm.str("");
	  strm.clear();
	  strm.imbue(loc);
	  strm << gmtm->tm_year + 1900;
	  year = strm.str();

	  datestr = day + "." + month + "." + year + " " + hour + ":" + minut
	      + ":" + second;
	  date->set_text(Glib::ustring(datestr));

	  grmsg->attach(*date, 0, rowcount, 1, 1);
	  rowcount++;

	  Gtk::Label *from = Gtk::make_managed<Gtk::Label>();
	  from->set_halign(Gtk::Align::START);
	  from->set_margin(2);
	  from->set_text("Я:");
	  from->set_name("notSentMsg");
	  from->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  grmsg->attach(*from, 0, rowcount, 1, 1);
	  rowcount++;

	  if(mw->repllabe != nullptr)
	    {
	      Gtk::Frame *repfr = Gtk::make_managed<Gtk::Frame>();
	      repfr->set_name("repFrame");
	      repfr->get_style_context()->add_provider(
		  mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      repfr->set_halign(Gtk::Align::START);
	      repfr->set_margin(2);
	      Gtk::Label *repl = Gtk::make_managed<Gtk::Label>();
	      repl->set_halign(Gtk::Align::START);
	      repl->set_margin(5);
	      repl->set_use_markup(true);
	      repl->set_max_width_chars(20);
	      repl->set_ellipsize(Pango::EllipsizeMode::END);
	      repl->set_markup(
		  "<span style=\"italic\">" + mw->repllabe->get_text()
		      + "</span>");
	      repfr->set_child(*repl);
	      grmsg->attach(*repfr, 0, rowcount, 1, 1);
	      rowcount++;
	    }

	  Gtk::Label *message = Gtk::make_managed<Gtk::Label>();
	  message->set_halign(Gtk::Align::START);
	  message->set_margin(5);
	  message->set_justify(Gtk::Justification::LEFT);
	  message->set_wrap(true);
	  message->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
	  message->set_selectable(true);
	  message->set_max_width_chars(mw->msg_width_chars);
	  message->set_margin_bottom(10);
	  Glib::ustring filemsg;
	  if(mw->attachedfile != nullptr)
	    {
	      filemsg = utxt;
	      utxt = mw->attachedfile->get_text();
	    }
	  std::string line = std::string(utxt);
	  std::string::size_type n;
	  n = line.find("TMFile Attachment ");
	  if(n == std::string::npos)
	    {
	      message->set_text(Glib::ustring(utxt));
	    }
	  else
	    {
	      line.erase(0, n + std::string("TMFile Attachment ").size());
	      std::string fnm = line.substr(0, line.find(" "));
	      line.erase(0, line.find(" ") + std::string(" ").size());
	      fnm = gettext("Message attached to file ") + fnm + "\n" + line;
	      message->set_text(Glib::ustring(fnm));
	    }
	  grmsg->attach(*message, 0, rowcount, 1, 1);
	  mw->msg_grid_vectmtx.lock();
	  if(mw->msg_grid_vect.size() == 0)
	    {
	      mw->msg_win_gr->attach(*fr, 0, 0, 1, 1);
	    }
	  else
	    {
	      Gtk::Widget *widg = std::get<0>(
		  mw->msg_grid_vect[mw->msg_grid_vect.size() - 1]);
	      int column, row, width, height;
	      mw->msg_win_gr->query_child(*widg, column, row, width, height);
	      mw->msg_win_gr->attach(*fr, 0, row + 1, 1, 1);
	    }
	  mw->msg_grid_vectmtx.unlock();
	  clck->signal_pressed().connect(
	      sigc::bind(sigc::mem_fun(*this, &MWMsgOperations::creatReply), fr,
			 message, clck, mw));
	  std::string index = "";
	  std::string key = mw->fordelkey;
	  mw->contmtx.lock();
	  auto cit = std::find_if(mw->contacts.begin(), mw->contacts.end(),
				  [&key]
				  (auto &el)
				    {
				      return std::get<1>(el) == key;
				    });
	  if(cit != mw->contacts.end())
	    {
	      strm.str("");
	      strm.clear();
	      strm.imbue(loc);
	      strm << std::get<0>(*cit);
	      index = strm.str();
	    }
	  mw->contmtx.unlock();
	  std::string filename;
	  AuxFunc af;
#ifdef __linux
	  filename = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
          filename =
            std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	  if(index != "")
	    {
	      LibCommunist Lc;
	      filename = filename + "/" + Lc.randomFileName() + "/" + index
		  + "/";
	      strm.str("");
	      strm.clear();
	      strm.imbue(loc);
	      strm << curtm;
	      filename = filename + strm.str();
	      std::filesystem::path filepath = std::filesystem::u8path(
		  filename);
	      if(!std::filesystem::exists(filepath.parent_path()))
		{
		  std::filesystem::create_directories(filepath.parent_path());
		}
	      std::fstream f;
	      f.open(filepath, std::ios_base::out | std::ios_base::binary);
	      std::string line;
	      std::string replstr = "";
	      if(mw->repllabe != nullptr)
		{
		  replstr = std::string(mw->repllabe->get_text());
		}
	      line = Glib::locale_from_utf8(utxt);
	      af.toutf8(line);
	      f.write(line.c_str(), line.size());
	      f.close();
	      std::filesystem::path ch;
	      if(mw->oper != nullptr)
		{
		  if(mw->attachedfile == nullptr)
		    {
		      ch = mw->oper->sendMessage(mw->fordelkey, "", replstr,
						 filepath);
		    }
		  if(mw->attachedfile != nullptr)
		    {
		      ch = mw->oper->sendFile(
			  mw->fordelkey, "", replstr,
			  std::string(mw->attachedfile->get_text()));
		      if(filemsg.size() > 0)
			{
			  std::filesystem::path spth = std::filesystem::u8path(
			      std::string(mw->attachedfile->get_text()));
			  std::string sendmsg = "TMFile Attachment "
			      + spth.filename().u8string() + " "
			      + std::string(filemsg);
			  std::string *smsg = new std::string(sendmsg);
			  Glib::Dispatcher *disp = new Glib::Dispatcher;
			  disp->connect(
			      sigc::bind(
				  sigc::mem_fun(*mw, &MainWindow::sendMsgFile),
				  smsg));
			  std::thread *thr = new std::thread([disp]
			  {
			    disp->emit();
			    delete disp;
			  });
			  thr->detach();
			  delete thr;
			}
		    }
		}
	      std::filesystem::remove_all(filepath.parent_path());
	      if(std::filesystem::exists(ch))
		{
		  mw->sentstatmtx.lock();
		  mw->sentstatus.push_back(
		      std::make_tuple(mw->fordelkey, ch, from));
		  mw->sentstatmtx.unlock();
		  std::filesystem::path pfg = ch;
		  mw->msg_grid_vectmtx.lock();
		  mw->msg_grid_vect.push_back(
		      std::make_tuple(fr, pfg, nullptr));
		  if(mw->msg_grid_vect.size() > 20)
		    {
		      Gtk::Widget *widg = std::get<0>(mw->msg_grid_vect[0]);
		      mw->msg_win_gr->remove(*widg);
		      mw->msg_grid_vect.erase(mw->msg_grid_vect.begin());
		    }
		  mw->msg_grid_vectmtx.unlock();
		}
	      else
		{
		  mw->msg_grid_vectmtx.lock();
		  mw->msg_grid_vect.erase(
		      std::remove_if(mw->msg_grid_vect.begin(),
				     mw->msg_grid_vect.end(), [fr]
				     (auto &el)
				       {
					 return std::get<0>(el) == fr;
				       }),
		      mw->msg_grid_vect.end());
		  mw->msg_grid_vectmtx.unlock();
		  mw->msg_win_gr->remove(*fr);
		}
	      std::filesystem::remove_all(filepath.parent_path().parent_path());
	    }
	}
    }
  if(mw->repllabe != nullptr)
    {
      mw->Rightgrid->remove(*(mw->replgrid));
    }
  if(mw->attachedfile != nullptr)
    {
      mw->Rightgrid->remove(*(mw->attach));
      mw->Rightgrid->remove(*(mw->attachcancel));
      mw->Rightgrid->remove(*(mw->attachedfile));
    }
  mw->repllabe = nullptr;
  mw->replcancel = nullptr;
  mw->attach = nullptr;
  mw->attachcancel = nullptr;
  mw->attachedfile = nullptr;
}

void
MWMsgOperations::msgRcvdSlot(std::string *key, std::filesystem::path *p,
			     std::mutex *disp3mtx)
{
  std::string keyloc = *key;
  std::filesystem::path ploc = *p;
  disp3mtx->unlock();
  Glib::ustring onlt = "";
  Gtk::Label *onl = nullptr;
  mw->chifcmtx.lock();
  auto itchfc = std::find_if(mw->chifc.begin(), mw->chifc.end(), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    });
  if(itchfc != mw->chifc.end())
    {
      onlt = std::get<2>(*itchfc)->get_text();
    }
  mw->frvectmtx.lock();
  auto frit = std::find_if(
      mw->friendvect.begin(), mw->friendvect.end(), [&keyloc]
      (auto &el)
	{
	  return std::string(std::get<2>(el)->get_text()) == keyloc;
	});

  std::stringstream strm;
  std::locale loc("C");
  if(frit != mw->friendvect.end())
    {
      mw->prefvectmtx.lock();
      auto itprv = std::find_if(mw->prefvect.begin(), mw->prefvect.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "SoundOn";
	});
      if(itprv != mw->prefvect.end())
	{
	  if(std::get<1>(*itprv) == "active")
	    {
	      if(mw->mf)
		{
		  mw->mf->play();
		}
	    }
	}
      else
	{
	  if(mw->mf)
	    {
	      mw->mf->play();
	    }
	}
      mw->prefvectmtx.unlock();
      if(mw->grnotific)
	{
	  Glib::RefPtr<Gio::Notification> notific = Gio::Notification::create(
	      gettext("New message"));
	  Glib::ustring msgnot;
	  msgnot = gettext("Message from ") + std::get<4>(*frit)->get_text();
	  notific->set_body(msgnot);
	  notific->set_priority(Gio::Notification::Priority::HIGH);
	  Glib::RefPtr<Gtk::Application> app = mw->get_application();
	  app->send_notification(notific);
	}
      auto tup = *frit;
      std::string num_rcvd_msgs = "0";
      Gtk::Label *labnumr = std::get<3>(tup);
      if(labnumr)
	{
	  num_rcvd_msgs = std::string(labnumr->get_text());
	}
      Gtk::Button *but = std::get<0>(tup);
      Gtk::Requisition rq1, rq2;
      but->get_preferred_size(rq1, rq2);
      mw->friendvect.erase(frit);
      Glib::ustring keyt = std::get<2>(tup)->get_text();
      Glib::ustring nickt = std::get<4>(tup)->get_text();
      Glib::ustring namet = std::get<5>(tup)->get_text();

      mw->fordelgrid->remove(*but);
      but = nullptr;
      but = Gtk::make_managed<Gtk::Button>();
      Gtk::Grid *ngr = Gtk::make_managed<Gtk::Grid>();
      but->set_child(*ngr);

      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
	  Gio::SimpleActionGroup::create();
      acgroup->add_action(
	  "info",
	  sigc::bind(sigc::mem_fun(*mw, &MainWindow::friendDetails), but));
      acgroup->add_action("delete",
			  sigc::mem_fun(*mw, &MainWindow::deleteContact));
      acgroup->add_action("block",
			  sigc::mem_fun(*mw, &MainWindow::tempBlockCont));
      but->insert_action_group("popupcont", acgroup);

      Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
      menu->append(gettext("Contact info"), "popupcont.info");
      menu->append(gettext("Remove contact"), "popupcont.delete");
      menu->append(gettext("Block (until restart)/Unblock"), "popupcont.block");
      Gtk::PopoverMenu *Menu = new Gtk::PopoverMenu;
      Menu->set_menu_model(menu);
      Menu->set_has_arrow(false);

      Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
      clck->set_button(3);
      clck->signal_pressed().connect(
	  sigc::bind(sigc::mem_fun(*mw, &MainWindow::contMenu), but, Menu));
      but->add_controller(clck);
      but->signal_unrealize().connect([Menu]
      {
	delete Menu;
      });

      std::get<0>(tup) = but;
      std::get<1>(tup) = ngr;
      but->set_halign(Gtk::Align::START);
      but->set_size_request(rq2.get_width(), -1);
      but->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

      Gtk::Label *keylab = Gtk::make_managed<Gtk::Label>();
      keylab->set_text(keyt);
      keylab->set_halign(Gtk::Align::START);
      std::get<2>(tup) = keylab;
      keylab->set_max_width_chars(20);
      keylab->set_ellipsize(Pango::EllipsizeMode::END);

      Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea>();
      drar->set_margin(2);
      drar->set_halign(Gtk::Align::CENTER);
      drar->set_size_request(50, 50);
      drar->set_draw_func(
	  sigc::bind(sigc::mem_fun(*mw, &MainWindow::on_draw_contactlist),
		     keylab));
      ngr->attach(*drar, 0, 0, 1, 3);

      ngr->attach(*keylab, 1, 0, 1, 1);

      Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label>();
      std::get<4>(tup) = nicklab;
      nicklab->set_text(nickt);
      nicklab->set_halign(Gtk::Align::START);
      ngr->attach(*nicklab, 1, 1, 1, 1);

      Gtk::Label *namelab = Gtk::make_managed<Gtk::Label>();
      std::get<5>(tup) = namelab;
      namelab->set_text(namet);
      namelab->set_halign(Gtk::Align::START);
      ngr->attach(*namelab, 1, 2, 1, 1);

      if(itchfc != mw->chifc.end())
	{
	  onl = Gtk::make_managed<Gtk::Label>();
	  onl->set_margin(5);
	  onl->set_halign(Gtk::Align::END);
	  onl->set_name("connectFr");
	  onl->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  onl->set_text(onlt);
	  ngr->attach_next_to(*onl, *namelab, Gtk::PositionType::RIGHT, 1, 1);
	  std::get<2>(*itchfc) = onl;
	}

      but->signal_clicked().connect(
	  sigc::bind(sigc::mem_fun(*mw, &MainWindow::selectContact),
		     mw->Winright, std::string(keylab->get_text()),
		     std::string(nicklab->get_text())));
      if(mw->fordelkey != keyloc)
	{
	  but->set_name("inactiveButton");
	}
      else
	{
	  but->set_name("activeButton");
	  mw->selectedc = but;
	}
      if(mw->friendvect.size() > 0)
	{
	  mw->fordelgrid->insert_next_to(*(std::get<0>(mw->friendvect[0])),
					 Gtk::PositionType::TOP);
	  mw->fordelgrid->attach_next_to(*but,
					 *(std::get<0>(mw->friendvect[0])),
					 Gtk::PositionType::TOP, 1, 1);
	}
      else
	{
	  mw->fordelgrid->attach(*but, 0, 0, 1, 1);
	}

      if(mw->fordelkey != keyloc)
	{
	  if(num_rcvd_msgs != "0")
	    {
	      std::string numb(num_rcvd_msgs);
	      int n;
	      strm.imbue(loc);
	      strm << numb;
	      strm >> n;
	      n = n + 1;
	      strm.str("");
	      strm.clear();
	      strm.imbue(loc);
	      strm << n;
	      numb = strm.str();
	      num_rcvd_msgs = strm.str();
	    }
	  else
	    {
	      num_rcvd_msgs = "1";
	    }
	  labnumr = Gtk::make_managed<Gtk::Label>();
	  labnumr->set_text(Glib::ustring(num_rcvd_msgs));
	  labnumr->set_name("msgNumLab");
	  labnumr->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  labnumr->set_margin(2);
	  labnumr->set_halign(Gtk::Align::END);
	  Gtk::Grid *gr = std::get<1>(tup);
	  gr->insert_next_to(*(std::get<2>(tup)), Gtk::PositionType::RIGHT);
	  gr->attach_next_to(*labnumr, *(std::get<2>(tup)),
			     Gtk::PositionType::RIGHT, 1, 1);
	  std::get<3>(tup) = labnumr;
	}
      else
	{
	  std::get<3>(tup) = nullptr;
	  LibCommunist Lc;
	  std::string filename = Lc.openMessage(ploc, keyloc, mw->Username,
						mw->Password);
	  std::filesystem::path filepath = std::filesystem::u8path(filename);
	  Gtk::Grid *grmsg = Gtk::make_managed<Gtk::Grid>();
	  Gtk::Frame *fr = Gtk::make_managed<Gtk::Frame>();
	  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
	  clck->set_button(0);
	  fr->set_child(*grmsg);
	  Gtk::Requisition rq1, rq2;
	  fr->set_halign(Gtk::Align::FILL);
	  fr->add_controller(clck);
	  fr->set_name("frMsg");
	  fr->set_margin(2);
	  fr->get_style_context()->add_provider(mw->css_provider,
	  GTK_STYLE_PROVIDER_PRIORITY_USER);

	  Gtk::Label *date = Gtk::make_managed<Gtk::Label>();
	  date->set_halign(Gtk::Align::START);
	  date->set_margin(2);
	  grmsg->attach(*date, 0, 0, 1, 1);

	  Gtk::Label *from = Gtk::make_managed<Gtk::Label>();
	  from->set_halign(Gtk::Align::START);
	  from->set_margin(2);
	  grmsg->attach(*from, 0, 1, 1, 1);

	  Gtk::Frame *repfr = Gtk::make_managed<Gtk::Frame>();
	  repfr->set_halign(Gtk::Align::START);
	  repfr->set_name("repFrame");
	  repfr->get_style_context()->add_provider(
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  repfr->set_margin(2);
	  Gtk::Label *repl = Gtk::make_managed<Gtk::Label>();
	  repl->set_halign(Gtk::Align::START);
	  repl->set_margin(5);
	  repl->set_use_markup(true);
	  repl->set_max_width_chars(20);
	  repl->set_ellipsize(Pango::EllipsizeMode::END);
	  repfr->set_child(*repl);
	  grmsg->attach(*repfr, 0, 2, 1, 1);
	  int count2 = 0;
	  int type;
	  std::fstream f;
	  f.open(filepath, std::ios_base::in);
	  while(!f.eof())
	    {
	      std::string gl;
	      getline(f, gl);
	      std::string::size_type n;
	      if(gl != "")
		{
		  if(count2 == 1)
		    {
		      gl.erase(
			  0,
			  gl.find("Resend from:")
			      + std::string("Resend from:").size());
		      if(gl.size() > 0)
			{
			  n = gl.find(" ");
			  if(n != std::string::npos)
			    {
			      gl.erase(0, n + std::string(" ").size());
			    }
			  Glib::ustring txt(gl.substr(0, gl.find(" ")));
			  txt = gettext("Resend from: ") + txt;
			  from->set_text(txt);
			}
		      else
			{
			  from->set_text(nickt + ":");
			}
		    }
		  if(count2 == 3)
		    {
		      n = gl.find("Creation time: ");
		      if(n != std::string::npos)
			{
			  gl.erase(n, std::string("Creation time: ").size());
			}
		      time_t timet = time(NULL);
		      std::string day, month, year, hour, minut, second;
		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gl;
		      strm >> timet;
		      tm *gmtm = localtime(&timet);

		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gmtm->tm_mday;
		      day = strm.str();
		      if(gmtm->tm_mday < 10)
			{
			  day = "0" + day;
			}

		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gmtm->tm_mon + 1;
		      month = strm.str();
		      if(gmtm->tm_mon < 10)
			{
			  month = "0" + month;
			}

		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gmtm->tm_year + 1900;
		      year = strm.str();

		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gmtm->tm_hour;
		      hour = strm.str();
		      if(gmtm->tm_hour < 10)
			{
			  hour = "0" + hour;
			}

		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gmtm->tm_min;
		      minut = strm.str();
		      if(gmtm->tm_min < 10)
			{
			  minut = "0" + minut;
			}

		      strm.str("");
		      strm.clear();
		      strm.imbue(loc);
		      strm << gmtm->tm_sec;
		      second = strm.str();
		      if(gmtm->tm_sec < 10)
			{
			  second = "0" + second;
			}

		      Glib::ustring txt(
			  std::string(
			      day + "." + month + "." + year + " " + hour + ":"
				  + minut + ":" + second));
		      date->set_text(txt);
		    }
		  if(count2 == 4)
		    {
		      n = gl.find("Type: ");
		      if(n != std::string::npos)
			{
			  gl.erase(n, std::string("Type: ").size());
			}
		      if(gl == "text message")
			{
			  type = 0;
			}
		      if(gl == "file message")
			{
			  type = 1;
			}
		    }
		  if(count2 == 5)
		    {
		      n = gl.find("Reply to: ");
		      if(n != std::string::npos)
			{
			  gl.erase(n, std::string("Reply to: ").size());
			  std::string line = gl;
			  repl->set_markup(
			      "<span style=\"italic\">" + Glib::ustring(line)
				  + "</span>");
			}
		      else
			{
			  grmsg->remove_row(2);
			}
		    }
		  if(count2 > 6)
		    {
		      Gtk::Widget *widg = grmsg->get_child_at(0, 3);
		      int rownum;
		      if(widg == nullptr)
			{
			  rownum = 3;
			}
		      else
			{
			  rownum = 4;
			}
		      if(type == 0 || type == 1)
			{
			  Gtk::Label *message = Gtk::make_managed<Gtk::Label>();
			  message->set_halign(Gtk::Align::START);
			  message->set_margin(5);
			  message->set_justify(Gtk::Justification::LEFT);
			  message->set_wrap(true);
			  message->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
			  message->set_margin_bottom(10);
			  std::string line = gl;
			  std::string::size_type n;
			  n = line.find("TMFile Attachment ");
			  if(n == std::string::npos)
			    {
			      message->set_text(Glib::ustring(gl));
			    }
			  else
			    {
			      line.erase(
				  0,
				  n + std::string("TMFile Attachment ").size());
			      std::string fnm = line.substr(0, line.find(" "));
			      line.erase(
				  0, line.find(" ") + std::string(" ").size());

			      fnm = gettext("Message attached to file ") + fnm
				  + "\n" + line;
			      message->set_text(Glib::ustring(fnm));
			    }
			  message->set_selectable(true);
			  message->set_max_width_chars(mw->msg_width_chars);
			  grmsg->attach(*message, 0, rownum, 1, 1);
			  clck->signal_pressed().connect(
			      sigc::bind(
				  sigc::mem_fun(*this,
						&MWMsgOperations::creatReply),
				  fr, message, clck, mw));
			}
		    }
		}
	      count2++;
	    }
	  f.close();
	  std::filesystem::remove_all(filepath);
	  mw->msg_grid_vectmtx.lock();
	  if(mw->msg_grid_vect.size() == 0)
	    {
	      mw->msg_win_gr->attach(*fr, 1, 0, 1, 1);
	      fr->get_preferred_size(rq1, rq2);
	      Gtk::Frame *empty = Gtk::make_managed<Gtk::Frame>();
	      empty->set_opacity(0);
	      mw->msg_win_gr->attach(*empty, 0, 0, 1, 1);
	      mw->msg_grid_vect.push_back(std::make_tuple(fr, ploc, nullptr));
	    }
	  else
	    {
	      Gtk::Widget *widg = std::get<0>(
		  mw->msg_grid_vect[mw->msg_grid_vect.size() - 1]);
	      int column, row, width, height;
	      mw->msg_win_gr->query_child(*widg, column, row, width, height);
	      mw->msg_win_gr->attach(*fr, 1, row + 1, width, height);
	      Gtk::Frame *empty = Gtk::make_managed<Gtk::Frame>();
	      empty->set_opacity(0);
	      mw->msg_win_gr->attach(*empty, 0, row + 1, width, height);
	      mw->msg_grid_vect.push_back(std::make_tuple(fr, ploc, nullptr));
	    }
	  if(mw->msg_grid_vect.size() > 20)
	    {
	      Gtk::Widget *widg = std::get<0>(mw->msg_grid_vect[0]);
	      int column, row, width, height;
	      mw->msg_win_gr->query_child(*widg, column, row, width, height);
	      mw->msg_grid_vect.erase(mw->msg_grid_vect.begin());
	      mw->msg_win_gr->remove_row(row);
	    }
	  mw->msg_grid_vectmtx.unlock();
	  if(mw->usermsgadj >= 0)
	    {
	      if(mw->msgovllab == nullptr)
		{
		  Gtk::Button *ovlbut = Gtk::make_managed<Gtk::Button>();
		  ovlbut->set_halign(Gtk::Align::END);
		  ovlbut->set_valign(Gtk::Align::END);
		  ovlbut->set_margin(10);
		  Gtk::Grid *ovlgr = Gtk::make_managed<Gtk::Grid>();
		  ovlbut->set_child(*ovlgr);
		  Gtk::Label *ovllab = Gtk::make_managed<Gtk::Label>();
		  ovllab->set_text("1");
		  mw->msgovllab = ovllab;
		  ovlgr->attach(*ovllab, 0, 0, 1, 1);
		  Gtk::DrawingArea *ovldrar =
		      Gtk::make_managed<Gtk::DrawingArea>();
		  ovlbut->get_preferred_size(rq1, rq2);
		  ovldrar->set_size_request(rq2.get_width(), rq2.get_height());
		  ovldrar->set_draw_func(
		      sigc::bind(sigc::mem_fun(*mw, &MainWindow::on_draw_sb),
				 mw->Sharepath + "/Arrow_Icon.png"));
		  ovlgr->attach(*ovldrar, 0, 1, 1, 1);

		  MainWindow *mwl = mw;
		  ovlbut->signal_clicked().connect(
		      [mwl, ovlbut]
		      {
			Glib::RefPtr<Gtk::Adjustment> adj =
			    mwl->Winright->get_vadjustment();
			adj->set_value(adj->get_upper() - adj->get_page_size());
			mwl->msgovl->remove_overlay(*ovlbut);
			mwl->msgovllab = nullptr;
		      });
		  mw->msgovl->add_overlay(*ovlbut);
		}
	      else
		{
		  std::string msgval = std::string(mw->msgovllab->get_text());
		  strm.str("");
		  strm.clear();
		  strm.imbue(loc);
		  strm << msgval;
		  int msgvali;
		  strm >> msgvali;
		  msgvali = msgvali + 1;
		  strm.str("");
		  strm.clear();
		  strm.imbue(loc);
		  strm << msgvali;
		  msgval = strm.str();
		  mw->msgovllab->set_text(Glib::ustring(msgval));
		}
	    }
	}
      mw->friendvect.insert(mw->friendvect.begin(), tup);
    }
  mw->frvectmtx.unlock();
  if(itchfc != mw->chifc.end())
    {
      if(onl != nullptr)
	{
	  std::get<2>(*itchfc) = onl;
	}
      else
	{
	  mw->chifc.erase(itchfc);
	}
    }
  mw->chifcmtx.unlock();
}

void
MWMsgOperations::creatReply(int numcl, double x, double y, Gtk::Frame *fr,
			    Gtk::Label *message,
			    Glib::RefPtr<Gtk::GestureClick> clck,
			    MainWindow *mwl)
{
  if(clck->get_current_button() == 3)
    {
      Gtk::PopoverMenu *Menu = nullptr;
      mwl->msg_grid_vectmtx.lock();
      auto itmgrv = std::find_if(mwl->msg_grid_vect.begin(),
				 mwl->msg_grid_vect.end(), [fr]
				 (auto &el)
				   {
				     return std::get<0>(el) == fr;
				   });
      if(itmgrv != mwl->msg_grid_vect.end())
	{
	  Menu = std::get<2>(*itmgrv);
	  if(Menu == nullptr)
	    {
	      Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
	      menu->append(gettext("Reply"), "popupmsg.reply");
	      menu->append(gettext("Forward"), "popupmsg.forward");
	      menu->append(gettext("Remove (only this machine)"),
			   "popupmsg.remove");
	      Menu = Gtk::make_managed<Gtk::PopoverMenu>();
	      std::get<2>(*itmgrv) = Menu;
	      Menu->set_parent(*fr);
	      Menu->set_menu_model(menu);
	      Menu->set_has_arrow(false);
	      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
		  Gio::SimpleActionGroup::create();
	      acgroup->add_action("reply", [mwl, message]
	      {
		if(mwl->repllabe != nullptr)
		  {
		    mwl->Rightgrid->remove(*(mwl->replgrid));
		    Glib::RefPtr<Glib::MainContext> mk =
			Glib::MainContext::get_default();
		    while(mk->pending())
		      {
			mk->iteration(true);
		      }
		    mwl->replgrid = nullptr;
		    mwl->replcancel = nullptr;
		    mwl->repllabe = nullptr;
		  }
		mwl->repllabe = Gtk::make_managed<Gtk::Label>();
		mwl->repllabe->set_halign(Gtk::Align::START);
		mwl->repllabe->set_margin(5);
		mwl->repllabe->set_max_width_chars(50);
		mwl->repllabe->set_ellipsize(Pango::EllipsizeMode::END);
		mwl->repllabe->set_text(message->get_text());
		mwl->repllabe->set_name("fileAddress");
		mwl->repllabe->get_style_context()->add_provider(mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

		mwl->replcancel = Gtk::make_managed<Gtk::Button>();
		mwl->replcancel->set_valign(Gtk::Align::CENTER);
		mwl->replcancel->set_halign(Gtk::Align::END);
		mwl->replcancel->set_margin(5);
		mwl->replcancel->set_name("cancelRepl");
		mwl->replcancel->get_style_context()->add_provider(
		    mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
		mwl->replcancel->set_icon_name("window-close-symbolic");
		mwl->replcancel->signal_clicked().connect([mwl]
		{
		  mwl->Rightgrid->remove(*(mwl->replgrid));
		  Glib::RefPtr<Glib::MainContext> mk =
		      Glib::MainContext::get_default();
		  while(mk->pending())
		    {
		      mk->iteration(true);
		    }
		  mwl->replgrid = nullptr;
		  mwl->replcancel = nullptr;
		  mwl->repllabe = nullptr;
		});

		mwl->Rightgrid->insert_next_to(*(mwl->msgovl),
					       Gtk::PositionType::BOTTOM);
		mwl->replgrid = Gtk::make_managed<Gtk::Grid>();
		mwl->replgrid->attach(*(mwl->repllabe), 0, 0, 1, 1);
		mwl->replgrid->attach(*(mwl->replcancel), 1, 0, 1, 1);
		mwl->Rightgrid->attach_next_to(*(mwl->replgrid), *(mwl->msgovl),
					       Gtk::PositionType::BOTTOM, 2, 1);
	      });
	      acgroup->add_action("forward", [mwl, message, fr]
	      {
		Gtk::Button *sb = mwl->selectedc;
		Gtk::Widget *w = fr;
		mwl->frvectmtx.lock();
		auto itfr = std::find_if(mwl->friendvect.begin(), mwl->friendvect.end(), [sb]
	      (auto & el)
		{
		  return std::get<0>(el) == sb;
		});
		if(itfr != mwl->friendvect.end())
		  {
		    Gtk::Label *keyl = std::get<2>(*itfr);
		    std::string othkey(keyl->get_text());
		    mwl->msg_grid_vectmtx.lock();
		    auto itgr = std::find_if(mwl->msg_grid_vect.begin(),
					     mwl->msg_grid_vect.end(), [w]
					     (auto &el)
					       {
						 return std::get<0>(el) == w;
					       });
		    if(itgr != mwl->msg_grid_vect.end())
		      {
			std::filesystem::path msgpath = std::get<1>(*itgr);
			AuxWindows aw(mwl);
			aw.resendWindow(othkey, msgpath);
		      }
		    mwl->msg_grid_vectmtx.unlock();
		  }
		mwl->frvectmtx.unlock();
	      });
	      acgroup->add_action("remove", [mwl, message, fr]
	      {
		Gtk::Button *sb = mwl->selectedc;
		Gtk::Widget *w = fr;
		mwl->frvectmtx.lock();
		auto itfr = std::find_if(mwl->friendvect.begin(), mwl->friendvect.end(), [sb]
	      (auto & el)
		{
		  return std::get<0>(el) == sb;
		});
		if(itfr != mwl->friendvect.end())
		  {
		    Gtk::Label *keyl = std::get<2>(*itfr);
		    std::string othkey(keyl->get_text());
		    mwl->msg_grid_vectmtx.lock();
		    auto itgr = std::find_if(mwl->msg_grid_vect.begin(),
					     mwl->msg_grid_vect.end(), [w]
					     (auto &el)
					       {
						 return std::get<0>(el) == w;
					       });
		    if(itgr != mwl->msg_grid_vect.end())
		      {
			std::filesystem::path msgpath = std::get<1>(*itgr);
			Gtk::Widget *widg = std::get<0>(*itgr);
			mwl->removeMsg(othkey, msgpath, widg);
		      }
		    mwl->msg_grid_vectmtx.unlock();
		  }
		mwl->frvectmtx.unlock();
	      });

	      fr->insert_action_group("popupmsg", acgroup);

	      Gdk::Rectangle rect(x, y, 1, 1);
	      Menu->set_pointing_to(rect);

	      Menu->set_autohide(false);
	    }
	  else
	    {
	      Gdk::Rectangle rect(x, y, 1, 1);
	      Menu->set_pointing_to(rect);
	    }
	}
      mwl->msg_grid_vectmtx.unlock();
      if(mwl->rightmenfordel != nullptr)
	{
	  mwl->rightmenfordel->hide();
	}
      mwl->rightmenfordel = Menu;
      Menu->popup();
    }
}

void
MWMsgOperations::removeMsg(std::string othkey, std::filesystem::path msgpath,
			   Gtk::Widget *widg)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Confirmation"));

  window->set_transient_for(*mw);
  window->set_modal(true);
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Are you sure?"));
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *confirm = Gtk::make_managed<Gtk::Button>();
  confirm->set_halign(Gtk::Align::CENTER);
  confirm->set_margin(5);
  confirm->set_name("clearButton");
  confirm->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  confirm->set_label(gettext("Yes"));
  confirm->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MWMsgOperations::removeMsgFunc), window,
		 mw, othkey, msgpath, widg));
  grid->attach(*confirm, 0, 1, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_name("rejectButton");
  cancel->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->set_label(gettext("No"));
  cancel->signal_clicked().connect([window]
  {
    window->close();
  });
  grid->attach(*cancel, 1, 1, 1, 1);

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
MWMsgOperations::removeMsgFunc(Gtk::Window *window, MainWindow *mwl,
			       std::string othkey,
			       std::filesystem::path msgpath, Gtk::Widget *widg)
{
  mwl->sentstatmtx.lock();
  mwl->sentstatus.erase(
      std::remove_if(
	  mwl->sentstatus.begin(), mwl->sentstatus.end(), [othkey, msgpath]
	  (auto &el)
	    {
	      if(std::get<0>(el) == othkey && std::get<1>(el) == msgpath)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }),
      mwl->sentstatus.end());
  mwl->sentstatmtx.unlock();
  std::filesystem::path retpath;
  retpath = mwl->oper->removeMsg(othkey, msgpath);
  mwl->fileprogrvectmtx.lock();
  auto itfprg = std::find_if(mwl->fileprogrvect.begin(),
			     mwl->fileprogrvect.end(), [&retpath]
			     (auto &el)
			       {
				 return std::get<1>(el) == retpath;
			       });
  if(itfprg != mwl->fileprogrvect.end())
    {
      mwl->dld_grid->remove(*(std::get<3>(*itfprg)));
      mwl->dld_grid->remove(*(std::get<4>(*itfprg)));
      mwl->dld_grid->remove(*(std::get<5>(*itfprg)));
      mwl->fileprogrvect.erase(itfprg);
    }
  if(mwl->fileprogrvect.size() == 0 && mwl->dld_win != nullptr)
    {
      mwl->dld_win->hide();
      delete mwl->dld_win;
      mwl->dld_win = nullptr;
      mwl->dld_grid = nullptr;
    }
  mwl->fileprogrvectmtx.unlock();
  mwl->msg_grid_vectmtx.lock();
  mwl->msg_grid_vect.erase(
      std::remove_if(mwl->msg_grid_vect.begin(), mwl->msg_grid_vect.end(),
		     [widg]
		     (auto &el)
		       {
			 return std::get<0>(el) == widg;
		       }),
      mwl->msg_grid_vect.end());
  mwl->msg_grid_vectmtx.unlock();
  int column, row, width, height;
  mwl->msg_win_gr->query_child(*widg, column, row, width, height);
  mwl->msg_win_gr->remove_row(row);
  window->close();
}

void
MWMsgOperations::msgSentSlot(std::string *key, std::filesystem::path *msgind,
			     std::mutex *disp2mtx)
{
  std::filesystem::path ind = *msgind;
  std::string keyloc = *key;
  disp2mtx->unlock();
  mw->sentstatmtx.lock();
  auto it = std::find_if(
      mw->sentstatus.begin(), mw->sentstatus.end(), [&keyloc, &ind]
      (auto &el)
	{
	  if(std::get<0>(el) == keyloc && std::get<1>(el) == ind)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if(it != mw->sentstatus.end())
    {
      Gtk::Label *lab = std::get<2>(*it);
      lab->set_name("SentMsg");
    }
  mw->sentstatmtx.unlock();
}
