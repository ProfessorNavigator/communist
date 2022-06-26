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

MainWindow::MainWindow ()
{
  dispclose.connect ( [this]
  {
    this->hide ();
  });
  this->signal_close_request ().connect ( [this]
  {
    if (this->oper != nullptr)
      {
	AuxFunc af;
	std::string filename;
	std::filesystem::path filepath;
	af.homePath (&filename);
	filename = filename + "/.cache/Communist/wsize";
	filepath = std::filesystem::u8path (filename);
	if (!std::filesystem::exists (filepath.parent_path ()))
	  {
	    std::filesystem::create_directories (filepath.parent_path ());
	  }
	int chsv = 1;
	this->prefvectmtx.lock ();
	auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (),
  []
  (auto &el)
    {
      return std::get<0>(el) == "Winsizesv";
    });
	if (itprv != this->prefvect.end ())
	  {
	    if (std::get<1> (*itprv) != "active")
	      {
		chsv = 0;
	      }
	  }
	this->prefvectmtx.unlock ();
	if (chsv == 1)
	  {
	    std::fstream f;
	    std::vector<std::tuple<std::string, std::string, std::string>> wsz;
	    if (std::filesystem::exists (filepath))
	      {
		f.open (filepath, std::ios_base::in);
		while (!f.eof ())
		  {
		    std::string line;
		    getline (f, line);
		    if (line != "")
		      {
			std::tuple<std::string, std::string, std::string> ttup;
			std::string tmp = line;
			tmp = tmp.substr (0, tmp.find (" "));
			std::get<0> (ttup) = tmp;
			tmp = line;
			tmp.erase (0,
				   tmp.find (" ") + std::string (" ").size ());
			tmp = tmp.substr (0, tmp.find (" "));
			std::get<1> (ttup) = tmp;
			tmp = line;
			tmp.erase (0,
				   tmp.find (" ") + std::string (" ").size ());
			tmp.erase (0,
				   tmp.find (" ") + std::string (" ").size ());
			std::get<2> (ttup) = tmp;
			wsz.push_back (ttup);
		      }
		  }
		f.close ();
	      }
	    std::string line;
	    std::tuple<std::string, std::string, std::string> ttup;
	    int width, height;
	    width = this->get_width ();
	    height = this->get_height ();
	    std::stringstream strm;
	    std::locale loc ("C");
	    strm.imbue (loc);
	    strm << width;
	    line = strm.str ();
	    std::get<1> (ttup) = line;
	    strm.clear ();
	    strm.str ("");
	    strm.imbue (loc);
	    strm << height;
	    line = strm.str ();
	    std::get<2> (ttup) = line;
	    line = "MainWindow";
	    std::get<0> (ttup) = line;
	    auto itwsz = std::find_if (wsz.begin (), wsz.end (), [&line]
	    (auto &el)
	      {
		return std::get<0>(el) == line;
	      });
	    if (itwsz != wsz.end ())
	      {
		*itwsz = ttup;
	      }
	    else
	      {
		wsz.push_back (ttup);
	      }
	    if (this->Mwpaned != nullptr)
	      {
		strm.clear ();
		strm.str ("");
		strm.imbue (loc);
		strm << this->Mwpaned->get_position ();
		line = "MwPaned";
		std::get<0> (ttup) = line;
		std::get<1> (ttup) = strm.str ();
		std::get<2> (ttup) = "";
		itwsz = std::find_if (wsz.begin (), wsz.end (), [&line]
		(auto &el)
		  {
		    return std::get<0>(el) == line;
		  });
		if (itwsz != wsz.end ())
		  {
		    *itwsz = ttup;
		  }
		else
		  {
		    wsz.push_back (ttup);
		  }
	      }
	    f.open (filepath, std::ios_base::out | std::ios_base::binary);
	    for (size_t i = 0; i < wsz.size (); i++)
	      {
		line = std::get<0> (wsz[i]) + " " + std::get<1> (wsz[i]) + " "
		    + std::get<2> (wsz[i]) + "\n";
		f.write (line.c_str (), line.size ());
	      }
	    f.close ();
	  }
	else
	  {
	    if (std::filesystem::exists (filepath))
	      {
		std::filesystem::remove (filepath);
	      }
	  }

	this->unset_child ();
	this->set_deletable (false);
	Gtk::ProgressBar *bar = Gtk::make_managed<Gtk::ProgressBar> ();
	bar->set_name ("clBar");
	bar->get_style_context ()->add_provider (
	    this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	bar->set_halign (Gtk::Align::CENTER);
	bar->set_valign (Gtk::Align::CENTER);
	bar->set_text (gettext ("Wait, application is closing..."));
	bar->set_show_text (true);

	int *thc = new int (0);
	std::mutex *thm = new std::mutex;
	this->set_child (*bar);
	Glib::Dispatcher *disppulscl = new Glib::Dispatcher;
	disppulscl->connect ( [bar]
	{
	  bar->pulse ();
	});
	this->dispclose.connect ( [disppulscl]
	{
	  delete disppulscl;
	});
	std::thread *wth = new std::thread ( [disppulscl, this, thc, thm]
	{
	  int count = 0;
	  for (;;)
	    {
	      thm->lock ();
	      if (*thc == 1 || count > 50)
		{
		  break;
		  thm->unlock ();
		}
	      thm->unlock ();
	      disppulscl->emit ();
	      count++;
	      usleep (100000);
	    }
	  delete thc;
	  delete thm;
	  this->dispclose.emit ();
	});
	wth->detach ();
	delete wth;
	this->oper->canceled.connect ( [this, thc, thm]
	{
	  thm->lock ();
	  *thc = 1;
	  thm->unlock ();
	});
	this->oper->cancelAll ();
      }

    AuxFunc af;
    std::string filename;
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/Communist";
    std::filesystem::path filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
    filename = filepath.u8string ();
    filename = filename + "Net";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistSM";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistRM";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistEProf";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistResent";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistRMM";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
#ifdef __linux
    filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
    filename =
    	std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommAR";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }
    if (this->Deleteprof == 1)
      {
	af.homePath (&filename);
	filename = filename + "/.Communist";
	filepath = std::filesystem::u8path (filename);
	std::filesystem::remove_all (filepath);
      }
    af.homePath (&filename);
    filename = filename + "/.Communist/Bufer";
    filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
	std::filesystem::remove_all (filepath);
      }

    if (this->oper == nullptr)
      {
	this->hide ();
      }
    return true;
  },
  false);
  AuxFunc af;
  std::filesystem::path p (std::filesystem::u8path (af.get_selfpath ()));
  Sharepath = p.parent_path ().u8string () + "/../share/Communist";
  std::string filename;
  af.homePath (&filename);
  filename = filename + "/.config/Communist/Prefs";
  std::filesystem::path prefpath = std::filesystem::u8path (filename);
  std::fstream f;
  f.open (prefpath, std::ios_base::in);
  if (f.is_open ())
    {
      while (!f.eof ())
	{
	  std::string line;
	  getline (f, line);
	  if (line != "")
	    {
	      std::tuple<std::string, std::string> ttup;
	      std::get<0> (ttup) = line.substr (0, line.find (":"));
	      line.erase (0, line.find (" ") + std::string (" ").size ());
	      std::get<1> (ttup) = line;
	      prefvectmtx.lock ();
	      prefvect.push_back (ttup);
	      prefvectmtx.unlock ();
	    }
	}
      f.close ();
    }
  prefvectmtx.lock ();
  auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Theme";
    });
  if (itprv != prefvect.end ())
    {
      Theme = std::get<1> (*itprv);
      filename = Sharepath + "/themes/" + Theme;
      std::filesystem::path thpath = std::filesystem::u8path (filename);
      if (!std::filesystem::exists (thpath))
	{
	  Theme = "default";
	}
    }
  else
    {
      Theme = "default";
    }
  prefvectmtx.unlock ();

  css_provider = Gtk::CssProvider::create ();

  css_provider->load_from_path (
      Glib::ustring (Sharepath + "/themes/" + Theme + "/mainWindow.css"));

#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist";
  p = std::filesystem::u8path (filename);
  if (std::filesystem::exists (p))
    {
      std::filesystem::remove_all (p);
    }
  filename = filename + "Net";
  p = std::filesystem::u8path (filename);
  if (std::filesystem::exists (p))
    {
      std::filesystem::remove_all (p);
    }

  af.homePath (&filename);
  filename = filename + "/.cache/Communist/wsize";
  p = std::filesystem::u8path (filename);
  if (std::filesystem::exists (p))
    {
      std::fstream f;
      f.open (p, std::ios_base::in);
      while (!f.eof ())
	{
	  std::string line;
	  getline (f, line);
	  if (line != "")
	    {
	      std::tuple<std::string, int, int> ttup;
	      int width, height;
	      std::string tmp = line;
	      tmp = tmp.substr (0, tmp.find (" "));
	      std::get<0> (ttup) = tmp;
	      tmp = line;
	      tmp.erase (0, tmp.find (" ") + std::string (" ").size ());
	      tmp = tmp.substr (0, tmp.find (" "));
	      std::stringstream strm;
	      std::locale loc ("C");
	      strm.imbue (loc);
	      strm << tmp;
	      strm >> width;
	      std::get<1> (ttup) = width;
	      tmp = line;
	      tmp.erase (0, tmp.find (" ") + std::string (" ").size ());
	      tmp.erase (0, tmp.find (" ") + std::string (" ").size ());
	      strm.clear ();
	      strm.str ("");
	      strm.imbue (loc);
	      strm << tmp;
	      strm >> height;
	      std::get<2> (ttup) = height;
	      winszs.push_back (ttup);
	    }
	}
      f.close ();
    }
  userCheck ();
}

MainWindow::~MainWindow ()
{
  if (spch != nullptr)
    {
      delete spch;
    }
}

void
MainWindow::userCheck ()
{
  std::string filename;
  AuxFunc af;
  af.homePath (&filename);
  filename = filename + "/.Communist";
  std::filesystem::path filepath = std::filesystem::u8path (filename);
  this->set_name ("mainWindow");
  this->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  if (!std::filesystem::exists (filepath))
    {
      std::filesystem::create_directories (filepath);
    }
  if (std::filesystem::is_empty (filepath))
    {
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      this->unset_child ();
      this->set_title (gettext ("Profile creation"));
      this->set_child (*grid);

      Gtk::Label *username = Gtk::make_managed<Gtk::Label> ();
      username->set_text (gettext ("User name:"));
      username->set_halign (Gtk::Align::CENTER);
      grid->attach (*username, 0, 0, 2, 1);

      Gtk::Entry *usname = new Gtk::Entry;
      usname->set_margin (5);
      usname->set_activates_default (true);
      grid->attach (*usname, 0, 1, 2, 1);

      Gtk::Label *passwd = Gtk::make_managed<Gtk::Label> ();
      passwd->set_text (gettext ("Password:"));
      passwd->set_halign (Gtk::Align::CENTER);
      grid->attach (*passwd, 0, 2, 2, 1);

      Gtk::Entry *password = Gtk::make_managed<Gtk::Entry> ();
      password->set_margin (5);
      password->set_activates_default (true);
      Glib::PropertyProxy<Gtk::InputPurpose> propentp =
	  password->property_input_purpose ();
      propentp.set_value (Gtk::InputPurpose::PASSWORD);
      Glib::PropertyProxy<bool> vis = password->property_visibility ();
      vis.set_value (false);
      grid->attach (*password, 0, 3, 2, 1);

      Gtk::Label *reppasswd = Gtk::make_managed<Gtk::Label> ();
      reppasswd->set_text (gettext ("Repeat password:"));
      reppasswd->set_halign (Gtk::Align::CENTER);
      grid->attach (*reppasswd, 0, 4, 2, 1);

      Gtk::Entry *reppassword = Gtk::make_managed<Gtk::Entry> ();
      reppassword->set_margin (5);
      reppassword->set_activates_default (true);
      Glib::PropertyProxy<Gtk::InputPurpose> propentp2 =
	  reppassword->property_input_purpose ();
      propentp2.set_value (Gtk::InputPurpose::PASSWORD);
      Glib::PropertyProxy<bool> vis2 = reppassword->property_visibility ();
      vis2.set_value (false);
      grid->attach (*reppassword, 0, 5, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
      yes->set_label (gettext ("Confirm"));
      yes->set_margin (5);
      yes->set_halign (Gtk::Align::CENTER);
      yes->set_name ("applyButton");
      yes->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      yes->signal_clicked ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::infoMessage), usname,
		      password, reppassword));
      grid->attach (*yes, 0, 6, 1, 1);

      this->set_default_widget (*yes);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
      cancel->set_label (gettext ("Cancel"));
      cancel->set_margin (5);
      cancel->set_halign (Gtk::Align::CENTER);
      cancel->set_name ("rejectButton");
      cancel->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      cancel->signal_clicked ().connect ( [this]
      {
	std::string filename;
	AuxFunc af;
	af.homePath (&filename);
	filename = filename + "/.config/Communist";
	std::filesystem::path prefpath = std::filesystem::u8path (filename);
	std::filesystem::remove_all (prefpath);
	this->close ();
      });
      grid->attach (*cancel, 1, 6, 1, 1);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size (rq1, rq2);
      this->set_default_size (rq2.get_width (), -1);
    }
  else
    {
      Deleteprof = 0;
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      this->unset_child ();
      this->set_title (gettext ("Login"));
      this->set_child (*grid);

      Gtk::Label *username = Gtk::make_managed<Gtk::Label> ();
      username->set_text (gettext ("User name:"));
      username->set_halign (Gtk::Align::CENTER);
      grid->attach (*username, 0, 0, 2, 1);

      Gtk::Entry *usname = new Gtk::Entry;
      usname->set_margin (5);
      usname->set_activates_default (true);
      grid->attach (*usname, 0, 1, 2, 1);

      Gtk::Label *passwd = Gtk::make_managed<Gtk::Label> ();
      passwd->set_text (gettext ("Password:"));
      passwd->set_halign (Gtk::Align::CENTER);
      grid->attach (*passwd, 0, 2, 2, 1);

      Gtk::Entry *password = Gtk::make_managed<Gtk::Entry> ();
      password->set_margin (5);
      password->set_activates_default (true);
      Glib::PropertyProxy<Gtk::InputPurpose> propentp =
	  password->property_input_purpose ();
      propentp.set_value (Gtk::InputPurpose::PASSWORD);
      Glib::PropertyProxy<bool> vis = password->property_visibility ();
      vis.set_value (false);
      grid->attach (*password, 0, 3, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
      yes->set_label (gettext ("Confirm"));
      yes->set_margin (5);
      yes->set_halign (Gtk::Align::CENTER);
      yes->set_name ("applyButton");
      yes->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      yes->signal_clicked ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::userCheckFun), usname,
		      password));
      grid->attach (*yes, 0, 4, 1, 1);

      this->set_default_widget (*yes);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
      cancel->set_label (gettext ("Cancel"));
      cancel->set_margin (5);
      cancel->set_halign (Gtk::Align::CENTER);
      cancel->set_name ("rejectButton");
      cancel->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      cancel->signal_clicked ().connect (
	  sigc::mem_fun (*this, &Gtk::Window::close));
      grid->attach (*cancel, 1, 4, 1, 1);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size (rq1, rq2);
      this->set_default_size (rq2.get_width (), -1);
    }
}

void
MainWindow::mainWindow ()
{
  contacts.clear ();
  AuxFunc af;
  prefvectmtx.lock ();
  auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "SoundOn";
    });
  if (itprv != prefvect.end ())
    {
      if (std::get<1> (*itprv) == "active")
	{
	  std::string soundfl = Sharepath;
	  soundfl = soundfl + "/Signal.flac";
	  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "SoundPath";
	    });
	  if (itprv != prefvect.end ())
	    {
	      std::string tsfl = std::get<1> (*itprv);
	      std::filesystem::path tsflp = std::filesystem::u8path (tsfl);
	      if (std::filesystem::exists (tsflp))
		{
		  soundfl = tsflp.u8string ();
		}
	    }
	  Glib::RefPtr<Gio::File> sfl = Gio::File::create_for_path (soundfl);
	  mf = Gtk::MediaFile::create (sfl);
	}
    }
  else
    {
      std::string soundfl = Sharepath;
      soundfl = soundfl + "/Signal.flac";
      Glib::RefPtr<Gio::File> sfl = Gio::File::create_for_path (soundfl);
      mf = Gtk::MediaFile::create (sfl);
    }

  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Language";
    });
  if (itprv != prefvect.end ())
    {
      std::filesystem::path dicp = std::filesystem::u8path (
	  std::string (Sharepath + "/HunDict/languages"));
      std::string langnum;
      std::fstream f;
      f.open (dicp, std::ios_base::in);
      if (f.is_open ())
	{
	  while (!f.eof ())
	    {
	      std::string line;
	      getline (f, line);
	      if (line != "")
		{
		  langnum = line.substr (0, line.find (" "));
		  line.erase (0, line.find (" ") + std::string (" ").size ());
		  if (line == std::get<1> (*itprv))
		    {
		      break;
		    }
		}
	    }
	  f.close ();
	}

      std::string vocp1, vocp2;
      vocp1 = Sharepath + "/HunDict/" + langnum + "/dic.aff";
      vocp2 = Sharepath + "/HunDict/" + langnum + "/dic.dic";
      spch = new Hunspell (vocp1.c_str (), vocp2.c_str ());
    }

  prefvectmtx.unlock ();
  autoRemoveMsg ();
  std::vector<std::string> blockedc;
  frvectmtx.lock ();
  for (size_t i = 0; i < friendvect.size (); i++)
    {
      if (std::get<6> (friendvect[i]) != nullptr)
	{
	  std::string key = std::string (
	      std::get<2> (friendvect[i])->get_text ());
	  blockedc.push_back (key);
	}
    }
  friendvect.clear ();
  frvectmtx.unlock ();
  std::string filename;
  std::filesystem::path filepath;
  std::string mainmenutr;
  this->unset_child ();
  avatar = nullptr;
  profilevector.clear ();
  Image = 0;
  this->set_title (gettext ("Communist"));
  Gtk::Overlay *mw_ovl = Gtk::make_managed<Gtk::Overlay> ();
  this->set_child (*mw_ovl);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::DrawingArea *bckgrnd = Gtk::make_managed<Gtk::DrawingArea> ();
  bckgrnd->set_draw_func (
      [this]
      (const Cairo::RefPtr<Cairo::Context> &cr,
       int width,
       int height)
	 {
	   Glib::ustring file = Glib::ustring (this->Sharepath + "/themes/" +
	       this->Theme + "/background.jpeg");
	   Glib::RefPtr<Gdk::Pixbuf> imageloc = Gdk::Pixbuf::create_from_file (file);
	   imageloc = imageloc->scale_simple (width, height, Gdk::InterpType::BILINEAR);
	   Gdk::Cairo::set_source_pixbuf (cr, imageloc, 0, 0);
	   cr->rectangle (0, 0, width, height);
	   cr->fill ();
	 });
  mw_ovl->set_child (*bckgrnd);
  mw_ovl->add_overlay (*grid);
  Glib::RefPtr<Gio::SimpleActionGroup> pref = Gio::SimpleActionGroup::create ();
  pref->add_action ("editpr", sigc::mem_fun (*this, &MainWindow::editProfile));
  pref->add_action ("ownkey", sigc::mem_fun (*this, &MainWindow::ownKey));
  pref->add_action ("editReqList",
		    sigc::mem_fun (*this, &MainWindow::editAddFriends));
  pref->add_action ("editSettings",
		    sigc::mem_fun (*this, &MainWindow::settingsWindow));
  pref->add_action ("aboutProg", sigc::mem_fun (*this, &MainWindow::aboutProg));
  this->insert_action_group ("prefgr", pref);
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create ();

  try
    {
      builder->add_from_file (
	  Sharepath + "/Translations.d/" + Glib::ustring (mainmenutr)
	      + "/mainmenu");
    }
  catch (const Glib::Error &er)
    {
      Glib::ustring buffer =
	  gettext (
	      "<interface>"
	      "<menu id='menubar'>"
	      "    <submenu>"
	      "		<attribute name='label' translatable='yes'>Contacts</attribute>"
	      "			<section>"
	      "				<item>"
	      "					<attribute name='label' translatable='yes'>Edit sent request table</attribute>"
	      "					<attribute name='action'>prefgr.editReqList</attribute>"
	      "				</item>"
	      "			</section>"
	      "	</submenu>"
	      "	<submenu>"
	      "		<attribute name='label' translatable='yes'>Instruments</attribute>"
	      "			<section>"
	      "				<item>"
	      "					<attribute name='label' translatable='yes'>Edit profile</attribute>"
	      "					<attribute name='action'>prefgr.editpr</attribute>"
	      "				</item>"
	      "				<item>"
	      "					<attribute name='label' translatable='yes'>Own key</attribute>"
	      "					<attribute name='action'>prefgr.ownkey</attribute>"
	      "				</item>"
	      "                         <item>"
	      "					<attribute name='label' translatable='yes'>Settings</attribute>"
	      "					<attribute name='action'>prefgr.editSettings</attribute>"
	      "				</item>"
	      "			</section>"
	      "	</submenu>"
	      "	<submenu>"
	      "		<attribute name='label' translatable='yes'>About</attribute>"
	      "			<section>"
	      "				<item>"
	      "					<attribute name='label' translatable='yes'>About Communist</attribute>"
	      "					<attribute name='action'>prefgr.aboutProg</attribute>"
	      "				</item>"
	      "			</section>"
	      "	</submenu>"
	      "</menu>"
	      "</interface>");
      builder->add_from_string (buffer);
    }
  Gtk::Box *box = new Gtk::Box;
  auto object = builder->get_object ("menubar");
  auto gmenu = std::dynamic_pointer_cast<Gio::Menu> (object);
  if (!gmenu)
    {
      g_warning("GMenu not found");
    }
  else
    {
      auto pMenuBar = Gtk::make_managed<Gtk::PopoverMenuBar> (gmenu);
      pMenuBar->set_name ("mainMenu");
      pMenuBar->get_style_context ()->add_provider (
	  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      box->append (*pMenuBar);
    }
  box->set_halign (Gtk::Align::START);
  grid->attach (*box, 0, 0, 1, 1);

  Gtk::Paned *pane = new Gtk::Paned;
  Mwpaned = pane;
  pane->set_orientation (Gtk::Orientation::HORIZONTAL);
  Gtk::Grid *leftgrid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::Grid *rightgrid = Gtk::make_managed<Gtk::Grid> ();
  pane->set_start_child (*leftgrid);
  pane->set_end_child (*rightgrid);
  auto itwnszs = std::find_if (winszs.begin (), winszs.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "MwPaned";
    });
  if (itwnszs != winszs.end ())
    {
      pane->set_position (std::get<1> (*itwnszs));
    }
  Rightgrid = rightgrid;

  Gtk::ScrolledWindow *winleft = Gtk::make_managed<Gtk::ScrolledWindow> ();
  Gtk::ScrolledWindow *winright = Gtk::make_managed<Gtk::ScrolledWindow> ();
  Winright = winright;
  Glib::RefPtr<Gtk::Adjustment> adj = Winright->get_vadjustment ();

  adj->signal_changed ().connect ( [this, adj]
  {
    if (this->usermsgadj < 0 && this->msgwinadj < 0)
      {

	this->msgwinadjdsp.emit ();
      }
    else
      {
	if (this->msgwinadj >= 0)
	  {
	    adj->set_value (adj->get_upper () - this->msgwinadj);
	    this->msgwinadj = -1;
	  }
      }
  });
  msgwinadjdsp.connect ( [adj]
  {
    adj->set_value (adj->get_upper () - adj->get_page_size ());
  });
  adj->signal_value_changed ().connect ( [adj, this]
  {
    this->usermsgadj = adj->get_value ();
    if (this->usermsgadj == adj->get_upper () - adj->get_page_size ())
      {
	this->usermsgadj = -1;
	if (this->msgovllab != nullptr)
	  {
	    if (this->msgovl != nullptr)
	      {
		Gtk::Widget *widg = this->msgovllab->get_parent ();
		if (widg != nullptr)
		  {
		    widg = widg->get_parent ();
		    if (widg != nullptr)
		      {
			this->msgovl->remove_overlay (*widg);
			this->msgovllab = nullptr;
		      }
		  }
	      }
	  }
      }
  });

  Winleft = winleft;
  winleft->set_expand (true);
  winright->set_expand (true);
  winright->set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  Gtk::Grid *winleftgr = Gtk::make_managed<Gtk::Grid> ();
  fordelgrid = winleftgr;
  leftgrid->attach (*winleft, 0, 0, 2, 1);
  Gtk::Overlay *ovl = Gtk::make_managed<Gtk::Overlay> ();
  ovl->set_child (*winright);
  msgovl = ovl;
  rightgrid->attach (*ovl, 0, 0, 3, 1);

  Gtk::Button *addcont = Gtk::make_managed<Gtk::Button> ();
  addcont->set_label (gettext ("Add contact"));
  addcont->set_margin (5);
  addcont->set_halign (Gtk::Align::CENTER);
  addcont->set_name ("applyButton");
  addcont->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  addcont->signal_clicked ().connect (
      sigc::mem_fun (*this, &MainWindow::addFriends));
  leftgrid->attach (*addcont, 0, 1, 1, 1);

  Gtk::Button *delcont = Gtk::make_managed<Gtk::Button> ();
  delcont->set_label (gettext ("Remove contact"));
  delcont->set_margin (5);
  delcont->set_halign (Gtk::Align::CENTER);
  delcont->set_name ("rejectButton");
  delcont->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  delcont->signal_clicked ().connect (
      sigc::mem_fun (*this, &MainWindow::deleteContact));
  Gtk::Button tb;
  Gtk::Grid tg;
  tb.set_child (tg);
  Gtk::DrawingArea dr;
  dr.set_size_request (50, 50);
  dr.set_margin (2);
  tg.attach (dr, 0, 0, 1, 1);
  Gtk::Requisition rqq1, rqq2;
  tb.get_preferred_size (rqq1, rqq2);
  winleft->set_min_content_height (rqq2.get_height () * 5);
  Gdk::Rectangle rctg = screenRes ();
  winleft->set_min_content_width (rctg.get_width () * 0.25);
  leftgrid->attach (*delcont, 1, 1, 1, 1);
  leftgrid->get_preferred_size (rqq1, rqq2);
  contact_button_width = rqq2.get_width () - 5;
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist";
  filepath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (filepath))
    {
      std::filesystem::remove_all (filepath);
    }
  std::filesystem::create_directories (filepath);
  std::string outfile = filepath.u8string ();
  outfile = outfile + "/Profile.zip";
  af.homePath (&filename);
  filename = filename + "/.Communist/Profile";
  proffopmtx.lock ();
  af.decryptFile (Username, Password, filename, outfile);
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist";
  af.unpacking (outfile, filename);
  filename = filename + "/Profile/Contacts";
  filepath = std::filesystem::u8path (filename);
  std::string line;
  std::fstream f;
  int count = 0;
  if (std::filesystem::exists (filepath))
    {
      f.open (filepath, std::ios_base::in);
      while (!f.eof ())
	{
	  getline (f, line);
	  if (line != "")
	    {
	      std::tuple<int, std::string> contact;
	      std::get<0> (contact) = count;
	      while (line.size () > 64)
		{
		  line.pop_back ();
		}
	      std::get<1> (contact) = line;
	      contmtx.lock ();
	      contacts.push_back (contact);
	      contmtx.unlock ();
	    }
	  count++;
	}
      f.close ();
    }
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist/Profile/RequestList";
  filepath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (filepath))
    {
      f.open (filepath, std::ios_base::in);
      while (!f.eof ())
	{
	  getline (f, line);
	  if (line != "")
	    {
	      addfrmtx.lock ();
	      Addfriends.push_back (line);
	      addfrmtx.unlock ();
	    }
	}
      f.close ();
    }

  af.homePath (&filename);
  filename = filename + "/.Communist";
  filepath = std::filesystem::u8path (filename);
  std::vector<std::pair<double, std::string>> accountlist;
  for (auto it : std::filesystem::directory_iterator (filepath))
    {
      std::filesystem::path p = it.path ();
      if (std::filesystem::is_directory (p)
	  && p.filename ().u8string () != "Bufer"
	  && p.filename ().u8string () != "SendBufer")
	{
	  std::vector<double> lastdate;
	  double date;
	  line = p.u8string ();
	  line = line + "/Yes";
	  std::filesystem::path pp = std::filesystem::u8path (line);
	  line = pp.u8string ();
	  if (std::filesystem::exists (pp))
	    {
	      outfile = p.u8string ();
	      outfile = outfile + "/YesPr";
	      pp = std::filesystem::u8path (outfile);
	      outfile = pp.u8string ();
	      yesmtx.lock ();
	      af.decryptFile (Username, Password, line, outfile);
	      std::vector<std::string> tempvect;
	      f.open (pp, std::ios_base::in);
	      int yespr = 0;
	      while (!f.eof ())
		{
		  getline (f, line);
		  if (line != "" && yespr > 0)
		    {
		      tempvect.push_back (line);
		    }
		  yespr++;
		}
	      f.close ();
	      std::filesystem::remove (pp);
	      yesmtx.unlock ();
	      line = tempvect[tempvect.size () - 1];
	      line.erase (0, line.find (" ") + std::string (" ").size ());
	      std::stringstream strm;
	      std::locale loc ("C");
	      strm << line;
	      strm >> date;
	      lastdate.push_back (date);
	    }
	  else
	    {
	      lastdate.push_back (0);
	    }
	  line = p.filename ().u8string ();
	  std::pair<double, std::string> accadd;
	  accadd.second = line;
	  if (lastdate.size () > 0)
	    {
	      std::sort (lastdate.begin (), lastdate.end (),
			 std::greater<double> ());
	      accadd.first = lastdate[0];
	    }
	  else
	    {
	      accadd.first = 0;
	    }
	  accountlist.push_back (accadd);
	}
    }
  if (accountlist.size () > 0)
    {
      std::sort (accountlist.begin (), accountlist.end (), []
      (auto el1, auto el2)
	{
	  if(el1.first == 0 && el2.first == 0)
	    {
	      return el2.second < el1.second;
	    }
	  else
	    {
	      return el1.first>el2.first;
	    }
	});
      Gtk::Requisition rq1, rq2;
      for (size_t i = 0; i < accountlist.size (); i++)
	{
	  af.homePath (&filename);
	  filename = filename + "/.Communist/" + accountlist[i].second
	      + "/Profile";
#ifdef __linux
	  outfile = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	  outfile = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	  outfile = outfile + "/Communist";
	  filepath = std::filesystem::u8path (outfile);
	  if (std::filesystem::exists (filepath))
	    {
	      std::filesystem::remove_all (filepath);
	    }
	  std::filesystem::create_directories (filepath);
	  outfile = outfile + "/ProfileOp.zip";
	  std::string uname;
	  std::string passwd;
	  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkeypair;
	  ownkeypair = lt::dht::ed25519_create_keypair (seed);
	  std::array<char, 32> scalar;
	  uname = lt::aux::to_hex (std::get<0> (ownkeypair).bytes);
	  lt::dht::public_key otherpk;
	  std::string otherpkstr = accountlist[i].second;
	  contmtx.lock ();
	  auto vit = std::find_if (contacts.begin (), contacts.end (),
				   [&otherpkstr]
				   (auto &el)
				     {
				       int t = std::stoi(otherpkstr);
				       return t == std::get<0>(el);
				     });
	  if (vit != contacts.end ())
	    {
	      otherpkstr = std::get<1> (*vit);
	      lt::aux::from_hex (otherpkstr, otherpk.bytes.data ());
	      scalar = lt::dht::ed25519_key_exchange (otherpk,
						      std::get<1> (ownkeypair));
	      lt::dht::public_key pkpass = lt::dht::ed25519_add_scalar (
		  std::get<0> (ownkeypair), scalar);
	      passwd = lt::aux::to_hex (pkpass.bytes);
	      af.decryptFile (uname, passwd, filename, outfile);
#ifdef __linux
	      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	      filename = filename + "/Communist";
	      af.unpacking (outfile, filename);
	      std::tuple<Gtk::Button*, Gtk::Grid*, Gtk::Label*, Gtk::Label*,
		  Gtk::Label*, Gtk::Label*, Gtk::Label*> frtup;
	      Gtk::Button *buttontr = Gtk::make_managed<Gtk::Button> ();
	      std::get<0> (frtup) = buttontr;
	      buttontr->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      buttontr->set_name ("inactiveButton");
	      Gtk::Grid *gridtr = Gtk::make_managed<Gtk::Grid> ();
	      buttontr->set_child (*gridtr);
	      Glib::RefPtr<Gtk::GestureClick> clck =
		  Gtk::GestureClick::create ();
	      clck->set_button (3);
	      clck->signal_pressed ().connect (
		  sigc::bind (sigc::mem_fun (*this, &MainWindow::contMenu),
			      clck, buttontr));
	      buttontr->add_controller (clck);
	      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
		  Gio::SimpleActionGroup::create ();
	      acgroup->add_action (
		  "info",
		  sigc::bind (sigc::mem_fun (*this, &MainWindow::friendDetails),
			      buttontr));
	      acgroup->add_action (
		  "delete", sigc::mem_fun (*this, &MainWindow::deleteContact));
	      acgroup->add_action (
		  "block", sigc::mem_fun (*this, &MainWindow::tempBlockCont));
	      buttontr->insert_action_group ("popupcont", acgroup);
	      std::get<1> (frtup) = gridtr;
#ifdef __linux
	      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	      filename = filename + "/Communist/Profile/Avatar.jpeg";
	      std::string avatarpath = filename;
	      Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea> ();
	      drar->set_margin (2);
	      drar->set_halign (Gtk::Align::CENTER);
	      drar->set_size_request (50, 50);

	      gridtr->attach (*drar, 0, 0, 1, 3);
	      std::vector<Glib::ustring> forlab;
	      forlab.push_back (Glib::ustring (otherpkstr));
#ifdef __linux
	      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	      filename = filename + "/Communist/Profile/Profile";
	      filepath = std::filesystem::u8path (filename);
	      f.open (filepath, std::ios_base::in);
	      std::string line;
	      count = 0;
	      while (!f.eof ())
		{
		  getline (f, line);
		  if (count == 2 && forlab.size () > 2)
		    {
		      forlab[2] = forlab[2] + " " + Glib::ustring (line);
		    }
		  else
		    {
		      forlab.push_back (Glib::ustring (line));
		    }
		  count++;
		}
	      f.close ();
	      Gtk::Label *keylab = Gtk::make_managed<Gtk::Label> ();
	      std::get<2> (frtup) = keylab;
	      keylab->set_max_width_chars (10);
	      keylab->set_ellipsize (Pango::EllipsizeMode::END);
	      keylab->set_text (forlab[0]);
	      gridtr->attach (*keylab, 1, 0, 1, 1);
	      filepath = std::filesystem::u8path (avatarpath);
	      if (std::filesystem::exists (filepath))
		{
		  Glib::RefPtr<Gdk::Pixbuf> imagec =
		      Gdk::Pixbuf::create_from_file (
			  Glib::ustring (avatarpath));
		  int drarw, drarh;
		  drar->get_size_request (drarw, drarh);
		  imagec = imagec->scale_simple (drarw, drarh,
						 Gdk::InterpType::BILINEAR);
		  std::pair<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> pair;
		  pair.first = keylab->get_text ();
		  pair.second = imagec;
		  conavmtx.lock ();
		  conavatars.push_back (pair);
		  conavmtx.unlock ();
		  drar->set_draw_func (
		      sigc::bind (
			  sigc::mem_fun (*this,
					 &MainWindow::on_draw_contactlist),
			  keylab));
		}
	      Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label> ();
	      nicklab->set_text (forlab[1]);
	      nicklab->set_halign (Gtk::Align::START);
	      gridtr->attach (*nicklab, 1, 1, 1, 1);
	      std::get<4> (frtup) = nicklab;

	      Gtk::Label *namelab = Gtk::make_managed<Gtk::Label> ();
	      if (forlab.size () > 2)
		{
		  namelab->set_text (forlab[2]);
		}
	      namelab->set_halign (Gtk::Align::START);
	      gridtr->attach (*namelab, 1, 2, 1, 1);
	      std::get<5> (frtup) = namelab;

	      buttontr->set_size_request (contact_button_width, -1);
	      winleftgr->attach (*buttontr, 0, i, 1, 1);
	      buttontr->get_preferred_size (rq1, rq2);
	      buttontr->signal_clicked ().connect (
		  sigc::bind (sigc::mem_fun (*this, &MainWindow::selectContact),
			      winright, std::string (keylab->get_text ()),
			      std::string (nicklab->get_text ())));
	      std::string searchk (forlab[0]);
	      auto itbll = std::find (blockedc.begin (), blockedc.end (),
				      searchk);
	      if (itbll != blockedc.end ())
		{
		  Gtk::Label *blocklab = Gtk::make_managed<Gtk::Label> ();
		  blocklab->set_name ("blockLab");
		  blocklab->get_style_context ()->add_provider (
		      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
		  blocklab->set_text (gettext ("Blocked"));
		  gridtr->attach_next_to (*blocklab, *nicklab,
					  Gtk::PositionType::RIGHT, 1, 1);
		  std::get<6> (frtup) = blocklab;
		}
	      else
		{
		  std::get<6> (frtup) = nullptr;
		}

	      std::get<3> (frtup) = nullptr;

	      frvectmtx.lock ();
	      friendvect.push_back (frtup);
	      frvectmtx.unlock ();
	    }
	  contmtx.unlock ();
	}
      winleft->set_margin (2);
    }
  proffopmtx.unlock ();
  Gtk::Requisition rq1, rq2;
  winleft->set_child (*winleftgr);
  winleft->set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  winleft->set_expand (true);
  leftgrid->get_preferred_size (rq1, rq2);
  pane->set_resize_start_child (false);
  winright->set_min_content_width (rq2.get_width ());

  Gtk::Button *attachfile = Gtk::make_managed<Gtk::Button> ();
  attachfile->set_margin (5);
  attachfile->set_halign (Gtk::Align::CENTER);
  attachfile->set_valign (Gtk::Align::CENTER);
  attachfile->get_preferred_size (rqq1, rqq2);
  attachfile->set_name ("sendButton");
  attachfile->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  attachfile->set_tooltip_text (gettext ("Attach file"));
  Gtk::DrawingArea *drarfb = Gtk::make_managed<Gtk::DrawingArea> ();
  drarfb->set_size_request (rqq1.get_width (), rqq1.get_height ());
  drarfb->set_draw_func (
      sigc::bind (
	  sigc::mem_fun (*this, &MainWindow::on_draw_sb),
	  Glib::ustring (Sharepath + "/themes/" + Theme + "/file-icon.png")));
  attachfile->set_child (*drarfb);
  attachfile->signal_clicked ().connect (
      sigc::mem_fun (*this, &MainWindow::attachFileDialog));
  attachfbutton = attachfile;
  rightgrid->attach (*attachfile, 0, 1, 1, 1);

  Gtk::ScrolledWindow *winmsg = Gtk::make_managed<Gtk::ScrolledWindow> ();
  Gtk::TextView *msgtos = Gtk::make_managed<Gtk::TextView> ();
  msgfrm = msgtos;
  msgtos->set_wrap_mode (Gtk::WrapMode::WORD_CHAR);
  Glib::RefPtr<Gtk::EventControllerKey> entcont =
      Gtk::EventControllerKey::create ();
  entcont->signal_key_pressed ().connect (
      [msgtos, this]
      (guint keyval,
       guint keycode,
       Gdk::ModifierType state)
	 {
	   if (keyval == GDK_KEY_Return)
	     {
	       this->prefvectmtx.lock();
	       auto itprv = std::find_if (this->prefvect.begin(), this->prefvect.end(), [](auto &el)
		     {
		       return std::get<0>(el) == "SendKey";
		     });
	       if (itprv != this->prefvect.end())
		 {
		   if (std::get<1>(*itprv) == "0")
		     {
		       if (keyval == GDK_KEY_Return &&
			   (state & (Gdk::ModifierType::SHIFT_MASK |
				   Gdk::ModifierType::CONTROL_MASK |
				   Gdk::ModifierType::ALT_MASK)) == Gdk::ModifierType::CONTROL_MASK)
			 {
			   this->sendMsg(msgtos);
			   this->prefvectmtx.unlock();
			   return true;
			 }
		       else
			 {
			   Glib::RefPtr<Gtk::TextBuffer> tb = msgtos->get_buffer();
			   Glib::ustring txt = tb->get_text(true);
			   Glib::ustring::size_type n = 0;
			   std::vector<std::tuple<Glib::ustring::size_type, Glib::ustring::size_type>> tv;
			   while (n != Glib::ustring::npos)
			     {
			       if (n > 0)
				 {
				   n = n + Glib::ustring (" ").size ();
				 }
			       Glib::ustring t;
			       t = txt.substr (n, txt.find (" ", n) - n);
			       if (t != "" && txt.find (" ", n) != Glib::ustring::npos)
				 {
				   tv.push_back (std::make_tuple (n, txt.find (" ", n) - n));
				 }
			       else
				 {
				   if (txt.find (" ", n) == Glib::ustring::npos)
				     {
				       tv.push_back (std::make_tuple (n, txt.size () - 1));
				     }
				 }
			       n = txt.find (" ", n);
			     }
			   this->prefvectmtx.unlock();
			   return false;
			 }
		     }
		   else
		     {
		       if (keyval == GDK_KEY_Return)
			 {
			   this->sendMsg(msgtos);
			   this->prefvectmtx.unlock();
			   return true;
			 }
		     }
		 }
	       else
		 {
		   if (keyval == GDK_KEY_Return &&
		       (state & (Gdk::ModifierType::SHIFT_MASK |
			       Gdk::ModifierType::CONTROL_MASK |
			       Gdk::ModifierType::ALT_MASK)) == Gdk::ModifierType::CONTROL_MASK)
		     {
		       this->sendMsg(msgtos);
		       this->prefvectmtx.unlock();
		       return true;
		     }
		   else
		     {
		       this->prefvectmtx.unlock();
		       return false;
		     }
		 }
	       this->prefvectmtx.unlock ();
	     }

	   return false;
	 },
      false);
  msgtos->add_controller (entcont);
  winmsg->set_min_content_width (rq2.get_width ());
  winmsg->set_margin (5);
  winmsg->set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  winmsg->set_has_frame (true);
  winmsg->set_hexpand (true);
  winmsg->set_child (*msgtos);
  rightgrid->attach (*winmsg, 1, 1, 1, 1);

  Glib::RefPtr<Gtk::TextBuffer> textb = msgtos->get_buffer ();
  std::string spchst = "1";
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Spellcheck";
    });
  if (itprv != prefvect.end ())
    {
      spchst = std::get<1> (*itprv);
    }
  prefvectmtx.unlock ();
  if (spchst == "1" && spch != nullptr)
    {
      textb->signal_changed ().connect ( [textb, this]
      {
	Glib::ustring txt = textb->get_text (true);
	if (txt.size () > 0)
	  {
	    Glib::ustring::size_type n = 0;
	    std::vector<std::tuple<Glib::ustring::size_type, Glib::ustring::size_type>> tv;
	    while (n != Glib::ustring::npos)
	      {
		if (n > 0)
		  {
		    n = n + Glib::ustring (" ").size ();
		  }
		Glib::ustring t;
		t = txt.substr (n, txt.find (" ", n) - n);
		if (t != "" && txt.find (" ", n) != Glib::ustring::npos)
		  {
		    tv.push_back (std::make_tuple (n, txt.find (" ", n) - n));
		  }
		else
		  {
		    if (txt.find (" ", n) == Glib::ustring::npos)
		      {
			tv.push_back (std::make_tuple (n, txt.size () - 1));
		      }
		  }
		n = txt.find (" ", n);
	      }

	    for (size_t i = 0; i < tv.size (); i++)
	      {
		std::string word = txt.substr (std::get<0> (tv[i]),
					       std::get<1> (tv[i]));
		AuxFunc af;
		word = af.utf8to (word);
		for (size_t j = 0, len = word.size (); j < len; j++)
		  {
		    if (std::ispunct ((word[j])))
		      {
			word.erase (j);
			len = word.size ();
		      }
		  }
		af.toutf8 (word);
		std::string wordlow = af.stringToLower (word);
		int cor = this->spch->spell (wordlow);
		if (cor < 1)
		  {
		    auto tgtb = textb->get_tag_table ();
		    Glib::RefPtr<Gtk::TextTag> tg;
		    tg = tgtb->lookup ("error");
		    if (tg == nullptr)
		      {
			tg = Gtk::TextBuffer::Tag::create ("error");
			Glib::PropertyProxy<Pango::Underline> prx =
			    tg->property_underline ();
#ifdef __linux
			prx.set_value (Pango::Underline::ERROR);
#endif
#ifdef _WIN32
			prx.set_value (Pango::Underline::ERROR_LINE);
#endif
			tgtb->add (tg);
		      }
		    Gtk::TextBuffer::iterator itb;
		    itb = textb->get_iter_at_offset (int (std::get<0> (tv[i])));

		    Gtk::TextBuffer::iterator ite;
		    ite = textb->get_iter_at_offset (
			int (std::get<0> (tv[i])) + int (std::get<1> (tv[i])));
		    textb->apply_tag (tg, itb, ite);
		  }
		else
		  {
		    Gtk::TextBuffer::iterator itb;
		    itb = textb->get_iter_at_offset (int (std::get<0> (tv[i])));

		    Gtk::TextBuffer::iterator ite;
		    ite = textb->get_iter_at_offset (
			int (std::get<0> (tv[i])) + int (std::get<1> (tv[i])));
		    textb->remove_all_tags (itb, ite);
		  }
	      }
	  }
      });
    }

  Gtk::Button *sendb = Gtk::make_managed<Gtk::Button> ();
  sendb->set_name ("sendButton");
  sendb->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  sendb->set_margin (5);
  sendb->set_halign (Gtk::Align::CENTER);
  sendb->set_valign (Gtk::Align::CENTER);
  sendb->get_preferred_size (rq1, rq2);
  sendb->set_tooltip_text (gettext ("Send message"));
  sendb->signal_clicked ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::sendMsg), msgtos));
  Gtk::DrawingArea *drarsb = Gtk::make_managed<Gtk::DrawingArea> ();
  drarsb->set_size_request (rq2.get_width (), rq2.get_height ());
  std::string spi = Sharepath + "/themes/" + Theme + "/ico.png";
  drarsb->set_draw_func (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::on_draw_sb),
		  Glib::ustring (spi)));
  sendb->set_child (*drarsb);
  rightgrid->attach (*sendb, 2, 1, 1, 1);
  grid->attach (*pane, 0, 1, 1, 1);
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist";
  filepath = std::filesystem::u8path (filename);
  std::filesystem::remove_all (filepath);

  grid->get_preferred_size (rq1, rq2);
  itwnszs = std::find_if (winszs.begin (), winszs.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "MainWindow";
    });
  if (itwnszs != winszs.end ())
    {
      this->set_default_size (std::get<1> (*itwnszs), std::get<2> (*itwnszs));
    }
  else
    {
      this->set_default_size (rq2.get_width (), rq2.get_height ());
    }

  Glib::RefPtr<Gtk::GestureClick> rightgridclck = Gtk::GestureClick::create ();
  rightgridclck->set_button (0);
  rightgridclck->signal_pressed ().connect ( [rightgridclck, this]
  (int clcknum, double x, double y)
    {
      if (rightgridclck->get_current_button () == 1)
	{
	  if (this->rightmenfordel != nullptr)
	    {
	      this->rightmenfordel->hide ();
	      this->rightmenfordel = nullptr;
	    }
	}
    });
  rightgrid->add_controller (rightgridclck);
  networkOp ();
}

