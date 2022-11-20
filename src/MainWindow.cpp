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
#include "MainWindow.h"

MainWindow::MainWindow()
{
  dispclose.connect([this]
  {
    this->hide();
  });
  this->signal_close_request().connect(
    sigc::mem_fun(*this, &MainWindow::mainWindowClose), false);
  AuxFunc af;
  std::filesystem::path p(std::filesystem::u8path(
                            af.get_selfpath()));
  Sharepath = p.parent_path().u8string() +
              "/../share/Communist";
  std::string filename;
  af.homePath(&filename);
  filename = filename + "/.config/Communist/Prefs";
  std::filesystem::path prefpath = std::filesystem::u8path(
                                     filename);
  std::fstream f;
  f.open(prefpath, std::ios_base::in);
  if(f.is_open())
    {
      while(!f.eof())
        {
          std::string line;
          getline(f, line);
          if(line != "")
            {
              std::tuple<std::string, std::string> ttup;
              std::get<0> (ttup) = line.substr(0, line.find(":"));
              line.erase(0, line.find(" ") + std::string(" ").size());
              std::get<1> (ttup) = line;
              prefvectmtx.lock();
              prefvect.push_back(ttup);
              prefvectmtx.unlock();
            }
        }
      f.close();
    }
  prefvectmtx.lock();
  auto itprv = std::find_if(prefvect.begin(),
                            prefvect.end(), []
                            (auto & el)
  {
    return std::get<0>(el) == "Userthemepath";
  });
  if(itprv != prefvect.end())
    {
      Userthemepath = std::get<1> (*itprv);
      if(Userthemepath == "")
        {
          Userthemepath = Sharepath + "/themes";
        }
    }
  else
    {
      Userthemepath = Sharepath + "/themes";
    }

  itprv = std::find_if(prefvect.begin(), prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Theme";
  });
  if(itprv != prefvect.end())
    {
      Theme = std::get<1> (*itprv);
      filename = Userthemepath + "/" + Theme;
      std::filesystem::path thpath = std::filesystem::u8path(
                                       filename);
      if(!std::filesystem::exists(thpath))
        {
          Theme = "default";
        }
    }
  else
    {
      Theme = "default";
    }

  itprv = std::find_if(prefvect.begin(), prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "MsgWidth";
  });
  if(itprv != prefvect.end())
    {
      std::string mesw = std::get<1> (*itprv);
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm >> msg_width_chars;
    }

  itprv = std::find_if(prefvect.begin(), prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "GrNotification";
  });
  if(itprv != prefvect.end())
    {
      std::string mesw = std::get<1> (*itprv);
      if(mesw == "yes")
        {
          grnotific = true;
        }
      else
        {
          grnotific = false;
        }
    }
  prefvectmtx.unlock();

  css_provider = Gtk::CssProvider::create();

  css_provider->load_from_path(
    Glib::ustring(Userthemepath + "/" + Theme +
                  "/mainWindow.css"));

  af.homePath(&filename);
  filename = filename + "/.cache/Communist/wsize";
  p = std::filesystem::u8path(filename);
  if(std::filesystem::exists(p))
    {
      std::fstream f;
      f.open(p, std::ios_base::in);
      while(!f.eof())
        {
          std::string line;
          getline(f, line);
          if(line != "")
            {
              std::tuple<std::string, int, int> ttup;
              int width, height;
              std::string tmp = line;
              tmp = tmp.substr(0, tmp.find(" "));
              std::get<0> (ttup) = tmp;
              tmp = line;
              tmp.erase(0, tmp.find(" ") + std::string(" ").size());
              tmp = tmp.substr(0, tmp.find(" "));
              std::stringstream strm;
              std::locale loc("C");
              strm.imbue(loc);
              strm << tmp;
              strm >> width;
              std::get<1> (ttup) = width;
              tmp = line;
              tmp.erase(0, tmp.find(" ") + std::string(" ").size());
              tmp.erase(0, tmp.find(" ") + std::string(" ").size());
              strm.clear();
              strm.str("");
              strm.imbue(loc);
              strm << tmp;
              strm >> height;
              std::get<2> (ttup) = height;
              winszs.push_back(ttup);
            }
        }
      f.close();
    }
  userCheck();
}

MainWindow::~MainWindow()
{
  if(spch != nullptr)
    {
      delete spch;
    }
}

bool
MainWindow::mainWindowClose()
{
  if(oper != nullptr)
    {
      AuxFunc af;
      std::string filename;
      std::filesystem::path filepath;
      af.homePath(&filename);
      filename = filename + "/.cache/Communist/wsize";
      filepath = std::filesystem::u8path(filename);
      if(!std::filesystem::exists(filepath.parent_path()))
        {
          std::filesystem::create_directories(
            filepath.parent_path());
        }
      int chsv = 1;
      prefvectmtx.lock();
      auto itprv = std::find_if(prefvect.begin(),
                                prefvect.end(), []
                                (auto & el)
      {
        return std::get<0>(el) == "Winsizesv";
      });
      if(itprv != prefvect.end())
        {
          if(std::get<1> (*itprv) != "active")
            {
              chsv = 0;
            }
        }
      prefvectmtx.unlock();
      if(chsv == 1)
        {
          std::fstream f;
          std::vector<std::tuple<std::string, std::string, std::string>>
              wsz;
          if(std::filesystem::exists(filepath))
            {
              f.open(filepath, std::ios_base::in);
              while(!f.eof())
                {
                  std::string line;
                  getline(f, line);
                  if(line != "")
                    {
                      std::tuple<std::string, std::string, std::string> ttup;
                      std::string tmp = line;
                      tmp = tmp.substr(0, tmp.find(" "));
                      std::get<0> (ttup) = tmp;
                      tmp = line;
                      tmp.erase(0, tmp.find(" ") + std::string(" ").size());
                      tmp = tmp.substr(0, tmp.find(" "));
                      std::get<1> (ttup) = tmp;
                      tmp = line;
                      tmp.erase(0, tmp.find(" ") + std::string(" ").size());
                      tmp.erase(0, tmp.find(" ") + std::string(" ").size());
                      std::get<2> (ttup) = tmp;
                      wsz.push_back(ttup);
                    }
                }
              f.close();
            }
          std::string line;
          std::tuple<std::string, std::string, std::string> ttup;
          int width, height;
          width = this->get_width();
          height = this->get_height();
          std::stringstream strm;
          std::locale loc("C");
          strm.imbue(loc);
          strm << width;
          line = strm.str();
          std::get<1> (ttup) = line;
          strm.clear();
          strm.str("");
          strm.imbue(loc);
          strm << height;
          line = strm.str();
          std::get<2> (ttup) = line;
          line = "MainWindow";
          std::get<0> (ttup) = line;
          auto itwsz = std::find_if(wsz.begin(), wsz.end(), [&line]
                                    (auto & el)
          {
            return std::get<0>(el) == line;
          });
          if(itwsz != wsz.end())
            {
              *itwsz = ttup;
            }
          else
            {
              wsz.push_back(ttup);
            }
          if(Mwpaned != nullptr)
            {
              strm.clear();
              strm.str("");
              strm.imbue(loc);
              strm << Mwpaned->get_position();
              line = "MwPaned";
              std::get<0> (ttup) = line;
              std::get<1> (ttup) = strm.str();
              std::get<2> (ttup) = "";
              itwsz = std::find_if(wsz.begin(), wsz.end(), [&line]
                                   (auto & el)
              {
                return std::get<0>(el) == line;
              });
              if(itwsz != wsz.end())
                {
                  *itwsz = ttup;
                }
              else
                {
                  wsz.push_back(ttup);
                }
            }
          Glib::PropertyProxy<bool> px = this->property_maximized();
          if(px.get_value())
            {
              line = "Fullscreened";
              std::get<0> (ttup) = line;
              std::get<1> (ttup) = "1";
              std::get<2> (ttup) = "";
              itwsz = std::find_if(wsz.begin(), wsz.end(), [&line]
                                   (auto & el)
              {
                return std::get<0>(el) == line;
              });
              if(itwsz != wsz.end())
                {
                  *itwsz = ttup;
                }
              else
                {
                  wsz.push_back(ttup);
                }
            }
          else
            {
              line = "Fullscreened";
              wsz.erase(std::remove_if(wsz.begin(), wsz.end(), [&line]
                                       (auto & el)
              {
                return std::get<0>(el) == line;
              }),
              wsz.end());
            }

          f.open(filepath, std::ios_base::out |
                 std::ios_base::binary);
          for(size_t i = 0; i < wsz.size(); i++)
            {
              line = std::get<0> (wsz[i]) + " " + std::get<1>
                     (wsz[i]) + " "
                     + std::get<2> (wsz[i]) + "\n";
              f.write(line.c_str(), line.size());
            }
          f.close();
        }
      else
        {
          if(std::filesystem::exists(filepath))
            {
              std::filesystem::remove_all(filepath);
            }
        }

      this->unset_default_widget();
      this->unset_child();
      this->set_deletable(false);
      Gtk::ProgressBar *bar =
        Gtk::make_managed<Gtk::ProgressBar> ();
      bar->set_name("clBar");
      bar->get_style_context()->add_provider(
        css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      bar->set_halign(Gtk::Align::CENTER);
      bar->set_valign(Gtk::Align::CENTER);
      bar->set_text(gettext("Wait, application is closing..."));
      bar->set_show_text(true);

      this->set_child(*bar);
      Glib::Dispatcher *disppulscl = new Glib::Dispatcher;
      disppulscl->connect([bar]
      {
        bar->pulse();
      });
      dispclose.connect([disppulscl]
      {
        delete disppulscl;
      });
      std::thread *wth = new std::thread([disppulscl, this]
      {
        int count = 0;
        for(;;)
          {
            if(oper == nullptr || count > 50)
              {
                break;
              }
            disppulscl->emit();
            count++;
            usleep(100000);
          }
        dispclose.emit();
      });
      wth->detach();
      delete wth;
      oper->cancelNetOperations();
    }

  if(oper == nullptr)
    {
      this->hide();
    }
  return true;
}

void
MainWindow::userCheck()
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.Communist";
  std::filesystem::path filepath = std::filesystem::u8path(
                                     filename);
  this->set_name("mainWindow");
  this->get_style_context()->add_provider(css_provider,
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
  if(!std::filesystem::exists(filepath))
    {
      std::filesystem::create_directories(filepath);
    }
  if(std::filesystem::is_empty(filepath))
    {
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign(Gtk::Align::CENTER);
      this->unset_default_widget();
      this->unset_child();
      this->set_title(gettext("Profile creation"));
      this->set_child(*grid);

      Gtk::Label *username = Gtk::make_managed<Gtk::Label> ();
      username->set_text(gettext("User name:"));
      username->set_halign(Gtk::Align::CENTER);
      grid->attach(*username, 0, 0, 2, 1);

      Gtk::Entry *usname = new Gtk::Entry;
      usname->set_margin(5);
      usname->set_activates_default(true);
      grid->attach(*usname, 0, 1, 2, 1);

      Gtk::Label *passwd = Gtk::make_managed<Gtk::Label> ();
      passwd->set_text(gettext("Password:"));
      passwd->set_halign(Gtk::Align::CENTER);
      grid->attach(*passwd, 0, 2, 2, 1);

      Gtk::Entry *password = Gtk::make_managed<Gtk::Entry> ();
      password->set_margin(5);
      password->set_activates_default(true);
      Glib::PropertyProxy<Gtk::InputPurpose> propentp =
        password->property_input_purpose();
      propentp.set_value(Gtk::InputPurpose::PASSWORD);
      Glib::PropertyProxy<bool> vis =
        password->property_visibility();
      vis.set_value(false);
      grid->attach(*password, 0, 3, 2, 1);

      Gtk::Label *reppasswd = Gtk::make_managed<Gtk::Label> ();
      reppasswd->set_text(gettext("Repeat password:"));
      reppasswd->set_halign(Gtk::Align::CENTER);
      grid->attach(*reppasswd, 0, 4, 2, 1);

      Gtk::Entry *reppassword = Gtk::make_managed<Gtk::Entry> ();
      reppassword->set_margin(5);
      reppassword->set_activates_default(true);
      Glib::PropertyProxy<Gtk::InputPurpose> propentp2 =
        reppassword->property_input_purpose();
      propentp2.set_value(Gtk::InputPurpose::PASSWORD);
      Glib::PropertyProxy<bool> vis2 =
        reppassword->property_visibility();
      vis2.set_value(false);
      grid->attach(*reppassword, 0, 5, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
      yes->set_label(gettext("Confirm"));
      yes->set_margin(5);
      yes->set_halign(Gtk::Align::CENTER);
      yes->set_name("applyButton");
      yes->get_style_context()->add_provider(css_provider,
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
      yes->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &MainWindow::infoMessage),
                   usname,
                   password, reppassword));
      grid->attach(*yes, 0, 6, 1, 1);

      this->set_default_widget(*yes);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
      cancel->set_label(gettext("Cancel"));
      cancel->set_margin(5);
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_name("rejectButton");
      cancel->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      cancel->signal_clicked().connect([this]
      {
        std::string filename;
        AuxFunc af;
        af.homePath(&filename);
        filename = filename + "/.config/Communist";
        std::filesystem::path prefpath = std::filesystem::u8path(filename);
        std::filesystem::remove_all(prefpath);
        this->close();
      });
      grid->attach(*cancel, 1, 6, 1, 1);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size(rq1, rq2);
      this->set_default_size(rq2.get_width(), -1);
    }
  else
    {
      Deleteprof = 0;
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign(Gtk::Align::CENTER);
      this->unset_default_widget();
      this->unset_child();
      this->set_title(gettext("Login"));
      this->set_child(*grid);

      Gtk::Label *username = Gtk::make_managed<Gtk::Label> ();
      username->set_text(gettext("User name:"));
      username->set_halign(Gtk::Align::CENTER);
      grid->attach(*username, 0, 0, 2, 1);

      Gtk::Entry *usname = new Gtk::Entry;
      usname->set_margin(5);
      usname->set_activates_default(true);
      grid->attach(*usname, 0, 1, 2, 1);

      Gtk::Label *passwd = Gtk::make_managed<Gtk::Label> ();
      passwd->set_text(gettext("Password:"));
      passwd->set_halign(Gtk::Align::CENTER);
      grid->attach(*passwd, 0, 2, 2, 1);

      Gtk::Entry *password = Gtk::make_managed<Gtk::Entry> ();
      password->set_margin(5);
      password->set_activates_default(true);
      Glib::PropertyProxy<Gtk::InputPurpose> propentp =
        password->property_input_purpose();
      propentp.set_value(Gtk::InputPurpose::PASSWORD);
      Glib::PropertyProxy<bool> vis =
        password->property_visibility();
      vis.set_value(false);
      grid->attach(*password, 0, 3, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
      yes->set_label(gettext("Confirm"));
      yes->set_margin(5);
      yes->set_halign(Gtk::Align::CENTER);
      yes->set_name("applyButton");
      yes->get_style_context()->add_provider(css_provider,
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
      yes->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this,
                                 &MainWindow::userCheckFun), usname,
                   password));
      grid->attach(*yes, 0, 4, 1, 1);

      this->set_default_widget(*yes);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
      cancel->set_label(gettext("Cancel"));
      cancel->set_margin(5);
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_name("rejectButton");
      cancel->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      cancel->signal_clicked().connect(
        sigc::mem_fun(*this, &Gtk::Window::close));
      grid->attach(*cancel, 1, 4, 1, 1);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size(rq1, rq2);
      this->set_default_size(rq2.get_width(), -1);
    }
}

