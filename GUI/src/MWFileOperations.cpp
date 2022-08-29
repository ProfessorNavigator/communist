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

#include <MWFileOperations.h>

MWFileOperations::MWFileOperations (MainWindow *mw)
{
  this->mw = mw;
}

MWFileOperations::~MWFileOperations ()
{
  // TODO Auto-generated destructor stub
}

void
MWFileOperations::fileDownloadProg (std::string *keyg,
				    std::filesystem::path *filepathg,
				    uint64_t *cursizeg, std::mutex *mtx)
{
  std::string key = *keyg;
  std::filesystem::path filepath = *filepathg;
  uint64_t cursize = *cursizeg;
  mtx->unlock ();
  mw->fileprogrvectmtx.lock ();
  auto it = std::find_if (
      mw->fileprogrvect.begin (), mw->fileprogrvect.end (), [key, filepath]
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
  if (it != mw->fileprogrvect.end ())
    {
      uint64_t locsz = cursize;
      double cursz = static_cast<double> (locsz);
      double fulsz = static_cast<double> (std::get<2> (*it));
      Gtk::ProgressBar *prb = std::get<3> (*it);
      prb->set_fraction (cursz / fulsz);
    }
  mw->fileprogrvectmtx.unlock ();
}

void
MWFileOperations::fileSendProg (std::string *keyg,
				std::filesystem::path *filepathg,
				uint64_t *cursizeg, std::mutex *mtx)
{
  std::string key = *keyg;
  std::filesystem::path filepath = *filepathg;
  uint64_t cursize = *cursizeg;
  mtx->unlock ();
  mw->fileprogrvectmtx.lock ();
  auto it = std::find_if (
      mw->fileprogrvect.begin (), mw->fileprogrvect.end (), [key, filepath]
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
  if (it != mw->fileprogrvect.end ())
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
      frprgb->get_style_context ()->add_provider (mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      frprgb->set_fraction (0);

      Gtk::Button *cncl = Gtk::make_managed<Gtk::Button> ();
      cncl->set_halign (Gtk::Align::CENTER);
      cncl->set_margin (5);
      cncl->set_name ("cancelRepl");
      cncl->get_style_context ()->add_provider (mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      cncl->set_icon_name ("window-close-symbolic");
      cncl->signal_clicked ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MWFileOperations::fileSendCncl),
		      mw, key, filepath));
      std::tuple<std::string, std::filesystem::path, uint64_t,
	  Gtk::ProgressBar*, Gtk::Label*, Gtk::Button*> prbtup;
      std::get<0> (prbtup) = key;
      std::get<1> (prbtup) = filepath;
      std::get<2> (prbtup) = std::filesystem::file_size (filepath);
      std::get<3> (prbtup) = frprgb;
      std::get<4> (prbtup) = lab;
      std::get<5> (prbtup) = cncl;

      if (mw->fileprogrvect.size () > 0)
	{
	  Gtk::Widget *sibl = std::get<3> (
	      mw->fileprogrvect[mw->fileprogrvect.size () - 1]);
	  mw->dld_grid->attach_next_to (*lab, *sibl, Gtk::PositionType::BOTTOM,
					1, 1);
	  sibl = lab;
	  mw->dld_grid->attach_next_to (*frprgb, *sibl,
					Gtk::PositionType::BOTTOM, 1, 1);
	  sibl = frprgb;
	  mw->dld_grid->attach_next_to (*cncl, *sibl, Gtk::PositionType::RIGHT,
					1, 1);
	}
      else
	{
	  mw->dld_win = new Gtk::Window;
	  mw->dld_win->set_application (mw->get_application ());
	  mw->dld_win->set_deletable (false);
	  mw->dld_win->set_name ("settingsWindow");
	  mw->dld_win->get_style_context ()->add_provider (
	      mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  mw->dld_win->set_title (
	      gettext ("Downloading/sending file progress"));
	  mw->dld_grid = Gtk::make_managed<Gtk::Grid> ();
	  mw->dld_grid->set_halign (Gtk::Align::CENTER);
	  mw->dld_win->set_child (*(mw->dld_grid));
	  mw->dld_grid->attach (*lab, 0, 0, 1, 1);
	  mw->dld_grid->attach (*frprgb, 0, 1, 1, 1);
	  mw->dld_grid->attach (*cncl, 1, 1, 1, 1);
	  mw->dld_win->show ();
	}
      mw->fileprogrvect.push_back (prbtup);
    }
  mw->fileprogrvectmtx.unlock ();
}
void
MWFileOperations::fileSendCncl (MainWindow *mwl, std::string key,
				std::filesystem::path filepath)
{
  mwl->fileprogrvectmtx.lock ();
  auto it = std::find_if (
      mwl->fileprogrvect.begin (), mwl->fileprogrvect.end (), [key, filepath]
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
  if (it != mwl->fileprogrvect.end ())
    {
      mwl->dld_grid->remove (*(std::get<3> (*it)));
      mwl->dld_grid->remove (*(std::get<4> (*it)));
      mwl->dld_grid->remove (*(std::get<5> (*it)));
      mwl->fileprogrvect.erase (it);
    }
  if (mwl->fileprogrvect.size () == 0)
    {
      mwl->dld_win->hide ();
      delete mwl->dld_win;
      mwl->dld_win = nullptr;
      mwl->dld_grid = nullptr;
    }
  mwl->fileprogrvectmtx.unlock ();
  mwl->oper->cancelSendFile (key, filepath);
}

void
MWFileOperations::fileRcvdStatus (std::string *key,
				  std::filesystem::path *filename,
				  std::mutex *dispNmtx, int variant)
{
  std::string keyloc = *key;
  std::filesystem::path filenm = *filename;
  Glib::ustring nick = "";
  dispNmtx->unlock ();

  mw->frvectmtx.lock ();
  auto itfrv = std::find_if (mw->friendvect.begin (), mw->friendvect.end (),
			     [&keyloc]
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
  if (itfrv != mw->friendvect.end ())
    {
      nick = std::get<4> (*itfrv)->get_text ();
    }
  mw->frvectmtx.unlock ();

  if (nick != "")
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application (mw->get_application ());
      window->set_title (gettext ("File received"));
      window->set_transient_for (*mw);
      window->set_modal (true);
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (mw->css_provider,
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
	      lab1 + " " + Glib::ustring (filenm.filename ().u8string ()) + "\n"
		  + lab2 + " " + nick + "\n" + lab3);
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
	      lab4 + " " + Glib::ustring (filenm.filename ().u8string ()) + "\n"
		  + lab2 + " " + nick + "\n " + lab5);
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
	      lab1 + " " + Glib::ustring (filenm.filename ().u8string ()) + " "
		  + lab6 + " " + nick + "\n" + lab7);
	}
      if (variant == 9)
	{
	  Glib::ustring lab8;
	  lab8 = gettext ("Error on sending");
	  Glib::ustring lab6;
	  lab6 = gettext ("for");
	  lab->set_text (
	      lab8 + " " + Glib::ustring (filenm.filename ().u8string ()) + " "
		  + lab6 + " " + nick);
	}

      lab->set_justify (Gtk::Justification::CENTER);
      lab->set_halign (Gtk::Align::CENTER);
      lab->set_margin (5);
      grid->attach (*lab, 0, 0, 1, 1);

      Gtk::CheckButton *remf = nullptr;
      if (variant == 7)
	{
	  remf = Gtk::make_managed<Gtk::CheckButton> ();
	  remf->set_halign (Gtk::Align::START);
	  remf->set_margin (5);
	  remf->set_active (false);
	  remf->set_label (gettext ("Remove received data"));
	  grid->attach (*remf, 0, 1, 1, 1);
	}

      Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
      close->set_label (gettext ("Close"));
      close->set_halign (Gtk::Align::CENTER);
      close->set_margin (5);
      close->set_name ("applyButton");
      close->get_style_context ()->add_provider (mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      close->signal_clicked ().connect (
	  sigc::mem_fun (*window, &Gtk::Window::close));
      if (variant == 7)
	{
	  grid->attach (*close, 0, 2, 1, 1);
	}
      else
	{
	  grid->attach (*close, 0, 1, 1, 1);
	}

      window->signal_close_request ().connect ( [window, remf, filenm]
      {
	if (remf != nullptr)
	  {
	    if (remf->get_active ())
	      {
		std::filesystem::remove_all (filenm);
	      }
	  }
	window->hide ();
	delete window;
	return true;
      },
					       false);
      window->show ();
    }

  mw->fileprogrvectmtx.lock ();
  MainWindow *mwl = mw;
  mw->fileprogrvect.erase (
      std::remove_if (
	  mw->fileprogrvect.begin (), mw->fileprogrvect.end (),
	  [&keyloc, filenm, mwl]
	  (auto &el)
	    {
	      if (std::get<0>(el) == keyloc && std::get<1>(el) == filenm)
		{
		  Gtk::Grid *gr = mwl->dld_grid;
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
      mw->fileprogrvect.end ());
  if (mw->fileprogrvect.size () == 0 && mw->dld_win != nullptr)
    {
      mw->dld_win->hide ();
      delete mw->dld_win;
      mw->dld_win = nullptr;
      mw->dld_grid = nullptr;
    }
  mw->fileprogrvectmtx.unlock ();
}

void
MWFileOperations::fileRejectedSlot (std::string *keyr,
				    std::filesystem::path *filer,
				    std::mutex *disp5mtx)
{
  std::string key = *keyr;
  std::filesystem::path locfp = *filer;
  disp5mtx->unlock ();
  mw->frvectmtx.lock ();
  auto itfrv = std::find_if (
      mw->friendvect.begin (), mw->friendvect.end (), [&key]
      (auto &el)
	{
	  return key == std::string (std::get<2>(el)->get_text());
	});
  if (itfrv != mw->friendvect.end ())
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_title (gettext ("File rejected"));
      window->set_transient_for (*mw);
      window->set_modal (true);
      window->set_name ("settingsWindow");
      window->get_style_context ()->add_provider (mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      grid->set_halign (Gtk::Align::CENTER);
      window->set_child (*grid);

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      Glib::ustring lstr;
      lstr = std::get<4> (*itfrv)->get_text ();
      lstr = lstr + lstr = lstr + gettext (" rejected to accept ")
	  + locfp.filename ().u8string () + gettext (" file!");
      lab->set_text (lstr);
      lab->set_halign (Gtk::Align::CENTER);
      lab->set_margin (5);
      grid->attach (*lab, 0, 0, 1, 1);

      Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
      close->set_label (gettext ("Close"));
      close->set_halign (Gtk::Align::CENTER);
      close->set_margin (5);
      close->set_name ("applyButton");
      close->get_style_context ()->add_provider (mw->css_provider,
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

      window->set_application (mw->get_application ());
      window->show ();
    }
  mw->frvectmtx.unlock ();
}

void
MWFileOperations::fileRequestSlot (std::string *key, uint64_t *tm,
				   uint64_t *filesize, std::string *filename,
				   std::mutex *disp4mtx)
{
  std::string keyloc = *key;
  uint64_t tml = *tm;
  uint64_t filesizeloc = *filesize;
  std::string fnmloc = *filename;
  disp4mtx->unlock ();
  mw->blockfreqmtx.lock ();
  int cheknum = 0;
  time_t curtime = time (NULL);
  mw->blockfreq.erase (
      std::remove_if (mw->blockfreq.begin (), mw->blockfreq.end (), [&curtime]
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
      mw->blockfreq.end ());
  for (size_t i = 0; i < mw->blockfreq.size (); i++)
    {
      if (std::get<0> (mw->blockfreq[i]) == keyloc
	  && std::get<2> (mw->blockfreq[i]) == 1)
	{
	  if (tml - std::get<1> (mw->blockfreq[i]) < 600)
	    {
	      cheknum++;
	    }
	}
    }
  if (cheknum <= 5)
    {
      auto it = std::find_if (
	  mw->blockfreq.begin (), mw->blockfreq.end (), [&keyloc, &tml]
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
      if (it == mw->blockfreq.end ())
	{
	  mw->frvectmtx.lock ();
	  auto itfrv = std::find_if (
	      mw->friendvect.begin (), mw->friendvect.end (), [&keyloc]
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
	  if (itfrv != mw->friendvect.end ())
	    {
	      AuxFunc af;
	      std::string defs;
	      af.homePath (&defs);
	      std::filesystem::path def = std::filesystem::u8path (defs);
	      mw->blockfreq.push_back (std::make_tuple (keyloc, tml, 0, def));
	      Glib::ustring nick (std::get<4> (*itfrv)->get_text ());
	      Gtk::Window *window = new Gtk::Window;
	      window->set_application (mw->get_application ());
	      window->set_name ("settingsWindow");
	      window->get_style_context ()->add_provider (
		  mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
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
		  mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      grid->attach (*yes, 0, 4, 1, 1);

	      yes->signal_clicked ().connect (
		  sigc::bind (
		      sigc::mem_fun (*this,
				     &MWFileOperations::acceptButtonSlot),
		      window, mw, keyloc, tml, fnmloc, fs));

	      Gtk::Button *no = Gtk::make_managed<Gtk::Button> ();
	      no->set_label (gettext ("No"));
	      no->set_margin (5);
	      no->set_halign (Gtk::Align::CENTER);
	      no->set_name ("rejectButton");
	      no->get_style_context ()->add_provider (
		  mw->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      grid->attach (*no, 1, 4, 1, 1);

	      no->signal_clicked ().connect (
		  sigc::bind (
		      sigc::mem_fun (*this,
				     &MWFileOperations::rejectButtonSlot),
		      window, mw, keyloc, tml));
	      window->signal_close_request ().connect ( [window]
	      {
		window->hide ();
		delete window;
		return true;
	      },
						       false);

	      window->show ();
	    }
	  mw->frvectmtx.unlock ();
	}
      else
	{
	  if (std::get<2> (*it) == 1)
	    {
	      mw->oper->fileReject (keyloc, tml);
	    }
	}
    }
  mw->blockfreqmtx.unlock ();
}

void
MWFileOperations::acceptButtonSlot (Gtk::Window *window, MainWindow *mwl,
				    std::string keyloc, uint64_t tml,
				    std::string fnmloc, uint64_t fs)
{
  Gtk::FileChooserDialog *fcd = new Gtk::FileChooserDialog (
      *mwl, gettext ("Path selection"),
      Gtk::FileChooser::Action::SELECT_FOLDER);
  fcd->set_transient_for (*mwl);
  std::string filename;
  AuxFunc af;
  af.homePath (&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path (filename);
  fcd->set_current_folder (fl);
  fcd->set_application (mwl->get_application ());
  Gtk::Button *btn;
  btn = fcd->add_button (gettext ("Open"), Gtk::ResponseType::APPLY);
  btn->set_halign (Gtk::Align::END);
  btn->set_margin (5);
  btn = fcd->add_button (gettext ("Cancel"), Gtk::ResponseType::CANCEL);
  btn->set_halign (Gtk::Align::START);
  btn->set_margin (5);
  fcd->signal_response ().connect (
      sigc::bind (sigc::mem_fun (*this, &MWFileOperations::openButtonSlot), fcd,
		  mwl, keyloc, tml, fnmloc, fs));
  fcd->signal_close_request ().connect ( [fcd]
  {
    fcd->hide ();
    delete fcd;
    return true;
  },
					false);

  fcd->show ();

  window->close ();
}

void
MWFileOperations::rejectButtonSlot (Gtk::Window *window, MainWindow *mwl,
				    std::string keyloc, uint64_t tml)
{
  mwl->blockfreqmtx.lock ();
  auto it = std::find_if (
      mwl->blockfreq.begin (), mwl->blockfreq.end (), [keyloc, tml]
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
  if (it != mwl->blockfreq.end ())
    {
      std::get<2> (*it) = 1;
      mwl->oper->fileReject (keyloc, tml);
    }
  mwl->blockfreqmtx.unlock ();
  window->close ();
}

void
MWFileOperations::openButtonSlot (int rid, Gtk::FileChooserDialog *fcd,
				  MainWindow *mwl, std::string keyloc,
				  uint64_t tml, std::string fnmloc, uint64_t fs)
{
  if (rid == Gtk::ResponseType::APPLY)
    {
      AuxFunc af;
      std::string loc;
      Glib::RefPtr<Gio::File> fl = fcd->get_file ();
      if (fl)
	{
	  loc = fl->get_path () + "/" + fnmloc;
	}
      else
	{
	  fl = fcd->get_current_folder ();
	  if (fl)
	    {
	      loc = fl->get_path () + "/" + fnmloc;
	    }
	  else
	    {
	      af.homePath (&loc);
	      loc = loc + "/" + fnmloc;
	    }
	}
      std::filesystem::path fp = std::filesystem::u8path (loc);
      if (std::filesystem::exists (fp))
	{
	  Gtk::Window *window = new Gtk::Window;
	  window->set_application (mwl->get_application ());
	  window->set_name ("settingsWindow");
	  window->get_style_context ()->add_provider (
	      mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  window->set_title (gettext ("Warning!"));
	  window->set_modal (true);
	  window->set_transient_for (*fcd);
	  Gtk::Grid *gr = Gtk::make_managed<Gtk::Grid> ();
	  gr->set_halign (Gtk::Align::CENTER);
	  window->set_child (*gr);

	  Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
	  lab->set_halign (Gtk::Align::CENTER);
	  lab->set_margin (5);
	  lab->set_text (gettext ("File already exists, replace?"));
	  gr->attach (*lab, 0, 0, 2, 1);

	  Gtk::Button *accept = Gtk::make_managed<Gtk::Button> ();
	  accept->set_halign (Gtk::Align::CENTER);
	  accept->set_margin (5);
	  accept->set_name ("applyButton");
	  accept->get_style_context ()->add_provider (
	      mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  accept->set_label (gettext ("Yes"));
	  accept->signal_clicked ().connect (
	      sigc::bind (
		  sigc::mem_fun (*this, &MWFileOperations::warningAcceptButton),
		  window, fcd, fp, mwl, keyloc, tml, fs));
	  gr->attach (*accept, 0, 1, 1, 1);

	  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
	  cancel->set_halign (Gtk::Align::CENTER);
	  cancel->set_margin (5);
	  cancel->set_name ("rejectButton");
	  cancel->get_style_context ()->add_provider (
	      mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	  cancel->set_label (gettext ("No"));
	  cancel->signal_clicked ().connect (
	      sigc::mem_fun (*window, &Gtk::Window::close));
	  gr->attach (*cancel, 1, 1, 1, 1);

	  window->signal_close_request ().connect ( [window]
	  {
	    window->hide ();
	    delete window;
	    return true;
	  },
						   false);
	  window->show ();
	}
      else
	{
	  mwl->blockfreqmtx.lock ();
	  auto it = std::find_if (
	      mwl->blockfreq.begin (), mwl->blockfreq.end (), [keyloc, tml]
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
	  if (it != mwl->blockfreq.end ())
	    {
	      std::get<2> (*it) = 2;
	      std::get<3> (*it) = fp;
	      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
	      lab->set_halign (Gtk::Align::CENTER);
	      lab->set_valign (Gtk::Align::START);
	      lab->set_margin_start (5);
	      lab->set_margin_end (5);
	      lab->set_text (
		  gettext ("Downloading ")
		      + Glib::ustring (fp.filename ().u8string ()));
	      Gtk::ProgressBar *frprgb = Gtk::make_managed<Gtk::ProgressBar> ();
	      frprgb->set_halign (Gtk::Align::CENTER);
	      frprgb->set_valign (Gtk::Align::START);
	      frprgb->set_margin (5);
	      frprgb->set_show_text (true);
	      frprgb->set_name ("fileRPrB");
	      frprgb->get_style_context ()->add_provider (mwl->css_provider,
	      GTK_STYLE_PROVIDER_PRIORITY_USER);
	      frprgb->set_fraction (0);

	      Gtk::Button *cncl = Gtk::make_managed<Gtk::Button> ();
	      cncl->set_halign (Gtk::Align::CENTER);
	      cncl->set_margin (5);
	      cncl->set_name ("cancelRepl");
	      cncl->get_style_context ()->add_provider (
		  mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
	      cncl->set_icon_name ("window-close-symbolic");
	      cncl->signal_clicked ().connect (
		  sigc::bind (
		      sigc::mem_fun (*this, &MWFileOperations::cancelRcvFile),
		      mwl, keyloc, fp));

	      mwl->fileprogrvectmtx.lock ();
	      std::tuple<std::string, std::filesystem::path, uint64_t,
		  Gtk::ProgressBar*, Gtk::Label*, Gtk::Button*> prbtup;
	      std::get<0> (prbtup) = keyloc;
	      std::get<1> (prbtup) = fp;
	      std::get<2> (prbtup) = fs;
	      std::get<3> (prbtup) = frprgb;
	      std::get<4> (prbtup) = lab;
	      std::get<5> (prbtup) = cncl;
	      if (mwl->fileprogrvect.size () > 0)
		{
		  Gtk::Widget *sibl = std::get<3> (
		      mwl->fileprogrvect[mwl->fileprogrvect.size () - 1]);
		  mwl->dld_grid->attach_next_to (*lab, *sibl,
						 Gtk::PositionType::BOTTOM, 1,
						 1);
		  sibl = lab;
		  mwl->dld_grid->attach_next_to (*frprgb, *sibl,
						 Gtk::PositionType::BOTTOM, 1,
						 1);
		  sibl = frprgb;
		  mwl->dld_grid->attach_next_to (*cncl, *sibl,
						 Gtk::PositionType::RIGHT, 1,
						 1);
		}
	      else
		{
		  mwl->dld_win = new Gtk::Window;
		  mwl->dld_win->set_application (mwl->get_application ());
		  mwl->dld_win->set_name ("settingsWindow");
		  mwl->dld_win->get_style_context ()->add_provider (
		      mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
		  mwl->dld_win->set_deletable (false);
		  mwl->dld_win->set_title (
		      gettext ("Downloading/sending file progress"));
		  mwl->dld_grid = Gtk::make_managed<Gtk::Grid> ();
		  mwl->dld_grid->set_halign (Gtk::Align::CENTER);
		  mwl->dld_win->set_child (*(mwl->dld_grid));
		  mwl->dld_grid->attach (*lab, 0, 0, 1, 1);
		  mwl->dld_grid->attach (*frprgb, 0, 1, 1, 1);
		  mwl->dld_grid->attach (*cncl, 1, 1, 1, 1);
		  mwl->dld_win->show ();
		}
	      mwl->fileprogrvect.push_back (prbtup);
	      mwl->fileprogrvectmtx.unlock ();
	      mwl->oper->fileAccept (keyloc, tml, fp);
	    }
	  mwl->blockfreqmtx.unlock ();
	  fcd->close ();
	}

    }
  if (rid == Gtk::ResponseType::CANCEL)
    {
      mwl->blockfreqmtx.lock ();
      auto it = std::find_if (
	  mwl->blockfreq.begin (), mwl->blockfreq.end (), [keyloc, tml]
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
      if (it != mwl->blockfreq.end ())
	{
	  std::get<2> (*it) = 1;
	  mwl->oper->fileReject (keyloc, tml);
	}
      mwl->blockfreqmtx.unlock ();
      fcd->close ();
    }
}

void
MWFileOperations::warningAcceptButton (Gtk::Window *window,
				       Gtk::FileChooserDialog *fcd,
				       std::filesystem::path fp,
				       MainWindow *mwl, std::string keyloc,
				       uint64_t tml, uint64_t fs)
{
  std::filesystem::remove_all (fp);
  mwl->blockfreqmtx.lock ();
  auto it = std::find_if (
      mwl->blockfreq.begin (), mwl->blockfreq.end (), [keyloc, tml]
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
  if (it != mwl->blockfreq.end ())
    {
      std::get<2> (*it) = 2;
      std::get<3> (*it) = fp;
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_halign (Gtk::Align::CENTER);
      lab->set_valign (Gtk::Align::START);
      lab->set_margin_start (5);
      lab->set_margin_end (5);
      lab->set_text (
	  gettext ("Downloading ")
	      + Glib::ustring (fp.filename ().u8string ()));
      Gtk::ProgressBar *frprgb = Gtk::make_managed<Gtk::ProgressBar> ();
      frprgb->set_halign (Gtk::Align::CENTER);
      frprgb->set_valign (Gtk::Align::START);
      frprgb->set_margin (5);
      frprgb->set_show_text (true);
      frprgb->set_name ("fileRPrB");
      frprgb->get_style_context ()->add_provider (mwl->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      frprgb->set_fraction (0);

      Gtk::Button *cncl = Gtk::make_managed<Gtk::Button> ();
      cncl->set_halign (Gtk::Align::CENTER);
      cncl->set_margin (5);
      cncl->set_name ("cancelRepl");
      cncl->get_style_context ()->add_provider (
	  mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
      cncl->set_icon_name ("window-close-symbolic");

      cncl->signal_clicked ().connect (
	  sigc::bind (sigc::mem_fun (*this, &MWFileOperations::cancelRcvFile),
		      mwl, keyloc, fp));

      mwl->fileprogrvectmtx.lock ();
      std::tuple<std::string, std::filesystem::path, uint64_t,
	  Gtk::ProgressBar*, Gtk::Label*, Gtk::Button*> prbtup;
      std::get<0> (prbtup) = keyloc;
      std::get<1> (prbtup) = fp;
      std::get<2> (prbtup) = fs;
      std::get<3> (prbtup) = frprgb;
      std::get<4> (prbtup) = lab;
      std::get<5> (prbtup) = cncl;
      if (mwl->fileprogrvect.size () > 0)
	{
	  Gtk::Widget *sibl = std::get<3> (
	      mwl->fileprogrvect[mwl->fileprogrvect.size () - 1]);
	  mwl->dld_grid->attach_next_to (*lab, *sibl, Gtk::PositionType::BOTTOM,
					 1, 1);
	  sibl = lab;
	  mwl->dld_grid->attach_next_to (*frprgb, *sibl,
					 Gtk::PositionType::BOTTOM, 1, 1);
	  sibl = frprgb;
	  mwl->dld_grid->attach_next_to (*cncl, *sibl, Gtk::PositionType::RIGHT,
					 1, 1);
	}
      else
	{
	  mwl->dld_win = new Gtk::Window;
	  mwl->dld_win->set_application (mwl->get_application ());
	  mwl->dld_win->set_deletable (false);
	  mwl->dld_win->set_name ("settingsWindow");
	  mwl->dld_win->get_style_context ()->add_provider (mwl->css_provider,
	  GTK_STYLE_PROVIDER_PRIORITY_USER);

	  mwl->dld_win->set_title (
	      gettext ("Downloading/sending file progress"));
	  mwl->dld_grid = Gtk::make_managed<Gtk::Grid> ();
	  mwl->dld_grid->set_halign (Gtk::Align::CENTER);
	  mwl->dld_win->set_child (*(mwl->dld_grid));
	  mwl->dld_grid->attach (*lab, 0, 0, 1, 1);
	  mwl->dld_grid->attach (*frprgb, 0, 1, 1, 1);
	  mwl->dld_grid->attach (*cncl, 1, 1, 1, 1);
	  mwl->dld_win->show ();
	}
      mwl->fileprogrvect.push_back (prbtup);
      mwl->fileprogrvectmtx.unlock ();
      mwl->oper->fileAccept (keyloc, tml, fp);
    }
  mwl->blockfreqmtx.unlock ();
  window->close ();
  fcd->close ();
}

void
MWFileOperations::cancelRcvFile (MainWindow *mwl, std::string keyloc,
				 std::filesystem::path fp)
{
  mwl->fileprogrvectmtx.lock ();
  auto it = std::find_if (
      mwl->fileprogrvect.begin (), mwl->fileprogrvect.end (), [keyloc, fp]
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
  if (it != mwl->fileprogrvect.end ())
    {
      mwl->dld_grid->remove (*(std::get<3> (*it)));
      mwl->dld_grid->remove (*(std::get<4> (*it)));
      mwl->dld_grid->remove (*(std::get<5> (*it)));
      mwl->fileprogrvect.erase (it);
    }
  if (mwl->fileprogrvect.size () == 0)
    {
      mwl->dld_win->hide ();
      delete mwl->dld_win;
      mwl->dld_win = nullptr;
      mwl->dld_grid = nullptr;
    }
  mwl->fileprogrvectmtx.unlock ();
  mwl->oper->cancelReceivFile (keyloc, fp);

  Gtk::Window *window = new Gtk::Window;
  window->set_application (mwl->get_application ());
  window->set_title (gettext ("Remove data"));
  window->set_name ("settingsWindow");
  window->get_style_context ()->add_provider (mwl->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_modal (true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign (Gtk::Align::CENTER);
  grid->set_valign (Gtk::Align::CENTER);
  window->set_child (*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
  lab->set_halign (Gtk::Align::CENTER);
  lab->set_margin (5);
  lab->set_text (gettext ("Remove received data?"));
  grid->attach (*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
  yes->set_halign (Gtk::Align::CENTER);
  yes->set_margin (5);
  yes->set_label (gettext ("Yes"));
  yes->set_name ("applyButton");
  yes->get_style_context ()->add_provider (mwl->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  yes->signal_clicked ().connect ( [fp, window]
  {
    std::filesystem::remove_all (fp);
    window->close ();
  });
  grid->attach (*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button> ();
  no->set_halign (Gtk::Align::CENTER);
  no->set_margin (5);
  no->set_label (gettext ("No"));
  no->set_name ("rejectButton");
  no->get_style_context ()->add_provider (mwl->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  no->signal_clicked ().connect (sigc::mem_fun (*window, &Gtk::Window::close));
  grid->attach (*no, 1, 1, 1, 1);

  window->signal_close_request ().connect ( [window]
  {
    window->hide ();
    delete window;
    return true;
  },
					   false);
  window->show ();
}