void
MainWindow::infoMessage (Gtk::Entry *usname, Gtk::Entry *passwd,
			 Gtk::Entry *reppasswd)
{
  Glib::ustring username = usname->get_text ();
  Glib::ustring password = passwd->get_text ();
  Glib::ustring reppassword = reppasswd->get_text ();
  if (username != "" && password != "" && password == reppassword)
    {
      Username = std::string (username);
      Password = std::string (password);
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      std::filesystem::create_directories (filepath);
      filename = filename + "/Profile";
      filepath = std::filesystem::u8path (filename);
      std::string line = "Future profile data will be placed here\n";
      std::fstream f;
      f.open (filepath, std::ios_base::out | std::ios_base::binary);
      f.write (line.c_str (), line.size ());
      f.close ();
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      filepath = std::filesystem::u8path (filename);
      filename = filepath.u8string ();
      af.packing (filename, filename + ".zip");
      std::filesystem::remove_all (filepath);
      std::string outfile;
      af.homePath (&outfile);
      outfile = outfile + "/.Communist/Profile";
      filename = filename + ".zip";
      af.cryptFile (Username, Password, filename, outfile);
      filepath = std::filesystem::u8path (filename);
      std::filesystem::remove (filepath);
      this->unset_default_widget ();
      createProfile ();
    }
  else
    {
      Gtk::Window *window = new Gtk::Window;
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      window->set_child (*grid);
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

      Gtk::Label *label = new Gtk::Label;
      if (username == "")
	{
	  label->set_text (gettext ("Input user name!"));
	}
      if (password == "" && username != "")
	{
	  label->set_text (gettext ("Input password!"));
	}
      if (password != reppassword && username != "" && password != "")
	{
	  label->set_text (gettext ("Passwords do not match!"));
	}
      label->set_margin (5);
      label->set_halign (Gtk::Align::CENTER);
      grid->attach (*label, 0, 0, 1, 1);

      Gtk::Button *close = new Gtk::Button;
      close->set_label (gettext ("Close"));
      close->set_margin (5);
      close->set_halign (Gtk::Align::CENTER);
      close->set_name ("applyButton");
      close->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      close->signal_clicked ().connect (
	  sigc::mem_fun (*window, &Gtk::Window::close));
      grid->attach (*close, 0, 1, 1, 1);

      window->signal_close_request ().connect ( [window]
      {
	window->hide ();
	delete window;
	return true;
      },
					       false);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size (rq1, rq2);
      window->set_default_size (rq2.get_width (), -1);
      Glib::RefPtr<Gtk::Application> app = this->get_application ();
      app->add_window (*window);
      window->show ();
    }
}

void
MainWindow::userCheckFun (Gtk::Entry *usname, Gtk::Entry *passwd)
{
  AuxFunc af;
  Glib::ustring username = usname->get_text ();
  Glib::ustring password = passwd->get_text ();
  if (username != "" && password != "")
    {
      Username = std::string (username);
      Password = std::string (password);
#ifdef __linux
      std::string dirstr = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      std::string dirstr = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      dirstr = dirstr + "/Communist";
      std::filesystem::path dir = std::filesystem::u8path (dirstr);
      if (std::filesystem::exists (dir))
	{
	  std::filesystem::remove_all (dir);
	}
      std::filesystem::create_directories (dir);
      std::string filename;
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      std::filesystem::path filepath;
      dirstr = dirstr + "/Profile.zip";
      af.decryptFile (Username, Password, filename, dirstr);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist";
      af.unpacking (dirstr, filename);
      dir = std::filesystem::u8path (dirstr);
      std::filesystem::remove (dir);
      filename = filename + "/Profile/Profile";
      filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
#ifdef __linux
	  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
          filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	  filename = filename + "/Communist/Profile/Key";
	  filepath = std::filesystem::u8path (filename);
	  std::fstream f;
	  f.open (filepath, std::ios_base::in | std::ios_base::binary);
	  f.read (seed.data (), seed.size ());
	  f.close ();
	  this->unset_default_widget ();
	  mainWindow ();
	}
    }
}

void
MainWindow::createProfile ()
{
  this->unset_child ();
  this->set_default_size (-1, -1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  this->set_title (gettext ("Profile creation"));
  this->set_child (*grid);

  Gtk::Grid *leftgrid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::Grid *rightgrid = Gtk::make_managed<Gtk::Grid> ();

  grid->attach (*leftgrid, 0, 0, 1, 1);
  grid->attach (*rightgrid, 1, 0, 1, 1);

  Gtk::Entry *key = Gtk::make_managed<Gtk::Entry> ();
  key->set_placeholder_text (gettext ("Generate key (compulsory)"));
  key->set_margin (5);
  key->set_editable (false);

  key->set_max_width_chars (64);
  key->set_halign (Gtk::Align::CENTER);
  rightgrid->attach (*key, 0, 0, 3, 1);
  profilevector.push_back (key);

  Gtk::Button *generate = Gtk::make_managed<Gtk::Button> ();
  generate->set_label (gettext ("Generate"));
  generate->set_margin (5);
  generate->set_halign (Gtk::Align::START);
  generate->set_name ("applyButton");
  generate->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  generate->signal_clicked ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::keyGenerate), key));
  rightgrid->attach (*generate, 0, 1, 3, 1);

  Gtk::Entry *nickname = Gtk::make_managed<Gtk::Entry> ();
  nickname->set_placeholder_text (gettext ("Nickname (compulsory)"));
  nickname->set_margin (5);
  rightgrid->attach (*nickname, 0, 2, 3, 1);
  profilevector.push_back (nickname);

  Gtk::Entry *name = Gtk::make_managed<Gtk::Entry> ();
  name->set_placeholder_text (gettext ("Name (not compulsory)"));
  name->set_margin (5);
  rightgrid->attach (*name, 0, 3, 3, 1);
  profilevector.push_back (name);

  Gtk::Entry *surname = Gtk::make_managed<Gtk::Entry> ();
  surname->set_placeholder_text (gettext ("Surame (not compulsory)"));
  surname->set_margin (5);
  rightgrid->attach (*surname, 0, 4, 3, 1);
  profilevector.push_back (surname);

  Gtk::Button *saveprof = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *spl = Gtk::make_managed<Gtk::Label> ();
  spl->set_text (gettext ("Save"));
  spl->set_justify (Gtk::Justification::CENTER);
  saveprof->set_child (*spl);
  saveprof->set_margin (5);
  saveprof->set_halign (Gtk::Align::CENTER);
  saveprof->set_name ("applyButton");
  saveprof->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  saveprof->signal_clicked ().connect (
      sigc::mem_fun (*this, &MainWindow::saveProfile));
  rightgrid->attach (*saveprof, 0, 5, 1, 1);

  Gtk::Button *cleardata = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *cdl = Gtk::make_managed<Gtk::Label> ();
  cdl->set_text (gettext ("Clear"));
  cdl->set_justify (Gtk::Justification::CENTER);
  cleardata->set_child (*cdl);
  cleardata->set_margin (5);
  cleardata->set_halign (Gtk::Align::CENTER);
  cleardata->set_name ("clearButton");
  cleardata->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  cleardata->signal_clicked ().connect ( [key, nickname, name, surname]
  {
    key->set_text ("");
    nickname->set_text ("");
    name->set_text ("");
    surname->set_text ("");
  });
  rightgrid->attach (*cleardata, 1, 5, 1, 1);

  Gtk::Button *cancela = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *cal = Gtk::make_managed<Gtk::Label> ();
  cal->set_text (gettext ("Cancel"));
  cal->set_justify (Gtk::Justification::CENTER);
  cancela->set_child (*cal);
  cancela->set_margin (5);
  cancela->set_halign (Gtk::Align::CENTER);
  cancela->set_name ("rejectButton");
  cancela->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancela->signal_clicked ().connect ( [this]
  {
    std::string filename;
    AuxFunc af;
    af.homePath (&filename);
    filename = filename + "/.config/Communist";
    std::filesystem::path prefpath = std::filesystem::u8path (filename);
    std::filesystem::remove_all (prefpath);
    this->close ();
  });
  rightgrid->attach (*cancela, 2, 5, 1, 1);

  avatar = Gtk::make_managed<Gtk::DrawingArea> ();
  avatar->set_margin (5);
  avatar->set_halign (Gtk::Align::CENTER);
  avatar->set_size_request (200, 200);
  leftgrid->attach (*avatar, 0, 0, 2, 1);

  Gtk::Button *addavatar = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *oblab = Gtk::make_managed<Gtk::Label> ();
  oblab->set_text (gettext ("Choose image"));
  oblab->set_justify (Gtk::Justification::CENTER);
  addavatar->set_child (*oblab);
  addavatar->set_margin (5);
  addavatar->set_halign (Gtk::Align::CENTER);
  addavatar->set_name ("applyButton");
  addavatar->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  addavatar->signal_clicked ().connect (
      sigc::mem_fun (*this, &MainWindow::openImage));
  leftgrid->attach (*addavatar, 0, 1, 1, 1);

  Gtk::Button *cleardraw = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *oblab2 = Gtk::make_managed<Gtk::Label> ();
  oblab2->set_text (gettext ("Remove image"));
  oblab2->set_justify (Gtk::Justification::CENTER);
  cleardraw->set_child (*oblab2);
  cleardraw->set_margin (5);
  cleardraw->set_halign (Gtk::Align::CENTER);
  cleardraw->set_name ("clearButton");
  cleardraw->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  cleardraw->signal_clicked ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::clearAvatar), leftgrid));
  leftgrid->attach (*cleardraw, 1, 1, 1, 1);
}

void
MainWindow::keyGenerate (Gtk::Entry *key)
{
  seed = lt::dht::ed25519_create_seed ();
  lt::dht::public_key pk;
  lt::dht::secret_key sk;
  std::tie (pk, sk) = lt::dht::ed25519_create_keypair (seed);
  AuxFunc af;
  Glib::ustring keystr (lt::aux::to_hex (pk.bytes));
  key->set_text (keystr);
}

void
MainWindow::openImage ()
{
  Gtk::FileChooserDialog *dialog;
  dialog = new Gtk::FileChooserDialog (gettext ("Choose image file"),
				       Gtk::FileChooser::Action::OPEN);
  dialog->set_transient_for (*this);
  Gtk::Button *but;
  but = dialog->add_button (gettext ("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin (5);
  but = dialog->add_button (gettext ("OK"), Gtk::ResponseType::OK);
  but->set_margin (5);
  dialog->signal_response ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::MainWindow::onopenImage),
		  dialog));
  Glib::RefPtr<Gtk::Application> app = this->get_application ();
  app->add_window (*dialog);
  dialog->show ();
}

void
MainWindow::onopenImage (int respons_id, Gtk::FileChooserDialog *dialog)
{
  Glib::ustring file;
  if (respons_id == Gtk::ResponseType::OK)
    {
      file = dialog->get_file ()->get_path ();
      avatar->set_draw_func (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::on_draw), file));
    }
  dialog->hide ();
  delete dialog;
}