void
MainWindow::mainWindow()
{
  contacts.clear();
  AuxFunc af;
  prefvectmtx.lock();
  auto itprv = std::find_if(prefvect.begin(),
                            prefvect.end(), []
                            (auto & el)
  {
    return std::get<0>(el) == "SoundOn";
  });
  if(itprv != prefvect.end())
    {
      if(std::get<1> (*itprv) == "active")
        {
          std::string soundfl = Sharepath;
          soundfl = soundfl + "/Signal.flac";
          itprv = std::find_if(prefvect.begin(), prefvect.end(), []
                               (auto & el)
          {
            return std::get<0>(el) == "SoundPath";
          });
          if(itprv != prefvect.end())
            {
              std::string tsfl = std::get<1> (*itprv);
              std::filesystem::path tsflp = std::filesystem::u8path(
                                              tsfl);
              if(std::filesystem::exists(tsflp))
                {
                  soundfl = tsflp.u8string();
                }
            }
          Glib::RefPtr<Gio::File> sfl = Gio::File::create_for_path(
                                          soundfl);
          mf = Gtk::MediaFile::create(sfl);
        }
    }
  else
    {
      std::string soundfl = Sharepath;
      soundfl = soundfl + "/Signal.flac";
      Glib::RefPtr<Gio::File> sfl = Gio::File::create_for_path(
                                      soundfl);
      mf = Gtk::MediaFile::create(sfl);
    }

  itprv = std::find_if(prefvect.begin(), prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Language";
  });
  if(itprv != prefvect.end())
    {
      std::filesystem::path dicp = std::filesystem::u8path(
                                     std::string(Sharepath + "/HunDict/languages"));
      std::string langnum;
      std::fstream f;
      f.open(dicp, std::ios_base::in);
      if(f.is_open())
        {
          while(!f.eof())
            {
              std::string line;
              getline(f, line);
              if(line != "")
                {
                  langnum = line.substr(0, line.find(" "));
                  line.erase(0, line.find(" ") + std::string(" ").size());
                  if(line == std::get<1> (*itprv))
                    {
                      break;
                    }
                }
            }
          f.close();
        }

      std::string vocp1, vocp2;
      vocp1 = Sharepath + "/HunDict/" + langnum + "/dic.aff";
      vocp2 = Sharepath + "/HunDict/" + langnum + "/dic.dic";
      spch = new Hunspell(vocp1.c_str(), vocp2.c_str());
    }

  prefvectmtx.unlock();
  autoRemoveMsg();
  std::vector<std::string> blockedc;
  frvectmtx.lock();
  for(size_t i = 0; i < friendvect.size(); i++)
    {
      if(std::get<6> (friendvect[i]) != nullptr)
        {
          std::string key = std::string(
                              std::get<2> (friendvect[i])->get_text());
          blockedc.push_back(key);
        }
    }
  friendvect.clear();
  frvectmtx.unlock();
  std::string filename;
  std::filesystem::path filepath;
  std::string mainmenutr;
  this->unset_default_widget();
  this->unset_child();
  avatar = nullptr;
  profilevector.clear();
  Image = 0;
  this->set_title(gettext("Communist"));
  Gtk::Overlay *mw_ovl = Gtk::make_managed<Gtk::Overlay> ();
  this->set_child(*mw_ovl);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::DrawingArea *bckgrnd =
    Gtk::make_managed<Gtk::DrawingArea> ();
  std::filesystem::path backgrpath = std::filesystem::u8path(
                                       std::string(Userthemepath + "/" + Theme +
                                           "/background.jpeg"));
  if(std::filesystem::exists(backgrpath))
    {
      bckgrnd->set_draw_func(
        [this]
        (const Cairo::RefPtr<Cairo::Context> &cr,
         int width,
         int height)
      {
        Glib::ustring file = Glib::ustring(this->Userthemepath +
                                           "/" +
                                           this->Theme + "/background.jpeg");
        Glib::RefPtr<Gdk::Pixbuf> imageloc =
          Gdk::Pixbuf::create_from_file(file);
        imageloc = imageloc->scale_simple(width, height,
                                          Gdk::InterpType::BILINEAR);
        Gdk::Cairo::set_source_pixbuf(cr, imageloc, 0, 0);
        cr->rectangle(0, 0, width, height);
        cr->fill();
      });
    }
  mw_ovl->set_child(*bckgrnd);
  mw_ovl->add_overlay(*grid);
  Glib::RefPtr<Gio::SimpleActionGroup> pref =
    Gio::SimpleActionGroup::create();
  pref->add_action("editpr", sigc::mem_fun(*this,
                   &MainWindow::editProfile));
  pref->add_action("ownkey", sigc::mem_fun(*this,
                   &MainWindow::ownKey));
  pref->add_action("editReqList",
                   sigc::mem_fun(*this, &MainWindow::editAddFriends));
  pref->add_action("editSettings",
                   sigc::mem_fun(*this, &MainWindow::settingsWindow));
  pref->add_action("aboutProg", sigc::mem_fun(*this,
                   &MainWindow::aboutProg));
  pref->add_action("relayList",
                   sigc::mem_fun(*this, &MainWindow::editRelayList));
  this->insert_action_group("prefgr", pref);
  Glib::RefPtr<Gtk::Builder> builder =
    Gtk::Builder::create();

  Glib::ustring buffer =
    "<interface>"
    "<menu id='menubar'>"
    "	<submenu>"
    "		<attribute name='label'>" +
    Glib::ustring(gettext("Contacts")) + "</attribute>"
    "			<section>"
    "				<item>"
    "					<attribute name='label'>" +
    Glib::ustring(gettext("Edit sent request table")) +
    "</attribute>"
    "					<attribute name='action'>prefgr.editReqList</attribute>"
    "				</item>"
    "				<item>"
    "					<attribute name='label'>" + Glib::ustring(
      gettext("Edit contacts to send by relay")) + "</attribute>"
    "					<attribute name='action'>prefgr.relayList</attribute>"
    "				</item>"
    "			</section>"
    "	</submenu>"
    "	<submenu>"
    "		<attribute name='label'>" + Glib::ustring(
      gettext("Instruments")) + "</attribute>"
    "			<section>"
    "				<item>"
    "					<attribute name='label'>" + Glib::ustring(
      gettext("Edit profile")) + "</attribute>"
    "					<attribute name='action'>prefgr.editpr</attribute>"
    "				</item>"
    "				<item>"
    "					<attribute name='label'>" + Glib::ustring(
      gettext("Own key")) + "</attribute>"
    "					<attribute name='action'>prefgr.ownkey</attribute>"
    "				</item>"
    "                         <item>"
    "					<attribute name='label'>" + Glib::ustring(
      gettext("Settings")) + "</attribute>"
    "					<attribute name='action'>prefgr.editSettings</attribute>"
    "				</item>"
    "			</section>"
    "	</submenu>"
    "	<submenu>"
    "		<attribute name='label'>" + Glib::ustring(
      gettext("About")) + "</attribute>"
    "			<section>"
    "				<item>"
    "					<attribute name='label'>" + Glib::ustring(
      gettext("About Communist")) + "</attribute>"
    "					<attribute name='action'>prefgr.aboutProg</attribute>"
    "				</item>"
    "			</section>"
    "	</submenu>"
    "</menu>"
    "</interface>";
  builder->add_from_string(buffer);
  Gtk::Box *box = Gtk::make_managed<Gtk::Box>();
  auto object = builder->get_object("menubar");
  auto gmenu = std::dynamic_pointer_cast<Gio::Menu> (object);
  if(!gmenu)
    {
      g_warning("GMenu not found");
    }
  else
    {
      auto pMenuBar = Gtk::make_managed<Gtk::PopoverMenuBar>
                      (gmenu);
      pMenuBar->set_name("mainMenu");
      pMenuBar->get_style_context()->add_provider(
        css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      box->append(*pMenuBar);
    }
  box->set_halign(Gtk::Align::START);
  grid->attach(*box, 0, 0, 1, 1);

  Gtk::Paned *pane = Gtk::make_managed<Gtk::Paned>();
  Mwpaned = pane;
  pane->set_orientation(Gtk::Orientation::HORIZONTAL);
  Gtk::Grid *leftgrid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::Grid *rightgrid = Gtk::make_managed<Gtk::Grid> ();
  pane->set_start_child(*leftgrid);
  pane->set_end_child(*rightgrid);
  auto itwnszs = std::find_if(winszs.begin(),
                              winszs.end(), []
                              (auto & el)
  {
    return std::get<0>(el) == "MwPaned";
  });
  if(itwnszs != winszs.end())
    {
      pane->set_position(std::get<1> (*itwnszs));
    }
  Rightgrid = rightgrid;

  Winleft = Gtk::make_managed<Gtk::ScrolledWindow> ();
  Winright = Gtk::make_managed<Gtk::ScrolledWindow> ();
  Glib::RefPtr<Gtk::Adjustment> adj =
    Winright->get_vadjustment();

  adj->signal_changed().connect([this, adj]
  {
    if(this->usermsgadj < 0 && this->msgwinadj < 0)
      {

        this->msgwinadjdsp.emit();
      }
    else
      {
        if(this->msgwinadj >= 0)
          {
            adj->set_value(adj->get_upper() - this->msgwinadj);
            this->msgwinadj = -1;
          }
      }
  });
  msgwinadjdsp.connect([adj]
  {
    adj->set_value(adj->get_upper() - adj->get_page_size());
  });
  adj->signal_value_changed().connect([adj, this]
  {
    this->usermsgadj = adj->get_value();
    if(this->usermsgadj == adj->get_upper() - adj->get_page_size())
      {
        this->usermsgadj = -1;
        if(this->msgovllab != nullptr)
          {
            if(this->msgovl != nullptr)
              {
                Gtk::Widget *widg = this->msgovllab->get_parent();
                if(widg != nullptr)
                  {
                    widg = widg->get_parent();
                    if(widg != nullptr)
                      {
                        this->msgovl->remove_overlay(*widg);
                        this->msgovllab = nullptr;
                      }
                  }
              }
          }
      }
  });
  Winleft->set_expand(true);
  Winright->set_expand(true);
  Winright->set_policy(Gtk::PolicyType::AUTOMATIC,
                       Gtk::PolicyType::AUTOMATIC);
  Gtk::Grid *winleftgr = Gtk::make_managed<Gtk::Grid> ();
  fordelgrid = winleftgr;
  leftgrid->attach(*Winleft, 0, 0, 2, 1);
  msgovl = Gtk::make_managed<Gtk::Overlay> ();
  msgovl->set_child(*Winright);
  rightgrid->attach(*msgovl, 0, 0, 3, 1);

  Gtk::Button *addcont = Gtk::make_managed<Gtk::Button> ();
  addcont->set_label(gettext("Add contact"));
  addcont->set_margin(5);
  addcont->set_halign(Gtk::Align::CENTER);
  addcont->set_name("applyButton");
  addcont->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  addcont->signal_clicked().connect(
    sigc::mem_fun(*this, &MainWindow::addFriends));
  leftgrid->attach(*addcont, 0, 1, 1, 1);

  Gtk::Button *delcont = Gtk::make_managed<Gtk::Button> ();
  delcont->set_label(gettext("Remove contact"));
  delcont->set_margin(5);
  delcont->set_halign(Gtk::Align::CENTER);
  delcont->set_name("rejectButton");
  delcont->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  delcont->signal_clicked().connect(
    sigc::mem_fun(*this, &MainWindow::deleteContact));
  Gtk::Button tb;
  Gtk::Grid tg;
  tb.set_child(tg);
  Gtk::DrawingArea dr;
  dr.set_size_request(50, 50);
  dr.set_margin(2);
  tg.attach(dr, 0, 0, 1, 1);
  Gtk::Requisition rqq1, rqq2;
  tb.get_preferred_size(rqq1, rqq2);
  Winleft->set_min_content_height(rqq2.get_height() * 5);
  Gdk::Rectangle rctg = screenRes();
  Winleft->set_min_content_width(rctg.get_width() * 0.25);
  leftgrid->attach(*delcont, 1, 1, 1, 1);
  leftgrid->get_preferred_size(rqq1, rqq2);
  contact_button_width = rqq2.get_width() - 5;
  af.homePath(&filename);
  proffopmtx.lock();
  std::fstream f;
  std::string line;
  std::string outfile;
  int count = 0;
  LibCommunist Lc;
  contmtx.lock();
  contacts = Lc.readContacts(filename, Username, Password);
  contmtx.unlock();

  addfrmtx.lock();
  Addfriends = Lc.readRequestList(filename, Username,
                                  Password);
  addfrmtx.unlock();
  std::vector<std::tuple<int, std::string, int>> accountlist;
  contmtx.lock();
  std::string homepath;
  af.homePath(&homepath);
#ifdef __linux
  filename =
    std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
  filename =
    std::filesystem::temp_directory_path().parent_path().u8string();
#endif
  filename = filename + "/" + Lc.randomFileName();
  for(size_t i = 0; i < contacts.size(); i++)
    {
      std::tuple<int, std::string, int> ttup;
      int ind = std::get<0> (contacts[i]);
      std::get<2> (ttup) = ind;
      std::string keyc = std::get<1> (contacts[i]);
      std::get<0> (ttup) = 0;
      std::get<1> (ttup) = keyc;
      yesmtx.lock();
      std::string logp = Lc.getContactMsgLog(homepath, Username,
                                             Password, ind,
                                             filename);

      filepath = std::filesystem::u8path(logp);
      count = 0;
      f.open(filepath, std::ios_base::in);
      if(f.is_open())
        {
          while(!f.eof())
            {
              std::string line;
              getline(f, line);
              if(!line.empty() && count > 0)
                {
                  line.erase(0, line.find(" ") + std::string(" ").size());
                  std::stringstream strm;
                  std::locale loc("C");
                  strm.imbue(loc);
                  strm << line;
                  int value;
                  strm >> value;
                  std::get<0> (ttup) = value;
                }
              count++;
            }
          f.close();
        }
      accountlist.push_back(ttup);
      filepath = std::filesystem::u8path(filename);
      std::filesystem::remove_all(filepath);
      yesmtx.unlock();
    }
  contmtx.unlock();
  if(accountlist.size() > 0)
    {
      std::sort(accountlist.begin(), accountlist.end(), []
                (auto el1, auto el2)
      {
        if(std::get<0>(el1) == 0 && std::get<0>(el2) == 0)
          {
            return std::get<2>(el2) < std::get<2>(el1);
          }
        else
          {
            return std::get<0>(el1) > std::get<0>(el2);
          }
      });
      Gtk::Requisition rq1, rq2;
      for(size_t i = 0; i < accountlist.size(); i++)
        {
          af.homePath(&homepath);
          std::vector<std::tuple<std::string, std::vector<char>>>
          frprofvect;
          contmtx.lock();
          frprofvect = Lc.readFriendProfile(homepath,
                                            std::get<1> (accountlist[i]),
                                            Username, Password);
          contmtx.unlock();
          if(frprofvect.size() > 0)
            {
              std::tuple<Gtk::Button*, Gtk::Grid*, Gtk::Label*, Gtk::Label*,
                  Gtk::Label*, Gtk::Label*, Gtk::Label*, Gtk::PopoverMenu*>
                  frtup;
              Gtk::Button *buttontr = Gtk::make_managed<Gtk::Button> ();
              std::get<0> (frtup) = buttontr;
              buttontr->get_style_context()->add_provider(
                css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
              buttontr->set_name("inactiveButton");
              Gtk::Grid *gridtr = Gtk::make_managed<Gtk::Grid> ();
              buttontr->set_child(*gridtr);
              Glib::RefPtr<Gtk::GestureClick> clck =
                Gtk::GestureClick::create();
              clck->set_button(3);
              clck->signal_pressed().connect(
                sigc::bind(sigc::mem_fun(*this, &MainWindow::contMenu),
                           clck, buttontr));
              buttontr->add_controller(clck);
              Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
                Gio::SimpleActionGroup::create();
              acgroup->add_action(
                "info",
                sigc::bind(sigc::mem_fun(*this,
                                         &MainWindow::friendDetails),
                           buttontr));
              acgroup->add_action(
                "delete", sigc::mem_fun(*this,
                                        &MainWindow::deleteContact));
              acgroup->add_action(
                "block", sigc::mem_fun(*this, &MainWindow::tempBlockCont));
              buttontr->insert_action_group("popupcont", acgroup);
              std::get<1> (frtup) = gridtr;
              std::vector<char> picture;
              auto itprv = std::find_if(frprofvect.begin(),
                                        frprofvect.end(),
                                        []
                                        (auto & el)
              {
                return std::get<0>(el) == "Avatar";
              });
              if(itprv != frprofvect.end())
                {
                  picture = std::get<1> (*itprv);
                }
              Gtk::DrawingArea *drar =
                Gtk::make_managed<Gtk::DrawingArea> ();
              drar->set_margin(2);
              drar->set_halign(Gtk::Align::CENTER);
              drar->set_size_request(50, 50);
              gridtr->attach(*drar, 0, 0, 1, 3);
              Gtk::Label *keylab = Gtk::make_managed<Gtk::Label> ();
              std::get<2> (frtup) = keylab;
              keylab->set_max_width_chars(20);
              keylab->set_ellipsize(Pango::EllipsizeMode::END);
              keylab->set_text(std::get<1> (accountlist[i]));
              gridtr->attach(*keylab, 1, 0, 1, 1);

              if(picture.size() > 0)
                {
                  std::string avatarpath;
#ifdef __linux
                  avatarpath =
                    std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
                  avatarpath =
                    std::filesystem::temp_directory_path().parent_path().u8string();
#endif
                  avatarpath = avatarpath + "/" + Lc.randomFileName()
                               + ".jpeg";
                  filepath = std::filesystem::u8path(avatarpath);
                  std::fstream f;
                  f.open(filepath, std::ios_base::out |
                         std::ios_base::binary);
                  f.write(&picture[0], picture.size());
                  f.close();
                  Glib::RefPtr<Gdk::Pixbuf> imagec =
                    Gdk::Pixbuf::create_from_file(
                      Glib::ustring(avatarpath));
                  int drarw, drarh;
                  drar->get_size_request(drarw, drarh);
                  imagec = imagec->scale_simple(drarw, drarh,
                                                Gdk::InterpType::BILINEAR);
                  std::pair<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> pair;
                  pair.first = keylab->get_text();
                  pair.second = imagec;
                  conavmtx.lock();
                  conavatars.push_back(pair);
                  conavmtx.unlock();
                  drar->set_draw_func(
                    sigc::bind(
                      sigc::mem_fun(*this,
                                    &MainWindow::on_draw_contactlist),
                      keylab));
                  std::filesystem::remove_all(filepath);
                }
              Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label> ();
              itprv = std::find_if(frprofvect.begin(),
                                   frprofvect.end(), []
                                   (auto & el)
              {
                return std::get<0>(el) == "Nick";
              });
              if(itprv != frprofvect.end())
                {
                  std::vector<char> val = std::get<1> (*itprv);
                  std::string line;
                  std::copy(val.begin(), val.end(),
                            std::back_inserter(line));
                  nicklab->set_text(Glib::ustring(line));
                }
              nicklab->set_halign(Gtk::Align::START);
              gridtr->attach(*nicklab, 1, 1, 1, 1);
              std::get<4> (frtup) = nicklab;

              Gtk::Label *namelab = Gtk::make_managed<Gtk::Label> ();
              itprv = std::find_if(frprofvect.begin(),
                                   frprofvect.end(), []
                                   (auto & el)
              {
                return std::get<0>(el) == "Surname";
              });
              if(itprv != frprofvect.end())
                {
                  std::vector<char> val = std::get<1> (*itprv);
                  std::string line;
                  std::copy(val.begin(), val.end(),
                            std::back_inserter(line));
                  Glib::ustring surnm(line);
                  itprv = std::find_if(frprofvect.begin(),
                                       frprofvect.end(),
                                       []
                                       (auto & el)
                  {
                    return std::get<0>(el) == "Name";
                  });
                  if(itprv != frprofvect.end())
                    {
                      val.clear();
                      line.clear();
                      val = std::get<1> (*itprv);
                      std::copy(val.begin(), val.end(),
                                std::back_inserter(line));
                      Glib::ustring nm(line);
                      namelab->set_text(Glib::ustring(nm + " " + surnm));
                    }
                  else
                    {
                      namelab->set_text(Glib::ustring(surnm));
                    }
                }
              else
                {
                  itprv = std::find_if(frprofvect.begin(),
                                       frprofvect.end(),
                                       []
                                       (auto & el)
                  {
                    return std::get<0>(el) == "Name";
                  });
                  if(itprv != frprofvect.end())
                    {
                      std::vector<char> val = std::get<1> (*itprv);
                      std::string line;
                      std::copy(val.begin(), val.end(),
                                std::back_inserter(line));
                      Glib::ustring nm(line);
                      namelab->set_text(nm);
                    }
                  else
                    {
                      namelab->set_text("");
                    }
                }
              namelab->set_halign(Gtk::Align::START);
              gridtr->attach(*namelab, 1, 2, 1, 1);
              std::get<5> (frtup) = namelab;

              buttontr->set_size_request(contact_button_width, -1);
              winleftgr->attach(*buttontr, 0, i, 1, 1);
              buttontr->get_preferred_size(rq1, rq2);
              buttontr->signal_clicked().connect(
                sigc::bind(sigc::mem_fun(*this,
                                         &MainWindow::selectContact),
                           Winright, std::string(keylab->get_text()),
                           std::string(nicklab->get_text())));
              std::string searchk(std::get<1> (accountlist[i]));
              auto itbll = std::find(blockedc.begin(), blockedc.end(),
                                     searchk);
              if(itbll != blockedc.end())
                {
                  Gtk::Label *blocklab = Gtk::make_managed<Gtk::Label> ();
                  blocklab->set_name("blockLab");
                  blocklab->get_style_context()->add_provider(
                    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
                  blocklab->set_text(gettext("Blocked"));
                  gridtr->attach_next_to(*blocklab, *nicklab,
                                         Gtk::PositionType::RIGHT, 1, 1);
                  std::get<6> (frtup) = blocklab;
                }
              else
                {
                  std::get<6> (frtup) = nullptr;
                }

              std::get<3> (frtup) = nullptr;
              std::get<7> (frtup) = nullptr;

              frvectmtx.lock();
              friendvect.push_back(frtup);
              frvectmtx.unlock();
            }
          else
            {
              std::cerr << "Error on opening friends' profile" <<
                        std::endl;
            }
        }
      Winleft->set_margin(2);
    }
  proffopmtx.unlock();
  Gtk::Requisition rq1, rq2;
  Winleft->set_child(*winleftgr);
  Winleft->set_policy(Gtk::PolicyType::AUTOMATIC,
                      Gtk::PolicyType::AUTOMATIC);
  Winleft->set_expand(true);
  leftgrid->get_preferred_size(rq1, rq2);
  pane->set_resize_start_child(false);

  Gtk::Button *attachfile = Gtk::make_managed<Gtk::Button> ();
  attachfile->set_margin(5);
  attachfile->set_halign(Gtk::Align::CENTER);
  attachfile->set_valign(Gtk::Align::CENTER);
  attachfile->get_preferred_size(rqq1, rqq2);
  attachfile->set_name("sendButton");
  attachfile->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  attachfile->set_tooltip_text(gettext("Attach file"));
  Gtk::DrawingArea *drarfb =
    Gtk::make_managed<Gtk::DrawingArea> ();
  drarfb->set_size_request(rqq1.get_width(),
                           rqq1.get_height());
  drarfb->set_draw_func(
    sigc::bind(
      sigc::mem_fun(*this, &MainWindow::on_draw_sb),
      Glib::ustring(Userthemepath + "/" + Theme +
                    "/file-icon.png")));
  attachfile->set_child(*drarfb);
  attachfile->signal_clicked().connect(
    sigc::mem_fun(*this, &MainWindow::attachFileDialog));
  attachfbutton = attachfile;
  rightgrid->attach(*attachfile, 0, 1, 1, 1);

  Gtk::ScrolledWindow *winmsg =
    Gtk::make_managed<Gtk::ScrolledWindow> ();
  msgfrm = Gtk::make_managed<Gtk::TextView> ();
  msgfrm->set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
  Glib::RefPtr<Gtk::EventControllerKey> entcont =
    Gtk::EventControllerKey::create();
  entcont->signal_key_pressed().connect(
    sigc::mem_fun(*this, &MainWindow::sendMsgByKeyBoard),
    false);
  msgfrm->add_controller(entcont);
  winmsg->set_min_content_width(rq2.get_width());
  winmsg->set_margin(5);
  winmsg->set_policy(Gtk::PolicyType::AUTOMATIC,
                     Gtk::PolicyType::AUTOMATIC);
  winmsg->set_has_frame(true);
  winmsg->set_hexpand(true);
  winmsg->set_child(*msgfrm);
  rightgrid->attach(*winmsg, 1, 1, 1, 1);

  Glib::RefPtr<Gtk::TextBuffer> textb = msgfrm->get_buffer();
  std::string spchst = "1";
  prefvectmtx.lock();
  itprv = std::find_if(prefvect.begin(), prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Spellcheck";
  });
  if(itprv != prefvect.end())
    {
      spchst = std::get<1> (*itprv);
    }
  prefvectmtx.unlock();
  if(spchst == "1" && spch != nullptr)
    {
      textb->signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &MainWindow::spellCheck),
                   textb));
    }

  Gtk::Button *sendb = Gtk::make_managed<Gtk::Button> ();
  sendb->set_name("sendButton");
  sendb->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  sendb->set_margin(5);
  sendb->set_halign(Gtk::Align::CENTER);
  sendb->set_valign(Gtk::Align::CENTER);
  sendb->get_preferred_size(rq1, rq2);
  sendb->set_tooltip_text(gettext("Send message"));
  sendb->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::sendMsg),
               msgfrm));
  Gtk::DrawingArea *drarsb =
    Gtk::make_managed<Gtk::DrawingArea> ();
  drarsb->set_size_request(rq2.get_width(),
                           rq2.get_height());
  std::string spi = Userthemepath + "/" + Theme + "/ico.png";
  drarsb->set_draw_func(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::on_draw_sb),
               Glib::ustring(spi)));
  sendb->set_child(*drarsb);
  rightgrid->attach(*sendb, 2, 1, 1, 1);
  grid->attach(*pane, 0, 1, 1, 1);

  Glib::PropertyProxy<bool> px = this->property_maximized();
  itwnszs = std::find_if(winszs.begin(), winszs.end(), []
                         (auto & el)
  {
    return std::get<0>(el) == "Fullscreened";
  });
  if(itwnszs != winszs.end())
    {
      if(std::get<1> (*itwnszs) == 1)
        {
          px.set_value(true);
        }
    }

  grid->get_preferred_size(rq1, rq2);
  itwnszs = std::find_if(winszs.begin(), winszs.end(), []
                         (auto & el)
  {
    return std::get<0>(el) == "MainWindow";
  });
  if(itwnszs != winszs.end())
    {
      this->set_default_size(std::get<1> (*itwnszs),
                             std::get<2> (*itwnszs));
    }
  else
    {
      this->set_default_size(rq2.get_width(),
                             rq2.get_height());
    }

  Glib::RefPtr<Gtk::GestureClick> rightgridclck =
    Gtk::GestureClick::create();
  rightgridclck->set_button(0);
  rightgridclck->signal_pressed().connect([rightgridclck,
                                          this]
                                          (int clcknum, double x, double y)
  {
    if(rightgridclck->get_current_button() == 1)
      {
        if(this->rightmenfordel != nullptr)
          {
            this->rightmenfordel->hide();
            this->rightmenfordel = nullptr;
          }
      }
  });
  rightgrid->add_controller(rightgridclck);

  networkOp();
}