void
MainWindow::on_draw (const Cairo::RefPtr<Cairo::Context> &cr, int width,
		     int height, Glib::ustring file)
{
  image = Gdk::Pixbuf::create_from_file (file);
  image = image->scale_simple (width, height, Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf (cr, image, 0, 0);
  cr->rectangle (0, 0, width, height);
  cr->fill ();
  Image = 1;
}

void
MainWindow::on_draw_ep (const Cairo::RefPtr<Cairo::Context> &cr, int width,
			int height, Glib::ustring file)
{
  image = Gdk::Pixbuf::create_from_file (file);
  image = image->scale_simple (width, height, Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf (cr, image, 0, 0);
  cr->rectangle (0, 0, width, height);
  cr->fill ();
  Image = 1;
  std::string fnm (file);
  AuxFunc af;
  std::filesystem::path p = std::filesystem::u8path (fnm);
  std::filesystem::remove_all (p.parent_path ().parent_path ());
}

void
MainWindow::on_draw_sb (const Cairo::RefPtr<Cairo::Context> &cr, int width,
			int height, Glib::ustring file)
{
  auto imagesb = Gdk::Pixbuf::create_from_file (file);
  imagesb = imagesb->scale_simple (width, height, Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf (cr, imagesb, 0, 0);
  cr->rectangle (0, 0, width, height);
  cr->fill ();
}

void
MainWindow::on_draw_contactlist (const Cairo::RefPtr<Cairo::Context> &cr,
				 int width, int height, Gtk::Label *keylab)
{
  conavmtx.lock ();
  auto iter = std::find_if (conavatars.begin (), conavatars.end (), [keylab]
  (auto &el)
    {
      return keylab->get_text() == el.first;
    });
  Glib::RefPtr<Gdk::Pixbuf> imagec;
  if (iter != conavatars.end ())
    {
      imagec = (*iter).second;
      Gdk::Cairo::set_source_pixbuf (cr, imagec, 0, 0);
      cr->rectangle (0, 0, width, height);
      cr->fill ();
    }
  conavmtx.unlock ();
}

void
MainWindow::clearAvatar (Gtk::Grid *leftgrid)
{
  leftgrid->remove (*avatar);
  avatar = Gtk::make_managed<Gtk::DrawingArea> ();
  avatar->set_margin (5);
  avatar->set_halign (Gtk::Align::CENTER);
  avatar->set_size_request (200, 200);
  leftgrid->attach (*avatar, 0, 0, 2, 1);
  Image = 0;
}

void
MainWindow::saveProfile ()
{
  int notcompl = 0;
  for (size_t i = 0; i < profilevector.size (); i++)
    {
      Gtk::Entry *ent = profilevector[i];
      if (ent->get_text () == "")
	{
	  Gtk::Window *window = new Gtk::Window;
	  window->set_name ("settingsWindow");
	  window->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  window->set_title (gettext ("Error"));
	  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
	  window->set_child (*grid);

	  Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
	  label->set_text ("");
	  if (i == 0)
	    {
	      label->set_text (gettext ("Generate the key!"));
	    }
	  if (i == 1)
	    {
	      label->set_text (gettext ("Input nickname!"));
	    }

	  label->set_halign (Gtk::Align::CENTER);
	  label->set_margin (5);
	  grid->attach (*label, 0, 0, 1, 1);

	  Gtk::Button *button = Gtk::make_managed<Gtk::Button> ();
	  button->set_label (gettext ("Close"));
	  button->set_halign (Gtk::Align::CENTER);
	  button->set_margin (5);
	  button->set_name ("applyButton");
	  button->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  button->signal_clicked ().connect (
	      sigc::mem_fun (*window, &Gtk::Window::close));
	  grid->attach (*button, 0, 1, 1, 1);
	  window->signal_close_request ().connect ( [window]
	  {
	    window->hide ();
	    delete window;
	    return true;
	  },
						   false);
	  if (label->get_text () != "")
	    {
	      window->set_application (this->get_application ());
	      window->show ();
	      notcompl = 1;
	      break;
	    }
	  else
	    {
	      delete window;
	    }
	}
    }
  if (notcompl == 0)
    {
      std::string filename;
      AuxFunc af;
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist";
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
	  std::filesystem::remove_all (filepath);
	}
      filename = filename + "/Profile";
      filepath = std::filesystem::u8path (filename);
      std::filesystem::create_directories (filepath);
      filename = filename + "/Profile";
      filepath = std::filesystem::u8path (filename);
      std::fstream f;
      f.open (filepath, std::ios_base::out | std::ios_base::binary);
      std::string line;
      for (size_t i = 1; i < profilevector.size (); i++)
	{
	  Gtk::Entry *ent = profilevector[i];
	  line = std::string (ent->get_text ());
	  line = line + "\n";
	  f.write (line.c_str (), line.size ());
	}
      f.close ();
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Profile/Key";
      filepath = std::filesystem::u8path (filename);
      f.open (filepath, std::ios_base::out | std::ios_base::binary);
      f.write (seed.data (), seed.size ());
      f.close ();
      if (Image == 1)
	{
#ifdef __linux
	  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
          filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	  filename = filename + "/Communist/Profile/Avatar.jpeg";
	  filepath = std::filesystem::u8path (filename);
	  image->save (filepath.string (), "jpeg");
	  Image = 0;
	}
      std::string outfile;
#ifdef __linux
      outfile = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      outfile = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      outfile = outfile + "/Communist/Profile.zip";
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Profile";
      af.packing (filename, outfile);
      filepath = std::filesystem::u8path (filename);
      std::filesystem::remove_all (filepath);
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      af.cryptFile (Username, Password, outfile, filename);
      Deleteprof = 0;
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist";
      filepath = std::filesystem::u8path (filename);
      std::filesystem::remove_all (filepath);
      mainWindow ();
    }
}

void
MainWindow::ownKey ()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_transient_for (*this);
  window->set_name ("ownKeyWin");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title (gettext ("Key"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  window->set_child (*grid);

  Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
  label->set_selectable (true);
  std::tuple<lt::dht::public_key, lt::dht::secret_key> keypair;
  keypair = lt::dht::ed25519_create_keypair (seed);
  AuxFunc af;
  std::string key = lt::aux::to_hex (std::get<0> (keypair).bytes);
  label->set_text (Glib::ustring (key));
  label->set_margin (5);
  label->set_halign (Gtk::Align::CENTER);
  grid->attach (*label, 0, 0, 2, 1);

  Gtk::Button *copy = Gtk::make_managed<Gtk::Button> ();
  copy->set_label (gettext ("Copy"));
  copy->set_halign (Gtk::Align::CENTER);
  copy->set_margin (5);
  copy->set_name ("applyButton");
  copy->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Glib::RefPtr<Gdk::Display> display = window->get_display ();
  Glib::RefPtr<Gdk::Clipboard> clipboard = display->get_clipboard ();
  copy->signal_clicked ().connect ( [label, clipboard]
  {
    clipboard->set_text (label->get_text ());
  });
  grid->attach (*copy, 0, 1, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
  close->set_label (gettext ("Close"));
  close->set_halign (Gtk::Align::CENTER);
  close->set_margin (5);
  close->set_name ("rejectButton");
  close->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  close->signal_clicked ().connect (
      sigc::mem_fun (*window, &Gtk::Window::close));
  grid->attach (*close, 1, 1, 1, 1);
  window->signal_close_request ().connect ( [window]
  {
    window->hide ();
    delete window;
    return true;
  },
					   false);

  window->show ();
}

void
MainWindow::selectContact (Gtk::ScrolledWindow *scr, std::string selkey,
			   std::string selnick)
{
  if (Scrwinovershot.connected ())
    {
      Scrwinovershot.disconnect ();
    }
  usermsgadj = -1;
  msgwinadj = -1;
  Gtk::Requisition rq1, rq2;
  Rightgrid->get_preferred_size (rq1, rq2);
  scr->set_size_request (rq2.get_width (), -1);
  sentstatmtx.lock ();
  sentstatus.clear ();
  sentstatmtx.unlock ();
  msg_grid_vectmtx.lock ();
  msg_grid_vect.clear ();
  msg_grid_vectmtx.unlock ();
  if (selectedc != nullptr)
    {
      selectedc->set_name ("inactiveButton");
    }
  Gtk::Widget *widg = scr->get_child ();
  Winright = scr;
  if (widg != nullptr)
    {
      if (msgovllab != nullptr)
	{
	  widg = msgovllab->get_parent ();
	  if (widg != nullptr)
	    {
	      widg = widg->get_parent ();
	      if (widg != nullptr)
		{
		  msgovl->remove_overlay (*widg);
		  msgovllab = nullptr;
		}
	    }
	}
      scr->unset_child ();
    }
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  msg_win_gr = grid;
  scr->set_child (*grid);

  std::string filename;
  AuxFunc af;
  filename = selkey;
  frvectmtx.lock ();
  auto itfrv = std::find_if (friendvect.begin (), friendvect.end (), [selkey]
  (auto &el)
    {
      return std::get<2>(el)->get_text() == Glib::ustring(selkey);
    });
  if (itfrv != friendvect.end ())
    {
      Gtk::Button *button = std::get<0> (*itfrv);
      button->set_name ("activeButton");
      selectedc = button;
      Gtk::Label *l = std::get<3> (*itfrv);
      if (l != nullptr)
	{
	  Gtk::Grid *gr = std::get<1> (*itfrv);
	  gr->remove (*l);
	  std::get<3> (*itfrv) = nullptr;
	}
    }
  frvectmtx.unlock ();
  int index = -1;
  contmtx.lock ();
  auto iter = std::find_if (contacts.begin (), contacts.end (), [&filename]
  (auto &el)
    {
      return filename == std::get<1>(el);
    });
  if (iter != contacts.end ())
    {
      index = std::get<0> (*iter);
      std::string key = fordelkey;
      fordelkey = std::get<1> (contacts[index]);
      if (key != fordelkey)
	{
	  Glib::RefPtr<Gtk::TextBuffer> buf = msgfrm->get_buffer ();
	  buf->set_text ("");
	}
    }
  if (index >= 0)
    {
      af.homePath (&filename);
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << index;
      filename = filename + "/.Communist/" + strm.str ();
      std::filesystem::path filepath = std::filesystem::u8path (filename);

      std::vector<std::filesystem::path> msg;
      for (auto it : std::filesystem::directory_iterator (filepath))
	{
	  std::filesystem::path p = it.path ();
	  std::string line = p.filename ().u8string ();
	  if (line != "Profile" && line != "Yes")
	    {
	      msg.push_back (p);
	    }
	}
      if (msg.size () > 0)
	{

	  std::sort (msg.begin (), msg.end (), []
	  (auto &el1, auto &el2)
	    {
	      int one = std::stoi(el1.filename().u8string());
	      int two = std::stoi(el2.filename().u8string());
	      return one < two;
	    });

	  Scrwinovershot =
	      scr->signal_edge_reached ().connect (
		  [this, grid, selkey, selnick, index, scr]
		  (Gtk::PositionType pos)
		    {
		      std::vector<std::filesystem::path> msgl;
		      AuxFunc af;
		      std::string filename;
		      af.homePath (&filename);
		      std::stringstream strm;
		      std::locale loc ("C");
		      strm.imbue (loc);
		      strm << index;
		      filename = filename + "/.Communist/" + strm.str ();
		      std::filesystem::path filepath = std::filesystem::u8path (filename);
		      for (auto it : std::filesystem::directory_iterator (filepath))
			{
			  std::filesystem::path p = it.path ();
			  std::string line = p.filename ().u8string ();
			  if (line != "Profile" && line != "Yes")
			    {
			      msgl.push_back (p);
			    }
			}
		      std::sort (msgl.begin (), msgl.end (), []
			  (auto &el1, auto &el2)
			    {
			      int one = std::stoi(el1.filename().u8string());
			      int two = std::stoi(el2.filename().u8string());
			      return one > two;
			    });
		      if (pos == Gtk::PositionType::TOP)
			{
			  if (msgl.size() > 20)
			    {
			      this->msg_grid_vectmtx.lock();
			      std::vector<std::filesystem::path> tmp;
			      size_t end;
			      if (msgl.size() - this->msg_grid_vect.size() > 20)
				{
				  end = this->msg_grid_vect.size() + 20;
				}
			      else
				{
				  end = msgl.size();
				}
			      for(size_t i = this->msg_grid_vect.size(); i < end; i++)
				{
				  tmp.push_back(msgl[i]);
				}
			      this->msg_grid_vectmtx.unlock();
			      std::sort (tmp.begin (), tmp.end (), []
				  (auto &el1, auto &el2)
				    {
				      int one = std::stoi(el1.filename().u8string());
				      int two = std::stoi(el2.filename().u8string());

				      return one > two;
				    });
			      Glib::RefPtr<Gtk::Adjustment> adj;
			      adj = scr->get_vadjustment ();
			      this->msgwinadj = adj->get_upper();
			      formMsgWinGrid (tmp, 0, tmp.size (), grid, selkey, selnick, index, 1);
			    }
			}
		      if (pos == Gtk::PositionType::BOTTOM)
			{
			  this->msgwinadj = -1;
			  this->msg_grid_vectmtx.lock();
			  while(this->msg_grid_vect.size() > 20)
			    {
			      Gtk::Widget *widg = std::get<0>(this->msg_grid_vect[0]);
			      grid->remove(*widg);
			      this->msg_grid_vect.erase(this->msg_grid_vect.begin());
			    }
			  this->msg_grid_vectmtx.unlock();
			}
		    });
	  size_t begin;
	  if (msg.size () > 20)
	    {
	      begin = msg.size () - 20;
	    }
	  else
	    {
	      begin = 0;
	    }
	  formMsgWinGrid (msg, begin, msg.size (), grid, selkey, selnick, index,
			  0);
	}
    }
  contmtx.unlock ();
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default ();
  while (mc->pending ())
    {
      mc->iteration (true);
    }
  Glib::RefPtr<Gtk::Adjustment> adj;
  adj = scr->get_vadjustment ();
  adj->set_value (adj->get_upper () - adj->get_page_size ());
}

void
MainWindow::formMsgWinGrid (std::vector<std::filesystem::path> &msg,
			    size_t begin, size_t end, Gtk::Grid *grid,
			    std::string key, std::string nick, int index,
			    int varform)
{
  AuxFunc af;
  std::string filename;
  std::filesystem::path filepath;
  Gtk::Requisition rq1, rq2;
  std::stringstream strm;
  std::locale loc ("C");
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist";
  filepath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (filepath))
    {
      std::filesystem::remove_all (filepath);
    }
  std::filesystem::create_directories (filepath);
  std::string outfile = filepath.u8string ();
  outfile = outfile + "/Msg";
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkeypair =
      lt::dht::ed25519_create_keypair (seed);

  std::string uname;
  std::string passwd;
  lt::dht::public_key otherpk;
  std::string otherstr (key);
  lt::aux::from_hex (otherstr, otherpk.bytes.data ());
  std::array<char, 32> scalar = lt::dht::ed25519_key_exchange (
      otherpk, std::get<1> (ownkeypair));
  lt::dht::public_key pkpass;
  int count = 0;
  for (size_t i = begin; i < end; i++)
    {
      std::string line = msg[i].u8string ();
      pkpass = lt::dht::ed25519_add_scalar (std::get<0> (ownkeypair), scalar);
      uname = lt::aux::to_hex (std::get<0> (ownkeypair).bytes);
      passwd = lt::aux::to_hex (pkpass.bytes);
      af.decryptFile (uname, passwd, line, outfile);
      std::fstream f;
      std::string::size_type n = std::string::npos;
      std::filesystem::path tmpp = std::filesystem::u8path (outfile);
      f.open (tmpp, std::ios_base::in);
      while (!f.eof ())
	{
	  std::string gl;
	  getline (f, gl);
	  n = gl.find (otherstr, 0);
	  break;
	}
      f.close ();
      if (n == std::string::npos)
	{
	  pkpass = lt::dht::ed25519_add_scalar (otherpk, scalar);

	  af.decryptFile (otherstr, lt::aux::to_hex (pkpass.bytes), line,
			  tmpp.u8string ());
	}
      Gtk::Grid *grmsg = Gtk::make_managed<Gtk::Grid> ();
      Gtk::Frame *fr = Gtk::make_managed<Gtk::Frame> ();
      Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create ();
      clck->set_button (0);

      fr->set_child (*grmsg);
      fr->set_margin (5);
      fr->add_controller (clck);
      fr->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      Winright->get_preferred_size (rq1, rq2);
      fr->set_size_request (rq2.get_width () * 0.5, -1);

      Gtk::Label *date = Gtk::make_managed<Gtk::Label> ();
      date->set_halign (Gtk::Align::START);
      date->set_margin (2);
      grmsg->attach (*date, 0, 0, 1, 1);

      Gtk::Label *from = Gtk::make_managed<Gtk::Label> ();
      from->set_halign (Gtk::Align::START);
      from->set_margin (2);
      grmsg->attach (*from, 0, 1, 1, 1);

      Gtk::Frame *repfr = Gtk::make_managed<Gtk::Frame> ();
      repfr->set_halign (Gtk::Align::START);
      repfr->set_name ("repFrame");
      repfr->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      repfr->set_margin (2);
      Gtk::Label *repl = Gtk::make_managed<Gtk::Label> ();
      repl->set_halign (Gtk::Align::START);
      repl->set_margin (5);
      repl->set_use_markup (true);
      repl->set_max_width_chars (20);
      repl->set_ellipsize (Pango::EllipsizeMode::END);
      repfr->set_child (*repl);
      grmsg->attach (*repfr, 0, 2, 1, 1);

      int count2 = 0;
      int type;
      int own = 0;
      f.open (tmpp, std::ios_base::in);
      while (!f.eof ())
	{
	  std::string gl;
	  getline (f, gl);
	  n = std::string::npos;
	  if (gl != "")
	    {
	      if (count2 == 0)
		{
		  n = gl.find (" ", 0);
		  if (n != std::string::npos)
		    {
		      Glib::ustring txt (gl.substr (0, gl.find (" ")));
		      Glib::ustring trnstxt;
		      trnstxt = gettext ("Forwarded from: ");
		      txt = trnstxt + txt;
		      from->set_text (txt);
		      gl = gl.erase (0,
				     gl.find (" ") + std::string (" ").size ());
		      if (gl
			  == lt::aux::to_hex (std::get<0> (ownkeypair).bytes))
			{
			  own = 0;
			  std::string msgindstr =
			      msg[i].filename ().u8string ();
			  int msgind;
			  strm.str ("");
			  strm.clear ();
			  strm.imbue (loc);
			  strm << msgindstr;
			  strm >> msgind;
			  sentstatmtx.lock ();
			  sentstatus.push_back (
			      std::make_tuple (fordelkey, msgind, from));
			  sentstatmtx.unlock ();
			  strm.str ("");
			  strm.clear ();
			  strm.imbue (loc);
			  strm << index;
			  af.homePath (&filename);
			  filename = filename + "/.Communist/SendBufer/"
			      + strm.str () + "/" + msgindstr;
			  filepath = std::filesystem::u8path (filename);
			  if (oper != nullptr)
			    {
			      int check = 0;
			      oper->checkMsg (filepath, &check);
			      if (check > 0)
				{
				  from->set_name ("notSentMsg");
				}
			      else
				{
				  from->set_name ("SentMsg");
				}
			    }
			  else
			    {

			      if (std::filesystem::exists (filepath))
				{
				  from->set_name ("notSentMsg");
				}
			      else
				{
				  from->set_name ("SentMsg");
				}
			    }

			  from->get_style_context ()->add_provider (
			      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
			  fr->set_name ("myMsg");
			}
		      if (gl == std::string (key))
			{
			  own = 1;
			  fr->set_name ("frMsg");
			}
		    }
		  else
		    {
		      if (gl
			  == lt::aux::to_hex (std::get<0> (ownkeypair).bytes))
			{
			  own = 0;
			  from->set_text (gettext ("Me:"));
			  std::string msgindstr =
			      msg[i].filename ().u8string ();
			  int msgind;
			  strm.str ("");
			  strm.clear ();
			  strm.imbue (loc);
			  strm << msgindstr;
			  strm >> msgind;
			  sentstatmtx.lock ();
			  sentstatus.push_back (
			      std::make_tuple (fordelkey, msgind, from));
			  sentstatmtx.unlock ();
			  strm.str ("");
			  strm.clear ();
			  strm.imbue (loc);
			  strm << index;
			  af.homePath (&filename);
			  filename = filename + "/.Communist/SendBufer/"
			      + strm.str () + "/" + msgindstr;
			  filepath = std::filesystem::u8path (filename);
			  if (oper != nullptr)
			    {
			      int check = 0;
			      oper->checkMsg (filepath, &check);
			      if (check > 0)
				{
				  from->set_name ("notSentMsg");
				}
			      else
				{
				  from->set_name ("SentMsg");
				}
			    }
			  else
			    {

			      if (std::filesystem::exists (filepath))
				{
				  from->set_name ("notSentMsg");
				}
			      else
				{
				  from->set_name ("SentMsg");
				}
			    }

			  from->get_style_context ()->add_provider (
			      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
			  fr->set_name ("myMsg");
			}
		      if (gl == std::string (key))
			{
			  own = 1;
			  from->set_text (Glib::ustring (nick) + ":");
			  fr->set_name ("frMsg");
			}
		    }
		}
	      n = std::string::npos;
	      if (count2 == 2)
		{
		  time_t timet;
		  std::string day, month, year, hour, minut, second;
		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gl;
		  strm >> timet;
		  tm *gmtm = localtime (&timet);

		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gmtm->tm_mday;
		  day = strm.str ();
		  if (gmtm->tm_mday < 10)
		    {
		      day = "0" + day;
		    }

		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gmtm->tm_mon + 1;
		  month = strm.str ();
		  if (gmtm->tm_mon < 10)
		    {
		      month = "0" + month;
		    }

		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gmtm->tm_year + 1900;
		  year = strm.str ();

		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gmtm->tm_hour;
		  hour = strm.str ();
		  if (gmtm->tm_hour < 10)
		    {
		      hour = "0" + hour;
		    }

		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gmtm->tm_min;
		  minut = strm.str ();
		  if (gmtm->tm_min < 10)
		    {
		      minut = "0" + minut;
		    }

		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gmtm->tm_sec;
		  second = strm.str ();
		  if (gmtm->tm_sec < 10)
		    {
		      second = "0" + second;
		    }

		  Glib::ustring txt (
		      std::string (
			  day + "." + month + "." + year + " " + hour + ":"
			      + minut + ":" + second));
		  date->set_text (txt);
		}
	      if (count2 == 3)
		{
		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << gl;
		  strm >> type;
		}
	      if (count2 == 4)
		{
		  if (gl.size () > 2)
		    {
		      std::string line = gl;
		      line = line.substr (2, std::string::npos);
		      repl->set_markup (
			  "<span style=\"italic\">" + Glib::ustring (line)
			      + "</span>");
		    }
		  else
		    {
		      grmsg->remove_row (2);
		    }
		}
	      if (count2 > 4)
		{
		  Gtk::Widget *widg = grmsg->get_child_at (0, 3);
		  int rownum;
		  if (widg == nullptr)
		    {
		      rownum = 3;
		    }
		  else
		    {
		      rownum = 4;
		    }
		  if (type == 0 || type == 1)
		    {
		      Gtk::Label *message = Gtk::make_managed<Gtk::Label> ();
		      message->set_halign (Gtk::Align::START);
		      message->set_margin (5);
		      message->set_justify (Gtk::Justification::LEFT);
		      message->set_wrap (true);
		      message->set_wrap_mode (Pango::WrapMode::WORD);
		      message->set_text (Glib::ustring (gl));
		      message->set_margin_bottom (10);
		      message->set_selectable (true);
		      grmsg->attach (*message, 0, rownum, 1, 1);
		      clck->signal_pressed ().connect (
			  sigc::bind (
			      sigc::mem_fun (*this, &MainWindow::creatReply),
			      fr, message, clck));
		    }
		}
	    }
	  count2++;
	}
      f.close ();
      if (own == 0)
	{
	  fr->set_halign (Gtk::Align::START);
	  if (varform == 0)
	    {
	      grid->attach (*fr, 0, count, 1, 1);
	    }
	  if (varform == 1)
	    {
	      grid->insert_row (0);
	      grid->attach (*fr, 0, 0, 1, 1);
	    }
	}
      else
	{
	  fr->set_halign (Gtk::Align::END);
	  if (varform == 0)
	    {
	      grid->attach (*fr, 1, count, 1, 1);
	    }
	  if (varform == 1)
	    {
	      grid->insert_row (0);
	      grid->attach (*fr, 1, 0, 1, 1);
	    }
	}
      msg_grid_vectmtx.lock ();
      if (varform == 0)
	{
	  msg_grid_vect.push_back (std::make_tuple (fr, msg[i]));
	}
      if (varform == 1)
	{
	  msg_grid_vect.insert (msg_grid_vect.begin (),
				std::make_tuple (fr, msg[i]));
	}
      msg_grid_vectmtx.unlock ();
      count++;
    }
}

void
MainWindow::deleteContact ()
{
  if (contmenupop != nullptr)
    {
      contmenupop->unparent ();
      contmenupop = nullptr;
    }
  if (fordelkey != "")
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application (this->get_application ());
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      window->set_title (gettext ("Remove contact"));
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      window->set_child (*grid);

      Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
      label->set_text (
	  gettext (
	      "Remove contact? This action will erase all messages forever."));
      label->set_max_width_chars (40);
      label->set_justify (Gtk::Justification::CENTER);
      label->set_wrap (true);
      label->set_wrap_mode (Pango::WrapMode::WORD);
      label->set_margin (5);
      grid->attach (*label, 0, 0, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
      yes->set_label (gettext ("Yes"));
      yes->set_margin (5);
      yes->set_halign (Gtk::Align::CENTER);
      yes->set_name ("clearButton");
      yes->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      yes->signal_clicked ().connect (
	  sigc::mem_fun (*this, &MainWindow::deleteContactFunc));
      yes->signal_clicked ().connect (
	  sigc::mem_fun (*window, &Gtk::Window::close));
      grid->attach (*yes, 0, 1, 1, 1);

      Gtk::Button *no = Gtk::make_managed<Gtk::Button> ();
      no->set_label (gettext ("No"));
      no->set_margin (5);
      no->set_halign (Gtk::Align::CENTER);
      no->set_name ("rejectButton");
      no->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      no->signal_clicked ().connect (
	  sigc::mem_fun (*window, &Gtk::Window::close));
      grid->attach (*no, 1, 1, 1, 1);

      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size (rq1, rq2);
      window->set_size_request (rq2.get_width (), rq2.get_height ());

      window->signal_close_request ().connect ( [window]
      {
	window->hide ();
	delete window;
	return true;
      },
					       false);

      window->show ();
    }
}

void
MainWindow::deleteContactFunc ()
{
  if (fordelgrid != nullptr)
    {
      if (selectedc != nullptr)
	{
	  if (fordelkey != "")
	    {
	      Gtk::Widget *widg = Winright->get_child ();
	      if (widg != nullptr)
		{
		  Winright->unset_child ();
		}

	      frvectmtx.lock ();
	      friendvect.erase (
		  std::remove_if (friendvect.begin (), friendvect.end (), [this]
		  (auto &el)
		    {
		      if (std::get<0>(el) == this->selectedc)
			{
			  fordelgrid->remove (*(this->selectedc));
			  this->selectedc = nullptr;
			  return true;
			}
		      else
			{
			  return false;
			}

		    }),
		  friendvect.end ());
	      frvectmtx.unlock ();

	      conavmtx.lock ();
	      if (conavatars.size () > 0)
		{
		  conavatars.erase (
		      std::remove_if (
			  conavatars.begin (),
			  conavatars.end (),
			  [this]
			  (auto &el)
			    { return el.first == Glib::ustring(this->fordelkey);}));
		}
	      conavmtx.unlock ();
	      fileprogrvectmtx.lock ();
	      fileprogrvect.erase (
		  std::remove_if (
		      fileprogrvect.begin (), fileprogrvect.end (), [this]
		      (auto &el)
			{
			  if (std::get<0>(el) == this->fordelkey)
			    {
			      this->dld_grid->remove(*(std::get<3>(el)));
			      this->dld_grid->remove(*(std::get<4>(el)));
			      this->dld_grid->remove(*(std::get<5>(el)));
			      return true;
			    }
			  else
			    {
			      return false;
			    }
			}),
		  fileprogrvect.end ());
	      if (fileprogrvect.size () == 0 && dld_win != nullptr)
		{
		  dld_win->hide ();
		  delete dld_win;
		  dld_win = nullptr;
		  dld_grid = nullptr;
		}
	      fileprogrvectmtx.unlock ();
	      if (oper != nullptr)
		{
		  oper->removeFriend (fordelkey);
		}
	      selectedc = nullptr;
	      fordelkey = "";
	      Winright = nullptr;
	    }
	}
    }
}

void
MainWindow::networkOp ()
{
  if (oper == nullptr)
    {
      contmtx.lock ();
      addfrmtx.lock ();
      prefvectmtx.lock ();
      NetworkOperations *op = new NetworkOperations (Username, Password,
						     &contacts, &seed,
						     &Addfriends, &prefvect,
						     Sharepath);
      oper = op;
      prefvectmtx.unlock ();
      addfrmtx.unlock ();
      contmtx.unlock ();
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
      int *indsm = new int;

      std::string *keyrm = new std::string;
      std::filesystem::path *rp = new std::filesystem::path;

      std::string *keyfr = new std::string;
      uint64_t *tm = new uint64_t;
      uint64_t *fs = new uint64_t;
      std::string *fn = new std::string;

      std::string *keyfrr = new std::string;

      std::string *keyfiler = new std::string;
      std::string *filenmr = new std::string;

      std::string *keyfilenr = new std::string;
      std::string *filenmnr = new std::string;

      std::string *keyfiles = new std::string;
      std::string *filenms = new std::string;

      std::string *keyfileerror = new std::string;
      std::string *filenmerror = new std::string;

      std::string *keyfileprg = new std::string;
      std::filesystem::path *fileprgp = new std::filesystem::path;
      uint64_t *fileprgsz = new uint64_t;

      std::string *keyfileprgs = new std::string;
      std::filesystem::path *fileprgps = new std::filesystem::path;
      uint64_t *fileprgszs = new uint64_t;

      std::string *keychcon = new std::string;
      uint64_t *tmchcon = new uint64_t;

      std::string *keyfrrem = new std::string;

      op->profReceived.connect ( [this, key, ind, disp1mtx]
      (std::string keyt, int indt)
	{
	  disp1mtx->lock();
	  *key = keyt;
	  *ind = indt;
	  this->disp1.emit ();
	});

      op->msgSent.connect ( [this, keysm, indsm, disp2mtx]
      (std::string key, int ind)
	{
	  disp2mtx->lock();
	  *keysm = key;
	  *indsm = ind;
	  this->disp2.emit();
	});

      op->messageReceived.connect ( [this, keyrm, rp, disp3mtx]
      (std::string key, std::filesystem::path p)
	{
	  disp3mtx->lock();
	  *keyrm = key;
	  *rp = p;
	  this->disp3.emit();
	});

      op->filerequest.connect ( [this, keyfr, tm, fs, fn, disp4mtx]
      (std::string key, uint64_t time, uint64_t filesize, std::string filename)
	{
	  disp4mtx->lock();
	  *keyfr = key;
	  *tm = time;
	  *fs = filesize;
	  *fn = filename;
	  this->disp4.emit();
	});
      op->fileRejected.connect ( [this, keyfrr, disp5mtx]
      (std::string key)
	{
	  disp5mtx->lock();
	  *keyfrr = key;
	  this->disp5.emit();
	});

      op->filercvd.connect ( [this, disp6mtx, keyfiler, filenmr]
      (std::string key, std::string filename)
	{
	  disp6mtx->lock();
	  *keyfiler = key;
	  *filenmr = filename;
	  this->disp6.emit();
	});

      op->filehasherr.connect ( [this, disp7mtx, keyfilenr, filenmnr]
      (std::string key, std::string filename)
	{
	  disp7mtx->lock();
	  *keyfilenr = key;
	  *filenmnr = filename;
	  this->disp7.emit();
	});

      op->filesentsig.connect ( [this, disp8mtx, keyfiles, filenms]
      (std::string key, std::string filename)
	{
	  disp8mtx->lock();
	  *keyfiles = key;
	  *filenms = filename;
	  this->disp8.emit();
	});

      op->filesenterror.connect ( [this, disp9mtx, keyfileerror, filenmerror]
      (std::string key, std::string filename)
	{
	  disp9mtx->lock();
	  *keyfileerror = key;
	  *filenmerror = filename;
	  this->disp9.emit();
	});

      op->ipv6signal.connect (sigc::mem_fun (*this, &MainWindow::formIPv6vect));
      op->ipv4signal.connect (sigc::mem_fun (*this, &MainWindow::formIPv4vect));

      op->ipv6signalfinished.connect ( [this]
      {
	this->disp10.emit ();
      });
      op->ipv4signalfinished.connect ( [this]
      {
	this->disp11.emit ();
      });
      op->filepartrcvdsig.connect (
	  [this, disp12mtx, keyfileprg, fileprgp, fileprgsz]
	  (std::string key, std::filesystem::path p, uint64_t sz)
	    {
	      disp12mtx->lock();
	      *keyfileprg = key;
	      *fileprgp = p;
	      *fileprgsz = sz;
	      this->disp12.emit();
	    });
      op->filepartsendsig.connect (
	  [this, disp13mtx, keyfileprgs, fileprgps, fileprgszs]
	  (std::string key, std::filesystem::path p, uint64_t sz)
	    {
	      disp13mtx->lock();
	      *keyfileprgs = key;
	      *fileprgps = p;
	      *fileprgszs = sz;
	      this->disp13.emit();
	    });
      op->smthrcvdsig.connect ( [this, disp14mtx, keychcon, tmchcon]
      (std::string key, uint64_t tm)
	{
	  disp14mtx->lock();
	  *keychcon = key;
	  *tmchcon = tm;
	  this->disp14.emit();
	});
      op->friendDeleted.connect ( [this, disp15mtx, keyfrrem]
      (std::string key)
	{
	  disp15mtx->lock();
	  *keyfrrem = key;
	  this->disp15.emit ();
	});

      std::vector<sigc::connection> dispv;
      sigc::connection con;
      con = disp1.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::addFriendSlot), key,
		      ind, disp1mtx));
      dispv.push_back (con);
      con = disp2.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::msgSentSlot), keysm,
		      indsm, disp2mtx));
      dispv.push_back (con);
      con = disp3.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::msgRcvdSlot), keyrm,
		      rp, disp3mtx));
      dispv.push_back (con);
      con = disp4.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileRequestSlot),
		      keyfr, tm, fs, fn, disp4mtx));
      dispv.push_back (con);
      con = disp5.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileRejectedSlot),
		      keyfrr, disp5mtx));
      dispv.push_back (con);
      con = disp6.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileRcvdStatus),
		      keyfiler, filenmr, disp6mtx, 6));
      dispv.push_back (con);
      con = disp7.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileRcvdStatus),
		      keyfilenr, filenmnr, disp7mtx, 7));
      dispv.push_back (con);
      con = disp8.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileRcvdStatus),
		      keyfiles, filenms, disp8mtx, 8));
      dispv.push_back (con);
      con = disp9.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileRcvdStatus),
		      keyfileerror, filenmerror, disp9mtx, 9));
      dispv.push_back (con);
      con = disp10.connect (sigc::mem_fun (*this, &MainWindow::ipv6Window));
      dispv.push_back (con);
      con = disp11.connect (sigc::mem_fun (*this, &MainWindow::ipv4Window));
      dispv.push_back (con);
      con = disp12.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileDownloadProg),
		      keyfileprg, fileprgp, fileprgsz, disp12mtx));
      dispv.push_back (con);
      con = disp13.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::fileSendProg),
		      keyfileprgs, fileprgps, fileprgszs, disp13mtx));
      dispv.push_back (con);
      con = disp14.connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::checkIfConnected),
		      keychcon, tmchcon, disp14mtx));
      dispv.push_back (con);
      con = disp15.connect ( [this, keyfrrem, disp15mtx]
      {
	this->friendRemoved (keyfrrem, disp15mtx);
      });
      dispv.push_back (con);

      op->mainFunc ();

      op->canceled.connect (
	  [this, dispv, disp1mtx, disp2mtx, disp3mtx, disp4mtx, disp5mtx,
	   disp6mtx, disp7mtx, disp8mtx, disp9mtx, disp12mtx, disp13mtx,
	   disp14mtx, disp15mtx, key, ind, keysm, indsm, keyrm, rp, keyfr, tm,
	   fs, fn, keyfrr, keyfiler, filenmr, keyfilenr, filenmnr, keyfiles,
	   filenms, keyfileerror, filenmerror, keyfileprg, fileprgp, fileprgsz,
	   keyfileprgs, fileprgps, fileprgszs, keychcon, tmchcon, keyfrrem]
	  {
	    for (size_t i = 0; i < dispv.size (); i++)
	      {
		sigc::connection con = dispv[i];
		con.disconnect ();
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
	    delete this->oper;
	    this->oper = nullptr;
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
	  });
    }
}

void
MainWindow::addFriends ()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title (gettext ("Add contact"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  window->set_child (*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
  lab->set_text (gettext ("Public key:"));
  lab->set_halign (Gtk::Align::START);
  lab->set_margin (5);
  grid->attach (*lab, 0, 0, 2, 1);

  Gtk::Entry *otherkey = Gtk::make_managed<Gtk::Entry> ();
  otherkey->set_margin (5);
  otherkey->set_halign (Gtk::Align::CENTER);
  otherkey->set_width_chars (64);
  grid->attach (*otherkey, 0, 1, 2, 1);

  Gtk::Button *accept = Gtk::make_managed<Gtk::Button> ();
  accept->set_label (gettext ("Add"));
  accept->set_halign (Gtk::Align::CENTER);
  accept->set_margin (5);
  accept->set_name ("applyButton");
  accept->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  accept->signal_clicked ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::addFriendsFunc), window,
		  otherkey));
  grid->attach (*accept, 0, 2, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
  cancel->set_label (gettext ("Cancel"));
  cancel->set_halign (Gtk::Align::CENTER);
  cancel->set_margin (5);
  cancel->set_name ("rejectButton");
  cancel->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->signal_clicked ().connect (
      sigc::mem_fun (*window, &Gtk::Window::close));
  grid->attach (*cancel, 1, 2, 1, 1);

  window->signal_close_request ().connect ( [window]
  {
    window->hide ();
    delete window;
    return true;
  },
					   false);

  window->show ();
}

void
MainWindow::addFriendsFunc (Gtk::Window *window, Gtk::Entry *entry)
{
  AuxFunc af;
  std::string key (entry->get_text ());
  for (;;)
    {
#ifdef __linux
      std::string tmpstr = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      std::string tmpstr = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      tmpstr = tmpstr + "/CommunistNet";
      std::filesystem::path p (std::filesystem::u8path (tmpstr));
      if (!std::filesystem::exists (p))
	{
	  break;
	}
      usleep (100);
    }
  window->close ();
  if (key.size () == 64)
    {
      std::string filename;
      std::filesystem::path filepath;
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/SendRequest";
      filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
	  std::filesystem::remove_all (filepath);
	}
      std::filesystem::create_directories (filepath);
      filename = filepath.u8string ();
      filename = filename + "/Profile.zip";
      filepath = std::filesystem::u8path (filename);
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      std::filesystem::path path (std::filesystem::u8path (filename));
      proffopmtx.lock ();
      af.decryptFile (Username, Password, path.u8string (),
		      filepath.u8string ());
      af.unpacking (filepath.u8string (), filepath.parent_path ().u8string ());
      std::filesystem::remove (filepath);
      filename = filepath.parent_path ().u8string ();
      filename = filename + "/Profile/RequestList";
      filepath = std::filesystem::u8path (filename);
      std::fstream f;
      std::vector<std::string> req;
      std::string line;

      if (std::filesystem::exists (filepath))
	{
	  f.open (filepath, std::ios_base::in);
	  while (!f.eof ())
	    {
	      getline (f, line);
	      if (line != "")
		{
		  req.push_back (line);
		}
	    }
	  f.close ();
	}

      line = key;

      line = line + "\n";
      addfrmtx.lock ();
      auto it = std::find (Addfriends.begin (), Addfriends.end (), key);
      if (it == Addfriends.end ())
	{
	  Addfriends.push_back (key);
	  oper->getNewFriends (key);
	}
      addfrmtx.unlock ();
      auto iter = std::find_if (req.begin (), req.end (), [&key]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find (key, 0);
	  if (n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
      if (iter == req.end ())
	{
	  req.push_back (line);
	}
      f.open (filepath, std::ios_base::out | std::ios_base::binary);
      for (size_t i = 0; i < req.size (); i++)
	{
	  line = req[i];
	  f.write (line.c_str (), line.size ());
	}
      f.close ();
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/SendRequest/Profile.zip";
      filepath = std::filesystem::u8path (filename);
      filepath = filepath.replace_extension ();
      path = std::filesystem::u8path (filename);
      af.packing (filepath.u8string (), path.u8string ());
      filename = filepath.u8string ();
      filename = filename + ".zip";
      filepath = std::filesystem::u8path (filename);
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      path = std::filesystem::u8path (filename);
      af.cryptFile (Username, Password, filepath.u8string (), path.u8string ());
      filepath = filepath.parent_path ().parent_path ();
      std::filesystem::remove_all (filepath);
      proffopmtx.unlock ();
    }
}

void
MainWindow::editAddFriends ()
{
  AuxFunc af;
  std::string filename;
  af.homePath (&filename);
  filename = filename + "/.Communist/Profile";
  std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist/Profile.zip";
  std::filesystem::path outfile = std::filesystem::u8path (filename);
  if (!std::filesystem::exists (outfile.parent_path ()))
    {
      std::filesystem::create_directories (outfile.parent_path ());
    }
  else
    {
      std::filesystem::remove_all (outfile.parent_path ());
      std::filesystem::create_directories (outfile.parent_path ());
    }
  proffopmtx.lock ();
  af.decryptFile (Username, Password, source.u8string (), outfile.u8string ());
  af.unpacking (outfile.u8string (), outfile.parent_path ().u8string ());
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/Communist/Profile/RequestList";
  source = std::filesystem::u8path (filename);
  std::vector<std::string> req;
  if (std::filesystem::exists (source))
    {
      std::fstream f;
      f.open (source, std::ios_base::in);
      std::string line;
      while (!f.eof ())
	{
	  getline (f, line);
	  if (line != "")
	    {
	      if (line.size () > 64)
		{
		  for (;;)
		    {
		      line.pop_back ();
		      if (line.size () == 64)
			{
			  break;
			}
		    }
		  req.push_back (line);
		}
	      else
		{
		  req.push_back (line);
		}
	    }
	}
      f.close ();
    }
  std::filesystem::remove_all (source.parent_path ().parent_path ());
  proffopmtx.unlock ();
  Gtk::Window *window = new Gtk::Window;
  window->set_title (gettext ("Friends request edit"));
  window->set_application (this->get_application ());
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  window->set_child (*grid);

  Gtk::ScrolledWindow *scrll = Gtk::make_managed<Gtk::ScrolledWindow> ();
  scrll->set_policy (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
  scrll->set_min_content_height (200);
  Gtk::Label l;
  std::array<char, 32> seed;
  seed = lt::dht::ed25519_create_seed ();
  std::string txt = lt::aux::to_hex (seed);
  l.set_text (Glib::ustring (txt));
  Gtk::Requisition rq1, rq2;
  l.get_preferred_size (rq1, rq2);
  Gtk::Grid *contgr = Gtk::make_managed<Gtk::Grid> ();
  scrll->set_child (*contgr);
  scrll->set_size_request (rq2.get_width (), -1);
  grid->attach (*scrll, 0, 0, 2, 1);

  std::vector<Gtk::Label*> *labvect = new std::vector<Gtk::Label*>;
  for (size_t i = 0; i < req.size (); i++)
    {

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_text (Glib::ustring (req[i]));
      lab->set_margin (5);
      lab->set_name ("blklistInactive");
      lab->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create ();
      clck->set_button (0);
      clck->signal_pressed ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::addFriendsSelected),
		      labvect, lab, clck, contgr));
      lab->add_controller (clck);
      contgr->attach (*lab, 0, i, 1, 1);
      labvect->push_back (lab);
    }

  Gtk::Button *remove = Gtk::make_managed<Gtk::Button> ();
  remove->set_label (gettext ("Remove from list"));
  remove->set_halign (Gtk::Align::CENTER);
  remove->set_margin (5);
  remove->set_name ("clearButton");
  remove->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  remove->signal_clicked ().connect (
      [this, contgr, &af, labvect]
      {
	if (activereq != nullptr)
	  {
	    std::string filename;
	    af.homePath (&filename);
	    filename = filename + "/.Communist/Profile";
	    std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
	filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
        filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	filename = filename + "/Communist/Profile.zip";
	std::filesystem::path outpath = std::filesystem::u8path (filename);
	if (!std::filesystem::exists (outpath.parent_path ()))
	  {
	    std::filesystem::create_directories (outpath.parent_path ());
	  }
	else
	  {
	    std::filesystem::remove_all (outpath.parent_path ());
	    std::filesystem::create_directories (outpath.parent_path ());
	  }

	this->proffopmtx.lock ();
	af.decryptFile (this->Username, this->Password, source.u8string (),
			outpath.u8string ());
	af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
	std::filesystem::remove (outpath);
#ifdef __linux
	filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
        filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	filename = filename + "/Communist/Profile/RequestList";
	source = std::filesystem::u8path (filename);
	if (std::filesystem::exists (source))
	  {
	    std::vector<std::string> req;
	    std::fstream f;
	    std::string line;
	    f.open (source, std::ios_base::in);
	    while (!f.eof ())
	      {
		getline (f, line);
		if (line != "")
		  {
		    req.push_back (line);
		  }
	      }
	    f.close ();
	    req.erase (std::remove_if (req.begin (), req.end (), [this]
	    (auto &el)
	      {
		std::string key (this->activereq->get_text());
		std::string::size_type n;
		n = el.find (key, 0);
		if (n != std::string::npos)
		  {
		    return true;
		  }
		else
		  {
		    return false;
		  }
	      }),
		       req.end ());
	    f.open (source, std::ios_base::out | std::ios_base::binary);
	    for (size_t j = 0; j < req.size (); j++)
	      {
		line = req[j] + "\n";
		f.write (line.c_str (), line.size ());
	      }
	    f.close ();
	    std::string key (this->activereq->get_text ());
	    if (this->oper != nullptr)
	      {
		this->oper->removeFriend (key);
	      }
	    labvect->erase (
		std::remove_if (labvect->begin (), labvect->end (), [this]
		(auto &el)
		  { return this->activereq == el;}),
		labvect->end ());
	    this->addfrmtx.lock ();
	    this->Addfriends.erase (
		std::remove (this->Addfriends.begin (), this->Addfriends.end (),
			     key),
		this->Addfriends.end ());
	    this->addfrmtx.unlock ();
	    contgr->remove (*this->activereq);
	    filename = source.parent_path ().u8string ();
	    source = std::filesystem::u8path (filename);
	    af.packing (source.u8string (), outpath.u8string ());
	    source = outpath;
	    af.homePath (&filename);
	    filename = filename + "/.Communist/Profile";
	    outpath = std::filesystem::u8path (filename);
	    af.cryptFile (this->Username, this->Password, source.u8string (),
			  outpath.u8string ());
	    std::filesystem::remove_all (source.parent_path ());
	    this->activereq = nullptr;
	  }
	this->proffopmtx.unlock ();
      }
  }   );
  grid->attach (*remove, 0, 1, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
  cancel->set_label (gettext ("Close"));
  cancel->set_halign (Gtk::Align::CENTER);
  cancel->set_margin (5);
  cancel->set_name ("rejectButton");
  cancel->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->signal_clicked ().connect (
      sigc::mem_fun (*window, &Gtk::Window::close));
  grid->attach (*cancel, 1, 1, 1, 1);

  window->signal_close_request ().connect ( [window, labvect]
  {
    window->hide ();
    delete window;
    delete labvect;
    return true;
  },
					   false);

  window->show ();
}

void
MainWindow::addFriendsSelected (int n_press, double x, double y,
				std::vector<Gtk::Label*> *labvect,
				Gtk::Label *lab,
				Glib::RefPtr<Gtk::GestureClick> clck,
				Gtk::Grid *contgr)
{
  std::string filename;
  std::filesystem::path filepath;
  AuxFunc af;
  if (clck->get_current_button () == 1)
    {
      for (size_t i = 0; i < labvect->size (); i++)
	{
	  labvect->at (i)->set_name ("blklistInactive");
	}
      lab->set_name ("blklistActive");
      activereq = lab;
    }
  if (clck->get_current_button () == 3)
    {
      for (size_t i = 0; i < labvect->size (); i++)
	{
	  labvect->at (i)->set_name ("blklistInactive");
	}
      lab->set_name ("blklistActive");
      Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create ();
      menu->append (gettext ("Remove from list"), "popup.remove");
      Gtk::PopoverMenu *Menu = Gtk::make_managed<Gtk::PopoverMenu> ();
      Menu->set_parent (*lab);
      Menu->set_menu_model (menu);
      Menu->set_has_arrow (false);
      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
	  Gio::SimpleActionGroup::create ();
      acgroup->add_action ("remove", [&af, contgr, labvect, lab, this]
      {
	std::string filename;
	af.homePath (&filename);
	filename = filename + "/.Communist/Profile";
	std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
			     filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	filename = filename + "/Communist/Profile.zip";
	std::filesystem::path outpath = std::filesystem::u8path (filename);
	if (!std::filesystem::exists (outpath.parent_path ()))
	  {
	    std::filesystem::create_directories (outpath.parent_path ());
	  }
	else
	  {
	    std::filesystem::remove_all (outpath.parent_path ());
	    std::filesystem::create_directories (outpath.parent_path ());
	  }
	proffopmtx.lock ();
	af.decryptFile (this->Username, this->Password, source.u8string (),
			outpath.u8string ());
	af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
	std::filesystem::remove (outpath);
#ifdef __linux
	filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
        filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	filename = filename + "/Communist/Profile/RequestList";
	source = std::filesystem::u8path (filename);
	if (std::filesystem::exists (source))
	  {
	    std::vector<std::string> req;
	    std::fstream f;
	    std::string line;
	    f.open (source, std::ios_base::in);
	    while (!f.eof ())
	      {
		getline (f, line);
		if (line != "")
		  {
		    req.push_back (line);
		  }
	      }
	    f.close ();
	    req.erase (std::remove_if (req.begin (), req.end (), [lab]
	    (auto &el)
	      {
		std::string key (lab->get_text());
		std::string::size_type n;
		n = el.find (key, 0);
		if (n != std::string::npos)
		  {
		    return true;
		  }
		else
		  {
		    return false;
		  }
	      }),
		       req.end ());
	    f.open (source, std::ios_base::out | std::ios_base::binary);
	    for (size_t j = 0; j < req.size (); j++)
	      {
		line = req[j] + "\n";
		f.write (line.c_str (), line.size ());
	      }
	    f.close ();
	    std::string key (this->activereq->get_text ());
	    if (this->oper != nullptr)
	      {
		this->oper->removeFriend (key);
	      }
	    labvect->erase (
		std::remove_if (labvect->begin (), labvect->end (), [lab]
		(auto &el)
		  { return lab == el;}),
		labvect->end ());
	    this->addfrmtx.lock ();
	    this->Addfriends.erase (
		std::remove (this->Addfriends.begin (), this->Addfriends.end (),
			     key),
		this->Addfriends.end ());
	    this->addfrmtx.unlock ();

	    contgr->remove (*lab);
	    this->activereq = nullptr;
	    filename = source.parent_path ().u8string ();
	    source = std::filesystem::u8path (filename);
	    af.packing (source.u8string (), outpath.u8string ());
	    source = outpath;
	    af.homePath (&filename);
	    filename = filename + "/.Communist/Profile";
	    outpath = std::filesystem::u8path (filename);
	    af.cryptFile (Username, Password, source.u8string (),
			  outpath.u8string ());
	    std::filesystem::remove_all (source.parent_path ());
	  }
	proffopmtx.unlock ();
      }
      );
      lab->insert_action_group ("popup", acgroup);
      const Gdk::Rectangle rect (x, y, 1, 1);
      Menu->set_pointing_to (rect);
      Menu->popup ();
    }
}