void
MainWindow::spellCheck(Glib::RefPtr<Gtk::TextBuffer> textb)
{
  Glib::ustring txt = textb->get_text(true);
  if(txt.size() > 0)
    {
      Glib::ustring::size_type n = 0;
      std::vector<std::tuple<Glib::ustring::size_type, Glib::ustring::size_type>>
          tv;
      while(n != Glib::ustring::npos)
        {
          if(n > 0)
            {
              n = n + Glib::ustring(" ").size();
            }
          Glib::ustring t;
          t = txt.substr(n, txt.find(" ", n) - n);
          if(t != "" && txt.find(" ", n) != Glib::ustring::npos)
            {
              tv.push_back(std::make_tuple(n, txt.find(" ", n) - n));
            }
          else
            {
              if(txt.find(" ", n) == Glib::ustring::npos)
                {
                  tv.push_back(std::make_tuple(n, txt.size() - 1));
                }
            }
          n = txt.find(" ", n);
        }

      for(size_t i = 0; i < tv.size(); i++)
        {
          std::string word = txt.substr(std::get<0> (tv[i]),
                                        std::get<1> (tv[i]));
          AuxFunc af;
          word = af.utf8to(word);
          for(size_t j = 0, len = word.size(); j < len; j++)
            {
              if(std::ispunct((word[j])))
                {
                  word.erase(j);
                  len = word.size();
                }
            }
          af.toutf8(word);
          std::string wordlow = af.stringToLower(word);
          int cor = spch->spell(wordlow);
          if(cor < 1)
            {
              auto tgtb = textb->get_tag_table();
              Glib::RefPtr<Gtk::TextTag> tg;
              tg = tgtb->lookup("error");
              if(tg == nullptr)
                {
                  tg = Gtk::TextBuffer::Tag::create("error");
                  Glib::PropertyProxy<Pango::Underline> prx =
                    tg->property_underline();
#ifdef __linux
                  prx.set_value(Pango::Underline::ERROR);
#endif
#ifdef _WIN32
                  prx.set_value(Pango::Underline::ERROR_LINE);
#endif
                  tgtb->add(tg);
                }
              Gtk::TextBuffer::iterator itb;
              itb = textb->get_iter_at_offset(int (std::get<0> (tv[i])));

              Gtk::TextBuffer::iterator ite;
              ite = textb->get_iter_at_offset(
                      int (std::get<0> (tv[i])) + int (std::get<1> (tv[i])));
              textb->apply_tag(tg, itb, ite);
            }
          else
            {
              Gtk::TextBuffer::iterator itb;
              itb = textb->get_iter_at_offset(int (std::get<0> (tv[i])));

              Gtk::TextBuffer::iterator ite;
              ite = textb->get_iter_at_offset(
                      int (std::get<0> (tv[i])) + int (std::get<1> (tv[i])));
              textb->remove_all_tags(itb, ite);
            }
        }
    }
}

bool
MainWindow::sendMsgByKeyBoard(guint keyval, guint keycode,
                              Gdk::ModifierType state)
{
  if(keyval == GDK_KEY_Return)
    {
      prefvectmtx.lock();
      auto itprv = std::find_if(prefvect.begin(),
                                prefvect.end(), []
                                (auto & el)
      {
        return std::get<0>(el) == "SendKey";
      });
      if(itprv != prefvect.end())
        {
          if(std::get<1> (*itprv) == "0")
            {
              if(keyval == GDK_KEY_Return
                  && (state
                      & (Gdk::ModifierType::SHIFT_MASK
                         | Gdk::ModifierType::CONTROL_MASK
                         | Gdk::ModifierType::ALT_MASK))
                  == Gdk::ModifierType::CONTROL_MASK)
                {
                  sendMsg(msgfrm);
                  prefvectmtx.unlock();
                  return true;
                }
              else
                {
                  Glib::RefPtr<Gtk::TextBuffer> tb = msgfrm->get_buffer();
                  Glib::ustring txt = tb->get_text(true);
                  Glib::ustring::size_type n = 0;
                  std::vector <
                  std::tuple<Glib::ustring::size_type,
                      Glib::ustring::size_type >> tv;
                  while(n != Glib::ustring::npos)
                    {
                      if(n > 0)
                        {
                          n = n + Glib::ustring(" ").size();
                        }
                      Glib::ustring t;
                      t = txt.substr(n, txt.find(" ", n) - n);
                      if(t != "" && txt.find(" ", n) != Glib::ustring::npos)
                        {
                          tv.push_back(
                            std::make_tuple(n, txt.find(" ", n) - n));
                        }
                      else
                        {
                          if(txt.find(" ", n) == Glib::ustring::npos)
                            {
                              tv.push_back(
                                std::make_tuple(n, txt.size() - 1));
                            }
                        }
                      n = txt.find(" ", n);
                    }
                  prefvectmtx.unlock();
                  return false;
                }
            }
          else
            {
              if(keyval == GDK_KEY_Return)
                {
                  sendMsg(msgfrm);
                  prefvectmtx.unlock();
                  return true;
                }
            }
        }
      else
        {
          if(keyval == GDK_KEY_Return
              && (state
                  & (Gdk::ModifierType::SHIFT_MASK
                     | Gdk::ModifierType::CONTROL_MASK
                     | Gdk::ModifierType::ALT_MASK))
              == Gdk::ModifierType::CONTROL_MASK)
            {
              sendMsg(msgfrm);
              prefvectmtx.unlock();
              return true;
            }
          else
            {
              prefvectmtx.unlock();
              return false;
            }
        }
      prefvectmtx.unlock();
    }

  return false;
}

void
MainWindow::infoMessage(Gtk::Entry *usname,
                        Gtk::Entry *passwd,
                        Gtk::Entry *reppasswd)
{
  Glib::ustring username = usname->get_text();
  Glib::ustring password = passwd->get_text();
  Glib::ustring reppassword = reppasswd->get_text();
  if(username != "" && password != ""
      && password == reppassword)
    {
      Username = std::string(username);
      Password = std::string(password);
      createProfile();
    }
  else
    {
      AuxWindows aw(this);
      aw.infoMessage(username, password, reppassword);
    }
}

void
MainWindow::userCheckFun(Gtk::Entry *usname,
                         Gtk::Entry *passwd)
{
  AuxFunc af;
  Glib::ustring username = usname->get_text();
  Glib::ustring password = passwd->get_text();
  if(username != "" && password != "")
    {
      Username = std::string(username);
      Password = std::string(password);
      std::string homepath;
      af.homePath(&homepath);
      LibCommunist Lc;
      int result = Lc.readSeed(homepath, Username, Password,
                               seed);
      if(result >= 0)
        {
          mainWindow();
        }
    }
}

void
MainWindow::createProfile()
{
  this->unset_default_widget();
  this->unset_child();
  this->set_default_size(-1, -1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign(Gtk::Align::CENTER);
  this->set_title(gettext("Profile creation"));
  this->set_child(*grid);

  Gtk::Grid *leftgrid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::Grid *rightgrid = Gtk::make_managed<Gtk::Grid> ();

  grid->attach(*leftgrid, 0, 0, 1, 1);
  grid->attach(*rightgrid, 1, 0, 1, 1);

  Gtk::Entry *key = Gtk::make_managed<Gtk::Entry> ();
  key->set_placeholder_text(
    gettext("Generate key (compulsory)"));
  key->set_margin(5);
  key->set_editable(false);

  key->set_max_width_chars(64);
  key->set_halign(Gtk::Align::CENTER);
  rightgrid->attach(*key, 0, 0, 3, 1);
  profilevector.push_back(key);

  Gtk::Button *generate = Gtk::make_managed<Gtk::Button> ();
  generate->set_label(gettext("Generate"));
  generate->set_margin(5);
  generate->set_halign(Gtk::Align::START);
  generate->set_name("applyButton");
  generate->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  generate->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::keyGenerate),
               key));
  rightgrid->attach(*generate, 0, 1, 3, 1);

  Gtk::Entry *nickname = Gtk::make_managed<Gtk::Entry> ();
  nickname->set_placeholder_text(
    gettext("Nickname (compulsory)"));
  nickname->set_margin(5);
  rightgrid->attach(*nickname, 0, 2, 3, 1);
  profilevector.push_back(nickname);

  Gtk::Entry *name = Gtk::make_managed<Gtk::Entry> ();
  name->set_placeholder_text(
    gettext("Name (not compulsory)"));
  name->set_margin(5);
  rightgrid->attach(*name, 0, 3, 3, 1);
  profilevector.push_back(name);

  Gtk::Entry *surname = Gtk::make_managed<Gtk::Entry> ();
  surname->set_placeholder_text(
    gettext("Surame (not compulsory)"));
  surname->set_margin(5);
  rightgrid->attach(*surname, 0, 4, 3, 1);
  profilevector.push_back(surname);

  Gtk::Button *saveprof = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *spl = Gtk::make_managed<Gtk::Label> ();
  spl->set_text(gettext("Save"));
  spl->set_justify(Gtk::Justification::CENTER);
  saveprof->set_child(*spl);
  saveprof->set_margin(5);
  saveprof->set_halign(Gtk::Align::CENTER);
  saveprof->set_name("applyButton");
  saveprof->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  saveprof->signal_clicked().connect(
    sigc::mem_fun(*this, &MainWindow::saveProfile));
  rightgrid->attach(*saveprof, 0, 5, 1, 1);

  Gtk::Button *cleardata = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *cdl = Gtk::make_managed<Gtk::Label> ();
  cdl->set_text(gettext("Clear"));
  cdl->set_justify(Gtk::Justification::CENTER);
  cleardata->set_child(*cdl);
  cleardata->set_margin(5);
  cleardata->set_halign(Gtk::Align::CENTER);
  cleardata->set_name("clearButton");
  cleardata->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  cleardata->signal_clicked().connect([key, nickname, name,
                                            surname]
  {
    key->set_text("");
    nickname->set_text("");
    name->set_text("");
    surname->set_text("");
  });
  rightgrid->attach(*cleardata, 1, 5, 1, 1);

  Gtk::Button *cancela = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *cal = Gtk::make_managed<Gtk::Label> ();
  cal->set_text(gettext("Cancel"));
  cal->set_justify(Gtk::Justification::CENTER);
  cancela->set_child(*cal);
  cancela->set_margin(5);
  cancela->set_halign(Gtk::Align::CENTER);
  cancela->set_name("rejectButton");
  cancela->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancela->signal_clicked().connect([this]
  {
    std::string filename;
    AuxFunc af;
    af.homePath(&filename);
    filename = filename + "/.config/Communist";
    std::filesystem::path prefpath = std::filesystem::u8path(filename);
    std::filesystem::remove_all(prefpath);
    this->close();
  });
  rightgrid->attach(*cancela, 2, 5, 1, 1);

  avatar = Gtk::make_managed<Gtk::DrawingArea> ();
  avatar->set_margin(5);
  avatar->set_halign(Gtk::Align::CENTER);
  avatar->set_size_request(200, 200);
  leftgrid->attach(*avatar, 0, 0, 2, 1);

  Gtk::Button *addavatar = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *oblab = Gtk::make_managed<Gtk::Label> ();
  oblab->set_text(gettext("Choose image"));
  oblab->set_justify(Gtk::Justification::CENTER);
  addavatar->set_child(*oblab);
  addavatar->set_margin(5);
  addavatar->set_halign(Gtk::Align::CENTER);
  addavatar->set_name("applyButton");
  addavatar->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  addavatar->signal_clicked().connect(
    sigc::mem_fun(*this, &MainWindow::openImage));
  leftgrid->attach(*addavatar, 0, 1, 1, 1);

  Gtk::Button *cleardraw = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *oblab2 = Gtk::make_managed<Gtk::Label> ();
  oblab2->set_text(gettext("Remove image"));
  oblab2->set_justify(Gtk::Justification::CENTER);
  cleardraw->set_child(*oblab2);
  cleardraw->set_margin(5);
  cleardraw->set_halign(Gtk::Align::CENTER);
  cleardraw->set_name("clearButton");
  cleardraw->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  cleardraw->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::clearAvatar),
               leftgrid));
  leftgrid->attach(*cleardraw, 1, 1, 1, 1);
}

void
MainWindow::keyGenerate(Gtk::Entry *key)
{
  LibCommunist Lc;
  seed = Lc.seedGenerate();
  Glib::ustring keystr(Lc.getKeyFmSeed(seed));
  key->set_text(keystr);
}

void
MainWindow::openImage()
{
  Glib::RefPtr<Gtk::FileChooserNative> dialog =
    Gtk::FileChooserNative::create(
      gettext("Open image"), *this,
      Gtk::FileChooser::Action::OPEN,
      gettext("Open"), gettext("Cancel"));
  dialog->signal_response().connect(
    sigc::bind(sigc::mem_fun(*this,
                             &MainWindow::MainWindow::onopenImage),
               dialog));
  dialog->show();
}