void
MainWindow::addFriendSlot (std::string *keyt, int *conind, std::mutex *disp1mtx)
{
  std::string key = *keyt;
  while (key.size () > 64)
    {
      key.pop_back ();
    }
  int conindloc = *conind;
  disp1mtx->unlock ();
  addfrmtx.lock ();
  Addfriends.erase (
      std::remove_if (Addfriends.begin (), Addfriends.end (), [&key]
      (auto &el)
	{ return el == key;}),
      Addfriends.end ());
  addfrmtx.unlock ();

  contmtx.lock ();
  auto conit = std::find_if (contacts.begin (), contacts.end (), [&key]
  (auto &el)
    {
      return std::get<1>(el) == key;});
  if (conit == contacts.end ())
    {
      contacts.push_back (std::make_tuple (conindloc, key));
    }
  contmtx.unlock ();

  conavmtx.lock ();
  conavatars.erase (
      std::remove_if (conavatars.begin (), conavatars.end (), [&key]
      (auto &el)
	{ return std::string(el.first) == key;}),
      conavatars.end ());
  conavmtx.unlock ();

  chifcmtx.lock ();
  frvectmtx.lock ();
  int chbutdel = 0;
  auto it = std::find_if (friendvect.begin (), friendvect.end (), [&key]
  (auto &el)
    { return std::get<2>(el)->get_text() == Glib::ustring(key);});
  if (it != friendvect.end ())
    {
      Gtk::Widget *widg = std::get<0> (*it);
      friendvect.erase (it);
      if (selectedc == widg)
	{
	  selectedc = nullptr;
	  chbutdel = 1;
	}
      fordelgrid->remove (*widg);
    }
  chifc.erase (std::remove_if (chifc.begin (), chifc.end (), [&key]
  (auto &el)
    {
      return std::get<0>(el) == key;
    }),
	       chifc.end ());
  frvectmtx.unlock ();
  chifcmtx.unlock ();
  contmtx.lock ();
  auto itcont = std::find_if (contacts.begin (), contacts.end (), [&key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itcont != contacts.end ())
    {
      AuxFunc af;
      std::string filename;
      std::filesystem::path source;
      std::filesystem::path outpath;
      af.homePath (&filename);
      filename = filename + "/.Communist/" + std::to_string (conindloc)
	  + "/Profile";
      source = std::filesystem::u8path (filename);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Prof";
      outpath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (outpath))
	{
	  std::filesystem::remove_all (outpath);
	  std::filesystem::create_directories (outpath);
	}
      else
	{
	  std::filesystem::create_directories (outpath);
	}
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Prof/ProfileOp.zip";
      outpath = std::filesystem::u8path (filename);
      std::string uname;
      std::string passwd;
      std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkeypair;
      ownkeypair = lt::dht::ed25519_create_keypair (seed);
      std::array<char, 32> scalar;
      uname = lt::aux::to_hex (std::get<0> (ownkeypair).bytes);
      lt::dht::public_key otherpk;
      std::string otherpkstr = std::get<1> (*itcont);
      lt::aux::from_hex (otherpkstr, otherpk.bytes.data ());
      scalar = lt::dht::ed25519_key_exchange (otherpk,
					      std::get<1> (ownkeypair));
      lt::dht::public_key pkpass = lt::dht::ed25519_add_scalar (
	  std::get<0> (ownkeypair), scalar);

      passwd = lt::aux::to_hex (pkpass.bytes);
      proffopmtx.lock ();
      af.decryptFile (uname, passwd, source.u8string (), outpath.u8string ());
      af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());

      Gtk::Button *buttontr = Gtk::make_managed<Gtk::Button> ();
      buttontr->get_style_context ()->add_provider (
	  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      buttontr->set_name ("inactiveButton");
      Gtk::Grid *gridtr = Gtk::make_managed<Gtk::Grid> ();
      buttontr->set_child (*gridtr);
      Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create ();
      clck->set_button (3);
      clck->signal_pressed ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::contMenu), clck,
		      buttontr));
      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
	  Gio::SimpleActionGroup::create ();
      acgroup->add_action (
	  "info",
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::friendDetails),
		      buttontr));
      acgroup->add_action ("delete",
			   sigc::mem_fun (*this, &MainWindow::deleteContact));
      acgroup->add_action ("block",
			   sigc::mem_fun (*this, &MainWindow::tempBlockCont));
      buttontr->insert_action_group ("popupcont", acgroup);
      buttontr->add_controller (clck);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Prof/Profile/Avatar.jpeg";
      source = std::filesystem::u8path (filename);
      std::string avatarpath = source.u8string ();

      if (std::filesystem::exists (source))
	{
	  std::cout << "Avatar found" << std::endl;
	}

      Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea> ();
      drar->set_margin (2);
      drar->set_halign (Gtk::Align::CENTER);
      drar->set_size_request (50, 50);
      gridtr->attach (*drar, 0, 0, 1, 3);

      std::vector<Glib::ustring> forlab;
      forlab.push_back (Glib::ustring (otherpkstr));
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Prof/Profile/Profile";
      source = std::filesystem::u8path (filename);
      std::fstream f;
      f.open (source, std::ios_base::in);
      std::string line;
      int count = 0;
      while (!f.eof ())
	{
	  getline (f, line);
	  if (count == 2 && forlab.size () > 2)
	    {
	      forlab[2] = forlab[2] + " " + Glib::ustring (line);
	    }
	  else
	    {
	      forlab.push_back (Glib::ustring (line));
	    }
	  count++;
	}
      f.close ();

      Gtk::Label *keylab = Gtk::make_managed<Gtk::Label> ();
      keylab->set_max_width_chars (10);
      keylab->set_ellipsize (Pango::EllipsizeMode::END);
      keylab->set_text (forlab[0]);
      gridtr->attach (*keylab, 1, 0, 1, 1);

      source = std::filesystem::u8path (avatarpath);
      if (std::filesystem::exists (source))
	{
	  Glib::RefPtr<Gdk::Pixbuf> imagec = Gdk::Pixbuf::create_from_file (
	      Glib::ustring (avatarpath));
	  int drarw, drarh;
	  drar->get_size_request (drarw, drarh);
	  imagec = imagec->scale_simple (drarw, drarh,
					 Gdk::InterpType::BILINEAR);
	  std::pair<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf>> pair;
	  pair.first = keylab->get_text ();
	  pair.second = imagec;
	  conavmtx.lock ();
	  conavatars.push_back (pair);
	  conavmtx.unlock ();
	  drar->set_draw_func (
	      sigc::bind (
		  sigc::mem_fun (*this, &MainWindow::on_draw_contactlist),
		  keylab));
	}
      Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label> ();
      nicklab->set_text (forlab[1]);
      nicklab->set_halign (Gtk::Align::START);
      gridtr->attach (*nicklab, 1, 1, 1, 1);

      Gtk::Label *namelab = Gtk::make_managed<Gtk::Label> ();
      if (forlab.size () > 2)
	{
	  namelab->set_text (forlab[2]);
	}
      namelab->set_halign (Gtk::Align::START);
      gridtr->attach (*namelab, 1, 2, 1, 1);

      buttontr->signal_clicked ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::selectContact),
		      Winright, std::string (keylab->get_text ()),
		      std::string (nicklab->get_text ())));

      buttontr->set_size_request (contact_button_width, -1);

      frvectmtx.lock ();
      if (friendvect.size () == 0)
	{
	  fordelgrid->attach (*buttontr, 0, 0, 1, 1);
	}
      else
	{
	  Gtk::Widget *widg = std::get<0> (friendvect[0]);
	  fordelgrid->attach_next_to (*buttontr, *widg, Gtk::PositionType::TOP,
				      1, 1);
	}

      friendvect.insert (
	  friendvect.begin (),
	  std::make_tuple (buttontr, gridtr, keylab, nullptr, nicklab, namelab,
			   nullptr));
      frvectmtx.unlock ();
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist";
      outpath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (outpath))
	{
	  std::filesystem::remove_all (outpath);
	}
      std::filesystem::create_directories (outpath);

      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      source = std::filesystem::u8path (filename);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/Communist/Profile.zip";
      outpath = std::filesystem::u8path (filename);
      af.decryptFile (Username, Password, source.u8string (),
		      outpath.u8string ());
      af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
      std::filesystem::remove (outpath);
      filename = outpath.parent_path ().u8string ();
      filename = filename + "/Profile/Contacts";
      source = std::filesystem::u8path (filename);
      f.open (source, std::ios_base::out | std::ios_base::binary);
      for (size_t i = 0; i < contacts.size (); i++)
	{
	  line = std::get<1> (contacts[i]) + "\n";
	  f.write (line.c_str (), line.size ());
	}
      f.close ();
      filename = outpath.parent_path ().u8string ();
      filename = filename + "/Profile/RequestList";
      source = std::filesystem::u8path (filename);
      if (std::filesystem::exists (source))
	{
	  std::vector<std::string> readreq;
	  f.open (source, std::ios_base::in);
	  while (!f.eof ())
	    {
	      getline (f, line);
	      if (line != "")
		{
		  readreq.push_back (line);
		}
	    }
	  f.close ();
	  readreq.erase (
	      std::remove_if (readreq.begin (), readreq.end (), [&key]
	      (auto &el)
		{
		  std::string::size_type n;
		  n = el.find (key);
		  if (n != std::string::npos)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		}),
	      readreq.end ());
	  std::filesystem::remove (source);
	  if (readreq.size () > 0)
	    {
	      f.open (source, std::ios_base::out | std::ios_base::binary);
	      for (size_t i = 0; i < readreq.size (); i++)
		{
		  line = readreq[i] + "\n";
		  f.write (line.c_str (), line.size ());
		}
	      f.close ();
	    }
	}
      filename = outpath.parent_path ().u8string ();
      filename = filename + "/Profile";
      source = std::filesystem::u8path (filename);
      af.packing (source.u8string (), outpath.u8string ());
      source = outpath;
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      outpath = std::filesystem::u8path (filename);
      af.cryptFile (Username, Password, source.u8string (),
		    outpath.u8string ());
      std::filesystem::remove_all (source.parent_path ());
      proffopmtx.unlock ();
      if (chbutdel > 0)
	{
	  selectedc = buttontr;
	  buttontr->set_name ("activeButton");
	}
    }
  contmtx.unlock ();
}

void
MainWindow::sendMsg (Gtk::TextView *tv)
{

  Glib::RefPtr<Gtk::TextBuffer> buf = tv->get_buffer ();
  Glib::ustring utxt = buf->get_text (true);
  if (utxt != "" || attachedfile != nullptr)
    {
      Gtk::Requisition rq1, rq2;
      if (msg_win_gr != nullptr)
	{
	  int rowcount = 0;
	  Winright->get_preferred_size (rq1, rq2);
	  Gtk::Grid *grmsg = Gtk::make_managed<Gtk::Grid> ();
	  Gtk::Frame *fr = Gtk::make_managed<Gtk::Frame> ();
	  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create ();
	  clck->set_button (0);
	  fr->set_name ("myMsg");
	  fr->get_style_context ()->add_provider (css_provider,
	  GTK_STYLE_PROVIDER_PRIORITY_USER);
	  fr->set_child (*grmsg);
	  fr->set_margin (5);
	  fr->set_size_request (rq2.get_width () * 0.5, -1);
	  fr->set_halign (Gtk::Align::START);
	  fr->add_controller (clck);

	  Gtk::Label *date = Gtk::make_managed<Gtk::Label> ();
	  date->set_halign (Gtk::Align::START);
	  date->set_margin (2);
	  time_t curtm = time (NULL);
	  tm *gmtm = localtime (&curtm);
	  std::string datestr;
	  std::string day;
	  std::string month;
	  std::string year;
	  std::string hour;
	  std::string minut;
	  std::string second;

	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << gmtm->tm_hour;
	  hour = strm.str ();
	  if (gmtm->tm_hour < 10)
	    {
	      hour = "0" + hour;
	    }

	  strm.str ("");
	  strm.clear ();
	  strm.imbue (loc);
	  strm << gmtm->tm_min;
	  minut = strm.str ();
	  if (gmtm->tm_min < 10)
	    {
	      minut = "0" + minut;
	    }

	  strm.str ("");
	  strm.clear ();
	  strm.imbue (loc);
	  strm << gmtm->tm_sec;
	  second = strm.str ();
	  if (gmtm->tm_sec < 10)
	    {
	      second = "0" + second;
	    }

	  strm.str ("");
	  strm.clear ();
	  strm.imbue (loc);
	  strm << gmtm->tm_mday;
	  day = strm.str ();
	  if (gmtm->tm_mday < 10)
	    {
	      day = "0" + day;
	    }

	  strm.str ("");
	  strm.clear ();
	  strm.imbue (loc);
	  strm << gmtm->tm_mon + 1;
	  month = strm.str ();
	  if (gmtm->tm_mon < 10)
	    {
	      month = "0" + month;
	    }

	  strm.str ("");
	  strm.clear ();
	  strm.imbue (loc);
	  strm << gmtm->tm_year + 1900;
	  year = strm.str ();

	  datestr = day + "." + month + "." + year + " " + hour + ":" + minut
	      + ":" + second;
	  date->set_text (Glib::ustring (datestr));

	  grmsg->attach (*date, 0, rowcount, 1, 1);
	  rowcount++;

	  Gtk::Label *from = Gtk::make_managed<Gtk::Label> ();
	  from->set_halign (Gtk::Align::START);
	  from->set_margin (2);
	  from->set_text ("Я:");
	  from->set_name ("notSentMsg");
	  from->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  grmsg->attach (*from, 0, rowcount, 1, 1);
	  rowcount++;

	  if (repllabe != nullptr)
	    {
	      Gtk::Frame *repfr = Gtk::make_managed<Gtk::Frame> ();
	      repfr->set_name ("repFrame");
	      repfr->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      repfr->set_halign (Gtk::Align::START);
	      repfr->set_margin (2);
	      Gtk::Label *repl = Gtk::make_managed<Gtk::Label> ();
	      repl->set_halign (Gtk::Align::START);
	      repl->set_margin (5);
	      repl->set_use_markup (true);
	      repl->set_max_width_chars (20);
	      repl->set_ellipsize (Pango::EllipsizeMode::END);
	      repl->set_markup (
		  "<span style=\"italic\">" + repllabe->get_text ()
		      + "</span>");
	      repfr->set_child (*repl);
	      grmsg->attach (*repfr, 0, rowcount, 1, 1);
	      rowcount++;
	    }

	  Gtk::Label *message = Gtk::make_managed<Gtk::Label> ();
	  message->set_halign (Gtk::Align::START);
	  message->set_margin (5);
	  message->set_justify (Gtk::Justification::LEFT);
	  message->set_wrap (true);
	  message->set_wrap_mode (Pango::WrapMode::WORD);
	  message->set_selectable (true);
	  message->set_margin_bottom (10);
	  if (attachedfile != nullptr)
	    {
	      utxt = attachedfile->get_text () + "\n\n" + utxt;
	    }
	  message->set_text (utxt);
	  grmsg->attach (*message, 0, rowcount, 1, 1);
	  msg_grid_vectmtx.lock ();
	  if (msg_grid_vect.size () == 0)
	    {
	      msg_win_gr->attach (*fr, 0, 0, 1, 1);
	    }
	  else
	    {
	      Gtk::Widget *widg = std::get<0> (
		  msg_grid_vect[msg_grid_vect.size () - 1]);
	      int column, row, width, height;
	      msg_win_gr->query_child (*widg, column, row, width, height);
	      msg_win_gr->attach (*fr, 0, row + 1, 1, 1);
	    }
	  msg_grid_vectmtx.unlock ();
	  clck->signal_pressed ().connect (
	      sigc::bind (sigc::mem_fun (*this, &MainWindow::creatReply), fr,
			  message, clck));
	  std::string index = "";
	  std::string key = fordelkey;
	  contmtx.lock ();
	  auto cit = std::find_if (contacts.begin (), contacts.end (), [&key]
	  (auto &el)
	    {
	      return std::get<1>(el) == key;
	    });
	  if (cit != contacts.end ())
	    {
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << std::get<0> (*cit);
	      index = strm.str ();
	    }
	  contmtx.unlock ();
	  std::string filename;
	  AuxFunc af;
#ifdef __linux
	  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
          filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	  if (index != "")
	    {
	      filename = filename + "/CommunistSM/" + index + "/";
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << curtm;
	      filename = filename + strm.str ();
	      std::filesystem::path filepath = std::filesystem::u8path (
		  filename);
	      if (!std::filesystem::exists (filepath.parent_path ()))
		{
		  std::filesystem::create_directories (filepath.parent_path ());
		}
	      std::fstream f;
	      f.open (filepath, std::ios_base::out | std::ios_base::binary);
	      std::string line;
	      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	      okp = lt::dht::ed25519_create_keypair (seed);
	      line = lt::aux::to_hex (std::get<0> (okp).bytes);
	      line = line + "\n";
	      f.write (line.c_str (), line.size ());
	      line = fordelkey + "\n";
	      f.write (line.c_str (), line.size ());
	      line = filepath.filename ().u8string () + "\n";
	      f.write (line.c_str (), line.size ());
	      if (attachedfile == nullptr)
		{
		  line = "0\n";
		}
	      if (attachedfile != nullptr)
		{
		  line = "1\n";
		}

	      f.write (line.c_str (), line.size ());
	      if (repllabe != nullptr)
		{
		  line = "r " + std::string (repllabe->get_text ()) + "\n";
		}
	      else
		{
		  line = "r\n";
		}
	      f.write (line.c_str (), line.size ());
	      line = Glib::locale_from_utf8 (utxt);
	      af.toutf8 (line);
	      f.write (line.c_str (), line.size ());
	      f.close ();
	      int ch = -1;
	      if (oper != nullptr)
		{
		  if (attachedfile == nullptr)
		    {
		      ch = oper->createMsg (fordelkey, filepath, 0);
		    }
		  if (attachedfile != nullptr)
		    {
		      ch = oper->createMsg (fordelkey, filepath, 1);
		    }
		}
	      if (ch >= 0)
		{
		  sentstatmtx.lock ();
		  sentstatus.push_back (std::make_tuple (fordelkey, ch, from));
		  sentstatmtx.unlock ();
		  af.homePath (&filename);
		  filename = filename + "/.Communist/" + index + "/Yes";
		  filepath = std::filesystem::u8path (filename);
		  if (std::filesystem::exists (filepath))
		    {
#ifdef __linux
		      filename =
			  std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
		      filename =
			  std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
		      filename = filename + "/YesPr";
		      std::filesystem::path outpath = std::filesystem::u8path (
			  filename);
		      yesmtx.lock ();
		      if (std::filesystem::exists (outpath))
			{
			  std::filesystem::remove (outpath);
			}
		      af.decryptFile (Username, Password, filepath.u8string (),
				      outpath.u8string ());
		      f.open (
			  outpath,
			  std::ios_base::out | std::ios_base::app
			      | std::ios_base::binary);
		      std::string timewr = strm.str ();
		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << ch;
		      line = strm.str () + " " + timewr + "\n";
		      f.write (line.c_str (), line.size ());
		      f.close ();
		      af.cryptFile (Username, Password, outpath.u8string (),
				    filepath.u8string ());
		      std::filesystem::remove (outpath);
		      yesmtx.unlock ();
		      line = filepath.parent_path ().u8string ();
		      if (attachedfile == nullptr)
			{
			  line = line + "/" + strm.str ();
			}
		      if (attachedfile != nullptr)
			{
			  line = line + "/" + strm.str () + "f";
			}
		      std::filesystem::path pfg = std::filesystem::u8path (
			  line);
		      msg_grid_vectmtx.lock ();
		      msg_grid_vect.push_back (std::make_tuple (fr, pfg));
		      if (msg_grid_vect.size () > 20)
			{
			  Gtk::Widget *widg = std::get<0> (msg_grid_vect[0]);
			  msg_win_gr->remove (*widg);
			  msg_grid_vect.erase (msg_grid_vect.begin ());
			}
		      msg_grid_vectmtx.unlock ();
		    }
		  else
		    {
#ifdef __linux
		      filename =
			  std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
		      filename =
			  std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
		      filename = filename + "/YesPr";
		      std::filesystem::path outpath = std::filesystem::u8path (
			  filename);
		      yesmtx.lock ();
		      f.open (outpath,
			      std::ios_base::out | std::ios_base::binary);
		      line = fordelkey + "\n";
		      f.write (line.c_str (), line.size ());
		      std::string timewr = strm.str ();
		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << ch;
		      line = strm.str () + " " + timewr + "\n";
		      f.write (line.c_str (), line.size ());
		      f.close ();
		      af.cryptFile (Username, Password, outpath.u8string (),
				    filepath.u8string ());
		      yesmtx.unlock ();
		      line = filepath.parent_path ().u8string ();
		      if (attachedfile == nullptr)
			{
			  line = line + "/" + strm.str ();
			}
		      if (attachedfile != nullptr)
			{
			  line = line + "/" + strm.str () + "f";
			}
		      std::filesystem::path pfg = std::filesystem::u8path (
			  line);
		      msg_grid_vectmtx.lock ();
		      msg_grid_vect.push_back (std::make_tuple (fr, pfg));
		      if (msg_grid_vect.size () > 20)
			{
			  Gtk::Widget *widg = std::get<0> (msg_grid_vect[0]);
			  msg_win_gr->remove (*widg);
			  msg_grid_vect.erase (msg_grid_vect.begin ());
			}
		      msg_grid_vectmtx.unlock ();
		    }
		}
	      else
		{
		  msg_grid_vectmtx.lock ();
		  msg_grid_vect.erase (
		      std::remove_if (msg_grid_vect.begin (),
				      msg_grid_vect.end (), [fr]
				      (auto &el)
					{
					  return std::get<0>(el) == fr;
					}),
		      msg_grid_vect.end ());
		  msg_grid_vectmtx.unlock ();
		  msg_win_gr->remove (*fr);
		}
	    }
	}
    }
  buf->set_text ("");
  if (repllabe != nullptr)
    {
      Rightgrid->remove (*replgrid);
    }
  if (attachedfile != nullptr)
    {
      Rightgrid->remove (*attach);
      Rightgrid->remove (*attachcancel);
      Rightgrid->remove (*attachedfile);
    }
  repllabe = nullptr;
  replcancel = nullptr;
  attach = nullptr;
  attachcancel = nullptr;
  attachedfile = nullptr;
}

void
MainWindow::creatReply (int numcl, double x, double y, Gtk::Frame *fr,
			Gtk::Label *message,
			Glib::RefPtr<Gtk::GestureClick> clck)
{
  if (clck->get_current_button () == 3)
    {
      Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create ();
      menu->append (gettext ("Reply"), "popupmsg.reply");
      menu->append (gettext ("Forward"), "popupmsg.forward");
      menu->append (gettext ("Remove (only this machine)"), "popupmsg.remove");
      Gtk::PopoverMenu *Menu = Gtk::make_managed<Gtk::PopoverMenu> ();
      Menu->set_parent (*fr);
      Menu->set_menu_model (menu);
      Menu->set_has_arrow (false);
      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
	  Gio::SimpleActionGroup::create ();
      acgroup->add_action ("reply", [this, message]
      {
	if (this->repllabe != nullptr)
	  {
	    this->Rightgrid->remove (*(this->replgrid));
	    this->replgrid = nullptr;
	    this->replcancel = nullptr;
	    this->repllabe = nullptr;
	  }
	this->repllabe = Gtk::make_managed<Gtk::Label> ();
	this->repllabe->set_halign (Gtk::Align::START);
	this->repllabe->set_margin (5);
	this->repllabe->set_max_width_chars (50);
	this->repllabe->set_ellipsize (Pango::EllipsizeMode::END);
	this->repllabe->set_text (message->get_text ());
	this->repllabe->set_name ("fileAddress");
	this->repllabe->get_style_context ()->add_provider (css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

	this->replcancel = Gtk::make_managed<Gtk::Button> ();
	this->replcancel->set_valign (Gtk::Align::CENTER);
	this->replcancel->set_halign (Gtk::Align::END);
	this->replcancel->set_margin (5);
	this->replcancel->set_name ("cancelRepl");
	this->replcancel->get_style_context ()->add_provider (
	    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	this->replcancel->set_icon_name ("window-close-symbolic");
	this->replcancel->signal_clicked ().connect ( [this]
	{
	  this->Rightgrid->remove (*(this->replgrid));
	  this->replgrid = nullptr;
	  this->replcancel = nullptr;
	  this->repllabe = nullptr;
	});

	this->Rightgrid->insert_next_to (*(this->msgovl),
					 Gtk::PositionType::BOTTOM);
	this->replgrid = Gtk::make_managed<Gtk::Grid> ();
	this->replgrid->attach (*(this->repllabe), 0, 0, 1, 1);
	this->replgrid->attach (*(this->replcancel), 1, 0, 1, 1);
	this->Rightgrid->attach_next_to (*(this->replgrid), *(this->msgovl),
					 Gtk::PositionType::BOTTOM, 2, 1);
	this->rightmenfordel->unparent ();
	this->rightmenfordel = nullptr;
      });
      acgroup->add_action ("forward", [this, message, fr]
      {
	Gtk::Button *sb = this->selectedc;
	Gtk::Widget *w = fr;
	this->frvectmtx.lock ();
	auto itfr = std::find_if (this->friendvect.begin (), this->friendvect.end (), [sb]
      (auto &el)
	{
	  return std::get<0>(el) == sb;
	});
	if (itfr != this->friendvect.end ())
	  {
	    Gtk::Label *keyl = std::get<2> (*itfr);
	    std::string othkey (keyl->get_text ());
	    this->msg_grid_vectmtx.lock ();
	    auto itgr = std::find_if (this->msg_grid_vect.begin (),
				      this->msg_grid_vect.end (), [w]
				      (auto &el)
					{
					  return std::get<0>(el) == w;
					});
	    if (itgr != this->msg_grid_vect.end ())
	      {
		std::filesystem::path msgpath = std::get<1> (*itgr);
		this->resendWindow (othkey, msgpath);
	      }
	    this->msg_grid_vectmtx.unlock ();
	  }
	this->frvectmtx.unlock ();
	this->rightmenfordel->unparent ();
	this->rightmenfordel = nullptr;
      });
      acgroup->add_action ("remove", [this, message, fr]
      {
	Gtk::Button *sb = this->selectedc;
	Gtk::Widget *w = fr;
	this->frvectmtx.lock ();
	auto itfr = std::find_if (this->friendvect.begin (), this->friendvect.end (), [sb]
      (auto &el)
	{
	  return std::get<0>(el) == sb;
	});
	if (itfr != this->friendvect.end ())
	  {
	    Gtk::Label *keyl = std::get<2> (*itfr);
	    std::string othkey (keyl->get_text ());
	    this->msg_grid_vectmtx.lock ();
	    auto itgr = std::find_if (this->msg_grid_vect.begin (),
				      this->msg_grid_vect.end (), [w]
				      (auto &el)
					{
					  return std::get<0>(el) == w;
					});
	    if (itgr != this->msg_grid_vect.end ())
	      {
		std::filesystem::path msgpath = std::get<1> (*itgr);
		Gtk::Widget *widg = std::get<0> (*itgr);
		this->removeMsg (othkey, msgpath, widg);
	      }
	    this->msg_grid_vectmtx.unlock ();
	  }
	this->frvectmtx.unlock ();
	this->rightmenfordel->unparent ();
	this->rightmenfordel = nullptr;
      });

      fr->insert_action_group ("popupmsg", acgroup);

      const Gdk::Rectangle rect (x, y, 1, 1);
      Menu->set_pointing_to (rect);
      if (rightmenfordel != nullptr)
	{
	  rightmenfordel->unparent ();
	  rightmenfordel = nullptr;
	}
      Menu->set_autohide (false);
      Menu->popup ();

      rightmenfordel = Menu;
    }
}

void
MainWindow::resendWindow (std::string othkey, std::filesystem::path msgpath)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_title (gettext ("Select contact"));
  window->set_transient_for (*this);
  window->set_modal (true);
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  window->set_child (*grid);

  for (size_t i = 0; i < friendvect.size (); i++)
    {
      if (std::get<0> (friendvect[i]) != selectedc)
	{
	  Gtk::Label *lab = std::get<4> (friendvect[i]);
	  Glib::ustring labt = lab->get_text ();
	  lab = Gtk::make_managed<Gtk::Label> ();
	  lab->set_halign (Gtk::Align::CENTER);
	  lab->set_margin (5);
	  lab->set_text (labt);
	  grid->attach (*lab, 0, i, 1, 1);

	  Gtk::CheckButton *tgbtn = Gtk::make_managed<Gtk::CheckButton> ();
	  tgbtn->set_halign (Gtk::Align::CENTER);
	  tgbtn->set_valign (Gtk::Align::CENTER);
	  tgbtn->set_active (false);
	  grid->attach (*tgbtn, 1, i, 1, 1);

	  std::tuple<std::string, Gtk::CheckButton*> ttup;
	  lab = std::get<2> (friendvect[i]);
	  std::get<0> (ttup) = std::string (lab->get_text ());
	  std::get<1> (ttup) = tgbtn;
	  resendactive.push_back (ttup);
	}
    }
  int last = friendvect.size ();

  Gtk::Button *conf = Gtk::make_managed<Gtk::Button> ();
  conf->set_halign (Gtk::Align::CENTER);
  conf->set_margin (5);
  conf->set_label (gettext ("Confirm"));
  conf->set_name ("applyButton");
  conf->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  conf->signal_clicked ().connect ( [this, window, othkey, msgpath]
  {
    if (this->resendactive.size () > 0)
      {
#ifdef __linux
    std::string filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	std::string filename =
	    std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistResent/" + msgpath.filename ().u8string ();
    std::filesystem::path outpath;
    AuxFunc af;
    outpath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (outpath.parent_path ()))
      {
	std::filesystem::remove_all (outpath.parent_path ());
      }
    std::filesystem::create_directories (outpath.parent_path ());
    std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
    okp = lt::dht::ed25519_create_keypair (this->seed);
    std::string ownkey = lt::aux::to_hex (std::get<0> (okp).bytes);
    lt::dht::public_key pkpass;
    lt::aux::from_hex (othkey, pkpass.bytes.data ());
    std::array<char, 32> scalar;
    scalar = lt::dht::ed25519_key_exchange (pkpass, std::get<1> (okp));
    pkpass = lt::dht::ed25519_add_scalar (std::get<0> (okp), scalar);
    std::string unm = ownkey;
    std::string passwd = lt::aux::to_hex (pkpass.bytes);
    af.decryptFile (unm, passwd, msgpath.u8string (), outpath.u8string ());
    std::string chstr;
    int count = 0;
    std::fstream f;
    f.open (outpath, std::ios_base::in);
    while (!f.eof ())
      {
	getline (f, chstr);
	if (count == 4)
	  {
	    break;
	  }
	count++;
      }
    f.close ();
    chstr = chstr.substr (0, chstr.find (" "));
    if (chstr != "r")
      {
	unm = othkey;
	lt::aux::from_hex (othkey, pkpass.bytes.data ());
	pkpass = lt::dht::ed25519_add_scalar (pkpass, scalar);
	passwd = lt::aux::to_hex (pkpass.bytes);
	af.decryptFile (unm, passwd, msgpath.u8string (), outpath.u8string ());
      }
    std::vector<std::string> msgv;
    f.open (outpath, std::ios_base::in);
    while (!f.eof ())
      {
	getline (f, chstr);
	if (chstr != "")
	  {
	    msgv.push_back (chstr);
	  }
      }
    f.close ();
    if (msgv.size () > 5)
      {
	chstr = msgv[0];
	std::string::size_type n;
	n = chstr.find (" ");
	if (n != std::string::npos)
	  {
	    chstr = chstr.substr (0, n);
	    chstr = chstr + " " + ownkey;
	  }
	else
	  {
	    this->frvectmtx.lock ();
	    auto itfrv = std::find_if (this->friendvect.begin (), this->friendvect.end (), [othkey]
  (auto &el)
    {
      Gtk::Label *lab = std::get<2>(el);
      return std::string(lab->get_text()) == othkey;
    });
	    if (itfrv != this->friendvect.end ())
	      {
		chstr = std::string (std::get<4> (*itfrv)->get_text ()) + " "
		    + ownkey;
	      }
	    this->frvectmtx.unlock ();
	  }
	msgv[0] = chstr;
	chstr = msgv[3];
	for (size_t k = 0; k < this->resendactive.size (); k++)
	  {
	    std::string selkey = std::get<0> (this->resendactive[k]);
	    Gtk::CheckButton *chbtn = std::get<1> (this->resendactive[k]);
	    if (chbtn->get_active ())
	      {
		msgv[1] = selkey;
		if (chstr == "0")
		  {
		    filename = outpath.parent_path ().u8string ();
		    time_t ct = time (NULL);
		    std::stringstream strm;
		    std::locale loc ("C");
		    strm.imbue (loc);
		    strm << ct;
		    msgv[2] = strm.str ();
		    filename = filename + "/" + strm.str ();
		    std::filesystem::path filepath;
		    filepath = std::filesystem::u8path (filename);
		    f.open (filepath,
			    std::ios_base::out | std::ios_base::binary);
		    for (size_t i = 0; i < msgv.size (); i++)
		      {
			std::string line = msgv[i];
			line = line + "\n";
			f.write (line.c_str (), line.size ());
		      }
		    f.close ();
		    this->oper->createMsg (selkey, filepath, 0);
		    int index = -1;
		    this->contmtx.lock ();
		    auto itcont = std::find_if (
			this->contacts.begin (), this->contacts.end (),
			[&selkey]
			(auto &el)
			  {
			    return std::get<1>(el) == selkey;
			  });
		    if (itcont != this->contacts.end ())
		      {
			index = std::get<0> (*itcont);
		      }
		    this->contmtx.unlock ();
		    if (index >= 0)
		      {
			strm.clear ();
			strm.str ("");
			strm.imbue (loc);
			strm << index;
			af.homePath (&filename);
			filename = filename + "/.Communist/" + strm.str ()
			    + "/Yes";
			filepath = std::filesystem::u8path (filename);
			if (std::filesystem::exists (filepath))
			  {
			    filename = outpath.parent_path ().u8string ();
			    filename = filename + "/YesPr";
			    std::filesystem::path tp = std::filesystem::u8path (
				filename);
			    af.decryptFile (this->Username, this->Password,
					    filepath.u8string (),
					    tp.u8string ());
			    std::vector<std::string> yesv;
			    f.open (tp, std::ios_base::in);
			    while (!f.eof ())
			      {
				std::string line;
				getline (f, line);
				if (line != "")
				  {
				    yesv.push_back (line);
				  }
			      }
			    f.close ();
			    if (yesv.size () > 0)
			      {
				std::string line = yesv[yesv.size () - 1];
				std::string::size_type n;
				n = line.find (" ");
				if (n != std::string::npos)
				  {
				    line = line.substr (0, n);
				    strm.clear ();
				    strm.str ("");
				    strm.imbue (loc);
				    strm << line;
				    int tint;
				    strm >> tint;
				    tint = tint + 1;
				    strm.clear ();
				    strm.str ("");
				    strm.imbue (loc);
				    strm << tint;
				    line = strm.str ();
				    strm.clear ();
				    strm.str ("");
				    strm.imbue (loc);
				    strm << ct;
				    line = line + " " + strm.str ();
				    yesv.push_back (line);
				  }
				else
				  {
				    strm.clear ();
				    strm.str ("");
				    strm.imbue (loc);
				    strm << ct;
				    line = "0 " + strm.str ();
				    yesv.push_back (line);
				  }
			      }
			    else
			      {
				std::string line = selkey;
				yesv.push_back (line);
			      }
			    f.open (tp,
				    std::ios_base::out | std::ios_base::binary);
			    for (size_t i = 0; i < yesv.size (); i++)
			      {
				std::string line = yesv[i];
				line = line + "\n";
				f.write (line.c_str (), line.size ());
			      }
			    f.close ();
			    af.cryptFile (this->Username, this->Password,
					  tp.u8string (), filepath.u8string ());
			  }
			else
			  {
			    filename = outpath.parent_path ().u8string ();
			    filename = filename + "/YesPr";
			    std::filesystem::path tp = std::filesystem::u8path (
				filename);
			    std::vector<std::string> yesv;
			    std::string line = selkey;
			    yesv.push_back (line);
			    strm.clear ();
			    strm.str ("");
			    strm.imbue (loc);
			    strm << ct;
			    line = "0 " + strm.str ();
			    yesv.push_back (line);
			    f.open (tp,
				    std::ios_base::out | std::ios_base::binary);
			    for (size_t i = 0; i < yesv.size (); i++)
			      {
				line = yesv[i];
				line = line + "\n";
				f.write (line.c_str (), line.size ());
			      }
			    f.close ();
			    af.cryptFile (this->Username, this->Password,
					  tp.u8string (), filepath.u8string ());
			  }
		      }
		  }
		if (chstr == "1")
		  {
		    chstr = msgv[5];
		    std::filesystem::path p = std::filesystem::u8path (chstr);
		    if (!std::filesystem::exists (p))
		      {
			Gtk::Window *errwin = new Gtk::Window;
			errwin->set_application (this->get_application ());
			errwin->set_name ("settingsWindow");
			errwin->get_style_context ()->add_provider (
			    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
			errwin->set_title (gettext ("Error!"));
			errwin->set_transient_for (*this);
			errwin->set_modal (true);
			Gtk::Grid *gr = Gtk::make_managed<Gtk::Grid> ();
			gr->set_halign (Gtk::Align::CENTER);
			errwin->set_child (*gr);

			Gtk::Label *errl = Gtk::make_managed<Gtk::Label> ();
			errl->set_halign (Gtk::Align::CENTER);
			errl->set_margin (5);
			errl->set_text (
			    gettext ("File you want to send does not exists"));
			gr->attach (*errl, 0, 0, 1, 1);

			Gtk::Button *clb = Gtk::make_managed<Gtk::Button> ();
			clb->set_halign (Gtk::Align::CENTER);
			clb->set_margin (5);
			clb->set_name ("clearButton");
			clb->get_style_context ()->add_provider (
			    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
			clb->set_label (gettext ("Close"));
			clb->signal_clicked ().connect ( [errwin]
			{
			  errwin->close ();
			});
			gr->attach (*clb, 0, 1, 1, 1);

			errwin->signal_close_request ().connect ( [errwin]
			{
			  errwin->hide ();
			  delete errwin;
			  return true;
			},
								 false);
			errwin->show ();
		      }
		    else
		      {
			filename = outpath.parent_path ().u8string ();
			time_t ct = time (NULL);
			std::stringstream strm;
			std::locale loc ("C");
			strm.imbue (loc);
			strm << ct;
			msgv[2] = strm.str ();
			filename = filename + "/" + strm.str ();
			std::filesystem::path filepath;
			filepath = std::filesystem::u8path (filename);
			f.open (filepath,
				std::ios_base::out | std::ios_base::binary);
			for (size_t i = 0; i < msgv.size (); i++)
			  {
			    std::string line = msgv[i];
			    line = line + "\n";
			    f.write (line.c_str (), line.size ());
			  }
			f.close ();
			this->oper->createMsg (selkey, filepath, 1);
			int index = -1;
			this->contmtx.lock ();
			auto itcont = std::find_if (
			    this->contacts.begin (), this->contacts.end (),
			    [&selkey]
			    (auto &el)
			      {
				return std::get<1>(el) == selkey;
			      });
			if (itcont != this->contacts.end ())
			  {
			    index = std::get<0> (*itcont);
			  }
			this->contmtx.unlock ();
			if (index >= 0)
			  {
			    strm.clear ();
			    strm.str ("");
			    strm.imbue (loc);
			    strm << index;
			    af.homePath (&filename);
			    filename = filename + "/.Communist/" + strm.str ()
				+ "/Yes";
			    filepath = std::filesystem::u8path (filename);
			    if (std::filesystem::exists (filepath))
			      {
				filename = outpath.parent_path ().u8string ();
				filename = filename + "/YesPr";
				std::filesystem::path tp =
				    std::filesystem::u8path (filename);
				this->yesmtx.lock ();
				af.decryptFile (this->Username, this->Password,
						filepath.u8string (),
						tp.u8string ());
				std::vector<std::string> yesv;
				f.open (tp, std::ios_base::in);
				while (!f.eof ())
				  {
				    std::string line;
				    getline (f, line);
				    if (line != "")
				      {
					yesv.push_back (line);
				      }
				  }
				f.close ();
				if (yesv.size () > 0)
				  {
				    std::string line = yesv[yesv.size () - 1];
				    std::string::size_type n;
				    n = line.find (" ");
				    if (n != std::string::npos)
				      {
					line = line.substr (0, n);
					strm.clear ();
					strm.str ("");
					strm.imbue (loc);
					strm << line;
					int tint;
					strm >> tint;
					tint = tint + 1;
					strm.clear ();
					strm.str ("");
					strm.imbue (loc);
					strm << tint;
					line = strm.str ();
					strm.clear ();
					strm.str ("");
					strm.imbue (loc);
					strm << ct;
					line = line + " " + strm.str ();
					yesv.push_back (line);
				      }
				    else
				      {
					strm.clear ();
					strm.str ("");
					strm.imbue (loc);
					strm << ct;
					line = "0 " + strm.str ();
					yesv.push_back (line);
				      }
				  }
				else
				  {
				    std::string line = selkey;
				    yesv.push_back (line);
				  }
				f.open (
				    tp,
				    std::ios_base::out | std::ios_base::binary);
				for (size_t i = 0; i < yesv.size (); i++)
				  {
				    std::string line = yesv[i];
				    line = line + "\n";
				    f.write (line.c_str (), line.size ());
				  }
				f.close ();
				af.cryptFile (this->Username, this->Password,
					      tp.u8string (),
					      filepath.u8string ());
				this->yesmtx.unlock ();
			      }
			    else
			      {
				this->yesmtx.lock ();
				filename = outpath.parent_path ().u8string ();
				filename = filename + "/YesPr";
				std::filesystem::path tp =
				    std::filesystem::u8path (filename);
				std::vector<std::string> yesv;
				std::string line = selkey;
				yesv.push_back (line);
				strm.clear ();
				strm.str ("");
				strm.imbue (loc);
				strm << ct;
				line = "0 " + strm.str ();
				yesv.push_back (line);
				f.open (
				    tp,
				    std::ios_base::out | std::ios_base::binary);
				for (size_t i = 0; i < yesv.size (); i++)
				  {
				    line = yesv[i];
				    line = line + "\n";
				    f.write (line.c_str (), line.size ());
				  }
				f.close ();
				af.cryptFile (this->Username, this->Password,
					      tp.u8string (),
					      filepath.u8string ());
				this->yesmtx.unlock ();
			      }
			  }
		      }
		    chstr = "1";
		  }
	      }
	  }
	std::filesystem::remove_all (outpath.parent_path ());
      }
    window->close ();
  }
});
  grid->attach (*conf, 0, last, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
  cancel->set_halign (Gtk::Align::CENTER);
  cancel->set_margin (5);
  cancel->set_name ("rejectButton");
  cancel->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->set_label (gettext ("Cancel"));
  cancel->signal_clicked ().connect ( [this, window]
  {
    window->close ();
  });
  grid->attach (*cancel, 1, last, 1, 1);

  window->signal_close_request ().connect ( [window, this]
  {
    this->resendactive.clear ();
    window->hide ();
    delete window;
    return true;
  },
					   false);
  window->show ();
}
void
MainWindow::removeMsg (std::string othkey, std::filesystem::path msgpath,
		       Gtk::Widget *widg)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_title (gettext ("Confirmation"));

  window->set_transient_for (*this);
  window->set_modal (true);
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  window->set_child (*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
  lab->set_halign (Gtk::Align::CENTER);
  lab->set_margin (5);
  lab->set_text (gettext ("Are you sure?"));
  grid->attach (*lab, 0, 0, 2, 1);

  Gtk::Button *confirm = Gtk::make_managed<Gtk::Button> ();
  confirm->set_halign (Gtk::Align::CENTER);
  confirm->set_margin (5);
  confirm->set_name ("clearButton");
  confirm->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  confirm->set_label (gettext ("Yes"));

  confirm->signal_clicked ().connect ( [window, this, othkey, msgpath, widg]
  {
    AuxFunc af;
    std::filesystem::remove (msgpath);
    std::string ind = msgpath.filename ().u8string ();
    ind = ind.substr (0, ind.find ("f"));
    std::stringstream strm;
    std::locale loc ("C");
    strm.imbue (loc);
    strm << ind;
    int indint;
    strm >> indint;
    this->sentstatmtx.lock ();
    this->sentstatus.erase (std::remove_if (this->sentstatus.begin (), this->sentstatus.end(), [othkey, indint](auto &el)
    {
      if (std::get<0>(el) == othkey && std::get<1>(el) == indint)
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }), this->sentstatus.end());
    this->sentstatmtx.unlock ();
    for (auto &dirit : std::filesystem::directory_iterator (
	msgpath.parent_path ()))
      {
	std::filesystem::path p = dirit.path ();
	if (p.filename ().u8string () != "Profile"
	    && p.filename ().u8string () != "Yes")
	  {
	    int tmpi;
	    ind = p.filename ().u8string ();
	    std::string::size_type n;
	    n = ind.find ("f");
	    ind = ind.substr (0, n);
	    strm.clear ();
	    strm.str ("");
	    strm.imbue (loc);
	    strm << ind;
	    strm >> tmpi;
	    if (tmpi > indint)
	      {
		tmpi = tmpi - 1;
		strm.clear ();
		strm.str ("");
		strm.imbue (loc);
		strm << tmpi;
		ind = p.parent_path ().u8string ();
		ind = ind + "/" + strm.str ();
		if (n != std::string::npos)
		  {
		    ind = ind + "f";
		  }
		std::filesystem::path np = std::filesystem::u8path (ind);
		std::filesystem::rename (p, np);
	      }
	  }
      }
    std::string filename;
    filename = msgpath.parent_path ().u8string ();
    filename = filename + "/Yes";
    std::filesystem::path filepath = std::filesystem::u8path (filename);
    if (std::filesystem::exists (filepath))
      {
#ifdef __linux
	filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	filename = filename + "/CommunistRMM/YesPr";
	std::filesystem::path outpath = std::filesystem::u8path (filename);
	if (std::filesystem::exists (outpath.parent_path ()))
	  {
	    std::filesystem::remove_all (outpath.parent_path ());
	  }
	std::filesystem::create_directories (outpath.parent_path ());
	this->yesmtx.lock ();
	af.decryptFile (this->Username, this->Password, filepath.u8string (),
			outpath.u8string ());
	std::fstream f;
	std::vector<std::string> tv;
	f.open (outpath, std::ios_base::in);
	while (!f.eof ())
	  {
	    std::string line;
	    getline (f, line);
	    if (line != "")
	      {
		tv.push_back (line);
	      }
	  }
	f.close ();
	tv.erase (std::remove_if (tv.begin () + 1, tv.end (), [&indint]
	(auto &el)
	  {
	    std::string tmp = el;
	    tmp = tmp.substr(0, tmp.find(" "));
	    std::stringstream strm;
	    std::locale loc ("C");
	    strm.imbue(loc);
	    strm << tmp;
	    int tint;
	    strm >> tint;
	    if (tint == indint)
	      {
		return true;
	      }
	    else
	      {
		return false;
	      }
	  }),
		  tv.end ());
	for (size_t i = 1; i < tv.size (); i++)
	  {
	    std::string tmp = tv[i];
	    tmp = tmp.substr (0, tmp.find (" "));
	    strm.clear ();
	    strm.str ("");
	    strm.imbue (loc);
	    strm << tmp;
	    int tint;
	    strm >> tint;
	    if (tint > indint)
	      {
		tint = tint - 1;
		strm.clear ();
		strm.str ("");
		strm.imbue (loc);
		strm << tint;
		tmp = tv[i];
		tmp.erase (0, tmp.find (" ") + std::string (" ").size ());
		tmp = strm.str () + " " + tmp;
		tv[i] = tmp;
	      }
	  }
	if (tv.size () > 1)
	  {
	    f.open (outpath, std::ios_base::out | std::ios_base::binary);
	    for (size_t i = 0; i < tv.size (); i++)
	      {
		std::string line = tv[i];
		line = line + "\n";
		f.write (line.c_str (), line.size ());
	      }
	    f.close ();
	    af.cryptFile (this->Username, this->Password, outpath.u8string (),
			  filepath.u8string ());
	  }
	else
	  {
	    std::filesystem::remove (filepath);
	  }
	std::filesystem::remove_all (outpath.parent_path ());
	this->yesmtx.unlock ();
	std::filesystem::path retpath;
	retpath = this->oper->removeMsg (othkey, msgpath);
	this->fileprogrvectmtx.lock ();
	auto itfprg = std::find_if (this->fileprogrvect.begin (),
				    this->fileprogrvect.end (), [&retpath]
				    (auto &el)
				      {
					return std::get<1>(el) == retpath;
				      });
	if (itfprg != this->fileprogrvect.end ())
	  {
	    this->dld_grid->remove (*(std::get<3> (*itfprg)));
	    this->dld_grid->remove (*(std::get<4> (*itfprg)));
	    this->dld_grid->remove (*(std::get<5> (*itfprg)));
	    this->fileprogrvect.erase (itfprg);
	  }
	if (this->fileprogrvect.size () == 0 && this->dld_win != nullptr)
	  {
	    this->dld_win->hide ();
	    delete this->dld_win;
	    this->dld_win = nullptr;
	    this->dld_grid = nullptr;
	  }
	this->fileprogrvectmtx.unlock ();
      }
    this->msg_grid_vectmtx.lock ();
    this->msg_grid_vect.erase (
	std::remove_if (this->msg_grid_vect.begin (),
			this->msg_grid_vect.end (), [widg]
			(auto &el)
			  {
			    return std::get<0>(el) == widg;
			  }),
	this->msg_grid_vect.end ());
    this->msg_grid_vectmtx.unlock ();
    this->msg_win_gr->remove (*widg);
    window->close ();
  });
  grid->attach (*confirm, 0, 1, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
  cancel->set_halign (Gtk::Align::CENTER);
  cancel->set_margin (5);
  cancel->set_name ("rejectButton");
  cancel->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->set_label (gettext ("No"));
  cancel->signal_clicked ().connect ( [window]
  {
    window->close ();
  });
  grid->attach (*cancel, 1, 1, 1, 1);

  window->signal_close_request ().connect ( [window]
  {
    window->hide ();
    delete window;
    return true;
  },
					   false);
  window->show ();
}

void
MainWindow::msgSentSlot (std::string *key, int *msgind, std::mutex *disp2mtx)
{
  int ind = *msgind;
  std::string keyloc = *key;
  disp2mtx->unlock ();
  sentstatmtx.lock ();
  auto it = std::find_if (
      sentstatus.begin (), sentstatus.end (), [&keyloc, &ind]
      (auto &el)
	{
	  if (std::get<0>(el) == keyloc && std::get<1>(el) == ind)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (it != sentstatus.end ())
    {
      Gtk::Label *lab = std::get<2> (*it);
      lab->set_name ("SentMsg");
    }
  sentstatmtx.unlock ();
}

void
MainWindow::msgRcvdSlot (std::string *key, std::filesystem::path *p,
			 std::mutex *disp3mtx)
{
  std::string keyloc = *key;
  std::filesystem::path ploc = *p;
  disp3mtx->unlock ();
  std::string msgnum = ploc.filename ().u8string ();
  AuxFunc af;
  std::string filename = ploc.parent_path ().u8string ();
  filename = filename + "/Yes";
  std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/YesPr";
  std::filesystem::path outpath = std::filesystem::u8path (filename);
  yesmtx.lock ();
  if (std::filesystem::exists (outpath))
    {
      std::filesystem::remove (outpath);
    }
  std::fstream f;
  if (std::filesystem::exists (source))
    {
      af.decryptFile (Username, Password, source.u8string (),
		      outpath.u8string ());
      f.open (outpath,
	      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
      std::string line;
      line = msgnum;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      time_t curtime = time (NULL);
      strm << curtime;
      line = line + " " + strm.str () + "\n";
      f.write (line.c_str (), line.size ());
      f.close ();
      af.cryptFile (Username, Password, outpath.u8string (),
		    source.u8string ());
      std::filesystem::remove (outpath);
    }
  else
    {
      f.open (outpath, std::ios_base::out | std::ios_base::binary);
      std::string line;
      line = keyloc + "\n";
      f.write (line.c_str (), line.size ());
      line = msgnum;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      time_t curtime = time (NULL);
      strm << curtime;
      line = line + " " + strm.str () + "\n";
      f.write (line.c_str (), line.size ());
      f.close ();
      af.cryptFile (Username, Password, outpath.u8string (),
		    source.u8string ());
      std::filesystem::remove (outpath);
    }
  yesmtx.unlock ();
  Glib::ustring onlt = "";
  Gtk::Label *onl = nullptr;
  chifcmtx.lock ();
  auto itchfc = std::find_if (chifc.begin (), chifc.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    });
  if (itchfc != chifc.end ())
    {
      onlt = std::get<2> (*itchfc)->get_text ();
    }
  frvectmtx.lock ();
  auto frit = std::find_if (friendvect.begin (), friendvect.end (), [&keyloc]
  (auto &el)
    {
      return std::string(std::get<2>(el)->get_text()) == keyloc;
    });

  std::stringstream strm;
  std::locale loc ("C");
  if (frit != friendvect.end ())
    {
      prefvectmtx.lock ();
      auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
      (auto &el)
	{
	  return std::get<0>(el) == "SoundOn";
	});
      if (itprv != prefvect.end ())
	{
	  if (std::get<1> (*itprv) == "active")
	    {
	      if (mf)
		{
		  mf->play ();
		}
	    }
	}
      else
	{
	  if (mf)
	    {
	      mf->play ();
	    }
	}
      prefvectmtx.unlock ();
      auto tup = *frit;
      Gtk::Button *but = std::get<0> (tup);
      Gtk::Requisition rq1, rq2;
      but->get_preferred_size (rq1, rq2);
      friendvect.erase (frit);
      Glib::ustring keyt = std::get<2> (tup)->get_text ();
      Glib::ustring nickt = std::get<4> (tup)->get_text ();
      Glib::ustring namet = std::get<5> (tup)->get_text ();

      fordelgrid->remove (*but);
      but = nullptr;
      but = Gtk::make_managed<Gtk::Button> ();
      Gtk::Grid *ngr = Gtk::make_managed<Gtk::Grid> ();
      but->set_child (*ngr);
      Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create ();
      clck->set_button (3);
      clck->signal_pressed ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::contMenu), clck, but));
      Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
	  Gio::SimpleActionGroup::create ();
      acgroup->add_action (
	  "info",
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::friendDetails), but));
      acgroup->add_action ("delete",
			   sigc::mem_fun (*this, &MainWindow::deleteContact));
      acgroup->add_action ("block",
			   sigc::mem_fun (*this, &MainWindow::tempBlockCont));
      but->insert_action_group ("popupcont", acgroup);
      but->add_controller (clck);
      std::get<0> (tup) = but;
      std::get<1> (tup) = ngr;
      but->set_halign (Gtk::Align::START);
      but->set_size_request (rq2.get_width (), -1);
      but->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

      Gtk::Label *keylab = Gtk::make_managed<Gtk::Label> ();
      keylab->set_text (keyt);
      keylab->set_halign (Gtk::Align::START);
      std::get<2> (tup) = keylab;
      keylab->set_max_width_chars (10);
      keylab->set_ellipsize (Pango::EllipsizeMode::END);

      Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea> ();
      drar->set_margin (2);
      drar->set_halign (Gtk::Align::CENTER);
      drar->set_size_request (50, 50);
      drar->set_draw_func (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::on_draw_contactlist),
		      keylab));
      ngr->attach (*drar, 0, 0, 1, 3);

      ngr->attach (*keylab, 1, 0, 1, 1);

      Gtk::Label *nicklab = Gtk::make_managed<Gtk::Label> ();
      std::get<4> (tup) = nicklab;
      nicklab->set_text (nickt);
      nicklab->set_halign (Gtk::Align::START);
      ngr->attach (*nicklab, 1, 1, 1, 1);

      Gtk::Label *namelab = Gtk::make_managed<Gtk::Label> ();
      std::get<5> (tup) = namelab;
      namelab->set_text (namet);
      namelab->set_halign (Gtk::Align::START);
      ngr->attach (*namelab, 1, 2, 1, 1);

      if (itchfc != chifc.end ())
	{
	  onl = Gtk::make_managed<Gtk::Label> ();
	  onl->set_margin (5);
	  onl->set_halign (Gtk::Align::END);
	  onl->set_name ("connectFr");
	  onl->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  onl->set_text (onlt);
	  ngr->attach_next_to (*onl, *namelab, Gtk::PositionType::RIGHT, 1, 1);
	  std::get<2> (*itchfc) = onl;
	}

      but->signal_clicked ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::selectContact),
		      Winright, std::string (keylab->get_text ()),
		      std::string (nicklab->get_text ())));
      if (fordelkey != keyloc)
	{
	  but->set_name ("inactiveButton");
	}
      else
	{
	  but->set_name ("activeButton");
	  selectedc = but;
	}
      if (friendvect.size () > 0)
	{
	  fordelgrid->insert_next_to (*(std::get<0> (friendvect[0])),
				      Gtk::PositionType::TOP);
	  fordelgrid->attach_next_to (*but, *(std::get<0> (friendvect[0])),
				      Gtk::PositionType::TOP, 1, 1);
	}
      else
	{
	  fordelgrid->attach (*but, 0, 0, 1, 1);
	}

      if (fordelkey != keyloc)
	{
	  Gtk::Label *lab = std::get<3> (tup);
	  if (lab != nullptr)
	    {
	      std::string numb (lab->get_text ());
	      int n;
	      strm.imbue (loc);
	      strm << numb;
	      strm >> n;
	      n = n + 1;
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << n;
	      numb = strm.str ();
	      lab->set_text (Glib::ustring (numb));
	    }
	  else
	    {
	      lab = Gtk::make_managed<Gtk::Label> ();
	      lab->set_text ("1");
	      lab->set_name ("msgNumLab");
	      lab->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      lab->set_margin (2);
	      lab->set_halign (Gtk::Align::END);
	      Gtk::Grid *gr = std::get<1> (tup);
	      gr->insert_next_to (*(std::get<2> (tup)),
				  Gtk::PositionType::RIGHT);
	      gr->attach_next_to (*lab, *(std::get<2> (tup)),
				  Gtk::PositionType::RIGHT, 1, 1);
	      std::get<3> (tup) = lab;
	    }
	}
      else
	{
	  std::get<3> (tup) = nullptr;
	  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	  okp = lt::dht::ed25519_create_keypair (seed);
	  std::string uname;
	  std::string passwd;
	  uname = lt::aux::to_hex (std::get<0> (okp).bytes);
	  lt::dht::public_key pkpass;
	  std::array<char, 32> scalar;
	  lt::aux::from_hex (keyloc, pkpass.bytes.data ());
	  scalar = lt::dht::ed25519_key_exchange (pkpass, std::get<1> (okp));
	  pkpass = lt::dht::ed25519_add_scalar (std::get<0> (okp), scalar);
	  passwd = lt::aux::to_hex (pkpass.bytes);
#ifdef __linux
	  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	  filename = filename + "/CommunistRM/"
	      + ploc.parent_path ().filename ().u8string () + "/"
	      + ploc.filename ().u8string ();
	  std::filesystem::path filepath = std::filesystem::u8path (filename);
	  if (!std::filesystem::exists (filepath.parent_path ()))
	    {
	      std::filesystem::create_directories (filepath.parent_path ());
	    }
	  af.decryptFile (uname, passwd, ploc.u8string (),
			  filepath.u8string ());
	  Gtk::Grid *grmsg = Gtk::make_managed<Gtk::Grid> ();
	  Gtk::Frame *fr = Gtk::make_managed<Gtk::Frame> ();
	  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create ();
	  clck->set_button (0);
	  fr->set_child (*grmsg);
	  fr->set_margin (5);
	  Gtk::Requisition rq1, rq2;
	  Winright->get_preferred_size (rq1, rq2);
	  fr->set_size_request (rq2.get_width () * 0.5, -1);
	  fr->set_halign (Gtk::Align::END);
	  fr->add_controller (clck);
	  fr->set_name ("frMsg");
	  fr->get_style_context ()->add_provider (css_provider,
	  GTK_STYLE_PROVIDER_PRIORITY_USER);

	  Gtk::Label *date = Gtk::make_managed<Gtk::Label> ();
	  date->set_halign (Gtk::Align::START);
	  date->set_margin (2);
	  grmsg->attach (*date, 0, 0, 1, 1);

	  Gtk::Label *from = Gtk::make_managed<Gtk::Label> ();
	  from->set_halign (Gtk::Align::START);
	  from->set_margin (2);
	  grmsg->attach (*from, 0, 1, 1, 1);

	  Gtk::Frame *repfr = Gtk::make_managed<Gtk::Frame> ();
	  repfr->set_halign (Gtk::Align::START);
	  repfr->set_name ("repFrame");
	  repfr->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  repfr->set_margin (2);
	  Gtk::Label *repl = Gtk::make_managed<Gtk::Label> ();
	  repl->set_halign (Gtk::Align::START);
	  repl->set_margin (5);
	  repl->set_use_markup (true);
	  repl->set_max_width_chars (20);
	  repl->set_ellipsize (Pango::EllipsizeMode::END);
	  repfr->set_child (*repl);
	  grmsg->attach (*repfr, 0, 2, 1, 1);
	  int count2 = 0;
	  int type;
	  f.open (filepath, std::ios_base::in);
	  while (!f.eof ())
	    {
	      std::string gl;
	      getline (f, gl);
	      std::string::size_type n;
	      if (gl != "")
		{
		  if (count2 == 0)
		    {
		      n = gl.find (" ", 0);
		      if (n != std::string::npos)
			{
			  Glib::ustring txt (gl.substr (0, gl.find (" ")));
			  txt = "Переслано из: " + txt;
			  from->set_text (txt);
			}
		      else
			{
			  from->set_text (nickt + ":");
			}
		    }
		  n = std::string::npos;
		  if (count2 == 2)
		    {
		      time_t timet = time (NULL);
		      std::string day, month, year, hour, minut, second;
		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gl;
		      strm >> timet;
		      tm *gmtm = localtime (&timet);

		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gmtm->tm_mday;
		      day = strm.str ();
		      if (gmtm->tm_mday < 10)
			{
			  day = "0" + day;
			}

		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gmtm->tm_mon + 1;
		      month = strm.str ();
		      if (gmtm->tm_mon < 10)
			{
			  month = "0" + month;
			}

		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gmtm->tm_year + 1900;
		      year = strm.str ();

		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gmtm->tm_hour;
		      hour = strm.str ();
		      if (gmtm->tm_hour < 10)
			{
			  hour = "0" + hour;
			}

		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gmtm->tm_min;
		      minut = strm.str ();
		      if (gmtm->tm_min < 10)
			{
			  minut = "0" + minut;
			}

		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gmtm->tm_sec;
		      second = strm.str ();
		      if (gmtm->tm_sec < 10)
			{
			  second = "0" + second;
			}

		      Glib::ustring txt (
			  std::string (
			      day + "." + month + "." + year + " " + hour + ":"
				  + minut + ":" + second));
		      date->set_text (txt);
		    }
		  if (count2 == 3)
		    {
		      strm.str ("");
		      strm.clear ();
		      strm.imbue (loc);
		      strm << gl;
		      strm >> type;
		    }
		  if (count2 == 4)
		    {
		      if (gl.size () > 2)
			{
			  std::string line = gl;
			  line = line.substr (2, std::string::npos);
			  repl->set_markup (
			      "<span style=\"italic\">" + Glib::ustring (line)
				  + "</span>");
			}
		      else
			{
			  grmsg->remove_row (2);
			}
		    }
		  if (count2 > 4)
		    {
		      Gtk::Widget *widg = grmsg->get_child_at (0, 3);
		      int rownum;
		      if (widg == nullptr)
			{
			  rownum = 3;
			}
		      else
			{
			  rownum = 4;
			}
		      if (type == 0 || type == 1)
			{
			  Gtk::Label *message =
			      Gtk::make_managed<Gtk::Label> ();
			  message->set_halign (Gtk::Align::START);
			  message->set_margin (5);
			  message->set_justify (Gtk::Justification::LEFT);
			  message->set_wrap (true);
			  message->set_wrap_mode (Pango::WrapMode::WORD);
			  message->set_margin_bottom (10);
			  message->set_text (Glib::ustring (gl));
			  message->set_selectable (true);
			  grmsg->attach (*message, 0, rownum, 1, 1);
			  clck->signal_pressed ().connect (
			      sigc::bind (
				  sigc::mem_fun (*this,
						 &MainWindow::creatReply),
				  fr, message, clck));
			}
		    }
		}
	      count2++;
	    }
	  f.close ();
	  std::filesystem::remove (filepath);
	  msg_grid_vectmtx.lock ();
	  if (msg_grid_vect.size () == 0)
	    {
	      msg_win_gr->attach (*fr, 1, 0, 1, 1);
	      msg_grid_vect.push_back (std::make_tuple (fr, ploc));
	    }
	  else
	    {
	      Gtk::Widget *widg = std::get<0> (
		  msg_grid_vect[msg_grid_vect.size () - 1]);
	      int column, row, width, height;
	      msg_win_gr->query_child (*widg, column, row, width, height);
	      msg_win_gr->attach (*fr, 1, row + 1, width, height);
	      msg_grid_vect.push_back (std::make_tuple (fr, ploc));
	    }
	  if (msg_grid_vect.size () > 20)
	    {
	      Gtk::Widget *widg = std::get<0> (msg_grid_vect[0]);
	      msg_grid_vect.erase (msg_grid_vect.begin ());
	      msg_win_gr->remove (*widg);
	    }
	  msg_grid_vectmtx.unlock ();
	  if (usermsgadj >= 0)
	    {
	      if (msgovllab == nullptr)
		{
		  Gtk::Button *ovlbut = Gtk::make_managed<Gtk::Button> ();
		  ovlbut->set_halign (Gtk::Align::END);
		  ovlbut->set_valign (Gtk::Align::END);
		  ovlbut->set_margin (10);
		  Gtk::Grid *ovlgr = Gtk::make_managed<Gtk::Grid> ();
		  ovlbut->set_child (*ovlgr);
		  Gtk::Label *ovllab = Gtk::make_managed<Gtk::Label> ();
		  ovllab->set_text ("1");
		  msgovllab = ovllab;
		  ovlgr->attach (*ovllab, 0, 0, 1, 1);
		  Gtk::DrawingArea *ovldrar =
		      Gtk::make_managed<Gtk::DrawingArea> ();
		  ovlbut->get_preferred_size (rq1, rq2);
		  ovldrar->set_size_request (rq2.get_width (),
					     rq2.get_height ());
		  ovldrar->set_draw_func (
		      sigc::bind (
			  sigc::mem_fun (*this, &MainWindow::on_draw_sb),
			  Sharepath + "/Arrow_Icon.png"));
		  ovlgr->attach (*ovldrar, 0, 1, 1, 1);

		  ovlbut->signal_clicked ().connect (
		      [this, ovlbut]
		      {
			Glib::RefPtr<Gtk::Adjustment> adj =
			    this->Winright->get_vadjustment ();
			adj->set_value (
			    adj->get_upper () - adj->get_page_size ());
			this->msgovl->remove_overlay (*ovlbut);
			this->msgovllab = nullptr;
		      });
		  msgovl->add_overlay (*ovlbut);
		}
	      else
		{
		  std::string msgval = std::string (msgovllab->get_text ());
		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << msgval;
		  int msgvali;
		  strm >> msgvali;
		  msgvali = msgvali + 1;
		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << msgvali;
		  msgval = strm.str ();
		  msgovllab->set_text (Glib::ustring (msgval));
		}
	    }
	}
      friendvect.insert (friendvect.begin (), tup);
    }
  frvectmtx.unlock ();
  if (itchfc != chifc.end ())
    {
      if (onl != nullptr)
	{
	  std::get<2> (*itchfc) = onl;
	}
      else
	{
	  chifc.erase (itchfc);
	}
    }
  chifcmtx.unlock ();
}