void
MainWindow::onopenImage(int respons_id,
                        Glib::RefPtr<Gtk::FileChooserNative> dialog)
{
  Glib::ustring file;
  if(respons_id == Gtk::ResponseType::ACCEPT)
    {
      file = dialog->get_file()->get_path();
      avatar->set_draw_func(
        sigc::bind(sigc::mem_fun(*this, &MainWindow::on_draw),
                   file));
    }
}

void
MainWindow::on_draw(const Cairo::RefPtr<Cairo::Context>
                    &cr, int width,
                    int height, Glib::ustring file)
{
  image = Gdk::Pixbuf::create_from_file(file);
  image = image->scale_simple(width, height,
                              Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
  cr->rectangle(0, 0, width, height);
  cr->fill();
  Image = 1;
}

void
MainWindow::on_draw_ep(const Cairo::RefPtr<Cairo::Context>
                       &cr, int width,
                       int height, std::vector<char> *picture)
{
  Glib::ustring file;
  LibCommunist Lc;
  std::string filename;
#ifdef __linux
  filename =
    std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
  filename =
    std::filesystem::temp_directory_path().parent_path().u8string();
#endif
  filename = filename + "/" + Lc.randomFileName();
  std::filesystem::path filepath = std::filesystem::u8path(
                                     filename);
  std::fstream f;
  if(picture->size() > 0)
    {
      std::vector<char> loc = *picture;
      f.open(filepath, std::ios_base::out |
             std::ios_base::binary);
      f.write(&loc[0], loc.size());
      f.close();
    }
  file = Glib::ustring(filename);
  image = Gdk::Pixbuf::create_from_file(file);
  image = image->scale_simple(width, height,
                              Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
  cr->rectangle(0, 0, width, height);
  cr->fill();
  Image = 1;
  delete picture;
  std::filesystem::remove_all(filepath);
}

void
MainWindow::on_draw_sb(const Cairo::RefPtr<Cairo::Context>
                       &cr, int width,
                       int height, Glib::ustring file)
{
  auto imagesb = Gdk::Pixbuf::create_from_file(file);
  imagesb = imagesb->scale_simple(width, height,
                                  Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf(cr, imagesb, 0, 0);
  cr->rectangle(0, 0, width, height);
  cr->fill();
}

void
MainWindow::on_draw_contactlist(const
                                Cairo::RefPtr<Cairo::Context> &cr,
                                int width, int height, Gtk::Label *keylab)
{
  conavmtx.lock();
  auto iter = std::find_if(conavatars.begin(),
                           conavatars.end(), [keylab]
                           (auto & el)
  {
    return keylab->get_text() == el.first;
  });
  Glib::RefPtr<Gdk::Pixbuf> imagec;
  if(iter != conavatars.end())
    {
      imagec = (*iter).second;
      Gdk::Cairo::set_source_pixbuf(cr, imagec, 0, 0);
      cr->rectangle(0, 0, width, height);
      cr->fill();
    }
  conavmtx.unlock();
}

void
MainWindow::clearAvatar(Gtk::Grid *leftgrid)
{
  leftgrid->remove(*avatar);
  avatar = Gtk::make_managed<Gtk::DrawingArea> ();
  avatar->set_margin(5);
  avatar->set_halign(Gtk::Align::CENTER);
  avatar->set_size_request(200, 200);
  leftgrid->attach(*avatar, 0, 0, 2, 1);
  Image = 0;
}

void
MainWindow::saveProfile()
{
  int notcompl = 0;
  for(size_t i = 0; i < profilevector.size(); i++)
    {
      Gtk::Entry *ent = profilevector[i];
      if(ent->get_text() == "")
        {
          Gtk::Window *window = new Gtk::Window;
          window->set_name("settingsWindow");
          window->get_style_context()->add_provider(
            css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
          window->set_title(gettext("Error"));
          Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
          window->set_child(*grid);

          Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
          label->set_text("");
          if(i == 0)
            {
              label->set_text(gettext("Generate the key!"));
            }
          if(i == 1)
            {
              label->set_text(gettext("Input nickname!"));
            }

          label->set_halign(Gtk::Align::CENTER);
          label->set_margin(5);
          grid->attach(*label, 0, 0, 1, 1);

          Gtk::Button *button = Gtk::make_managed<Gtk::Button> ();
          button->set_label(gettext("Close"));
          button->set_halign(Gtk::Align::CENTER);
          button->set_margin(5);
          button->set_name("applyButton");
          button->get_style_context()->add_provider(
            css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
          button->signal_clicked().connect(
            sigc::mem_fun(*window, &Gtk::Window::close));
          grid->attach(*button, 0, 1, 1, 1);
          window->signal_close_request().connect([window]
          {
            window->hide();
            delete window;
            return true;
          },
          false);
          if(label->get_text() != "")
            {
              window->set_application(this->get_application());
              window->show();
              notcompl = 1;
              break;
            }
          else
            {
              delete window;
            }
        }
    }
  if(notcompl == 0)
    {
      std::vector<std::tuple<std::string, std::string>> profvect;
      for(size_t i = 1; i < profilevector.size(); i++)
        {
          Gtk::Entry *ent = profilevector[i];
          std::tuple<std::string, std::string> ttup;
          if(i == 1)
            {
              std::get<0> (ttup) = "Nick";
              std::get<1> (ttup) = std::string(ent->get_text());
            }
          if(i == 2)
            {
              std::get<0> (ttup) = "Name";
              std::get<1> (ttup) = std::string(ent->get_text());
            }
          if(i == 3)
            {
              std::get<0> (ttup) = "Surname";
              std::get<1> (ttup) = std::string(ent->get_text());
            }
          profvect.push_back(ttup);
        }
      std::string filename;
      AuxFunc af;
      if(Image == 1)
        {
#ifdef __linux
          filename =
            std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
          filename =
            std::filesystem::temp_directory_path().parent_path().u8string();
#endif
          filename = filename + "/Avatar.jpeg";
          std::filesystem::path filepath = std::filesystem::u8path(
                                             filename);
          image->save(filepath.string(), "jpeg");
          profvect.push_back(std::make_tuple("Avatar", filename));
          Image = 0;
        }
      af.homePath(&filename);
      LibCommunist Lc;
      int result = Lc.createProfile(Username, Password, filename,
                                    profvect,
                                    seed);
      if(result >= 0)
        {
          auto itprv = std::find_if(profvect.begin(),
                                    profvect.end(), []
                                    (auto & el)
          {
            return std::get<0>(el) == "Avatar";
          });
          if(itprv != profvect.end())
            {
              filename = std::get<1> (*itprv);
              if(filename != "")
                {
                  std::filesystem::path filepath = std::filesystem::u8path(
                                                     filename);
                  std::filesystem::remove_all(filepath);
                }
            }
          Deleteprof = 0;
          if(result == 0)
            {
              std::cout << "Avatar not found!" << std::endl;
            }
          mainWindow();
        }
      else
        {
          std::cerr << "Error on profile saving!" << std::endl;
        }
    }
}

void
MainWindow::ownKey()
{
  AuxWindows aw(this);
  aw.ownKey();
}

void
MainWindow::selectContact(Gtk::ScrolledWindow *scr,
                          std::string selkey,
                          std::string selnick)
{
  ContactOperations cop(this);
  cop.selectContact(scr, selkey, selnick);
}

void
MainWindow::formMsgWinGrid(
  std::vector<std::filesystem::path> &msg,
  size_t begin, size_t end, Gtk::Grid *grid,
  std::string key, std::string nick, int index,
  int varform)
{
  ContactOperations cop(this);
  cop.formMsgWinGrid(msg, begin, end, grid, key, nick, index,
                     varform);
}

void
MainWindow::deleteContact()
{
  ContactOperations cop(this);
  cop.deleteContact();
}

void
MainWindow::networkOp()
{
  if(oper == nullptr)
    {
      std::string homepath;
      AuxFunc af;
      LibCommunist Lc;
      af.homePath(&homepath);
      oper = NetOperationsComm::create_object();
      oper->setHomePath(homepath);
      prefvectmtx.lock();
      oper->setPrefVector(prefvect);
      prefvectmtx.unlock();
      oper->setStunListPath(Sharepath + "/StunList");
      oper->setUsernamePasswd(Username, Password);
      std::mutex *disp1mtx = new std::mutex;
      std::mutex *disp2mtx = new std::mutex;
      std::mutex *disp3mtx = new std::mutex;
      std::mutex *disp4mtx = new std::mutex;
      std::mutex *disp5mtx = new std::mutex;
      std::mutex *disp6mtx = new std::mutex;
      std::mutex *disp7mtx = new std::mutex;
      std::mutex *disp8mtx = new std::mutex;
      std::mutex *disp9mtx = new std::mutex;
      std::mutex *disp12mtx = new std::mutex;
      std::mutex *disp13mtx = new std::mutex;
      std::mutex *disp14mtx = new std::mutex;
      std::mutex *disp15mtx = new std::mutex;

      std::string *key = new std::string;
      int *ind = new int;

      std::string *keysm = new std::string;
      std::filesystem::path *indsm = new std::filesystem::path;

      std::string *keyrm = new std::string;
      std::filesystem::path *rp = new std::filesystem::path;

      std::string *keyfr = new std::string;
      uint64_t *tm = new uint64_t;
      uint64_t *fs = new uint64_t;
      std::string *fn = new std::string;

      std::string *keyfrr = new std::string;
      std::filesystem::path *filefrr = new std::filesystem::path;

      std::string *keyfiler = new std::string;
      std::filesystem::path *filenmr = new std::filesystem::path;

      std::string *keyfilenr = new std::string;
      std::filesystem::path *filenmnr = new std::filesystem::path;

      std::string *keyfiles = new std::string;
      std::filesystem::path *filenms = new std::filesystem::path;

      std::string *keyfileerror = new std::string;
      std::filesystem::path *filenmerror = new
      std::filesystem::path;

      std::string *keyfileprg = new std::string;
      std::filesystem::path *fileprgp = new std::filesystem::path;
      uint64_t *fileprgsz = new uint64_t;

      std::string *keyfileprgs = new std::string;
      std::filesystem::path *fileprgps = new
      std::filesystem::path;
      uint64_t *fileprgszs = new uint64_t;

      std::string *keychcon = new std::string;
      uint64_t *tmchcon = new uint64_t;

      std::string *keyfrrem = new std::string;

      oper->profReceived_signal = [this, key, ind, disp1mtx]
                                  (std::string keyt, int indt)
      {
        disp1mtx->lock();
        *key = keyt;
        *ind = indt;
        this->disp1.emit();
      };

      oper->msgSent_signal = [this, keysm, indsm, disp2mtx]
                             (std::string key, std::filesystem::path ind)
      {
        disp2mtx->lock();
        *keysm = key;
        *indsm = ind;
        this->disp2.emit();
      };

      oper->messageReceived_signal = [this, keyrm, rp, disp3mtx]
                                     (std::string key, std::filesystem::path p)
      {
        disp3mtx->lock();
        *keyrm = key;
        *rp = p;
        this->disp3.emit();
      };

      oper->filerequest_signal = [this, keyfr, tm, fs, fn,
                                        disp4mtx]
            (std::string key, uint64_t time, uint64_t filesize,
             std::string filename)
      {
        disp4mtx->lock();
        *keyfr = key;
        *tm = time;
        *fs = filesize;
        *fn = filename;
        this->disp4.emit();
      };
      oper->fileRejected_signal = [this, keyfrr, filefrr,
                                         disp5mtx]
            (std::string key, std::filesystem::path p)
      {
        disp5mtx->lock();
        *keyfrr = key;
        *filefrr = p;
        this->disp5.emit();
      };

      oper->filercvd_signal = [this, disp6mtx, keyfiler, filenmr]
                              (std::string key, std::filesystem::path filepath)
      {
        disp6mtx->lock();
        *keyfiler = key;
        *filenmr = filepath;
        this->disp6.emit();
      };

      oper->filehasherr_signal = [this, disp7mtx, keyfilenr,
                                        filenmnr]
            (std::string key, std::filesystem::path filepath)
      {
        disp7mtx->lock();
        *keyfilenr = key;
        *filenmnr = filepath;
        this->disp7.emit();
      };

      oper->filesent_signal = [this, disp8mtx, keyfiles, filenms]
                              (std::string key, std::filesystem::path filepath)
      {
        disp8mtx->lock();
        *keyfiles = key;
        *filenms = filepath;
        this->disp8.emit();
      };

      oper->filesenterror_signal = [this, disp9mtx, keyfileerror,
                                          filenmerror]
            (std::string key, std::filesystem::path filepath)
      {
        disp9mtx->lock();
        *keyfileerror = key;
        *filenmerror = filepath;
        this->disp9.emit();
      };

      oper->ipv6_signal = std::bind(&MainWindow::formIPv6vect,
                                    this,
                                    std::placeholders::_1);

      oper->ipv4_signal = std::bind(&MainWindow::formIPv4vect,
                                    this,
                                    std::placeholders::_1);

      oper->ipv6finished_signal = [this]
      {
        this->disp10.emit();
      };

      oper->ipv4finished_signal = [this]
      {
        this->disp11.emit();
      };

      oper->filepartrcvd_signal = [this, disp12mtx, keyfileprg,
                                         fileprgp,
                                         fileprgsz]
            (std::string key, std::filesystem::path p, uint64_t sz)
      {
        disp12mtx->lock();
        *keyfileprg = key;
        *fileprgp = p;
        *fileprgsz = sz;
        this->disp12.emit();
      };

      oper->filepartsend_signal = [this, disp13mtx, keyfileprgs,
                                         fileprgps,
                                         fileprgszs]
            (std::string key, std::filesystem::path p, uint64_t sz)
      {
        disp13mtx->lock();
        *keyfileprgs = key;
        *fileprgps = p;
        *fileprgszs = sz;
        this->disp13.emit();
      };

      oper->smthrcvd_signal = [this, disp14mtx, keychcon, tmchcon]
                              (std::string key, uint64_t tm)
      {
        disp14mtx->lock();
        *keychcon = key;
        *tmchcon = tm;
        this->disp14.emit();
      };

      oper->friendDeleted_signal = [this, disp15mtx, keyfrrem]
                                   (std::string key)
      {
        disp15mtx->lock();
        *keyfrrem = key;
        this->disp15.emit();
      };
      oper->friendDelPulse_signal = [this]
      {
        this->disp16.emit();
      };
      oper->friendBlocked_signal = [this]
      {
        this->disp17.emit();
      };

      std::vector<sigc::connection> dispv;
      sigc::connection con;
      con = disp1.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::addFriendSlot), key,
                         ind, disp1mtx));
      dispv.push_back(con);
      con = disp2.connect([this, keysm, indsm, disp2mtx]
      {
        MWMsgOperations mop(this);
        mop.msgSentSlot(keysm, indsm, disp2mtx);
      });
      dispv.push_back(con);
      con = disp3.connect([this, keyrm, rp, disp3mtx]
      {
        MWMsgOperations mop(this);
        mop.msgRcvdSlot(keyrm, rp, disp3mtx);
      });
      dispv.push_back(con);
      con = disp4.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileRequestSlot),
                         keyfr, tm, fs, fn, disp4mtx));
      dispv.push_back(con);
      con = disp5.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileRejectedSlot),
                         keyfrr, filefrr, disp5mtx));
      dispv.push_back(con);
      con = disp6.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileRcvdStatus),
                         keyfiler, filenmr, disp6mtx, 6));
      dispv.push_back(con);
      con = disp7.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileRcvdStatus),
                         keyfilenr, filenmnr, disp7mtx, 7));
      dispv.push_back(con);
      con = disp8.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileRcvdStatus),
                         keyfiles, filenms, disp8mtx, 8));
      dispv.push_back(con);
      con = disp9.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileRcvdStatus),
                         keyfileerror, filenmerror, disp9mtx, 9));
      dispv.push_back(con);
      con = disp10.connect([this]
      {
        AuxWindows aw(this);
        aw.ipv6Window();
      });
      dispv.push_back(con);
      con = disp11.connect([this]
      {
        AuxWindows aw(this);
        aw.ipv4Window();
      });
      dispv.push_back(con);
      con = disp12.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileDownloadProg),
                         keyfileprg, fileprgp, fileprgsz, disp12mtx));
      dispv.push_back(con);
      con = disp13.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::fileSendProg),
                         keyfileprgs, fileprgps, fileprgszs, disp13mtx));
      dispv.push_back(con);
      con = disp14.connect(
              sigc::bind(sigc::mem_fun(*this,
                                       &MainWindow::checkIfConnected),
                         keychcon, tmchcon, disp14mtx));
      dispv.push_back(con);
      con = disp15.connect([this, keyfrrem, disp15mtx]
      {
        ContactOperations cop(this);
        cop.friendRemoved(keyfrrem, disp15mtx);
      });
      dispv.push_back(con);
      con = disp16.connect([this]
      {
        if(this->friend_block_bar)
          {
            this->friend_block_bar->pulse();
          }
      });
      dispv.push_back(con);
      con = disp17.connect([this]
      {
        if(this->Contdelwin)
          {
            this->Contdelwin->close();
          }
      });
      dispv.push_back(con);

      oper->net_op_canceled_signal = [this, dispv, disp1mtx,
                                            disp2mtx, disp3mtx,
                                            disp4mtx, disp5mtx, disp6mtx, disp7mtx,
                                            disp8mtx, disp9mtx, disp12mtx, disp13mtx,
                                            disp14mtx, disp15mtx, key, ind, keysm,
                                            indsm, keyrm, rp, keyfr, tm, fs, fn,
                                            keyfrr, keyfiler, filenmr, keyfilenr,
                                            filenmnr, keyfiles, filenms, keyfileerror,
                                            filenmerror, keyfileprg, fileprgp,
                                            fileprgsz, keyfileprgs, fileprgps,
                                            fileprgszs, keychcon, tmchcon, keyfrrem]
      {
        for(size_t i = 0; i < dispv.size(); i++)
          {
            sigc::connection con = dispv[i];
            con.disconnect();
          }
        delete disp1mtx;
        delete disp2mtx;
        delete disp3mtx;
        delete disp4mtx;
        delete disp5mtx;
        delete disp6mtx;
        delete disp7mtx;
        delete disp8mtx;
        delete disp9mtx;
        delete disp12mtx;
        delete disp13mtx;
        delete disp14mtx;
        delete disp15mtx;
        delete key;
        delete ind;
        delete keysm;
        delete indsm;
        delete keyrm;
        delete rp;
        delete keyfr;
        delete tm;
        delete fs;
        delete fn;
        delete keyfrr;
        delete keyfiler;
        delete filenmr;
        delete keyfilenr;
        delete filenmnr;
        delete keyfiles;
        delete filenms;
        delete keyfileerror;
        delete filenmerror;
        delete keyfileprg;
        delete fileprgp;
        delete fileprgsz;
        delete keyfileprgs;
        delete fileprgps;
        delete fileprgszs;
        delete keychcon;
        delete tmchcon;
        delete keyfrrem;
        NetOperationsComm::cleanMemory(this->oper);
      };
      oper->startNetOperations();
    }
}

void
MainWindow::addFriends()
{
  AuxWindows aw(this);
  aw.addFriends();
}

void
MainWindow::editAddFriends()
{
  AuxWindows aw(this);
  aw.editAddFriends();
}

void
MainWindow::addFriendSlot(std::string *keyt, int *conind,
                          std::mutex *disp1mtx)
{
  std::string key = *keyt;
  while(key.size() > 64)
    {
      key.pop_back();
    }
  int conindloc = *conind;
  disp1mtx->unlock();
  addfrmtx.lock();
  Addfriends.erase(
    std::remove_if(Addfriends.begin(),
                   Addfriends.end(), [&key]
                   (auto & el)
  {
    return el == key;
  }),
  Addfriends.end());
  addfrmtx.unlock();

  contmtx.lock();
  auto conit = std::find_if(contacts.begin(),
                            contacts.end(), [&key]
                            (auto & el)
  {
    return std::get<1>(el) == key;
  });
  if(conit == contacts.end())
    {
      contacts.push_back(std::make_tuple(conindloc, key));
    }
  else
    {
      std::tuple<int, std::string> ttup;
      std::get<0> (ttup) = conindloc;
      std::get<1> (ttup) = key;
      *conit = ttup;
    }
  contmtx.unlock();

  conavmtx.lock();
  conavatars.erase(
    std::remove_if(conavatars.begin(),
                   conavatars.end(), [&key]
                   (auto & el)
  {
    return std::string(el.first) == key;
  }),
  conavatars.end());
  conavmtx.unlock();

  chifcmtx.lock();
  frvectmtx.lock();
  int chbutdel = 0;
  auto it = std::find_if(friendvect.begin(),
                         friendvect.end(), [&key]
                         (auto & el)
  {
    return std::get<2>(el)->get_text() == Glib::ustring(key);
  });
  if(it != friendvect.end())
    {
      Gtk::Widget *widg = std::get<0> (*it);
      friendvect.erase(it);
      if(selectedc == widg)
        {
          selectedc = nullptr;
          chbutdel = 1;
        }
      fordelgrid->remove(*widg);
    }
  chifc.erase(std::remove_if(chifc.begin(),
                             chifc.end(), [&key]
                             (auto & el)
  {
    return std::get<0>(el) == key;
  }),
  chifc.end());
  frvectmtx.unlock();
  chifcmtx.unlock();
  contmtx.lock();
  auto itcont = std::find_if(contacts.begin(),
                             contacts.end(), [&key]
                             (auto & el)
  {
    return std::get<1>(el) == key;
  });
  if(itcont != contacts.end())
    {
      AuxFunc af;
      std::string filename;
      std::string homepath;
      std::filesystem::path source;
      std::filesystem::path outpath;
      af.homePath(&homepath);
      LibCommunist Lc;
      std::vector<std::tuple<std::string, std::vector<char>>>
      frprofvect;
      frprofvect = Lc.readFriendProfile(homepath, key, Username,
                                        Password);
      if(frprofvect.size() > 0)
        {
          Gtk::Button *buttontr = Gtk::make_managed<Gtk::Button> ();
          buttontr->get_style_context()->add_provider(
            css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
          buttontr->set_name("inactiveButton");
          Gtk::Grid *gridtr = Gtk::make_managed<Gtk::Grid> ();
          buttontr->set_child(*gridtr);
          Glib::RefPtr<Gtk::GestureClick> clck =
            Gtk::GestureClick::create();
          clck->set_button(3);
          clck->signal_pressed().connect(
            sigc::bind(sigc::mem_fun(*this, &MainWindow::contMenu),
                       clck,
                       buttontr));
          Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
            Gio::SimpleActionGroup::create();
          acgroup->add_action(
            "info",
            sigc::bind(sigc::mem_fun(*this,
                                     &MainWindow::friendDetails),
                       buttontr));
          acgroup->add_action(
            "delete", sigc::mem_fun(*this,
                                    &MainWindow::deleteContact));
          acgroup->add_action(
            "block", sigc::mem_fun(*this, &MainWindow::tempBlockCont));
          buttontr->insert_action_group("popupcont", acgroup);
          buttontr->add_controller(clck);
          std::vector<char> picture;
          auto itfrpr = std::find_if(frprofvect.begin(),
                                     frprofvect.end(), []
                                     (auto & el)
          {
            return std::get<0>(el) == "Avatar";
          });
          if(itfrpr != frprofvect.end())
            {
              picture = std::get<1> (*itfrpr);
            }
          if(picture.size() > 0)
            {
              std::cout << "Avatar found" << std::endl;
            }
          else
            {
              std::cout << "Avatar not found" << std::endl;
            }

          Gtk::DrawingArea *drar =
            Gtk::make_managed<Gtk::DrawingArea> ();
          drar->set_margin(2);
          drar->set_halign(Gtk::Align::CENTER);
          drar->set_size_request(50, 50);
          gridtr->attach(*drar, 0, 0, 1, 3);

          Gtk::Label *keylab = Gtk::make_managed<Gtk::Label> ();
          keylab->set_max_width_chars(20);
          keylab->set_ellipsize(Pango::EllipsizeMode::END);
          keylab->set_text(Glib::ustring(key));
          gridtr->attach(*keylab, 1, 0, 1, 1);

          if(picture.size() > 0)
            {
              std::string avatarpath;
#ifdef __linux
              avatarpath =
                std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
              avatarpath =
                std::filesystem::temp_directory_path().parent_path().u8string();
#endif
              avatarpath = avatarpath + "/" + Lc.randomFileName() +
                           ".jpeg";
              std::filesystem::path filepath = std::filesystem::u8path(
                                                 avatarpath);
              std::fstream f;
              f.open(filepath, std::ios_base::out |
                     std::ios_base::binary);
              f.write(&picture[0], picture.size());
              f.close();
              Glib::RefPtr<Gdk::Pixbuf> imagec =
                Gdk::Pixbuf::create_from_file(
                  Glib::ustring(avatarpath));
              int drarw, drarh;
              drar->get_size_request(drarw, drarh);
              imagec = imagec->scale_simple(drarw, drarh,
                                            Gdk::InterpType::BILINEAR);
              std::pair<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> pair;
              pair.first = keylab->get_text();
              pair.second = imagec;
              conavmtx.lock();
              conavatars.push_back(pair);
              conavmtx.unlock();
              drar->set_draw_func(
                sigc::bind(
                  sigc::mem_fun(*this, &MainWindow::on_draw_contactlist),
                  keylab));
              std::filesystem::remove_all(filepath);
            }
          Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label> ();
          itfrpr = std::find_if(frprofvect.begin(),
                                frprofvect.end(), []
                                (auto & el)
          {
            return std::get<0>(el) == "Nick";
          });
          if(itfrpr != frprofvect.end())
            {
              std::vector<char> val = std::get<1> (*itfrpr);
              std::string line;
              std::copy(val.begin(), val.end(),
                        std::back_inserter(line));
              nicklab->set_text(Glib::ustring(line));
            }
          nicklab->set_halign(Gtk::Align::START);
          gridtr->attach(*nicklab, 1, 1, 1, 1);

          Gtk::Label *namelab = Gtk::make_managed<Gtk::Label> ();
          itfrpr = std::find_if(frprofvect.begin(),
                                frprofvect.end(), []
                                (auto & el)
          {
            return std::get<0>(el) == "Surname";
          });
          if(itfrpr != frprofvect.end())
            {
              std::vector<char> val = std::get<1> (*itfrpr);
              std::string line;
              std::copy(val.begin(), val.end(),
                        std::back_inserter(line));
              Glib::ustring surnm(line);
              itfrpr = std::find_if(frprofvect.begin(),
                                    frprofvect.end(), []
                                    (auto & el)
              {
                return std::get<0>(el) == "Name";
              });
              if(itfrpr != frprofvect.end())
                {
                  val = std::get<1> (*itfrpr);
                  line.clear();
                  std::copy(val.begin(), val.end(),
                            std::back_inserter(line));
                  Glib::ustring nm(line);
                  namelab->set_text(Glib::ustring(nm + " " + surnm));
                }
              else
                {
                  namelab->set_text(surnm);
                }
            }
          else
            {
              itfrpr = std::find_if(frprofvect.begin(),
                                    frprofvect.end(), []
                                    (auto & el)
              {
                return std::get<0>(el) == "Name";
              });
              if(itfrpr != frprofvect.end())
                {
                  std::vector<char> val = std::get<1> (*itfrpr);
                  std::string line;
                  std::copy(val.begin(), val.end(),
                            std::back_inserter(line));
                  Glib::ustring nm(line);
                  namelab->set_text(nm);
                }
              else
                {
                  namelab->set_text("");
                }
            }
          namelab->set_halign(Gtk::Align::START);
          gridtr->attach(*namelab, 1, 2, 1, 1);

          buttontr->signal_clicked().connect(
            sigc::bind(sigc::mem_fun(*this,
                                     &MainWindow::selectContact),
                       Winright, std::string(keylab->get_text()),
                       std::string(nicklab->get_text())));

          buttontr->set_size_request(contact_button_width, -1);

          frvectmtx.lock();
          if(friendvect.size() == 0)
            {
              fordelgrid->attach(*buttontr, 0, 0, 1, 1);
            }
          else
            {
              Gtk::Widget *widg = std::get<0> (friendvect[0]);
              fordelgrid->attach_next_to(*buttontr, *widg,
                                         Gtk::PositionType::TOP, 1, 1);
            }

          friendvect.insert(
            friendvect.begin(),
            std::make_tuple(buttontr, gridtr, keylab, nullptr, nicklab,
                            namelab, nullptr, nullptr));
          frvectmtx.unlock();

          if(chbutdel > 0)
            {
              selectedc = buttontr;
              buttontr->set_name("activeButton");
            }
        }
      else
        {
          std::cerr <<
                    "Error on openning friend's profile (on rcving)"
                    << std::endl;
        }
      proffopmtx.unlock();
    }
  contmtx.unlock();
}

void
MainWindow::sendMsg(Gtk::TextView *tv)
{
  MWMsgOperations mop(this);
  Glib::RefPtr<Gtk::TextBuffer> buf = tv->get_buffer();
  Glib::ustring txt = buf->get_text(true);
  buf->set_text("");
  mop.sendMsg(txt);
}

void
MainWindow::sendMsgFile(std::string *txt)
{
  Glib::ustring lt(*txt);
  delete txt;
  MWMsgOperations mop(this);
  mop.sendMsg(lt);
}

void
MainWindow::removeMsg(std::string othkey,
                      std::filesystem::path msgpath,
                      Gtk::Widget *widg)
{
  MWMsgOperations mop(this);
  mop.removeMsg(othkey, msgpath, widg);
}

void
MainWindow::editProfile()
{
  profilevector.clear();
  AuxFunc af;
  std::string homepath;
  af.homePath(&homepath);
  LibCommunist Lc;
  std::vector<std::tuple<std::string, std::vector<char>>>
  profvect;
  proffopmtx.lock();
  profvect = Lc.readProfile(homepath, Username, Password);
  proffopmtx.unlock();
  if(profvect.size() > 0)
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(this->get_application());
      window->set_name("settingsWindow");
      window->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign(Gtk::Align::CENTER);
      window->set_title(gettext("Edit profile"));
      window->set_child(*grid);

      Gtk::Grid *leftgrid = Gtk::make_managed<Gtk::Grid> ();
      Gtk::Grid *rightgrid = Gtk::make_managed<Gtk::Grid> ();

      grid->attach(*leftgrid, 0, 0, 1, 1);
      grid->attach(*rightgrid, 1, 0, 1, 1);

      Gtk::Entry *nickname = Gtk::make_managed<Gtk::Entry> ();
      auto itprv = std::find_if(profvect.begin(),
                                profvect.end(), []
                                (auto & el)
      {
        return std::get<0>(el) == "Nick";
      });
      if(itprv != profvect.end())
        {
          std::vector<char> val = std::get<1> (*itprv);
          std::string valstr;
          std::copy(val.begin(), val.end(),
                    std::back_inserter(valstr));
          nickname->set_text(Glib::ustring(valstr));
        }
      nickname->set_margin(5);
      rightgrid->attach(*nickname, 0, 0, 3, 1);
      profilevector.push_back(nickname);

      Gtk::Entry *name = Gtk::make_managed<Gtk::Entry> ();
      itprv = std::find_if(profvect.begin(), profvect.end(), []
                           (auto & el)
      {
        return std::get<0>(el) == "Name";
      });
      if(itprv != profvect.end())
        {
          std::vector<char> val = std::get<1> (*itprv);
          std::string valstr;
          std::copy(val.begin(), val.end(),
                    std::back_inserter(valstr));
          name->set_text(Glib::ustring(valstr));
        }
      name->set_placeholder_text(
        gettext("Name (not compalsory)"));
      name->set_margin(5);
      rightgrid->attach(*name, 0, 1, 3, 1);
      profilevector.push_back(name);

      Gtk::Entry *surname = Gtk::make_managed<Gtk::Entry> ();
      itprv = std::find_if(profvect.begin(), profvect.end(), []
                           (auto & el)
      {
        return std::get<0>(el) == "Surname";
      });
      if(itprv != profvect.end())
        {
          std::vector<char> val = std::get<1> (*itprv);
          std::string valstr;
          std::copy(val.begin(), val.end(),
                    std::back_inserter(valstr));
          surname->set_text(Glib::ustring(valstr));
        }
      surname->set_placeholder_text(
        gettext("Surname (not compalsory)"));
      surname->set_margin(5);
      rightgrid->attach(*surname, 0, 2, 3, 1);
      profilevector.push_back(surname);

      Gtk::Button *saveprof = Gtk::make_managed<Gtk::Button> ();
      Gtk::Label *spl = Gtk::make_managed<Gtk::Label> ();
      spl->set_text(gettext("Save profile"));
      spl->set_justify(Gtk::Justification::CENTER);
      saveprof->set_child(*spl);
      saveprof->set_margin(5);
      saveprof->set_halign(Gtk::Align::CENTER);
      saveprof->set_name("applyButton");
      saveprof->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      saveprof->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this,
                                 &MainWindow::saveProfileEP),
                   window));
      rightgrid->attach(*saveprof, 0, 3, 1, 1);

      Gtk::Button *cleardata = Gtk::make_managed<Gtk::Button> ();
      Gtk::Label *cdl = Gtk::make_managed<Gtk::Label> ();
      cdl->set_text(gettext("Clear"));
      cdl->set_justify(Gtk::Justification::CENTER);
      cleardata->set_child(*cdl);
      cleardata->set_margin(5);
      cleardata->set_halign(Gtk::Align::CENTER);
      cleardata->set_name("clearButton");
      cleardata->get_style_context()->add_provider(
        css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      cleardata->signal_clicked().connect([nickname, name,
                                                     surname]
      {
        nickname->set_text("");
        name->set_text("");
        surname->set_text("");
      });
      rightgrid->attach(*cleardata, 1, 3, 1, 1);

      Gtk::Button *cancela = Gtk::make_managed<Gtk::Button> ();
      Gtk::Label *cal = Gtk::make_managed<Gtk::Label> ();
      cal->set_text(gettext("Cancel"));
      cal->set_justify(Gtk::Justification::CENTER);
      cancela->set_child(*cal);
      cancela->set_margin(5);
      cancela->set_halign(Gtk::Align::CENTER);
      cancela->set_name("rejectButton");
      cancela->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      cancela->signal_clicked().connect(
        sigc::mem_fun(*window, &Gtk::Window::close));
      rightgrid->attach(*cancela, 2, 3, 1, 1);

      avatar = Gtk::make_managed<Gtk::DrawingArea> ();
      avatar->set_margin(5);
      avatar->set_halign(Gtk::Align::CENTER);
      avatar->set_size_request(200, 200);

      std::vector<char> *picture = new std::vector<char>;
      itprv = std::find_if(profvect.begin(), profvect.end(), []
                           (auto & el)
      {
        return std::get<0>(el) == "Avatar";
      });
      if(itprv != profvect.end())
        {
          *picture = std::get<1> (*itprv);
        }

      if(picture->size() > 0)
        {
          avatar->set_draw_func(
            sigc::bind(sigc::mem_fun(*this, &MainWindow::on_draw_ep),
                       picture));
        }
      else
        {
          delete picture;
        }
      leftgrid->attach(*avatar, 0, 0, 2, 1);

      Gtk::Button *addavatar = Gtk::make_managed<Gtk::Button> ();
      Gtk::Label *oblab = Gtk::make_managed<Gtk::Label> ();
      oblab->set_text(gettext("Open avatar"));
      oblab->set_justify(Gtk::Justification::CENTER);
      addavatar->set_child(*oblab);
      addavatar->set_margin(5);
      addavatar->set_halign(Gtk::Align::CENTER);
      addavatar->set_name("applyButton");
      addavatar->get_style_context()->add_provider(
        css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      addavatar->signal_clicked().connect(
        [this, window]
      {
        Glib::RefPtr<Gtk::FileChooserNative> dialog =
        Gtk::FileChooserNative::create(gettext("Open image"), *this,
                                       Gtk::FileChooser::Action::OPEN,
                                       gettext("Open"),
                                       gettext("Cancel"));
        dialog->set_transient_for(*window);
        dialog->signal_response().connect([dialog, this]
                                          (int respons_id)
        {

          Glib::ustring file;
          if(respons_id == Gtk::ResponseType::ACCEPT)
            {
              file = dialog->get_file()->get_path();
              this->avatar->set_draw_func(
                sigc::bind(sigc::mem_fun(*this,
                                         &MainWindow::on_draw),
                           file));
            }
        });
        dialog->show();
      });
      leftgrid->attach(*addavatar, 0, 1, 1, 1);

      Gtk::Button *cleardraw = Gtk::make_managed<Gtk::Button> ();
      Gtk::Label *oblab2 = Gtk::make_managed<Gtk::Label> ();
      oblab2->set_text(gettext("Remove avatar"));
      oblab2->set_justify(Gtk::Justification::CENTER);
      cleardraw->set_child(*oblab2);
      cleardraw->set_margin(5);
      cleardraw->set_halign(Gtk::Align::CENTER);
      cleardraw->set_name("clearButton");
      cleardraw->get_style_context()->add_provider(
        css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      cleardraw->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &MainWindow::clearAvatar),
                   leftgrid));
      leftgrid->attach(*cleardraw, 1, 1, 1, 1);

      window->signal_close_request().connect(
        sigc::bind(sigc::mem_fun(*this,
                                 &MainWindow::editProfileClose),
                   window),
        false);
      window->show();
    }
  else
    {
      std::cerr << "Profile not opened" << std::endl;
    }
}