void
MainWindow::editProfile ()
{
  profilevector.clear ();
  AuxFunc af;
  std::string filename;
  af.homePath (&filename);
  filename = filename + "/.Communist/Profile";
  std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/CommunistEProf/Profile.zip";
  std::filesystem::path outpath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (outpath.parent_path ()))
    {
      std::filesystem::remove_all (outpath.parent_path ());
    }
  std::filesystem::create_directories (outpath.parent_path ());
  proffopmtx.lock ();
  af.decryptFile (Username, Password, source.u8string (), outpath.u8string ());
  proffopmtx.unlock ();
  af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
  filename = outpath.parent_path ().u8string ();
  filename = filename + "/Profile/Profile";
  source = std::filesystem::u8path (filename);
  std::string line;
  std::vector<std::string> profvect;
  std::fstream f;
  f.open (source, std::ios_base::in);

  while (!f.eof ())
    {
      getline (f, line);
      if (line != "")
	{
	  profvect.push_back (line);
	}
    }
  f.close ();

  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  window->set_title (gettext ("Edit profile"));
  window->set_child (*grid);

  Gtk::Grid *leftgrid = Gtk::make_managed<Gtk::Grid> ();
  Gtk::Grid *rightgrid = Gtk::make_managed<Gtk::Grid> ();

  grid->attach (*leftgrid, 0, 0, 1, 1);
  grid->attach (*rightgrid, 1, 0, 1, 1);

  Gtk::Entry *nickname = Gtk::make_managed<Gtk::Entry> ();
  nickname->set_text (Glib::ustring (profvect[0]));
  nickname->set_margin (5);
  rightgrid->attach (*nickname, 0, 0, 3, 1);
  profilevector.push_back (nickname);

  Gtk::Entry *name = Gtk::make_managed<Gtk::Entry> ();
  if (profvect.size () >= 2)
    {
      name->set_text (Glib::ustring (profvect[1]));
    }
  name->set_placeholder_text (gettext ("Name (not compalsory)"));
  name->set_margin (5);
  rightgrid->attach (*name, 0, 1, 3, 1);
  profilevector.push_back (name);

  Gtk::Entry *surname = Gtk::make_managed<Gtk::Entry> ();
  if (profvect.size () >= 3)
    {
      surname->set_text (profvect[2]);
    }
  surname->set_placeholder_text (gettext ("Surname (not compalsory)"));
  surname->set_margin (5);
  rightgrid->attach (*surname, 0, 2, 3, 1);
  profilevector.push_back (surname);

  Gtk::Button *saveprof = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *spl = Gtk::make_managed<Gtk::Label> ();
  spl->set_text (gettext ("Save profile"));
  spl->set_justify (Gtk::Justification::CENTER);
  saveprof->set_child (*spl);
  saveprof->set_margin (5);
  saveprof->set_halign (Gtk::Align::CENTER);
  saveprof->set_name ("applyButton");
  saveprof->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  saveprof->signal_clicked ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::saveProfileEP), window));
  rightgrid->attach (*saveprof, 0, 3, 1, 1);

  Gtk::Button *cleardata = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *cdl = Gtk::make_managed<Gtk::Label> ();
  cdl->set_text (gettext ("Clear"));
  cdl->set_justify (Gtk::Justification::CENTER);
  cleardata->set_child (*cdl);
  cleardata->set_margin (5);
  cleardata->set_halign (Gtk::Align::CENTER);
  cleardata->set_name ("clearButton");
  cleardata->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  cleardata->signal_clicked ().connect ( [nickname, name, surname]
  {
    nickname->set_text ("");
    name->set_text ("");
    surname->set_text ("");
  });
  rightgrid->attach (*cleardata, 1, 3, 1, 1);

  Gtk::Button *cancela = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *cal = Gtk::make_managed<Gtk::Label> ();
  cal->set_text (gettext ("Cancel"));
  cal->set_justify (Gtk::Justification::CENTER);
  cancela->set_child (*cal);
  cancela->set_margin (5);
  cancela->set_halign (Gtk::Align::CENTER);
  cancela->set_name ("rejectButton");
  cancela->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancela->signal_clicked ().connect (
      sigc::mem_fun (*window, &Gtk::Window::close));
  rightgrid->attach (*cancela, 2, 3, 1, 1);

  avatar = Gtk::make_managed<Gtk::DrawingArea> ();
  avatar->set_margin (5);
  avatar->set_halign (Gtk::Align::CENTER);
  avatar->set_size_request (200, 200);
  filename = source.parent_path ().u8string ();
  filename = filename + "/Avatar.jpeg";
  source = std::filesystem::u8path (filename);
  if (std::filesystem::exists (source))
    {
      avatar->set_draw_func (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::on_draw_ep),
		      Glib::ustring (source.u8string ())));
    }
  leftgrid->attach (*avatar, 0, 0, 2, 1);

  Gtk::Button *addavatar = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *oblab = Gtk::make_managed<Gtk::Label> ();
  oblab->set_text (gettext ("Open avatar"));
  oblab->set_justify (Gtk::Justification::CENTER);
  addavatar->set_child (*oblab);
  addavatar->set_margin (5);
  addavatar->set_halign (Gtk::Align::CENTER);
  addavatar->set_name ("applyButton");
  addavatar->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  addavatar->signal_clicked ().connect (
      [this, window]
      {
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog (
	    gettext ("Avatar file choice"), Gtk::FileChooser::Action::OPEN);
	dialog->set_transient_for (*window);
	Gtk::Button *but;
	but = dialog->add_button (gettext ("Cancel"),
				  Gtk::ResponseType::CANCEL);
	but->set_margin (5);
	but = dialog->add_button (gettext ("Choose"), Gtk::ResponseType::OK);
	but->set_margin (5);
	dialog->signal_response ().connect ( [dialog, this]
	(int respons_id)
	  {

	    Glib::ustring file;
	    if (respons_id == Gtk::ResponseType::OK)
	      {
		file = dialog->get_file ()->get_path ();
		this->avatar->set_draw_func (
		    sigc::bind (sigc::mem_fun (*this,
			    &MainWindow::on_draw),
			file));
	      }
	    dialog->hide ();
	    delete dialog;
	  });
	Glib::RefPtr<Gtk::Application> app = this->get_application ();
	app->add_window (*dialog);
	dialog->show ();
      });
  leftgrid->attach (*addavatar, 0, 1, 1, 1);

  Gtk::Button *cleardraw = Gtk::make_managed<Gtk::Button> ();
  Gtk::Label *oblab2 = Gtk::make_managed<Gtk::Label> ();
  oblab2->set_text (gettext ("Remove avatar"));
  oblab2->set_justify (Gtk::Justification::CENTER);
  cleardraw->set_child (*oblab2);
  cleardraw->set_margin (5);
  cleardraw->set_halign (Gtk::Align::CENTER);
  cleardraw->set_name ("clearButton");
  cleardraw->get_style_context ()->add_provider (
      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  cleardraw->signal_clicked ().connect (
      sigc::bind (sigc::mem_fun (*this, &MainWindow::clearAvatar), leftgrid));
  leftgrid->attach (*cleardraw, 1, 1, 1, 1);

  window->signal_close_request ().connect ( [window, &af, this]
  {
    this->profilevector.clear ();
    std::string filename;
#ifdef __linux
					     filename =
						 std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
    filename = filename + "/CommunistEProf";
    std::filesystem::path p = std::filesystem::u8path (filename);
    if (std::filesystem::exists (p))
      {
	std::filesystem::remove_all (p);
      }
    window->hide ();
    delete window;
    this->avatar = nullptr;
    return true;
  },
  false);
  window->show ();
}

void
MainWindow::saveProfileEP (Gtk::Window *windowb)
{
  Gtk::Entry *ent = profilevector[0];
  if (ent->get_text () == "")
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application (this->get_application ());
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      window->set_title (gettext ("Error"));
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      window->set_child (*grid);

      Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
      label->set_text (gettext ("Input nickname!"));
      label->set_halign (Gtk::Align::CENTER);
      label->set_margin (5);
      grid->attach (*label, 0, 0, 1, 1);

      Gtk::Button *button = Gtk::make_managed<Gtk::Button> ();
      button->set_label (gettext ("Close"));
      button->set_halign (Gtk::Align::CENTER);
      button->set_margin (5);
      button->set_name ("applyButton");
      button->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      button->signal_clicked ().connect (
	  sigc::mem_fun (*window, &Gtk::Window::close));
      grid->attach (*button, 0, 1, 1, 1);
      window->signal_close_request ().connect ( [window]
      {
	window->hide ();
	delete window;
	return true;
      },
					       false);
      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size (rq1, rq2);
      window->set_default_size (rq2.get_width () + 10, rq2.get_height () + 10);
      window->show ();
    }
  else
    {
      AuxFunc af;
      std::string filename;
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/CommunistEProf/Profile.zip";
      std::filesystem::path outpath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (outpath.parent_path ()))
	{
	  std::filesystem::remove_all (outpath.parent_path ());
	}
      std::filesystem::create_directories (outpath.parent_path ());
      proffopmtx.lock ();
      af.decryptFile (Username, Password, source.u8string (),
		      outpath.u8string ());
      proffopmtx.unlock ();
      af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
      std::filesystem::remove (outpath);
      filename = outpath.parent_path ().u8string ();
      filename = filename + "/Profile/Profile";
      source = std::filesystem::u8path (filename);
      std::string line;
      std::fstream f;
      f.open (source, std::ios_base::out | std::ios_base::binary);
      for (size_t i = 0; i < profilevector.size (); i++)
	{
	  Gtk::Entry *ent = profilevector[i];
	  line = std::string (ent->get_text ());
	  line = line + "\n";
	  f.write (line.c_str (), line.size ());
	}
      f.close ();
      if (Image > 0)
	{
	  filename = source.parent_path ().u8string ();
	  filename = filename + "/Avatar.jpeg";
	  source = std::filesystem::u8path (filename);
	  if (std::filesystem::exists (source))
	    {
	      std::filesystem::remove (source);
	    }
	  image->save (source.string (), "jpeg");
	  Image = 0;
	}
      else
	{
	  filename = source.parent_path ().u8string ();
	  filename = filename + "/Avatar.jpeg";
	  source = std::filesystem::u8path (filename);
	  if (std::filesystem::exists (source))
	    {
	      std::filesystem::remove (source);
	    }
	}
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/CommunistEProf/Profile";
      source = std::filesystem::u8path (filename);
      filename = filename + ".zip";
      outpath = std::filesystem::u8path (filename);
      af.packing (source.u8string (), outpath.u8string ());
      source = outpath;
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      outpath = std::filesystem::u8path (filename);
      proffopmtx.lock ();
      af.cryptFile (Username, Password, source.u8string (),
		    outpath.u8string ());
      contmtx.lock ();
      for (size_t i = 0; i < contacts.size (); i++)
	{
	  std::string key = std::get<1> (contacts[i]);
	  oper->renewProfile (key);
	}
      contmtx.unlock ();
      proffopmtx.unlock ();
      windowb->close ();
    }
}

void
MainWindow::attachFileDialog ()
{
  if (selectedc != nullptr)
    {
      Gtk::FileChooserDialog *fcd = new Gtk::FileChooserDialog (
	  *this, gettext ("File choose"), Gtk::FileChooser::Action::OPEN);
      fcd->set_transient_for (*this);
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path (filename);
      fcd->set_current_folder (fl);
      fcd->set_application (this->get_application ());

      Gtk::Button *btn;
      btn = fcd->add_button (gettext ("Open"), Gtk::ResponseType::APPLY);
      btn->set_halign (Gtk::Align::END);
      btn->set_margin (5);
      btn = fcd->add_button (gettext ("Cancel"), Gtk::ResponseType::CANCEL);
      btn->set_halign (Gtk::Align::START);
      btn->set_margin (5);
      fcd->signal_response ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MainWindow::attachFileFunc), fcd));
      fcd->signal_close_request ().connect ( [fcd]
      {
	fcd->hide ();
	delete fcd;
	return true;
      },
					    false);

      fcd->show ();
    }
}
void
MainWindow::attachFileFunc (int rid, Gtk::FileChooserDialog *fcd)
{
  if (rid == Gtk::ResponseType::CANCEL)
    {
      fcd->close ();
    }
  if (rid == Gtk::ResponseType::APPLY)
    {
      Glib::RefPtr<Gio::File> fl = fcd->get_file ();
      fcd->close ();
      std::string filename = fl->get_path ();
      frvectmtx.lock ();
      auto itfr = std::find_if (friendvect.begin (), friendvect.end (), [this]
      (auto &el)
	{
	  return std::get<0>(el) == this->selectedc;
	});
      if (itfr != friendvect.end ())
	{
	  std::string key (std::get<2> (*itfr)->get_text ());
	  if (Rightgrid != nullptr && attachfbutton != nullptr
	      && attachedfile == nullptr)
	    {
	      attach = Gtk::make_managed<Gtk::Label> ();
	      attach->set_text (gettext (" File: "));
	      attach->set_halign (Gtk::Align::START);
	      attach->set_margin (5);
	      attach->set_name ("highlightText");
	      attach->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      Rightgrid->insert_next_to (*attachfbutton,
					 Gtk::PositionType::BOTTOM);
	      Rightgrid->attach_next_to (*attach, *attachfbutton,
					 Gtk::PositionType::BOTTOM, 1, 1);

	      attachedfile = Gtk::make_managed<Gtk::Label> ();
	      attachedfile->set_text (Glib::ustring (filename));
	      attachedfile->set_halign (Gtk::Align::START);
	      attachedfile->set_margin (5);
	      attachedfile->set_ellipsize (Pango::EllipsizeMode::START);
	      attachedfile->set_name ("fileAddress");
	      attachedfile->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      Rightgrid->insert_next_to (*attachfbutton,
					 Gtk::PositionType::BOTTOM);
	      Rightgrid->attach_next_to (*attachedfile, *attach,
					 Gtk::PositionType::RIGHT, 1, 1);

	      attachcancel = Gtk::make_managed<Gtk::Button> ();
	      attachcancel->set_valign (Gtk::Align::CENTER);
	      attachcancel->set_halign (Gtk::Align::END);
	      attachcancel->set_margin (5);
	      attachcancel->set_name ("cancelRepl");
	      attachcancel->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      attachcancel->set_icon_name ("window-close-symbolic");
	      attachcancel->signal_clicked ().connect ( [this]
	      {
		this->Rightgrid->remove (*(this->attachedfile));
		this->Rightgrid->remove (*(this->attachcancel));
		this->Rightgrid->remove (*this->attach);
		this->attachedfile = nullptr;
		this->attachcancel = nullptr;
		this->attach = nullptr;
	      });
	      Rightgrid->attach_next_to (*attachcancel, *attachedfile,
					 Gtk::PositionType::RIGHT, 1, 1);
	    }
	}
      frvectmtx.unlock ();
    }
}