bool
MainWindow::editProfileClose(Gtk::Window *window)
{
  profilevector.clear();
  window->hide();
  delete window;
  this->avatar = nullptr;
  return true;
}

void
MainWindow::saveProfileEP(Gtk::Window *windowb)
{
  Gtk::Entry *ent = profilevector[0];
  if(ent->get_text() == "")
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(this->get_application());
      window->set_name("settingsWindow");
      window->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      window->set_title(gettext("Error"));
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign(Gtk::Align::CENTER);
      window->set_child(*grid);

      Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
      label->set_text(gettext("Input nickname!"));
      label->set_halign(Gtk::Align::CENTER);
      label->set_margin(5);
      grid->attach(*label, 0, 0, 1, 1);

      Gtk::Button *button = Gtk::make_managed<Gtk::Button> ();
      button->set_label(gettext("Close"));
      button->set_halign(Gtk::Align::CENTER);
      button->set_margin(5);
      button->set_name("applyButton");
      button->get_style_context()->add_provider(css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      button->signal_clicked().connect(
        sigc::mem_fun(*window, &Gtk::Window::close));
      grid->attach(*button, 0, 1, 1, 1);
      window->signal_close_request().connect([window]
      {
        window->hide();
        delete window;
        return true;
      },
      false);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size(rq1, rq2);
      window->set_default_size(rq2.get_width() + 10,
                               rq2.get_height() + 10);
      window->show();
    }
  else
    {
      AuxFunc af;
      std::vector<std::tuple<std::string, std::vector<char>>>
      profvect;
      for(size_t i = 0; i < profilevector.size(); i++)
        {
          Gtk::Entry *ent = profilevector[i];
          std::tuple<std::string, std::vector<char>> ttup;
          if(i == 0)
            {
              std::get<0> (ttup) = "Nick";
              std::vector<char> val;
              std::string line = std::string(ent->get_text());
              std::copy(line.begin(), line.end(),
                        std::back_inserter(val));
              std::get<1> (ttup) = val;
            }
          if(i == 1)
            {
              std::get<0> (ttup) = "Name";
              std::vector<char> val;
              std::string line = std::string(ent->get_text());
              std::copy(line.begin(), line.end(),
                        std::back_inserter(val));
              std::get<1> (ttup) = val;
            }
          if(i == 2)
            {
              std::get<0> (ttup) = "Surname";
              std::vector<char> val;
              std::string line = std::string(ent->get_text());
              std::copy(line.begin(), line.end(),
                        std::back_inserter(val));
              std::get<1> (ttup) = val;
            }
          profvect.push_back(ttup);
        }
      std::string filename;
#ifdef __linux
      filename =
        std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
      filename =
        std::filesystem::temp_directory_path().parent_path().u8string();
#endif

      std::filesystem::path source;
      if(Image > 0)
        {
          LibCommunist Lc;
          filename = filename + "/" + Lc.randomFileName() + ".jpeg";
          source = std::filesystem::u8path(filename);
          if(std::filesystem::exists(source))
            {
              std::filesystem::remove_all(source);
            }
          image->save(source.string(), "jpeg");
          Image = 0;
          if(std::filesystem::exists(source))
            {
              std::vector<char> val;
              int filesize = std::filesystem::file_size(source);
              if(filesize > 0)
                {
                  val.resize(filesize);
                  std::fstream f;
                  f.open(source, std::ios_base::in | std::ios_base::binary);
                  if(f.is_open())
                    {
                      f.read(&val[0], val.size());
                      profvect.push_back(std::make_tuple("Avatar", val));
                      f.close();
                    }
                }
            }
          std::filesystem::remove_all(source);
        }
      std::string homepath;
      af.homePath(&homepath);
      LibCommunist Lc;
      proffopmtx.lock();
      int result = Lc.editProfile(Username, Password, homepath,
                                  profvect);
      proffopmtx.unlock();
      if(result >= 0)
        {
          contmtx.lock();
          for(size_t i = 0; i < contacts.size(); i++)
            {
              std::string key = std::get<1> (contacts[i]);
              oper->renewProfile(key);
            }
          contmtx.unlock();
        }
      else
        {
          std::cerr << "Profile renewing error!" << std::endl;
        }
      windowb->close();
    }
}

void
MainWindow::attachFileDialog()
{
  AuxWindows aw(this);
  aw.attachFileDialog();
}