void
MainWindow::fileRequestSlot (std::string *key, uint64_t *tm, uint64_t *filesize,
			     std::string *filename, std::mutex *disp4mtx)
{
  std::string keyloc = *key;
  uint64_t tml = *tm;
  uint64_t filesizeloc = *filesize;
  std::string fnmloc = *filename;
  disp4mtx->unlock ();
  blockfreqmtx.lock ();
  int cheknum = 0;
  time_t curtime = time (NULL);
  blockfreq.erase (
      std::remove_if (blockfreq.begin (), blockfreq.end (), [&curtime]
      (auto &el)
	{
	  if (curtime - std::get<1>(el) > 1200 && std::get<2>(el) != 0)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
      blockfreq.end ());
  for (size_t i = 0; i < blockfreq.size (); i++)
    {
      if (std::get<0> (blockfreq[i]) == keyloc
	  && std::get<2> (blockfreq[i]) == 1)
	{
	  if (tml - std::get<1> (blockfreq[i]) < 600)
	    {
	      cheknum++;
	    }
	}
    }
  if (cheknum <= 5)
    {
      auto it = std::find_if (
	  blockfreq.begin (), blockfreq.end (), [&keyloc, &tml]
	  (auto &el)
	    {
	      if (std::get<0>(el) == keyloc && std::get<1>(el) == tml)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    });
      if (it == blockfreq.end ())
	{
	  frvectmtx.lock ();
	  auto itfrv = std::find_if (
	      friendvect.begin (), friendvect.end (), [&keyloc]
	      (auto &el)
		{
		  Gtk::Label *lab = std::get<2>(el);
		  std::string sk = std::string (lab->get_text());
		  if (sk == keyloc)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		});
	  if (itfrv != friendvect.end ())
	    {
	      AuxFunc af;
	      std::string defs;
	      af.homePath (&defs);
	      std::filesystem::path def = std::filesystem::u8path (defs);
	      blockfreq.push_back (std::make_tuple (keyloc, tml, 0, def));
	      Glib::ustring nick (std::get<4> (*itfrv)->get_text ());
	      Gtk::Window *window = new Gtk::Window;
	      window->set_application (this->get_application ());
	      window->set_name ("settingsWindow");
	      window->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      window->set_title (gettext ("File request"));
	      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
	      grid->set_halign (Gtk::Align::CENTER);
	      window->set_child (*grid);

	      Gtk::Label *lab1 = Gtk::make_managed<Gtk::Label> ();
	      std::stringstream strm;
	      std::locale loc ("C");
	      strm.imbue (loc);
	      uint64_t fs = filesizeloc;
	      double fsd;
	      std::string fsstr;
	      if (fs > 1024)
		{
		  fsd = (static_cast<double> (fs) / static_cast<double> (1024));
		  strm << std::fixed << std::setprecision (2) << fsd;
		  fsstr = strm.str () + " kB";
		  if (fs > 1024 * 1024)
		    {
		      fsd = (static_cast<double> (fs)
			  / static_cast<double> (1024 * 1024));
		      strm.clear ();
		      strm.str ("");
		      strm.imbue (loc);
		      strm << std::setprecision (2) << fsd;
		      fsstr = strm.str () + " MB";
		      if (fs > 1024 * 1024 * 1024)
			{
			  fsd = (static_cast<double> (fs)
			      / static_cast<double> (1024 * 1024 * 1024));
			  strm.clear ();
			  strm.str ("");
			  strm.imbue (loc);
			  strm << std::setprecision (2) << fsd;
			  fsstr = strm.str () + " GB";
			}
		    }
		}
	      else
		{
		  strm << std::fixed << fs;
		  fsstr = strm.str () + "B";
		}

	      lab1->set_text (
		  nick + gettext (" is trying to send you a file."));
	      lab1->set_margin_top (5);
	      lab1->set_margin_start (5);
	      lab1->set_margin_end (5);
	      lab1->set_halign (Gtk::Align::CENTER);
	      grid->attach (*lab1, 0, 0, 2, 1);

	      Gtk::Label *lab2 = Gtk::make_managed<Gtk::Label> ();
	      lab2->set_margin_start (5);
	      lab2->set_margin_end (5);
	      lab2->set_halign (Gtk::Align::CENTER);
	      lab2->set_text (Glib::ustring (fnmloc));
	      grid->attach (*lab2, 0, 1, 2, 1);

	      Gtk::Label *lab3 = Gtk::make_managed<Gtk::Label> ();
	      lab3->set_margin_start (5);
	      lab3->set_margin_end (5);
	      lab3->set_halign (Gtk::Align::CENTER);
	      lab3->set_text (gettext ("Size: ") + Glib::ustring (fsstr));
	      grid->attach (*lab3, 0, 2, 2, 1);

	      Gtk::Label *lab4 = Gtk::make_managed<Gtk::Label> ();
	      lab4->set_margin_start (5);
	      lab4->set_margin_end (5);
	      lab4->set_margin_bottom (5);
	      lab4->set_halign (Gtk::Align::CENTER);
	      lab4->set_text (gettext ("Accept?"));
	      grid->attach (*lab4, 0, 3, 2, 1);

	      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
	      yes->set_label (gettext ("Yes"));
	      yes->set_margin (5);
	      yes->set_halign (Gtk::Align::CENTER);
	      yes->set_name ("applyButton");
	      yes->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      grid->attach (*yes, 0, 4, 1, 1);

	      yes->signal_clicked ().connect (
		  [window, this, keyloc, tml, fnmloc, fs]
		  {
		    Gtk::FileChooserDialog *fcd = new Gtk::FileChooserDialog (
			*this, gettext ("Path selection"),
			Gtk::FileChooser::Action::SELECT_FOLDER);
		    fcd->set_transient_for (*this);
		    std::string filename;
		    AuxFunc af;
		    af.homePath (&filename);
		    Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path (
			filename);
		    fcd->set_current_folder (fl);
		    fcd->set_application (this->get_application ());
		    Gtk::Button *btn;
		    btn = fcd->add_button (gettext ("Open"),
					   Gtk::ResponseType::APPLY);
		    btn->set_halign (Gtk::Align::END);
		    btn->set_margin (5);
		    btn = fcd->add_button (gettext ("Cancel"),
					   Gtk::ResponseType::APPLY);
		    btn->set_halign (Gtk::Align::START);
		    btn->set_margin (5);
		    fcd->signal_response ().connect (
			[fcd, this, keyloc, tml, fnmloc, fs]
			(int rid)
			  {
			    if (rid == Gtk::ResponseType::APPLY)
			      {
				AuxFunc af;
				std::string loc;
				Glib::RefPtr<Gio::File> fl = fcd->get_file ();
				if (fl)
				  {
				    loc = fl->get_path() + "/" + fnmloc;
				  }
				else
				  {
				    fl = fcd->get_current_folder();
				    if (fl)
				      {
					loc = fl->get_path() + "/" + fnmloc;
				      }
				    else
				      {
					af.homePath(&loc);
					loc = loc + "/" + fnmloc;
				      }
				  }
				std::filesystem::path fp = std::filesystem::u8path(loc);
				if (std::filesystem::exists(fp))
				  {
				    Gtk::Window *window = new Gtk::Window;
				    window->set_application(this->get_application());
				    window->set_name("settingsWindow");
				    window->get_style_context ()->add_provider (
					this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
				    window->set_title(gettext("Warning!"));
				    window->set_modal(true);
				    window->set_transient_for(*fcd);
				    Gtk::Grid *gr = Gtk::make_managed<Gtk::Grid>();
				    gr->set_halign(Gtk::Align::CENTER);
				    window->set_child(*gr);

				    Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
				    lab->set_halign(Gtk::Align::CENTER);
				    lab->set_margin(5);
				    lab->set_text(gettext("File already exists, replace?"));
				    gr->attach(*lab, 0, 0, 2, 1);

				    Gtk::Button *accept = Gtk::make_managed<Gtk::Button>();
				    accept->set_halign(Gtk::Align::CENTER);
				    accept->set_margin(5);
				    accept->set_name ("applyButton");
				    accept->get_style_context ()->add_provider (
					this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
				    accept->set_label(gettext("Yes"));
				    accept->signal_clicked().connect([window, fcd, fp, this, keyloc, tml, fs]
					  {
					    std::filesystem::remove (fp);
					    this->blockfreqmtx.lock ();
					    auto it =
					    std::find_if (
						this->blockfreq.begin (),
						this->blockfreq.end (),
						[keyloc, tml]
						(auto &el)
						  {
						    if (std::get<0>(el) == keyloc && std::get<1>(el) == tml)
						      {
							return true;
						      }
						    else
						      {
							return false;
						      }
						  });
					    if (it != this->blockfreq.end ())
					      {
						std::get<2> (*it) = 2;
						std::get<3>(*it) = fp;
						Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
						lab->set_halign(Gtk::Align::CENTER);
						lab->set_valign(Gtk::Align::START);
						lab->set_margin_start(5);
						lab->set_margin_end(5);
						lab->set_text(gettext("Downloading ") + Glib::ustring(fp.filename().u8string()));
						Gtk::ProgressBar *frprgb = Gtk::make_managed<Gtk::ProgressBar>();
						frprgb->set_halign (Gtk::Align::CENTER);
						frprgb->set_valign (Gtk::Align::START);
						frprgb->set_margin (5);
						frprgb->set_show_text (true);
						frprgb->set_name("fileRPrB");
						frprgb->get_style_context ()->add_provider (css_provider,
						    GTK_STYLE_PROVIDER_PRIORITY_USER);
						frprgb->set_fraction (0);

						Gtk::Button *cncl = Gtk::make_managed<Gtk::Button>();
						cncl->set_halign(Gtk::Align::CENTER);
						cncl->set_margin(5);
						cncl->set_name ("cancelRepl");
						cncl->get_style_context ()->add_provider (
						    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
						cncl->set_icon_name ("window-close-symbolic");

						cncl->signal_clicked ().connect ( [this, keyloc, fp]
						      {
							this->fileprogrvectmtx.lock ();
							auto it = std::find_if (this->fileprogrvect.begin (), this->fileprogrvect.end (), [keyloc, fp]
							    (auto &el)
							      {
								if (std::get<0>(el) == keyloc && std::get<1>(el) == fp)
								  {
								    return true;
								  }
								else
								  {
								    return false;
								  }
							      });
							if (it != this->fileprogrvect.end ())
							  {
							    this->dld_grid->remove (*(std::get<3> (*it)));
							    this->dld_grid->remove (*(std::get<4> (*it)));
							    this->dld_grid->remove (*(std::get<5> (*it)));
							    this->fileprogrvect.erase (it);
							  }
							if (this->fileprogrvect.size () == 0)
							  {
							    this->dld_win->hide ();
							    delete this->dld_win;
							    this->dld_win = nullptr;
							    this->dld_grid = nullptr;
							  }
							this->fileprogrvectmtx.unlock ();
							this->oper->cancelReceivF(keyloc, fp);
						      });

						this->fileprogrvectmtx.lock ();
						std::tuple<std::string, std::filesystem::path, uint64_t, Gtk::ProgressBar*,
						Gtk::Label*, Gtk::Button*>prbtup;
						std::get<0>(prbtup) = keyloc;
						std::get<1>(prbtup) = fp;
						std::get<2>(prbtup) = fs;
						std::get<3>(prbtup) = frprgb;
						std::get<4>(prbtup) = lab;
						std::get<5>(prbtup) = cncl;
						if (this->fileprogrvect.size () > 0)
						  {
						    Gtk::Widget *sibl = std::get<3>(this->fileprogrvect[this->fileprogrvect.size() - 1]);
						    this->dld_grid->attach_next_to (*lab, *sibl, Gtk::PositionType::BOTTOM, 1, 1);
						    sibl = lab;
						    this->dld_grid->attach_next_to (*frprgb, *sibl, Gtk::PositionType::BOTTOM, 1, 1);
						    sibl = frprgb;
						    this->dld_grid->attach_next_to (*cncl, *sibl, Gtk::PositionType::RIGHT, 1, 1);
						  }
						else
						  {
						    this->dld_win = new Gtk::Window;
						    this->dld_win->set_application(this->get_application());
						    this->dld_win->set_deletable(false);
						    this->dld_win->set_name ("settingsWindow");
						    this->dld_win->get_style_context ()->add_provider (
							this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

						    this->dld_win->set_title(gettext("Downloading/sending file progress"));
						    this->dld_grid = Gtk::make_managed<Gtk::Grid>();
						    this->dld_grid->set_halign(Gtk::Align::CENTER);
						    this->dld_win->set_child(*(this->dld_grid));
						    this->dld_grid->attach(*lab, 0, 0, 1, 1);
						    this->dld_grid->attach(*frprgb, 0, 1, 1, 1);
						    this->dld_grid->attach(*cncl, 1, 1, 1, 1);
						    this->dld_win->show();
						  }
						this->fileprogrvect.push_back (prbtup);
						this->fileprogrvectmtx.unlock();
						this->oper->fileAccept (keyloc, tml, fp);
					      }
					    this->blockfreqmtx.unlock ();
					    window->close();
					    fcd->close ();
					  });
				    gr->attach(*accept, 0, 1, 1, 1);

				    Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
				    cancel->set_halign(Gtk::Align::CENTER);
				    cancel->set_margin(5);
				    cancel->set_name ("rejectButton");
				    cancel->get_style_context ()->add_provider (
					this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
				    cancel->set_label(gettext("No"));
				    cancel->signal_clicked().connect([window]
					  {
					    window->close();
					  });
				    gr->attach(*cancel, 1, 1, 1, 1);

				    window->signal_close_request().connect([window]
					  {
					    window->hide();
					    delete window;
					    return true;
					  }, false);
				    window->show();
				  }
				else
				  {
				    this->blockfreqmtx.lock ();
				    auto it =
				    std::find_if (
					this->blockfreq.begin (),
					this->blockfreq.end (),
					[keyloc, tml]
					(auto &el)
					  {
					    if (std::get<0>(el) == keyloc && std::get<1>(el) == tml)
					      {
						return true;
					      }
					    else
					      {
						return false;
					      }
					  });
				    if (it != this->blockfreq.end ())
				      {
					std::get<2> (*it) = 2;
					std::get<3>(*it) = fp;
					Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
					lab->set_halign(Gtk::Align::CENTER);
					lab->set_valign(Gtk::Align::START);
					lab->set_margin_start(5);
					lab->set_margin_end(5);
					lab->set_text(gettext("Downloading ") + Glib::ustring(fp.filename().u8string()));
					Gtk::ProgressBar *frprgb = Gtk::make_managed<Gtk::ProgressBar>();
					frprgb->set_halign (Gtk::Align::CENTER);
					frprgb->set_valign (Gtk::Align::START);
					frprgb->set_margin (5);
					frprgb->set_show_text (true);
					frprgb->set_name("fileRPrB");
					frprgb->get_style_context ()->add_provider (css_provider,
					    GTK_STYLE_PROVIDER_PRIORITY_USER);
					frprgb->set_fraction (0);

					Gtk::Button *cncl = Gtk::make_managed<Gtk::Button>();
					cncl->set_halign(Gtk::Align::CENTER);
					cncl->set_margin(5);
					cncl->set_name ("cancelRepl");
					cncl->get_style_context ()->add_provider (
					    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
					cncl->set_icon_name ("window-close-symbolic");
					cncl->signal_clicked ().connect ( [this, keyloc, fp]
					      {
						this->fileprogrvectmtx.lock ();
						auto it = std::find_if (this->fileprogrvect.begin (), this->fileprogrvect.end (), [keyloc, fp]
						    (auto &el)
						      {
							if (std::get<0>(el) == keyloc && std::get<1>(el) == fp)
							  {
							    return true;
							  }
							else
							  {
							    return false;
							  }
						      });
						if (it != this->fileprogrvect.end ())
						  {
						    this->dld_grid->remove (*(std::get<3> (*it)));
						    this->dld_grid->remove (*(std::get<4> (*it)));
						    this->dld_grid->remove (*(std::get<5> (*it)));
						    this->fileprogrvect.erase (it);
						  }
						if (this->fileprogrvect.size () == 0)
						  {
						    this->dld_win->hide ();
						    delete this->dld_win;
						    this->dld_win = nullptr;
						    this->dld_grid = nullptr;
						  }
						this->fileprogrvectmtx.unlock ();
						this->oper->cancelReceivF(keyloc, fp);
					      });

					this->fileprogrvectmtx.lock ();
					std::tuple<std::string, std::filesystem::path, uint64_t, Gtk::ProgressBar*,
					Gtk::Label*, Gtk::Button*>prbtup;
					std::get<0>(prbtup) = keyloc;
					std::get<1>(prbtup) = fp;
					std::get<2>(prbtup) = fs;
					std::get<3>(prbtup) = frprgb;
					std::get<4>(prbtup) = lab;
					std::get<5>(prbtup) = cncl;
					if (this->fileprogrvect.size () > 0)
					  {
					    Gtk::Widget *sibl = std::get<3>(this->fileprogrvect[this->fileprogrvect.size() - 1]);
					    this->dld_grid->attach_next_to (*lab, *sibl, Gtk::PositionType::BOTTOM, 1, 1);
					    sibl = lab;
					    this->dld_grid->attach_next_to (*frprgb, *sibl, Gtk::PositionType::BOTTOM, 1, 1);
					    sibl = frprgb;
					    this->dld_grid->attach_next_to (*cncl, *sibl, Gtk::PositionType::RIGHT, 1, 1);
					  }
					else
					  {
					    this->dld_win = new Gtk::Window;
					    this->dld_win->set_application(this->get_application());
					    this->dld_win->set_name ("settingsWindow");
					    this->dld_win->get_style_context ()->add_provider (
						this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
					    this->dld_win->set_deletable(false);
					    this->dld_win->set_title(gettext("Downloading/sending file progress"));
					    this->dld_grid = Gtk::make_managed<Gtk::Grid>();
					    this->dld_grid->set_halign(Gtk::Align::CENTER);
					    this->dld_win->set_child(*(this->dld_grid));
					    this->dld_grid->attach(*lab, 0, 0, 1, 1);
					    this->dld_grid->attach(*frprgb, 0, 1, 1, 1);
					    this->dld_grid->attach(*cncl, 1, 1, 1, 1);
					    this->dld_win->show();
					  }
					this->fileprogrvect.push_back (prbtup);
					this->fileprogrvectmtx.unlock();
					this->oper->fileAccept (keyloc, tml, fp);
				      }
				    this->blockfreqmtx.unlock ();
				    fcd->close ();
				  }

			      }
			    if (rid == Gtk::ResponseType::CANCEL)
			      {
				this->blockfreqmtx.lock ();
				auto it = std::find_if (this->blockfreq.begin (), this->blockfreq.end (),
				    [keyloc, tml]
				    (auto &el)
				      {
					if (std::get<0>(el) == keyloc && std::get<1>(el) == tml)
					  {
					    return true;
					  }
					else
					  {
					    return false;
					  }
				      });
				if (it != this->blockfreq.end ())
				  {
				    std::get<2> (*it) = 1;
				    this->oper->fileReject (keyloc, tml);
				  }
				this->blockfreqmtx.unlock ();
				fcd->close();
			      }
			  });
		    fcd->signal_close_request ().connect ( [fcd]
		    {
		      fcd->hide ();
		      delete fcd;
		      return true;
		    },
							  false);

		    fcd->show ();

		    window->close ();
		  });

	      Gtk::Button *no = Gtk::make_managed<Gtk::Button> ();
	      no->set_label (gettext ("No"));
	      no->set_margin (5);
	      no->set_halign (Gtk::Align::CENTER);
	      no->set_name ("rejectButton");
	      no->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      grid->attach (*no, 1, 4, 1, 1);

	      no->signal_clicked ().connect ( [window, this, keyloc, tml]
	      {
		this->blockfreqmtx.lock ();
		auto it = std::find_if (this->blockfreq.begin (), this->blockfreq.end (),
	      [keyloc, tml]
	      (auto &el)
		{
		  if (std::get<0>(el) == keyloc && std::get<1>(el) == tml)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		});
		if (it != this->blockfreq.end ())
		  {
		    std::get<2> (*it) = 1;
		    this->oper->fileReject (keyloc, tml);
		  }
		this->blockfreqmtx.unlock ();
		window->close ();
	      });
	      window->signal_close_request ().connect ( [window]
	      {
		window->hide ();
		delete window;
		return true;
	      },
						       false);

	      window->show ();
	    }
	  frvectmtx.unlock ();
	}
      else
	{
	  if (std::get<2> (*it) == 1)
	    {
	      oper->fileReject (keyloc, tml);
	    }
	  if (std::get<2> (*it) == 2)
	    {
	      std::filesystem::path fp = std::get<3> (*it);
	      oper->fileAccept (keyloc, tml, fp);
	    }
	}
    }
  blockfreqmtx.unlock ();
}

void
MainWindow::fileRejectedSlot (std::string *keyr, std::mutex *disp5mtx)
{
  std::string key = *keyr;
  disp5mtx->unlock ();
  frvectmtx.lock ();
  auto itfrv = std::find_if (friendvect.begin (), friendvect.end (), [&key]
  (auto &el)
    {
      return key == std::string (std::get<2>(el)->get_text());
    });
  if (itfrv != friendvect.end ())
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_title (gettext ("File rejected"));
      window->set_transient_for (*this);
      window->set_modal (true);
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      window->set_child (*grid);

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      Glib::ustring lstr;
      lstr = std::get<4> (*itfrv)->get_text ();
      lstr = lstr + gettext (" rejected to accept file!");
      lab->set_text (lstr);
      lab->set_halign (Gtk::Align::CENTER);
      lab->set_margin (5);
      grid->attach (*lab, 0, 0, 1, 1);

      Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
      close->set_label (gettext ("Close"));
      close->set_halign (Gtk::Align::CENTER);
      close->set_margin (5);
      close->set_name ("applyButton");
      close->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      grid->attach (*close, 0, 1, 1, 1);

      close->signal_clicked ().connect (
	  sigc::mem_fun (*window, &Gtk::Window::close));
      window->signal_close_request ().connect ( [window]
      {
	window->hide ();
	delete window;
	return true;
      },
					       false);

      window->set_application (this->get_application ());
      window->show ();
    }
  frvectmtx.unlock ();
}

void
MainWindow::fileRcvdStatus (std::string *key, std::string *filename,
			    std::mutex *dispNmtx, int variant)
{
  std::string keyloc = *key;
  std::string filenm = *filename;
  Glib::ustring nick = "";
  dispNmtx->unlock ();

  frvectmtx.lock ();
  auto itfrv = std::find_if (friendvect.begin (), friendvect.end (), [&keyloc]
  (auto &el)
    {
      Glib::ustring str = Glib::ustring(keyloc);
      if (str == std::get<2>(el)->get_text ())
	{
	  return true;
	}
      else
	{
	  return false;
	}
    });
  if (itfrv != friendvect.end ())
    {
      nick = std::get<4> (*itfrv)->get_text ();
    }
  frvectmtx.unlock ();

  if (nick != "")
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application (this->get_application ());
      window->set_title (gettext ("File received"));
      window->set_transient_for (*this);
      window->set_modal (true);
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      window->set_child (*grid);

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();

      if (variant == 6)
	{
	  Glib::ustring lab1;
	  lab1 = gettext ("File");
	  Glib::ustring lab2;
	  lab2 = gettext ("from");
	  Glib::ustring lab3;
	  lab3 = gettext ("received");
	  lab->set_text (
	      lab1 + " " + Glib::ustring (filenm) + "\n" + lab2 + " " + nick
		  + "\n" + lab3);
	}
      if (variant == 7)
	{
	  Glib::ustring lab4;
	  lab4 = gettext ("Error occurred while receiving file");
	  Glib::ustring lab2;
	  lab2 = gettext ("from");
	  Glib::ustring lab5;
	  lab5 = gettext ("- hash is not correct!");
	  lab->set_text (
	      lab4 + " " + Glib::ustring (filenm) + "\n" + lab2 + " " + nick
		  + "\n " + lab5);
	}
      if (variant == 8)
	{
	  Glib::ustring lab1;
	  lab1 = gettext ("File");
	  Glib::ustring lab6;
	  lab6 = gettext ("for");
	  Glib::ustring lab7;
	  lab7 = gettext ("was sent");
	  lab->set_text (
	      lab1 + " " + Glib::ustring (filenm) + " " + lab6 + " " + nick
		  + "\n" + lab7);
	}
      if (variant == 9)
	{
	  Glib::ustring lab8;
	  lab8 = gettext ("Error on sending");
	  Glib::ustring lab6;
	  lab6 = gettext ("for");
	  lab->set_text (
	      lab8 + " " + Glib::ustring (filenm) + " " + lab6 + " " + nick);
	}

      lab->set_justify (Gtk::Justification::CENTER);
      lab->set_halign (Gtk::Align::CENTER);
      lab->set_margin (5);
      grid->attach (*lab, 0, 0, 1, 1);

      Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
      close->set_label (gettext ("Close"));
      close->set_halign (Gtk::Align::CENTER);
      close->set_margin (5);
      close->set_name ("applyButton");
      close->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      close->signal_clicked ().connect ( [window]
      {
	window->close ();
      });
      grid->attach (*close, 0, 1, 1, 1);

      window->signal_close_request ().connect ( [window]
      {
	window->hide ();
	delete window;
	return true;
      },
					       false);
      window->show ();
    }

  fileprogrvectmtx.lock ();
  fileprogrvect.erase (
      std::remove_if (
	  fileprogrvect.begin (),
	  fileprogrvect.end (),
	  [&keyloc, filenm, this]
	  (auto &el)
	    {
	      if (std::get<0>(el) == keyloc && std::get<1>(el).filename().u8string() == filenm)
		{
		  Gtk::Grid *gr = this->dld_grid;
		  if (gr != nullptr)
		    {
		      gr->remove(*(std::get<3>(el)));
		      gr->remove(*(std::get<4>(el)));
		      gr->remove(*(std::get<5>(el)));
		    }
		  return true;
		}
	      else
		{
		  return false;
		}
	    }),
      fileprogrvect.end ());
  if (fileprogrvect.size () == 0 && dld_win != nullptr)
    {
      dld_win->hide ();
      delete dld_win;
      dld_win = nullptr;
      dld_grid = nullptr;
    }
  fileprogrvectmtx.unlock ();
}

void
MainWindow::contMenu (int numcl, double x, double y,
		      Glib::RefPtr<Gtk::GestureClick> clck,
		      Gtk::Button *contbutton)
{
  Gtk::Label *keylab = nullptr;
  Gtk::Label *nicklab = nullptr;
  frvectmtx.lock ();
  auto itfrv = std::find_if (friendvect.begin (), friendvect.end (),
			     [contbutton]
			     (auto &el)
			       {
				 return std::get<0>(el) == contbutton;
			       });
  if (itfrv != friendvect.end ())
    {
      keylab = std::get<2> (*itfrv);
      nicklab = std::get<4> (*itfrv);
    }
  frvectmtx.unlock ();
  if (keylab != nullptr && nicklab != nullptr)
    {
      selectContact (Winright, std::string (keylab->get_text ()),
		     std::string (nicklab->get_text ()));
    }

  Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create ();
  menu->append (gettext ("Contact info"), "popupcont.info");
  menu->append (gettext ("Remove contact"), "popupcont.delete");
  menu->append (gettext ("Block (until restart)/Unblock"), "popupcont.block");
  Gtk::PopoverMenu *Menu = Gtk::make_managed<Gtk::PopoverMenu> ();
  Menu->set_parent (*contbutton);
  Menu->set_menu_model (menu);
  Menu->set_has_arrow (false);
  const Gdk::Rectangle rect (x, y, 1, 1);
  Menu->set_pointing_to (rect);
  if (contmenupop != nullptr)
    {
      contmenupop->unparent ();
      contmenupop = nullptr;
    }
  Menu->popup ();
  contmenupop = Menu;
}

void
MainWindow::friendDetails (Gtk::Button *button)
{
  if (contmenupop != nullptr)
    {
      contmenupop->unparent ();
      contmenupop = nullptr;
    }
  frvectmtx.lock ();
  auto itfr = std::find_if (friendvect.begin (), friendvect.end (), [button]
  (auto &el)
    {
      return std::get<0> (el) == button;
    });
  if (itfr != friendvect.end ())
    {
      std::string key = std::string (std::get<2> (*itfr)->get_text ());
      contmtx.lock ();
      auto itcont = std::find_if (contacts.begin (), contacts.end (), [key]
      (auto &el)
	{
	  return std::get<1>(el) == key;
	});
      if (itcont != contacts.end ())
	{
	  int cid = std::get<0> (*itcont);
	  std::string filename, index;
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << cid;
	  index = strm.str ();
	  AuxFunc af;
	  af.homePath (&filename);
	  filename = filename + "/.Communist/" + index + "/Profile";
	  std::filesystem::path filepath = std::filesystem::u8path (filename);
#ifdef __linux
	  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
	  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
	  filename = filename + "/CommCP/Profile.zip";
	  std::filesystem::path outpath = std::filesystem::u8path (filename);
	  if (std::filesystem::exists (outpath.parent_path ()))
	    {
	      std::filesystem::remove_all (outpath.parent_path ());
	    }
	  std::filesystem::create_directories (outpath.parent_path ());
	  lt::dht::public_key othpk;
	  lt::aux::from_hex (key, othpk.bytes.data ());
	  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	  okp = lt::dht::ed25519_create_keypair (seed);
	  std::string unm = lt::aux::to_hex (std::get<0> (okp).bytes);
	  std::array<char, 32> scalar;
	  scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (okp));
	  othpk = lt::dht::ed25519_add_scalar (std::get<0> (okp), scalar);
	  std::string passwd = lt::aux::to_hex (othpk.bytes);
	  af.decryptFile (unm, passwd, filepath.u8string (),
			  outpath.u8string ());
	  af.unpacking (outpath.u8string (),
			outpath.parent_path ().u8string ());
	  filename = outpath.parent_path ().u8string ();
	  filename = filename + "/Profile/Profile";
	  filepath = std::filesystem::u8path (filename);
	  std::fstream f;
	  std::vector<Glib::ustring> profvect;
	  f.open (filepath, std::ios_base::in);
	  if (f.is_open ())
	    {
	      while (!f.eof ())
		{
		  std::string line;
		  getline (f, line);
		  if (line != "")
		    {
		      profvect.push_back (Glib::ustring (line));
		    }
		}
	      f.close ();
	    }

	  filename = outpath.parent_path ().u8string ();
	  filename = filename + "/Profile/Avatar.jpeg";
	  filepath = std::filesystem::u8path (filename);
	  Glib::ustring picturepath = "";
	  if (std::filesystem::exists (filepath))
	    {
	      picturepath = filepath.u8string ();
	    }

	  Gtk::Window *window = new Gtk::Window;
	  if (profvect.size () > 0)
	    {
	      window->set_title (profvect[0]);
	    }
	  window->set_transient_for (*this);
	  window->set_name ("settingsWindow");
	  window->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
	  grid->set_halign (Gtk::Align::CENTER);
	  window->set_child (*grid);

	  Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea> ();
	  drar->set_margin (5);
	  drar->set_halign (Gtk::Align::CENTER);
	  drar->set_size_request (200, 200);
	  grid->attach (*drar, 0, 0, 1, 4);
	  if (picturepath != "")
	    {
	      drar->set_draw_func (
		  sigc::bind (sigc::mem_fun (*this, &MainWindow::on_draw_frpd),
			      picturepath));
	    }

	  Gtk::Label *lab1 = Gtk::make_managed<Gtk::Label> ();
	  lab1->set_text (gettext ("Key: "));
	  lab1->set_halign (Gtk::Align::START);
	  lab1->set_margin (5);
	  grid->attach (*lab1, 1, 0, 1, 1);

	  Gtk::Label *lab2 = Gtk::make_managed<Gtk::Label> ();
	  lab2->set_text (gettext ("Nickname: "));
	  lab2->set_halign (Gtk::Align::START);
	  lab2->set_margin (5);
	  grid->attach (*lab2, 1, 1, 1, 1);

	  Gtk::Label *lab3 = Gtk::make_managed<Gtk::Label> ();
	  lab3->set_text (gettext ("Name: "));
	  lab3->set_halign (Gtk::Align::START);
	  lab3->set_margin (5);
	  grid->attach (*lab3, 1, 2, 1, 1);

	  Gtk::Label *lab4 = Gtk::make_managed<Gtk::Label> ();
	  lab4->set_text (gettext ("Surname: "));
	  lab4->set_halign (Gtk::Align::START);
	  lab4->set_margin (5);
	  grid->attach (*lab4, 1, 3, 1, 1);

	  Gtk::Label *keylab = Gtk::make_managed<Gtk::Label> ();
	  keylab->set_markup ("<i>" + Glib::ustring (key) + "</i>");
	  keylab->set_halign (Gtk::Align::START);
	  keylab->set_margin (5);
	  keylab->set_name ("friendDetLab");
	  keylab->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  keylab->set_selectable (true);
	  grid->attach (*keylab, 2, 0, 1, 1);

	  for (size_t i = 0; i < profvect.size (); i++)
	    {
	      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
	      lab->set_markup ("<i>" + profvect[i] + "</i>");
	      lab->set_halign (Gtk::Align::START);
	      lab->set_margin (5);
	      lab->set_name ("friendDetLab");
	      lab->get_style_context ()->add_provider (
		  css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      lab->set_selectable (true);
	      grid->attach (*lab, 2, i + 1, 1, 1);
	    }
	  Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
	  close->set_label (gettext ("Close"));
	  close->set_halign (Gtk::Align::CENTER);
	  close->set_margin (5);
	  close->set_name ("applyButton");
	  close->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  close->signal_clicked ().connect ( [window]
	  {
	    window->close ();
	  });
	  grid->attach_next_to (*close, *drar, Gtk::PositionType::BOTTOM, 3, 1);

	  window->signal_close_request ().connect ( [window, outpath]
	  {
	    window->hide ();
	    delete window;
	    std::filesystem::remove_all (outpath.parent_path ());
	    return true;
	  },
						   false);
	  window->set_application (this->get_application ());
	  window->show ();
	}
      contmtx.unlock ();
    }
  frvectmtx.unlock ();
}

void
MainWindow::on_draw_frpd (const Cairo::RefPtr<Cairo::Context> &cr, int width,
			  int height, Glib::ustring file)
{
  Glib::RefPtr<Gdk::Pixbuf> pic = Gdk::Pixbuf::create_from_file (file);
  pic = pic->scale_simple (width, height, Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf (cr, pic, 0, 0);
  cr->rectangle (0, 0, width, height);
  cr->fill ();
}

void
MainWindow::tempBlockCont ()
{
  if (contmenupop != nullptr)
    {
      contmenupop->unparent ();
      contmenupop = nullptr;
    }
  Gtk::Button *but = selectedc;
  frvectmtx.lock ();
  auto itfrv = std::find_if (friendvect.begin (), friendvect.end (), [but]
  (auto &el)
    {
      return std::get<0> (el) == but;
    });
  if (itfrv != friendvect.end ())
    {
      Gtk::Grid *grid = std::get<1> (*itfrv);
      Gtk::Label *lab = std::get<6> (*itfrv);
      if (lab == nullptr)
	{
	  lab = Gtk::make_managed<Gtk::Label> ();
	  lab->set_name ("blockLab");
	  lab->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  lab->set_text (gettext ("Blocked"));
	  Gtk::Label *sibl = std::get<4> (*itfrv);
	  grid->attach_next_to (*lab, *sibl, Gtk::PositionType::RIGHT, 1, 1);
	  std::get<6> (*itfrv) = lab;
	  if (oper != nullptr)
	    {
	      std::string key (std::get<2> (*itfrv)->get_text ());
	      oper->blockFriend (key);
	    }
	}
      else
	{
	  std::string key (std::get<2> (*itfrv)->get_text ());
	  contmtx.lock ();
	  auto itcont = std::find_if (contacts.begin (), contacts.end (), [&key]
	  (auto &el)
	    {
	      return std::get<1>(el) == key;
	    });
	  if (itcont != contacts.end ())
	    {
	      int ind = std::get<0> (*itcont);
	      if (oper != nullptr)
		{
		  oper->startFriend (key, ind);
		}
	    }
	  contmtx.unlock ();
	  grid->remove (*lab);
	  std::get<6> (*itfrv) = nullptr;
	}
    }
  frvectmtx.unlock ();
}

void
MainWindow::settingsWindow ()
{
  std::string filename;
  std::filesystem::path filepath;
  AuxFunc af;
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title (gettext ("Settings"));
  window->set_transient_for (*this);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow> ();
  scrl->set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  scrl->set_halign (Gtk::Align::CENTER);
  Gtk::Grid *grwin = Gtk::make_managed<Gtk::Grid> ();
  grwin->set_halign (Gtk::Align::CENTER);
  window->set_child (*grwin);
  grwin->attach (*scrl, 0, 0, 2, 1);

  Gtk::Box *bx;
  Gtk::ListBoxRow *lbr;
  Gtk::ListBox *listb = Gtk::make_managed<Gtk::ListBox> ();
  listb->set_halign (Gtk::Align::CENTER);
  listb->set_show_separators (true);
  listb->set_name ("settingsLB");
  listb->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  scrl->set_child (*listb);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *thmlb = Gtk::make_managed<Gtk::Label> ();
  thmlb->set_halign (Gtk::Align::START);
  thmlb->set_justify (Gtk::Justification::LEFT);
  thmlb->set_margin (5);
  thmlb->set_wrap_mode (Pango::WrapMode::WORD);
  thmlb->set_text (gettext ("Theme:"));
  bx->append (*thmlb);

  Gtk::ComboBoxText *cmbthm = Gtk::make_managed<Gtk::ComboBoxText> ();
  cmbthm->set_halign (Gtk::Align::END);
  cmbthm->set_margin (5);
  std::filesystem::path thmp = std::filesystem::u8path (
      std::string (Sharepath + "/themes"));
  for (auto &dirit : std::filesystem::directory_iterator (thmp))
    {
      std::filesystem::path p = dirit.path ();
      cmbthm->append (Glib::ustring (p.filename ().u8string ()));
    }
  prefvectmtx.lock ();
  auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Theme";
    });
  if (itprv != prefvect.end ())
    {
      std::string tmp = std::get<1> (*itprv);
      cmbthm->set_active_text (Glib::ustring (tmp));
    }
  else
    {
      if (cmbthm->get_has_entry ())
	{
	  cmbthm->set_active_text ("default");
	}
    }
  prefvectmtx.unlock ();
  bx->append (*cmbthm);
  lbr->set_child (*bx);
  listb->append (*lbr);

  cmbthm->signal_changed ().connect ( [cmbthm, this]
  {
    std::string tmp (cmbthm->get_active_text ());
    this->prefvectmtx.lock ();
    auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Theme";
    });
    if (itprv != this->prefvect.end ())
      {
	std::get<1> (*itprv) = tmp;
      }
    else
      {
	std::tuple<std::string, std::string> ttup;
	std::get<0> (ttup) = "Theme";
	std::get<1> (ttup) = tmp;
	this->prefvect.push_back (ttup);
      }
    this->prefvectmtx.unlock ();
  });

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *langchenb = Gtk::make_managed<Gtk::Label> ();
  langchenb->set_halign (Gtk::Align::START);
  langchenb->set_justify (Gtk::Justification::LEFT);
  langchenb->set_margin (5);
  langchenb->set_wrap_mode (Pango::WrapMode::WORD);
  langchenb->set_text (gettext ("Enable spell checking"));
  bx->append (*langchenb);

  Gtk::CheckButton *langchenbch = Gtk::make_managed<Gtk::CheckButton> ();
  langchenbch->set_margin (5);
  langchenbch->set_halign (Gtk::Align::CENTER);
  langchenbch->set_valign (Gtk::Align::CENTER);
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Spellcheck";
    });
  if (itprv != prefvect.end ())
    {
      if (std::get<1> (*itprv) == "1")
	{
	  langchenbch->set_active (true);
	}
      else
	{
	  langchenbch->set_active (false);
	}
    }
  else
    {
      langchenbch->set_active (true);
    }
  prefvectmtx.unlock ();
  bx->append (*langchenbch);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *langl = Gtk::make_managed<Gtk::Label> ();
  langl->set_halign (Gtk::Align::START);
  langl->set_justify (Gtk::Justification::LEFT);
  langl->set_margin (5);
  langl->set_wrap_mode (Pango::WrapMode::WORD);
  langl->set_text (gettext ("Language for spell checking:"));
  bx->append (*langl);

  Gtk::ComboBoxText *cmbtxt = Gtk::make_managed<Gtk::ComboBoxText> ();
  cmbtxt->set_halign (Gtk::Align::END);
  cmbtxt->set_margin (5);
  std::filesystem::path dictp = std::filesystem::u8path (
      std::string (Sharepath + "/HunDict/languages"));
  std::vector<std::tuple<std::string, std::string>> langvect;
  std::fstream f;
  f.open (dictp, std::ios_base::in);
  if (f.is_open ())
    {
      while (!f.eof ())
	{
	  std::string line;
	  getline (f, line);
	  if (line != "")
	    {
	      line.erase (0, line.find (" ") + std::string (" ").size ());
	      cmbtxt->append (Glib::ustring (line));
	    }
	}
      f.close ();
    }
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Language";
    });
  if (itprv != prefvect.end ())
    {
      std::string tmp = std::get<1> (*itprv);
      cmbtxt->set_active_text (Glib::ustring (tmp));
    }
  else
    {
      if (cmbtxt->get_has_entry ())
	{
	  cmbtxt->set_active (1);
	}
    }
  prefvectmtx.unlock ();
  bx->append (*cmbtxt);
  lbr->set_child (*bx);
  listb->append (*lbr);

  cmbtxt->signal_changed ().connect ( [cmbtxt, this]
  {
    std::string tmp (cmbtxt->get_active_text ());
    this->prefvectmtx.lock ();
    auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Language";
    });
    if (itprv != this->prefvect.end ())
      {
	std::get<1> (*itprv) = tmp;
      }
    else
      {
	std::tuple<std::string, std::string> ttup;
	std::get<0> (ttup) = "Language";
	std::get<1> (ttup) = tmp;
	this->prefvect.push_back (ttup);
      }
    this->prefvectmtx.unlock ();
  });

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *netl = Gtk::make_managed<Gtk::Label> ();
  netl->set_halign (Gtk::Align::START);
  netl->set_justify (Gtk::Justification::LEFT);
  netl->set_margin (5);
  netl->set_wrap_mode (Pango::WrapMode::WORD);
  netl->set_text (gettext ("Network mode:"));
  bx->append (*netl);

  Gtk::ComboBoxText *netcmb = Gtk::make_managed<Gtk::ComboBoxText> ();
  netcmb->set_halign (Gtk::Align::END);
  netcmb->set_margin (5);
  netcmb->append (gettext ("Internet"));
  netcmb->append (gettext ("Local"));
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Netmode";
    });
  if (itprv != prefvect.end ())
    {
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << std::get<1> (*itprv);
      int sel;
      strm >> sel;
      if (sel <= 1)
	{
	  netcmb->set_active (sel);
	}
      else
	{
	  netcmb->set_active (0);
	}
    }
  else
    {
      netcmb->set_active (0);
    }
  prefvectmtx.unlock ();
  netcmb->signal_changed ().connect ( [netcmb, this]
  {
    std::stringstream strm;
    std::locale loc ("C");
    strm.imbue (loc);
    strm << netcmb->get_active_row_number ();
    std::string ch = strm.str ();
    this->prefvectmtx.lock ();
    auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Netmode";
    });
    if (itprv != this->prefvect.end ())
      {
	std::get<1> (*itprv) = ch;
      }
    else
      {
	this->prefvect.push_back (std::make_tuple ("Netmode", ch));
      }
    this->prefvectmtx.unlock ();
  });
  bx->append (*netcmb);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *locip4l = Gtk::make_managed<Gtk::Label> ();
  locip4l->set_halign (Gtk::Align::START);
  locip4l->set_margin (5);
  locip4l->set_wrap_mode (Pango::WrapMode::WORD);
  locip4l->set_wrap (true);
  locip4l->set_text (
      gettext ("Local network group multicast ipv4 and port (ipv4:port)"));
  locip4l->set_justify (Gtk::Justification::LEFT);
  bx->append (*locip4l);

  Gtk::Entry *locip4val = Gtk::make_managed<Gtk::Entry> ();
  locip4val->set_halign (Gtk::Align::END);
  locip4val->set_valign (Gtk::Align::CENTER);
  locip4val->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> locip4valbuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Locip4port";
    });
  if (itprv != prefvect.end ())
    {
      locip4valbuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      locip4valbuf->set_text ("239.192.150.8:48655");
    }
  prefvectmtx.unlock ();
  if (locip4valbuf->get_length () > 0)
    {
      locip4val->set_width_chars (locip4valbuf->get_length ());
    }
  locip4val->set_buffer (locip4valbuf);
  bx->append (*locip4val);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *locip6l = Gtk::make_managed<Gtk::Label> ();
  locip6l->set_halign (Gtk::Align::START);
  locip6l->set_margin (5);
  locip6l->set_wrap_mode (Pango::WrapMode::WORD);
  locip6l->set_wrap (true);
  locip6l->set_text (
      gettext ("Local network group multicast ipv6 and port ([ipv6]:port)"));
  locip6l->set_justify (Gtk::Justification::LEFT);
  bx->append (*locip6l);

  Gtk::Entry *locip6val = Gtk::make_managed<Gtk::Entry> ();
  locip6val->set_halign (Gtk::Align::END);
  locip6val->set_valign (Gtk::Align::CENTER);
  locip6val->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> locip6valbuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Locip6port";
    });
  if (itprv != prefvect.end ())
    {
      locip6valbuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      locip6valbuf->set_text ("[ff15::13]:48666");
    }
  prefvectmtx.unlock ();
  if (locip6valbuf->get_length () > 0)
    {
      locip6val->set_width_chars (locip6valbuf->get_length ());
    }
  locip6val->set_buffer (locip6valbuf);
  bx->append (*locip6val);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *autorem = Gtk::make_managed<Gtk::Label> ();
  autorem->set_halign (Gtk::Align::START);
  autorem->set_justify (Gtk::Justification::LEFT);
  autorem->set_margin (5);
  autorem->set_wrap_mode (Pango::WrapMode::WORD);
  autorem->set_text (gettext ("Messages auto remove after:"));
  bx->append (*autorem);

  Gtk::ComboBoxText *autoremcmb = Gtk::make_managed<Gtk::ComboBoxText> ();
  autoremcmb->set_halign (Gtk::Align::END);
  autoremcmb->set_valign (Gtk::Align::CENTER);
  autoremcmb->set_margin (5);
  autoremcmb->append (gettext ("never"));
  autoremcmb->append (gettext ("1 day"));
  autoremcmb->append (gettext ("1 week"));
  autoremcmb->append (gettext ("1 month"));
  autoremcmb->append (gettext ("1 year"));
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Autoremove";
    });
  if (itprv != prefvect.end ())
    {
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << std::get<1> (*itprv);
      int sel;
      strm >> sel;
      if (sel <= 4)
	{
	  autoremcmb->set_active (sel);
	}
      else
	{
	  autoremcmb->set_active (0);
	}
    }
  else
    {
      autoremcmb->set_active (0);
    }
  prefvectmtx.unlock ();
  autoremcmb->signal_changed ().connect ( [autoremcmb, this]
  {
    std::stringstream strm;
    std::locale loc ("C");
    strm.imbue (loc);
    strm << autoremcmb->get_active_row_number ();
    std::string ch = strm.str ();
    this->prefvectmtx.lock ();
    auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Autoremove";
    });
    if (itprv != this->prefvect.end ())
      {
	std::get<1> (*itprv) = ch;
      }
    else
      {
	this->prefvect.push_back (std::make_tuple ("Autoremove", ch));
      }
    this->prefvectmtx.unlock ();
  });
  bx->append (*autoremcmb);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *lifcs = Gtk::make_managed<Gtk::Label> ();
  lifcs->set_halign (Gtk::Align::START);
  lifcs->set_margin (5);
  lifcs->set_wrap_mode (Pango::WrapMode::WORD);
  lifcs->set_wrap (true);
  lifcs->set_text (gettext ("DHT listen interfaces (ipv4:port,[ipv6]:port):"));
  lifcs->set_justify (Gtk::Justification::LEFT);
  bx->append (*lifcs);

  Gtk::Entry *lifcsval = Gtk::make_managed<Gtk::Entry> ();
  lifcsval->set_halign (Gtk::Align::END);
  lifcsval->set_valign (Gtk::Align::CENTER);
  lifcsval->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> lifcsvalbuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Listenifcs";
    });
  if (itprv != prefvect.end ())
    {
      lifcsvalbuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      lifcsvalbuf->set_text ("0.0.0.0:0,[::]:0");
    }
  prefvectmtx.unlock ();
  if (lifcsvalbuf->get_length () > 0)
    {
      lifcsval->set_width_chars (lifcsvalbuf->get_length ());
    }
  lifcsval->set_buffer (lifcsvalbuf);
  bx->append (*lifcsval);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *bootstrl = Gtk::make_managed<Gtk::Label> ();
  bootstrl->set_halign (Gtk::Align::START);
  bootstrl->set_margin (5);
  bootstrl->set_wrap_mode (Pango::WrapMode::WORD);
  bootstrl->set_wrap (true);
  bootstrl->set_text (
      gettext (
	  "Comma separated DHT bootstrap nodes (address:port or ipv4:port or [ipv6]:port):"));
  bootstrl->set_justify (Gtk::Justification::LEFT);
  bx->append (*bootstrl);

  Gtk::Entry *bootstre = Gtk::make_managed<Gtk::Entry> ();
  bootstre->set_halign (Gtk::Align::END);
  bootstre->set_valign (Gtk::Align::CENTER);
  bootstre->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> bootstrebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Bootstrapdht";
    });
  if (itprv != prefvect.end ())
    {
      bootstrebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      bootstrebuf->set_text ("router.bittorrent.com:6881");
    }
  prefvectmtx.unlock ();
  if (bootstrebuf->get_length () > 0)
    {
      bootstre->set_width_chars (bootstrebuf->get_length ());
    }
  bootstre->set_buffer (bootstrebuf);
  bx->append (*bootstre);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *msgszl = Gtk::make_managed<Gtk::Label> ();
  msgszl->set_halign (Gtk::Align::START);
  msgszl->set_justify (Gtk::Justification::LEFT);
  msgszl->set_margin (5);
  msgszl->set_wrap_mode (Pango::WrapMode::WORD);
  msgszl->set_text (
      gettext ("Maximum permitted incomming messages size (in bytes):"));
  bx->append (*msgszl);

  Gtk::Entry *msgsze = Gtk::make_managed<Gtk::Entry> ();
  msgsze->set_halign (Gtk::Align::END);
  msgsze->set_valign (Gtk::Align::CENTER);
  msgsze->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> msgszebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Maxmsgsz";
    });
  if (itprv != prefvect.end ())
    {
      msgszebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      msgszebuf->set_text ("1048576");
    }
  prefvectmtx.unlock ();
  if (msgszebuf->get_length () > 0)
    {
      msgsze->set_width_chars (msgszebuf->get_length ());
    }
  msgsze->set_buffer (msgszebuf);
  bx->append (*msgsze);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *partszl = Gtk::make_managed<Gtk::Label> ();
  partszl->set_halign (Gtk::Align::START);
  partszl->set_justify (Gtk::Justification::LEFT);
  partszl->set_margin (5);
  partszl->set_wrap_mode (Pango::WrapMode::WORD);
  partszl->set_text (gettext ("Sending part size (in bytes):"));
  bx->append (*partszl);

  Gtk::Entry *partsze = Gtk::make_managed<Gtk::Entry> ();
  partsze->set_halign (Gtk::Align::END);
  partsze->set_valign (Gtk::Align::CENTER);
  partsze->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> partszebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Partsize";
    });
  if (itprv != prefvect.end ())
    {
      partszebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      partszebuf->set_text ("1371");
    }
  prefvectmtx.unlock ();
  if (partszebuf->get_length () > 0)
    {
      partsze->set_width_chars (partszebuf->get_length ());
    }
  partsze->set_buffer (partszebuf);
  bx->append (*partsze);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *shuttmtl = Gtk::make_managed<Gtk::Label> ();
  shuttmtl->set_halign (Gtk::Align::START);
  shuttmtl->set_justify (Gtk::Justification::LEFT);
  shuttmtl->set_margin (5);
  shuttmtl->set_wrap_mode (Pango::WrapMode::WORD);
  shuttmtl->set_text (
      gettext (
	  "Contact connection maintenance messages shutdown timeout in seconds:"));
  bx->append (*shuttmtl);

  Gtk::Entry *shutmte = Gtk::make_managed<Gtk::Entry> ();
  shutmte->set_halign (Gtk::Align::END);
  shutmte->set_valign (Gtk::Align::CENTER);
  shutmte->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> shutmtebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "ShutTmt";
    });
  if (itprv != prefvect.end ())
    {
      shutmtebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      shutmtebuf->set_text ("600");
    }
  prefvectmtx.unlock ();
  if (shutmtebuf->get_length () > 0)
    {
      shutmte->set_width_chars (shutmtebuf->get_length ());
    }
  shutmte->set_buffer (shutmtebuf);
  bx->append (*shutmte);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *tmttearl = Gtk::make_managed<Gtk::Label> ();
  tmttearl->set_halign (Gtk::Align::START);
  tmttearl->set_justify (Gtk::Justification::LEFT);
  tmttearl->set_margin (5);
  tmttearl->set_wrap_mode (Pango::WrapMode::WORD);
  tmttearl->set_text (gettext ("Connection broken timeout in seconds:"));
  bx->append (*tmttearl);

  Gtk::Entry *tmtteare = Gtk::make_managed<Gtk::Entry> ();
  tmtteare->set_halign (Gtk::Align::END);
  tmtteare->set_valign (Gtk::Align::CENTER);
  tmtteare->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> tmttearebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "TmtTear";
    });
  if (itprv != prefvect.end ())
    {
      tmttearebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
    }
  else
    {
      tmttearebuf->set_text ("20");
    }
  prefvectmtx.unlock ();
  if (tmttearebuf->get_length () > 0)
    {
      tmtteare->set_width_chars (tmttearebuf->get_length ());
    }
  tmtteare->set_buffer (tmttearebuf);
  bx->append (*tmtteare);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *winszl = Gtk::make_managed<Gtk::Label> ();
  winszl->set_halign (Gtk::Align::START);
  winszl->set_justify (Gtk::Justification::LEFT);
  winszl->set_margin (5);
  winszl->set_wrap_mode (Pango::WrapMode::WORD);
  winszl->set_text (gettext ("Save window size before closing"));
  bx->append (*winszl);

  Gtk::CheckButton *winszch = Gtk::make_managed<Gtk::CheckButton> ();
  winszch->set_margin (5);
  winszch->set_halign (Gtk::Align::CENTER);
  winszch->set_valign (Gtk::Align::CENTER);
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Winsizesv";
    });
  if (itprv != prefvect.end ())
    {
      if (std::get<1> (*itprv) == "active")
	{
	  winszch->set_active (true);
	}
      else
	{
	  winszch->set_active (false);
	}
    }
  else
    {
      winszch->set_active (true);
    }
  prefvectmtx.unlock ();
  bx->append (*winszch);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *sendkeyl = Gtk::make_managed<Gtk::Label> ();
  sendkeyl->set_halign (Gtk::Align::START);
  sendkeyl->set_justify (Gtk::Justification::LEFT);
  sendkeyl->set_margin (5);
  sendkeyl->set_wrap_mode (Pango::WrapMode::WORD);
  sendkeyl->set_text (gettext ("Send message by"));
  bx->append (*sendkeyl);

  Gtk::ComboBoxText *sendkeycmb = Gtk::make_managed<Gtk::ComboBoxText> ();
  sendkeycmb->set_halign (Gtk::Align::END);
  sendkeycmb->set_margin (5);
  sendkeycmb->append ("Ctrl + Enter");
  sendkeycmb->append ("Enter");
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "SendKey";
    });
  if (itprv != prefvect.end ())
    {
      std::string modes = std::get<1> (*itprv);
      if (modes != "")
	{
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << modes;
	  int modei;
	  strm >> modei;
	  sendkeycmb->set_active (modei);
	}
      else
	{
	  sendkeycmb->set_active (0);
	}
    }
  else
    {
      sendkeycmb->set_active (0);
    }
  prefvectmtx.unlock ();
  bx->append (*sendkeycmb);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *soundl = Gtk::make_managed<Gtk::Label> ();
  soundl->set_halign (Gtk::Align::START);
  soundl->set_justify (Gtk::Justification::LEFT);
  soundl->set_margin (5);
  soundl->set_wrap_mode (Pango::WrapMode::WORD);
  soundl->set_text (gettext ("Enable message received sound"));
  bx->append (*soundl);

  Gtk::CheckButton *soundch = Gtk::make_managed<Gtk::CheckButton> ();
  soundch->set_margin (5);
  soundch->set_halign (Gtk::Align::CENTER);
  soundch->set_valign (Gtk::Align::CENTER);
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "SoundOn";
    });
  if (itprv != prefvect.end ())
    {
      if (std::get<1> (*itprv) == "active")
	{
	  soundch->set_active (true);
	}
      else
	{
	  soundch->set_active (false);
	}
    }
  else
    {
      soundch->set_active (true);
    }
  prefvectmtx.unlock ();
  bx->append (*soundch);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *soundpl = Gtk::make_managed<Gtk::Label> ();
  soundpl->set_halign (Gtk::Align::START);
  soundpl->set_justify (Gtk::Justification::LEFT);
  soundpl->set_margin (5);
  soundpl->set_wrap_mode (Pango::WrapMode::WORD);
  soundpl->set_text (gettext ("Message sound path:"));
  bx->append (*soundpl);

  Gtk::Entry *soundpe = Gtk::make_managed<Gtk::Entry> ();
  soundpe->set_halign (Gtk::Align::END);
  soundpe->set_valign (Gtk::Align::CENTER);
  soundpe->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> soundpebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "SoundPath";
    });
  if (itprv != prefvect.end ())
    {
      if (Glib::ustring (std::get<1> (*itprv)) != "")
	{
	  soundpebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
	  soundpe->set_buffer (soundpebuf);
	}
      else
	{
	  soundpe->set_placeholder_text (gettext ("default"));
	}
    }
  else
    {
      soundpe->set_placeholder_text (gettext ("default"));
    }
  prefvectmtx.unlock ();
  soundpe->set_width_chars (50);
  bx->append (*soundpe);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *enstunl = Gtk::make_managed<Gtk::Label> ();
  enstunl->set_halign (Gtk::Align::START);
  enstunl->set_justify (Gtk::Justification::LEFT);
  enstunl->set_margin (5);
  enstunl->set_wrap_mode (Pango::WrapMode::WORD);
  enstunl->set_text (gettext ("Enable local STUN server"));
  bx->append (*enstunl);

  Gtk::CheckButton *enstunch = Gtk::make_managed<Gtk::CheckButton> ();
  enstunch->set_margin (5);
  enstunch->set_halign (Gtk::Align::CENTER);
  enstunch->set_valign (Gtk::Align::CENTER);
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Stun";
    });
  if (itprv != prefvect.end ())
    {
      if (std::get<1> (*itprv) == "active")
	{
	  enstunch->set_active (true);
	}
      else
	{
	  enstunch->set_active (false);
	}
    }
  else
    {
      enstunch->set_active (false);
    }
  prefvectmtx.unlock ();
  bx->append (*enstunch);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *stunpl = Gtk::make_managed<Gtk::Label> ();
  stunpl->set_halign (Gtk::Align::START);
  stunpl->set_justify (Gtk::Justification::LEFT);
  stunpl->set_margin (5);
  stunpl->set_wrap_mode (Pango::WrapMode::WORD);
  stunpl->set_text (gettext ("STUN port:"));
  bx->append (*stunpl);

  Gtk::Entry *stunpe = Gtk::make_managed<Gtk::Entry> ();
  stunpe->set_halign (Gtk::Align::END);
  stunpe->set_valign (Gtk::Align::CENTER);
  stunpe->set_margin (5);
  Glib::RefPtr<Gtk::EntryBuffer> stunpebuf = Gtk::EntryBuffer::create ();
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Stunport";
    });
  if (itprv != prefvect.end ())
    {
      if (Glib::ustring (std::get<1> (*itprv)) != "")
	{
	  stunpebuf->set_text (Glib::ustring (std::get<1> (*itprv)));
	  stunpe->set_buffer (stunpebuf);
	}
      else
	{
	  stunpebuf->set_text ("3478");
	  stunpe->set_buffer (stunpebuf);
	}
    }
  else
    {
      stunpebuf->set_text ("3478");
      stunpe->set_buffer (stunpebuf);
    }
  prefvectmtx.unlock ();
  stunpe->set_width_chars (5);
  bx->append (*stunpe);
  lbr->set_child (*bx);
  listb->append (*lbr);

  bx = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable (false);
  Gtk::Label *directinetl = Gtk::make_managed<Gtk::Label> ();
  directinetl->set_halign (Gtk::Align::START);
  directinetl->set_justify (Gtk::Justification::LEFT);
  directinetl->set_margin (5);
  directinetl->set_wrap_mode (Pango::WrapMode::WORD);
  directinetl->set_text (gettext ("Direct connection to Internet"));
  bx->append (*directinetl);

  Gtk::CheckButton *directinetch = Gtk::make_managed<Gtk::CheckButton> ();
  directinetch->set_margin (5);
  directinetch->set_halign (Gtk::Align::CENTER);
  directinetch->set_valign (Gtk::Align::CENTER);
  prefvectmtx.lock ();
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "DirectInet";
    });
  if (itprv != prefvect.end ())
    {
      if (std::get<1> (*itprv) == "direct")
	{
	  directinetch->set_active (true);
	}
      else
	{
	  directinetch->set_active (false);
	}
    }
  else
    {
      directinetch->set_active (false);
    }
  prefvectmtx.unlock ();
  bx->append (*directinetch);
  lbr->set_child (*bx);
  listb->append (*lbr);

  Gtk::Button *apply = Gtk::make_managed<Gtk::Button> ();
  apply->set_halign (Gtk::Align::START);
  apply->set_margin (5);
  apply->set_name ("applyButton");
  apply->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  apply->set_label (gettext ("Apply"));
  grwin->attach (*apply, 0, 1, 1, 1);

  apply->signal_clicked ().connect (
      [window, this, lifcsval, locip6val, locip4val, bootstre, msgsze, partsze,
       winszch, sendkeycmb, soundch, soundpe, shutmte, tmtteare, langchenbch,
       enstunch, stunpe, directinetch]
      {
	std::string filename;
	std::filesystem::path filepath;
	AuxFunc af;
	af.homePath (&filename);
	filename = filename + "/.config/Communist/Prefs";
	filepath = std::filesystem::u8path (filename);
	if (!std::filesystem::exists (filepath))
	  {
	    std::filesystem::create_directories (filepath.parent_path ());
	  }

	this->prefvectmtx.lock ();
	auto itprv = std::find_if (this->prefvect.begin (),
				   this->prefvect.end (), []
				   (auto &el)
				     {
				       return std::get<0>(el) == "Listenifcs";
				     });
	if (itprv != this->prefvect.end ())
	  {
	    if (lifcsval->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    lifcsval->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "0.0.0.0:0,[::]:0";
	      }
	  }
	else
	  {
	    if (lifcsval->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Listenifcs",
				     lifcsval->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Listenifcs", "0.0.0.0:0,[::]:0"));
	      }
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Locip6port";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (locip6val->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    locip6val->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "[ff15::13]:48666";
	      }
	  }
	else
	  {
	    if (locip6val->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Locip6port",
				     locip6val->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Locip6port", "[ff15::13]:48666"));
	      }
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Locip4port";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (locip4val->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    locip4val->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "239.192.150.8:48655";
	      }
	  }
	else
	  {
	    if (locip4val->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Locip4port",
				     locip4val->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Locip4port", "239.192.150.8:48655"));
	      }
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Spellcheck";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (langchenbch->get_active ())
	      {
		std::get<1> (*itprv) = "1";
	      }
	    else
	      {
		std::get<1> (*itprv) = "0";
	      }
	  }
	else
	  {
	    std::tuple<std::string, std::string> ttup;
	    std::get<0> (ttup) = "Spellcheck";
	    if (langchenbch->get_active ())
	      {
		std::get<1> (ttup) = "1";
	      }
	    else
	      {
		std::get<1> (ttup) = "0";
	      }
	    this->prefvect.push_back (ttup);
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Bootstrapdht";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (bootstre->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    bootstre->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "router.bittorrent.com:6881";
	      }
	  }
	else
	  {
	    if (bootstre->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Bootstrapdht",
				     bootstre->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Bootstrapdht",
				     "router.bittorrent.com:6881"));
	      }
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Maxmsgsz";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (msgsze->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    msgsze->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "1048576";
	      }
	  }
	else
	  {
	    if (msgsze->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Maxmsgsz",
				     msgsze->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Maxmsgsz", "1048576"));
	      }
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Partsize";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (partsze->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    partsze->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "1371";
	      }
	  }
	else
	  {
	    if (partsze->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Partsize",
				     partsze->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (std::make_tuple ("Partsize", "1371"));
	      }
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Winsizesv";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (winszch->get_active ())
	      {
		std::get<1> (*itprv) = "active";
	      }
	    else
	      {
		std::get<1> (*itprv) = "notactive";
	      }
	  }
	else
	  {
	    if (winszch->get_active ())
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Winsizesv", "active"));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Winsizesv", "notactive"));
	      }
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "SendKey";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    int modei = sendkeycmb->get_active_row_number ();
	    std::stringstream strm;
	    std::locale loc ("C");
	    strm.imbue (loc);
	    strm << modei;
	    std::get<1> (*itprv) = strm.str ();
	  }
	else
	  {
	    int modei = sendkeycmb->get_active_row_number ();
	    std::stringstream strm;
	    std::locale loc ("C");
	    strm.imbue (loc);
	    strm << modei;
	    this->prefvect.push_back (std::make_tuple ("SendKey", "0"));
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "SoundOn";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (soundch->get_active ())
	      {
		std::get<1> (*itprv) = "active";
	      }
	    else
	      {
		std::get<1> (*itprv) = "notactive";
	      }
	  }
	else
	  {
	    if (soundch->get_active ())
	      {
		this->prefvect.push_back (
		    std::make_tuple ("SoundOn", "active"));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("SoundOn", "notactive"));
	      }
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "SoundPath";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    std::get<1> (*itprv) = std::string (
		soundpe->get_buffer ()->get_text ());
	  }
	else
	  {
	    this->prefvect.push_back (
		std::make_tuple ("SoundPath",
				 soundpe->get_buffer ()->get_text ()));
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "ShutTmt";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (shutmte->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    shutmte->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "600";
	      }
	  }
	else
	  {
	    if (shutmte->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("ShutTmt",
				     shutmte->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (std::make_tuple ("ShutTmt", "600"));
	      }
	  }

	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "TmtTear";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (tmtteare->get_buffer ()->get_text () != "")
	      {
		std::get<1> (*itprv) = std::string (
		    tmtteare->get_buffer ()->get_text ());
	      }
	    else
	      {
		std::get<1> (*itprv) = "20";
	      }
	  }
	else
	  {
	    if (tmtteare->get_buffer ()->get_text () != "")
	      {
		this->prefvect.push_back (
		    std::make_tuple ("TmtTear",
				     tmtteare->get_buffer ()->get_text ()));
	      }
	    else
	      {
		this->prefvect.push_back (std::make_tuple ("TmtTear", "20"));
	      }
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Stun";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (enstunch->get_active ())
	      {
		std::get<1> (*itprv) = "active";
	      }
	    else
	      {
		std::get<1> (*itprv) = "notactive";
	      }
	  }
	else
	  {
	    if (enstunch->get_active ())
	      {
		this->prefvect.push_back (std::make_tuple ("Stun", "active"));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("Stun", "notactive"));
	      }
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "Stunport";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    std::get<1> (*itprv) = std::string (
		stunpe->get_buffer ()->get_text ());
	  }
	else
	  {
	    this->prefvect.push_back (
		std::make_tuple ("Stunport",
				 stunpe->get_buffer ()->get_text ()));
	  }
	itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
	(auto &el)
	  {
	    return std::get<0>(el) == "DirectInet";
	  });
	if (itprv != this->prefvect.end ())
	  {
	    if (directinetch->get_active ())
	      {
		std::get<1> (*itprv) = "direct";
	      }
	    else
	      {
		std::get<1> (*itprv) = "notdirect";
	      }
	  }
	else
	  {
	    if (directinetch->get_active ())
	      {
		this->prefvect.push_back (
		    std::make_tuple ("DirectInet", "direct"));
	      }
	    else
	      {
		this->prefvect.push_back (
		    std::make_tuple ("DirectInet", "notdirect"));
	      }
	  }
	std::fstream f;
	f.open (filepath, std::ios_base::out | std::ios_base::binary);
	for (size_t i = 0; i < this->prefvect.size (); i++)
	  {
	    std::string line = std::get<0> (this->prefvect.at (i));
	    line = line + ": ";
	    line = line + std::get<1> (this->prefvect.at (i)) + "\n";
	    f.write (line.c_str (), line.size ());
	  }
	this->prefvectmtx.unlock ();
	f.close ();

	window->close ();
	Gtk::Window *windowinfo = new Gtk::Window;
	windowinfo->set_application (this->get_application ());
	windowinfo->set_modal (true);
	windowinfo->set_transient_for (*this);
	windowinfo->set_name ("settingsWindow");
	windowinfo->get_style_context ()->add_provider (
	    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	windowinfo->set_title (gettext ("Info"));
	Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
	grid->set_halign (Gtk::Align::CENTER);
	grid->set_valign (Gtk::Align::CENTER);
	windowinfo->set_child (*grid);

	Gtk::Label *info = Gtk::make_managed<Gtk::Label> ();
	info->set_halign (Gtk::Align::CENTER);
	info->set_margin (5);
	info->set_text (gettext ("Application will be closed now. "
				 "Start it again to apply changes."));
	info->set_justify (Gtk::Justification::CENTER);
	info->set_max_width_chars (40);
	info->set_wrap (true);
	info->set_wrap_mode (Pango::WrapMode::WORD);
	grid->attach (*info, 0, 0, 1, 1);

	Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
	close->set_halign (Gtk::Align::CENTER);
	close->set_margin (5);
	close->set_name ("applyButton");
	close->get_style_context ()->add_provider (
	    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	close->set_label (gettext ("Close"));
	grid->attach (*close, 0, 1, 1, 1);

	Gtk::Requisition rq1, rq2;
	grid->get_preferred_size (rq1, rq2);
	windowinfo->set_size_request (rq2.get_width (), rq2.get_height ());

	close->signal_clicked ().connect ( [this, windowinfo]
	{
	  windowinfo->close ();
	  this->close ();
	});

	windowinfo->signal_close_request ().connect ( [windowinfo]
	{
	  windowinfo->hide ();
	  delete windowinfo;
	  return true;
	},
						     false);

	windowinfo->show ();

      });

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
  cancel->set_halign (Gtk::Align::END);
  cancel->set_margin (5);
  cancel->set_name ("rejectButton");
  cancel->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->set_label (gettext ("Cancel"));
  grwin->attach (*cancel, 1, 1, 1, 1);

  cancel->signal_clicked ().connect (
      sigc::mem_fun (*window, &Gtk::Window::close));

  Gtk::Requisition rq1, rq2;
  listb->get_preferred_size (rq1, rq2);
  Gdk::Rectangle req = screenRes ();
  if (rq1.get_height () <= 0.7 * req.get_height ())
    {
      scrl->set_min_content_height (rq1.get_height ());
    }
  else
    {
      scrl->set_min_content_height (0.7 * req.get_height ());
    }

  if (rq1.get_width () <= 0.7 * req.get_width ())
    {
      scrl->set_min_content_width (rq1.get_width ());
    }
  else
    {
      scrl->set_min_content_width (0.7 * req.get_width ());
    }

  window->signal_close_request ().connect ( [window]
  {
    window->hide ();
    delete window;
    return true;
  },
					   false);
  window->show ();
}

Gdk::Rectangle
MainWindow::screenRes ()
{
  Glib::RefPtr<Gdk::Surface> surf = this->get_surface ();
  Glib::RefPtr<Gdk::Display> disp = this->get_display ();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface (surf);
  Gdk::Rectangle req;
  mon->get_geometry (req);
  return req;
}

void
MainWindow::formIPv6vect (std::string ip)
{
  ipv6vectmtx.lock ();
  ipv6vect.push_back (ip);
  ipv6vectmtx.unlock ();
}

void
MainWindow::formIPv4vect (std::string ip)
{
  ipv4vectmtx.lock ();
  ipv4vect.push_back (ip);
  ipv4vectmtx.unlock ();
}

void
MainWindow::ipv6Window ()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_modal (true);
  window->set_transient_for (*this);
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title (gettext ("Choose IPv6"));

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  grid->set_margin (5);
  window->set_child (*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
  lab->set_halign (Gtk::Align::CENTER);
  lab->set_margin (5);
  lab->set_justify (Gtk::Justification::FILL);
  lab->set_wrap (true);
  lab->set_wrap_mode (Pango::WrapMode::WORD);
  lab->set_text (
      gettext ("Program cannot automatically identify machine's IPv6. "
	       "Please choose manually. "
	       "Available variants are: "));
  lab->set_max_width_chars (40);
  grid->attach (*lab, 0, 0, 1, 1);

  Gtk::ComboBoxText *cmbtxt = Gtk::make_managed<Gtk::ComboBoxText> ();
  cmbtxt->set_halign (Gtk::Align::CENTER);
  cmbtxt->set_margin (5);
  ipv6vectmtx.lock ();
  for (size_t i = 0; i < ipv6vect.size (); i++)
    {
      cmbtxt->append (Glib::ustring (ipv6vect[i]));
    }
  ipv6vectmtx.unlock ();
  cmbtxt->set_active (0);
  grid->attach (*cmbtxt, 0, 1, 1, 1);

  Gtk::Button *choose = Gtk::make_managed<Gtk::Button> ();
  choose->set_halign (Gtk::Align::CENTER);
  choose->set_margin (5);
  choose->set_name ("applyButton");
  choose->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  choose->set_label (gettext ("Choose"));
  grid->attach (*choose, 0, 2, 1, 1);

  choose->signal_clicked ().connect ( [window]
  {
    window->close ();
  });

  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size (rq1, rq2);
  window->set_size_request (rq2.get_width (), rq2.get_height ());

  window->signal_close_request ().connect ( [window, cmbtxt, this]
  {
    std::string value (cmbtxt->get_active_text ());
    value.erase (0, value.find (" ") + std::string (" ").size ());
    this->oper->setIPv6 (value);
    this->ipv6vectmtx.lock ();
    this->ipv6vect.clear ();
    this->ipv6vectmtx.unlock ();
    window->hide ();
    delete window;
    return true;
  },
					   false);

  window->show ();
}

void
MainWindow::ipv4Window ()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_modal (true);
  window->set_transient_for (*this);
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title (gettext ("Choose IPv4"));

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  window->set_child (*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
  lab->set_halign (Gtk::Align::CENTER);
  lab->set_margin (5);
  lab->set_justify (Gtk::Justification::FILL);
  lab->set_wrap (true);
  lab->set_wrap_mode (Pango::WrapMode::WORD);
  lab->set_text (
      gettext ("Program cannot automatically identify machine's IPv4. "
	       "Please choose manually. "
	       "Available variants are: "));
  lab->set_max_width_chars (40);
  grid->attach (*lab, 0, 0, 1, 1);

  Gtk::ComboBoxText *cmbtxt = Gtk::make_managed<Gtk::ComboBoxText> ();
  cmbtxt->set_halign (Gtk::Align::CENTER);
  cmbtxt->set_margin (5);
  ipv6vectmtx.lock ();
  for (size_t i = 0; i < ipv4vect.size (); i++)
    {
      cmbtxt->append (Glib::ustring (ipv4vect[i]));
    }
  ipv6vectmtx.unlock ();
  cmbtxt->set_active (0);
  grid->attach (*cmbtxt, 0, 1, 1, 1);

  Gtk::Button *choose = Gtk::make_managed<Gtk::Button> ();
  choose->set_halign (Gtk::Align::CENTER);
  choose->set_margin (5);
  choose->set_name ("applyButton");
  choose->get_style_context ()->add_provider (css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  choose->set_label (gettext ("Choose"));
  grid->attach (*choose, 0, 2, 1, 1);

  choose->signal_clicked ().connect ( [window]
  {
    window->close ();
  });

  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size (rq1, rq2);
  window->set_size_request (rq2.get_width (), rq2.get_height ());

  window->signal_close_request ().connect ( [window, cmbtxt, this]
  {
    std::string value (cmbtxt->get_active_text ());
    value.erase (0, value.find (" ") + std::string (" ").size ());
    this->oper->setIPv4 (value);
    this->ipv4vectmtx.lock ();
    this->ipv4vect.clear ();
    this->ipv4vectmtx.unlock ();
    window->hide ();
    delete window;
    return true;
  },
					   false);

  window->show ();
}

void
MainWindow::fileDownloadProg (std::string *keyg,
			      std::filesystem::path *filepathg,
			      uint64_t *cursizeg, std::mutex *mtx)
{
  std::string key = *keyg;
  std::filesystem::path filepath = *filepathg;
  uint64_t cursize = *cursizeg;
  mtx->unlock ();
  fileprogrvectmtx.lock ();
  auto it = std::find_if (
      fileprogrvect.begin (), fileprogrvect.end (), [key, filepath]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == filepath)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (it != fileprogrvect.end ())
    {
      uint64_t locsz = cursize;
      double cursz = static_cast<double> (locsz);
      double fulsz = static_cast<double> (std::get<2> (*it));
      Gtk::ProgressBar *prb = std::get<3> (*it);
      prb->set_fraction (cursz / fulsz);
    }
  fileprogrvectmtx.unlock ();
}

void
MainWindow::fileSendProg (std::string *keyg, std::filesystem::path *filepathg,
			  uint64_t *cursizeg, std::mutex *mtx)
{
  std::string key = *keyg;
  std::filesystem::path filepath = *filepathg;
  uint64_t cursize = *cursizeg;
  mtx->unlock ();
  fileprogrvectmtx.lock ();
  auto it = std::find_if (
      fileprogrvect.begin (), fileprogrvect.end (), [key, filepath]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == filepath)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (it != fileprogrvect.end ())
    {
      uint64_t locsz = cursize;
      double cursz = static_cast<double> (locsz);
      double fulsz = static_cast<double> (std::get<2> (*it));
      Gtk::ProgressBar *prb = std::get<3> (*it);
      prb->set_fraction (cursz / fulsz);
    }
  else
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_halign (Gtk::Align::CENTER);
      lab->set_valign (Gtk::Align::START);
      lab->set_margin_start (5);
      lab->set_margin_end (5);
      lab->set_text (
	  gettext ("Sending ")
	      + Glib::ustring (filepath.filename ().u8string ()));
      Gtk::ProgressBar *frprgb = Gtk::make_managed<Gtk::ProgressBar> ();
      frprgb->set_halign (Gtk::Align::CENTER);
      frprgb->set_valign (Gtk::Align::START);
      frprgb->set_margin (5);
      frprgb->set_show_text (true);
      frprgb->set_name ("fileRPrB");
      frprgb->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      frprgb->set_fraction (0);

      Gtk::Button *cncl = Gtk::make_managed<Gtk::Button> ();
      cncl->set_halign (Gtk::Align::CENTER);
      cncl->set_margin (5);
      cncl->set_name ("cancelRepl");
      cncl->get_style_context ()->add_provider (css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      cncl->set_icon_name ("window-close-symbolic");
      cncl->signal_clicked ().connect ( [this, key, filepath]
      {
	this->fileprogrvectmtx.lock ();
	auto it = std::find_if (this->fileprogrvect.begin (), this->fileprogrvect.end (), [key, filepath]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == filepath)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
	if (it != this->fileprogrvect.end ())
	  {
	    this->dld_grid->remove (*(std::get<3> (*it)));
	    this->dld_grid->remove (*(std::get<4> (*it)));
	    this->dld_grid->remove (*(std::get<5> (*it)));
	    this->fileprogrvect.erase (it);
	  }
	if (this->fileprogrvect.size () == 0)
	  {
	    this->dld_win->hide ();
	    delete this->dld_win;
	    this->dld_win = nullptr;
	    this->dld_grid = nullptr;
	  }
	this->fileprogrvectmtx.unlock ();
	this->oper->cancelSendF (key, filepath);
      });
      std::tuple<std::string, std::filesystem::path, uint64_t,
	  Gtk::ProgressBar*, Gtk::Label*, Gtk::Button*> prbtup;
      std::get<0> (prbtup) = key;
      std::get<1> (prbtup) = filepath;
      std::get<2> (prbtup) = std::filesystem::file_size (filepath);
      std::get<3> (prbtup) = frprgb;
      std::get<4> (prbtup) = lab;
      std::get<5> (prbtup) = cncl;

      if (this->fileprogrvect.size () > 0)
	{
	  Gtk::Widget *sibl = std::get<3> (
	      this->fileprogrvect[this->fileprogrvect.size () - 1]);
	  this->dld_grid->attach_next_to (*lab, *sibl,
					  Gtk::PositionType::BOTTOM, 1, 1);
	  sibl = lab;
	  this->dld_grid->attach_next_to (*frprgb, *sibl,
					  Gtk::PositionType::BOTTOM, 1, 1);
	  sibl = frprgb;
	  this->dld_grid->attach_next_to (*cncl, *sibl,
					  Gtk::PositionType::RIGHT, 1, 1);
	}
      else
	{
	  this->dld_win = new Gtk::Window;
	  this->dld_win->set_application (this->get_application ());
	  this->dld_win->set_deletable (false);
	  this->dld_win->set_name ("settingsWindow");
	  this->dld_win->get_style_context ()->add_provider (
	      css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  this->dld_win->set_title (
	      gettext ("Downloading/sending file progress"));
	  this->dld_grid = Gtk::make_managed<Gtk::Grid> ();
	  this->dld_grid->set_halign (Gtk::Align::CENTER);
	  this->dld_win->set_child (*(this->dld_grid));
	  this->dld_grid->attach (*lab, 0, 0, 1, 1);
	  this->dld_grid->attach (*frprgb, 0, 1, 1, 1);
	  this->dld_grid->attach (*cncl, 1, 1, 1, 1);
	  this->dld_win->show ();
	}
      this->fileprogrvect.push_back (prbtup);
    }
  fileprogrvectmtx.unlock ();
}

void
MainWindow::autoRemoveMsg ()
{
  std::string mode;
  prefvectmtx.lock ();
  auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Autoremove";
    });
  if (itprv != prefvect.end ())
    {
      mode = std::get<1> (*itprv);
    }
  else
    {
      mode = "0";
    }
  prefvectmtx.unlock ();
  if (mode != "0")
    {
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist";
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
	  for (auto &cdirit : std::filesystem::directory_iterator (filepath))
	    {
	      std::filesystem::path p = cdirit.path ();
	      if (std::filesystem::is_directory (p)
		  && p.filename ().u8string () != "SendBufer"
		  && p.filename ().u8string () != "Bufer")
		{
		  std::vector<std::filesystem::path> msgs;
		  for (auto dirit : std::filesystem::directory_iterator (p))
		    {
		      std::filesystem::path p2 = dirit.path ();
		      if (p2.filename ().u8string () != "Profile"
			  && p2.filename ().u8string () != "Yes")
			{
			  msgs.push_back (p2);
			}
		    }
		  std::sort (msgs.begin (), msgs.end (), []
		  (auto &el1, auto &el2)
		    {
		      std::stringstream strm;
		      std::locale loc ("C");
		      strm.imbue(loc);
		      std::string f = el1.filename().u8string();
		      f = f.substr(0, f.find("f"));
		      std::string s = el2.filename().u8string();
		      s = s.substr(0, s.find("f"));
		      int fi;
		      strm << f;
		      strm >> fi;
		      strm.clear();
		      strm.str("");
		      strm.imbue(loc);
		      int si;
		      strm << s;
		      strm >> si;
		      return fi < si;
		    });
		  filename = p.u8string ();
		  filename = filename + "/Yes";
		  std::filesystem::path sp = std::filesystem::u8path (filename);
		  if (std::filesystem::exists (sp))
		    {
#ifdef __linux
		      filename =
			  std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
		      filename =
			  std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
		      filename = filename + "/CommAR/Yes";
		      std::filesystem::path outpath = std::filesystem::u8path (
			  filename);
		      if (std::filesystem::exists (outpath.parent_path ()))
			{
			  std::filesystem::remove_all (outpath.parent_path ());
			}
		      std::filesystem::create_directories (
			  outpath.parent_path ());
		      af.decryptFile (Username, Password, sp.u8string (),
				      outpath.u8string ());
		      std::vector<std::tuple<std::string, uint64_t>> yesv;
		      std::fstream f;
		      f.open (outpath, std::ios_base::in);
		      int count = 0;
		      std::string key;
		      while (!f.eof ())
			{
			  std::string line;
			  getline (f, line);
			  if (line != "" && count > 0)
			    {
			      std::tuple<std::string, uint64_t> ttup;
			      std::string tstr = line;
			      tstr = tstr.substr (0, tstr.find (" "));
			      std::get<0> (ttup) = tstr;
			      tstr = line;
			      tstr.erase (
				  0,
				  tstr.find (" ") + std::string (" ").size ());
			      std::stringstream strm;
			      std::locale loc ("C");
			      strm.imbue (loc);
			      strm << tstr;
			      uint64_t tm;
			      strm >> tm;
			      std::get<1> (ttup) = tm;
			      yesv.push_back (ttup);
			    }
			  if (count == 0)
			    {
			      key = line;
			      key = key + "\n";
			    }
			  count++;
			}
		      f.close ();
		      uint64_t dif;
		      time_t curtime = time (NULL);
		      if (mode == "1")
			{
			  dif = 24 * 3600;
			}
		      if (mode == "2")
			{
			  dif = 7 * 24 * 3600;
			}
		      if (mode == "3")
			{
			  dif = 31 * 24 * 3600;
			}
		      if (mode == "4")
			{
			  dif = 365 * 24 * 3600;
			}
		      yesv.erase (
			  std::remove_if (
			      yesv.begin (), yesv.end (), [&dif, &curtime]
			      (auto &el)
				{
				  return std::get<1>(el) < curtime - dif;
				}),
			  yesv.end ());
		      if (yesv.size () > 0)
			{
			  std::string lessrm = std::get<0> (yesv[0]);
			  std::stringstream strm;
			  std::locale loc ("C");
			  strm.imbue (loc);
			  strm << lessrm;
			  int lrm;
			  strm >> lrm;
			  msgs.erase (
			      std::remove_if (msgs.begin (), msgs.end (), [&lrm]
			      (auto &el)
				{
				  std::string fnm = el.filename().u8string();
				  fnm = fnm.substr(0, fnm.find ("f"));
				  std::stringstream strm;
				  std::locale loc ("C");
				  strm.imbue(loc);
				  strm << fnm;
				  int l;
				  strm >> l;
				  if (l < lrm)
				    {
				      std::filesystem::remove (el);
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			      msgs.end ());
			  for (size_t i = 0; i < msgs.size (); i++)
			    {
			      filename = msgs[i].filename ().u8string ();
			      std::string::size_type n;
			      n = filename.find ("f");
			      strm.clear ();
			      strm.str ("");
			      strm.imbue (loc);
			      strm << i;
			      filename = msgs[i].parent_path ().u8string ()
				  + "/" + strm.str ();
			      if (n != std::string::npos)
				{
				  filename = filename + "f";
				}
			      std::filesystem::path rnm =
				  std::filesystem::u8path (filename);
			      std::filesystem::rename (msgs[i], rnm);
			    }
			  f.open (outpath,
				  std::ios_base::out | std::ios_base::binary);
			  f.write (key.c_str (), key.size ());
			  for (size_t i = 0; i < yesv.size (); i++)
			    {
			      strm.clear ();
			      strm.str ("");
			      strm.imbue (loc);
			      strm << i;
			      std::string line;
			      line = strm.str ();
			      strm.clear ();
			      strm.str ("");
			      strm.imbue (loc);
			      strm << std::get<1> (yesv[i]);
			      line = line + " " + strm.str () + "\n";
			      f.write (line.c_str (), line.size ());
			    }
			  f.close ();
			  f.open (outpath, std::ios_base::in);
			  while (!f.eof ())
			    {
			      std::string line;
			      getline (f, line);
			      std::cout << line << std::endl;
			    }
			  f.close ();
			  af.cryptFile (Username, Password, outpath.u8string (),
					sp.u8string ());
			}
		      else
			{
			  for (size_t i = 0; i < msgs.size (); i++)
			    {
			      std::filesystem::remove (msgs[i]);
			    }
			  std::filesystem::remove (sp);
			}
		      std::filesystem::remove_all (outpath.parent_path ());
		    }
		}
	    }
	}
    }
}

void
MainWindow::aboutProg ()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application (this->get_application ());
  window->set_title (gettext ("About"));
  Gtk::Notebook *noteb = Gtk::make_managed<Gtk::Notebook> ();
  noteb->set_halign (Gtk::Align::CENTER);
  window->set_child (*noteb);

  std::fstream f;
  std::string filename;
  AuxFunc af;
  std::filesystem::path filepath;
  Glib::RefPtr<Gtk::TextBuffer> txtab = Gtk::TextBuffer::create ();
  Glib::ustring abbuf = gettext (
      "Communist is simple \"peer to peer\" messenger.\n"
      "Author Yury Bobylev.\n"
      "Program use next libraries:\n"
      "GTK https://www.gtk.org\n"
      "Libgcrypt https://www.gnupg.org/software/libgcrypt/index.html\n"
      "Libzip https://libzip.org\n"
      "Libtorrent https://www.libtorrent.org\n"
      "ICU https://icu.unicode.org");
  txtab->set_text (abbuf);
  Gtk::TextView *tvab = Gtk::make_managed<Gtk::TextView> ();
  tvab->set_buffer (txtab);
  tvab->set_editable (false);
  tvab->set_margin (5);
  Gdk::Rectangle scrres = screenRes ();
  Gtk::ScrolledWindow *scrab = Gtk::make_managed<Gtk::ScrolledWindow> ();
  scrab->set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  scrab->set_halign (Gtk::Align::CENTER);

  if (scrres.get_width () < 700)
    {
      scrab->set_min_content_width (scrres.get_width () * 0.7);
    }
  else
    {
      scrab->set_min_content_width (700);
    }
  if (scrres.get_height () < 700)
    {
      scrab->set_min_content_height (scrres.get_height () * 0.7);
    }
  else
    {
      scrab->set_min_content_height (700);
    }
  scrab->set_child (*tvab);
  noteb->append_page (*scrab, gettext ("About"));

  filename = Sharepath + "/COPYING";
  filepath = std::filesystem::u8path (filename);
  Glib::RefPtr<Gtk::TextBuffer> txtlc = Gtk::TextBuffer::create ();
  f.open (filepath, std::ios_base::in | std::ios_base::binary);
  if (f.is_open ())
    {
      size_t sz = std::filesystem::file_size (filepath);
      std::vector<char> ab;
      ab.resize (sz);
      f.read (&ab[0], ab.size ());
      f.close ();
      Glib::ustring abbuf (ab.begin (), ab.end ());
      txtlc->set_text (abbuf);
    }
  else
    {
      std::cerr << "License file not found" << std::endl;
    }

  Gtk::TextView *tvlc = Gtk::make_managed<Gtk::TextView> ();
  tvlc->set_buffer (txtlc);
  tvlc->set_editable (false);
  tvlc->set_margin (5);

  Gtk::ScrolledWindow *scrlc = Gtk::make_managed<Gtk::ScrolledWindow> ();
  scrlc->set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  scrlc->set_halign (Gtk::Align::CENTER);
  if (scrres.get_width () < 700)
    {
      scrlc->set_min_content_width (scrres.get_width () * 0.7);
    }
  else
    {
      scrlc->set_min_content_width (700);
    }
  if (scrres.get_height () < 700)
    {
      scrlc->set_min_content_height (scrres.get_height () * 0.7);
    }
  else
    {
      scrlc->set_min_content_height (700);
    }
  scrlc->set_child (*tvlc);
  noteb->append_page (*scrlc, gettext ("License"));

  window->signal_close_request ().connect ( [window]
  {
    window->hide ();
    delete window;
    return true;
  },
					   false);
  window->show ();
}

void
MainWindow::checkIfConnected (std::string *key, uint64_t *tm, std::mutex *mtx)
{
  std::string keyloc = *key;
  uint64_t tmloc = *tm;
  mtx->unlock ();
  if (tmloc == 0)
    {
      chifcmtx.lock ();
      auto itchfc = std::find_if (chifc.begin (), chifc.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	});
      if (itchfc != chifc.end ())
	{
	  time_t curtime = time (NULL);
	  time_t tmttear = 20;
	  prefvectmtx.lock ();
	  auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "TmtTear";
	    });
	  if (itprv != prefvect.end ())
	    {
	      std::stringstream strm;
	      std::locale loc ("C");
	      strm.imbue (loc);
	      strm << std::get<1> (*itprv);
	      strm >> tmttear;
	    }
	  prefvectmtx.unlock ();
	  if (curtime - std::get<1> (*itchfc) > uint64_t (tmttear))
	    {
	      frvectmtx.lock ();
	      auto itfrv =
		  std::find_if (
		      friendvect.begin (),
		      friendvect.end (),
		      [&keyloc]
		      (auto &el)
			{
			  return std::get<2>(el)->get_text() == Glib::ustring(keyloc);
			});
	      if (itfrv != friendvect.end ())
		{
		  Gtk::Grid *bg = std::get<1> (*itfrv);
		  bg->remove (*(std::get<2> (*itchfc)));
		}
	      frvectmtx.unlock ();
	      chifc.erase (itchfc);
	    }
	}
      chifcmtx.unlock ();
    }
  else
    {
      chifcmtx.lock ();
      auto itchfc = std::find_if (chifc.begin (), chifc.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	});
      if (itchfc != chifc.end ())
	{
	  std::get<1> (*itchfc) = tmloc;
	}
      else
	{
	  frvectmtx.lock ();
	  auto itfrv = std::find_if (
	      friendvect.begin (), friendvect.end (), [&keyloc]
	      (auto &el)
		{
		  return std::get<2>(el)->get_text() == Glib::ustring(keyloc);
		});
	  if (itfrv != friendvect.end ())
	    {
	      Gtk::Label *onl = Gtk::make_managed<Gtk::Label> ();
	      onl->set_text (gettext ("online"));
	      onl->set_margin (5);
	      onl->set_halign (Gtk::Align::END);
	      onl->set_name ("connectFr");
	      onl->get_style_context ()->add_provider (
		  this->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      Gtk::Grid *bg = std::get<1> (*itfrv);
	      Gtk::Label *nml = std::get<5> (*itfrv);
	      bg->attach_next_to (*onl, *nml, Gtk::PositionType::RIGHT, 1, 1);

	      chifc.push_back (std::make_tuple (keyloc, tmloc, onl));
	    }
	  frvectmtx.unlock ();

	}
      chifcmtx.unlock ();
    }
}

void
MainWindow::friendRemoved (std::string *key, std::mutex *mtx)
{
  std::string keyloc = *key;
  mtx->unlock ();

  contmtx.lock ();
  auto iter = std::find_if (contacts.begin (), contacts.end (), [&keyloc]
  (auto &el)
    {
      return keyloc == std::get<1>(el);
    });
  if (iter != contacts.end ())
    {
      AuxFunc af;
      std::string folder;
      af.homePath (&folder);
      std::stringstream strm;
      std::locale loc ("C");
      std::filesystem::path folderpath;
      int indep = std::get<0> (*iter);
      contacts.erase (iter);
#ifdef __linux
      std::string filename =
	  std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      std::string filename =
  		      std::filesystem::temp_directory_path ().parent_path ().u8string ();
  #endif
      filename = filename + "/Communist";
      folderpath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (folderpath))
	{
	  std::filesystem::remove_all (folderpath);
	}
      std::filesystem::create_directories (folderpath);
      filename = filename + "/Profile.zip";
      af.homePath (&folder);
      folder = folder + "/.Communist/Profile";
      proffopmtx.lock ();
      af.decryptFile (Username, Password, folder, filename);
#ifdef __linux
      folder = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      folder = std::filesystem::temp_directory_path ().parent_path ().u8string ();
  #endif
      folder = folder + "/Communist";
      af.unpacking (filename, folder);
      folderpath = std::filesystem::u8path (filename);
      std::filesystem::remove (folderpath);
      folder = folder + "/Profile/Contacts";
      folderpath = std::filesystem::u8path (folder);
      std::string line;
      std::fstream f;
      f.open (folderpath, std::ios_base::out | std::ios_base::binary);
      for (size_t i = 0; i < contacts.size (); i++)
	{
	  line = std::get<1> (contacts[i]);
	  line = line + "\n";
	  f.write (line.c_str (), line.size ());
	}
      f.close ();
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
  #endif
      filename = filename + "/Communist/Profile";
#ifdef __linux
      folder = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      folder = std::filesystem::temp_directory_path ().parent_path ().u8string ();
  #endif
      folder = folder + "/Communist/Profile.zip";
      af.packing (filename, folder);
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      folderpath = std::filesystem::u8path (filename);
      std::filesystem::remove (folderpath);
      filename = folderpath.u8string ();
      af.cryptFile (Username, Password, folder, filename);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
  #endif
      filename = filename + "/Communist";
      folderpath = std::filesystem::u8path (filename);
      std::filesystem::remove_all (folderpath);
      proffopmtx.unlock ();
      for (size_t i = 0; i < contacts.size (); i++)
	{
	  if (std::get<0> (contacts[i]) > indep)
	    {
	      std::get<0> (contacts[i]) = std::get<0> (contacts[i]) - 1;
	    }
	}
    }
  contmtx.unlock ();
  mainWindow ();
}