void
MainWindow::fileRequestSlot(std::string *key, uint64_t *tm,
                            uint64_t *filesize,
                            std::string *filename, std::mutex *disp4mtx)
{
  MWFileOperations fop(this);
  fop.fileRequestSlot(key, tm, filesize, filename, disp4mtx);
}

void
MainWindow::fileRejectedSlot(std::string *keyr,
                             std::filesystem::path *filer,
                             std::mutex *disp5mtx)
{
  MWFileOperations fop(this);
  fop.fileRejectedSlot(keyr, filer, disp5mtx);
}

void
MainWindow::fileRcvdStatus(std::string *key,
                           std::filesystem::path *filepath,
                           std::mutex *dispNmtx, int variant)
{
  MWFileOperations fop(this);
  fop.fileRcvdStatus(key, filepath, dispNmtx, variant);
}

void
MainWindow::contMenu(int numcl, double x, double y,
                     Glib::RefPtr<Gtk::GestureClick> clck,
                     Gtk::Button *contbutton)
{
  Gtk::Label *keylab = nullptr;
  Gtk::Label *nicklab = nullptr;
  Gtk::PopoverMenu *Menu = nullptr;
  frvectmtx.lock();
  auto itfrv = std::find_if(friendvect.begin(),
                            friendvect.end(),
                            [contbutton]
                            (auto & el)
  {
    return std::get<0>(el) == contbutton;
  });
  if(itfrv != friendvect.end())
    {
      keylab = std::get<2> (*itfrv);
      nicklab = std::get<4> (*itfrv);
      Menu = std::get<7> (*itfrv);
      if(Menu == nullptr)
        {
          Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
          menu->append(gettext("Contact info"), "popupcont.info");
          menu->append(gettext("Remove contact"),
                       "popupcont.delete");
          menu->append(gettext("Block (until restart)/Unblock"),
                       "popupcont.block");
          Menu = Gtk::make_managed<Gtk::PopoverMenu> ();
          Menu->set_parent(*contbutton);
          Menu->set_menu_model(menu);
          Menu->set_has_arrow(false);
          Gdk::Rectangle rect(x, y, 1, 1);
          Menu->set_pointing_to(rect);
          std::get<7> (*itfrv) = Menu;
        }
      else
        {
          Gdk::Rectangle rect(x, y, 1, 1);
          Menu->set_pointing_to(rect);
        }
    }
  frvectmtx.unlock();
  if(keylab != nullptr && nicklab != nullptr)
    {
      selectContact(Winright, std::string(keylab->get_text()),
                    std::string(nicklab->get_text()));
    }
  Menu->popup();
}

void
MainWindow::friendDetails(Gtk::Button *button)
{
  AuxWindows aw(this);
  aw.friendDetails(button);
}

void
MainWindow::on_draw_frpd(const
                         Cairo::RefPtr<Cairo::Context> &cr, int width,
                         int height, std::vector<char> *picture)
{
  Glib::ustring file;
  LibCommunist Lc;
  std::string filename;
#ifdef __linux
  filename =
    std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
  filename =
    std::filesystem::temp_directory_path().parent_path().u8string();
#endif
  filename = filename + "/" + Lc.randomFileName();
  std::filesystem::path filepath = std::filesystem::u8path(
                                     filename);
  std::fstream f;
  if(picture->size() > 0)
    {
      std::vector<char> loc = *picture;
      f.open(filepath, std::ios_base::out |
             std::ios_base::binary);
      f.write(&loc[0], loc.size());
      f.close();
    }
  file = Glib::ustring(filename);

  Glib::RefPtr<Gdk::Pixbuf> pic =
    Gdk::Pixbuf::create_from_file(file);
  pic = pic->scale_simple(width, height,
                          Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf(cr, pic, 0, 0);
  cr->rectangle(0, 0, width, height);
  cr->fill();
  delete picture;
  std::filesystem::remove_all(filepath);
}

void
MainWindow::tempBlockCont()
{
  Gtk::Button *but = selectedc;
  frvectmtx.lock();
  auto itfrv = std::find_if(friendvect.begin(),
                            friendvect.end(), [but]
                            (auto & el)
  {
    return std::get<0> (el) == but;
  });
  if(itfrv != friendvect.end())
    {
      Gtk::Grid *grid = std::get<1> (*itfrv);
      Gtk::Label *lab = std::get<6> (*itfrv);
      if(lab == nullptr)
        {
          lab = Gtk::make_managed<Gtk::Label> ();
          lab->set_name("blockLab");
          lab->get_style_context()->add_provider(
            css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
          lab->set_text(gettext("Blocked"));
          Gtk::Label *sibl = std::get<4> (*itfrv);
          grid->attach_next_to(*lab, *sibl, Gtk::PositionType::RIGHT,
                               1, 1);
          std::get<6> (*itfrv) = lab;
          if(oper != nullptr)
            {
              Contdelwin = new Gtk::Window;
              Contdelwin->set_application(this->get_application());
              Contdelwin->set_title(gettext("Blocking contact"));
              Contdelwin->set_modal(true);
              Contdelwin->set_transient_for(*this);
              Contdelwin->set_name("settingsWindow");
              Contdelwin->get_style_context()->add_provider(
                this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
              Gtk::Grid *rmwimgr = Gtk::make_managed<Gtk::Grid> ();
              Contdelwin->set_child(*rmwimgr);
              rmwimgr->set_halign(Gtk::Align::CENTER);
              friend_block_bar = Gtk::make_managed<Gtk::ProgressBar> ();
              friend_block_bar->set_name("clBar");
              friend_block_bar->get_style_context()->add_provider(
                this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
              friend_block_bar->set_halign(Gtk::Align::CENTER);
              friend_block_bar->set_valign(Gtk::Align::CENTER);
              friend_block_bar->set_text(gettext("Blocking..."));
              friend_block_bar->set_show_text(true);
              friend_block_bar->set_margin(5);
              rmwimgr->attach(*friend_block_bar, 0, 0, 1, 1);

              Contdelwin->signal_close_request().connect([this]
              {
                this->Contdelwin->hide();
                delete this->Contdelwin;
                this->Contdelwin = nullptr;
                this->friend_block_bar = nullptr;
                return true;
              },
              false);
              Contdelwin->show();
              std::string key(std::get<2> (*itfrv)->get_text());
              oper->blockFriend(key);
            }
        }
      else
        {
          std::string key(std::get<2> (*itfrv)->get_text());
          contmtx.lock();
          auto itcont = std::find_if(contacts.begin(),
                                     contacts.end(), [&key]
                                     (auto & el)
          {
            return std::get<1>(el) == key;
          });
          if(itcont != contacts.end())
            {
              int ind = std::get<0> (*itcont);
              if(oper != nullptr)
                {
                  oper->startFriend(key, ind);
                }
            }
          contmtx.unlock();
          grid->remove(*lab);
          std::get<6> (*itfrv) = nullptr;
        }
    }
  frvectmtx.unlock();
}

void
MainWindow::settingsWindow()
{
  SettingsWindow sw(this);
  sw.settingsWindow();
}

Gdk::Rectangle
MainWindow::screenRes()
{
  Glib::RefPtr<Gdk::Surface> surf = this->get_surface();
  Glib::RefPtr<Gdk::Display> disp = this->get_display();
  Glib::RefPtr<Gdk::Monitor> mon =
    disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);
  return req;
}

void
MainWindow::formIPv6vect(std::string ip)
{
  ipv6vectmtx.lock();
  ipv6vect.push_back(ip);
  ipv6vectmtx.unlock();
}

void
MainWindow::formIPv4vect(std::string ip)
{
  ipv4vectmtx.lock();
  ipv4vect.push_back(ip);
  ipv4vectmtx.unlock();
}

void
MainWindow::fileDownloadProg(std::string *keyg,
                             std::filesystem::path *filepathg,
                             uint64_t *cursizeg, std::mutex *mtx)
{
  MWFileOperations fop(this);
  fop.fileDownloadProg(keyg, filepathg, cursizeg, mtx);
}

void
MainWindow::fileSendProg(std::string *keyg,
                         std::filesystem::path *filepathg,
                         uint64_t *cursizeg, std::mutex *mtx)
{
  MWFileOperations fop(this);
  fop.fileSendProg(keyg, filepathg, cursizeg, mtx);
}

void
MainWindow::autoRemoveMsg()
{
  std::string mode;
  prefvectmtx.lock();
  auto itprv = std::find_if(prefvect.begin(),
                            prefvect.end(), []
                            (auto & el)
  {
    return std::get<0>(el) == "Autoremove";
  });
  if(itprv != prefvect.end())
    {
      mode = std::get<1> (*itprv);
    }
  else
    {
      mode = "0";
    }
  prefvectmtx.unlock();
  if(mode != "0")
    {
      std::string filename;
      AuxFunc af;
      af.homePath(&filename);
      LibCommunist Lc;
      Lc.msgAutoRemove(filename, Username, Password, mode);
    }
}

void
MainWindow::aboutProg()
{
  Gtk::AboutDialog *aboutd = new Gtk::AboutDialog;
  aboutd->set_transient_for(*this);
  aboutd->set_application(this->get_application());

  aboutd->set_program_name(gettext("Communist"));
  aboutd->set_version("2.0.3");
  aboutd->set_copyright("Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>");
  AuxFunc af;
  std::filesystem::path p = std::filesystem::u8path(
                              af.get_selfpath());
  std::string filename = Sharepath + "/COPYING";
  std::filesystem::path filepath = std::filesystem::u8path(
                                     filename);
  std::fstream f;
  Glib::ustring abbuf;
  f.open(filepath, std::ios_base::in |
         std::ios_base::binary);
  if(f.is_open())
    {
      size_t sz = std::filesystem::file_size(filepath);
      std::vector<char> ab;
      ab.resize(sz);
      f.read(&ab[0], ab.size());
      f.close();
      abbuf = Glib::ustring(ab.begin(), ab.end());
    }
  else
    {
      std::cerr << "Licence file not found" << std::endl;
    }

  if(abbuf.size() == 0)
    {
      aboutd->set_license_type(Gtk::License::GPL_3_0_ONLY);
    }
  else
    {
      aboutd->set_license(abbuf);
    }

  filename = Sharepath + "/themes/default/ico.png";
  Glib::RefPtr<Gio::File> logofile =
    Gio::File::create_for_path(filename);
  aboutd->set_logo(Gdk::Texture::create_from_file(
                     logofile));
  abbuf = gettext(
            "Communist is simple \"peer to peer\" messenger.\n"
            "Author Yury Bobylev.\n\n"
            "Program uses next libraries:\n"
            "GTK https://www.gtk.org\n"
            "ICU https://icu.unicode.org\n"
            "Hunspell http://hunspell.github.io/\n"
            "libcommunist https://github.com/ProfessorNavigator/libcommunist\n"
            "https://gitflic.ru/project/professornavigator/libcommunist");
  aboutd->set_comments(abbuf);

  aboutd->signal_close_request().connect([aboutd]
  {
    aboutd->hide();
    delete aboutd;
    return true;
  },
  false);
  aboutd->show();
}

void
MainWindow::checkIfConnected(std::string *key,
                             uint64_t *tm, std::mutex *mtx)
{
  std::string keyloc = *key;
  uint64_t tmloc = *tm;
  mtx->unlock();
  if(tmloc == 0)
    {
      chifcmtx.lock();
      auto itchfc = std::find_if(chifc.begin(),
                                 chifc.end(), [&keyloc]
                                 (auto & el)
      {
        return std::get<0>(el) == keyloc;
      });
      if(itchfc != chifc.end())
        {
          time_t curtime = time(NULL);
          time_t tmttear = 20;
          prefvectmtx.lock();
          auto itprv = std::find_if(prefvect.begin(),
                                    prefvect.end(), []
                                    (auto & el)
          {
            return std::get<0>(el) == "TmtTear";
          });
          if(itprv != prefvect.end())
            {
              std::stringstream strm;
              std::locale loc("C");
              strm.imbue(loc);
              strm << std::get<1> (*itprv);
              strm >> tmttear;
            }
          prefvectmtx.unlock();
          if(curtime - std::get<1> (*itchfc) > uint64_t (tmttear))
            {
              frvectmtx.lock();
              auto itfrv =
                std::find_if(
                  friendvect.begin(),
                  friendvect.end(),
                  [&keyloc]
                  (auto & el)
              {
                return std::get<2>(el)->get_text() == Glib::ustring(keyloc);
              });
              if(itfrv != friendvect.end())
                {
                  Gtk::Grid *bg = std::get<1> (*itfrv);
                  bg->remove(*(std::get<2> (*itchfc)));
                }
              frvectmtx.unlock();
              chifc.erase(itchfc);
            }
        }
      chifcmtx.unlock();
    }
  else
    {
      chifcmtx.lock();
      auto itchfc = std::find_if(chifc.begin(),
                                 chifc.end(), [&keyloc]
                                 (auto & el)
      {
        return std::get<0>(el) == keyloc;
      });
      if(itchfc != chifc.end())
        {
          std::get<1> (*itchfc) = tmloc;
        }
      else
        {
          frvectmtx.lock();
          auto itfrv = std::find_if(
                         friendvect.begin(), friendvect.end(), [&keyloc]
                         (auto & el)
          {
            return std::get<2>(el)->get_text() == Glib::ustring(keyloc);
          });
          if(itfrv != friendvect.end())
            {
              Gtk::Label *onl = Gtk::make_managed<Gtk::Label> ();
              onl->set_text(gettext("online"));
              onl->set_margin(5);
              onl->set_halign(Gtk::Align::END);
              onl->set_name("connectFr");
              onl->get_style_context()->add_provider(
                this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
              Gtk::Grid *bg = std::get<1> (*itfrv);
              Gtk::Label *nml = std::get<5> (*itfrv);
              bg->attach_next_to(*onl, *nml, Gtk::PositionType::RIGHT, 1,
                                 1);

              chifc.push_back(std::make_tuple(keyloc, tmloc, onl));
            }
          frvectmtx.unlock();

        }
      chifcmtx.unlock();
    }
}

void
MainWindow::editRelayList()
{
  AuxWindows aw(this);
  aw.editRelayList();
}
