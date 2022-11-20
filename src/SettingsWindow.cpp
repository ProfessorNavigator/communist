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

#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(MainWindow *Mw)
{
  mw = Mw;
}

SettingsWindow::~SettingsWindow()
{
  // TODO Auto-generated destructor stub
}

void
SettingsWindow::settingsWindow()
{
  std::string filename;
  std::filesystem::path filepath;
  AuxFunc af;
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_name("settingsWindow");
  window->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  window->set_title(gettext("Settings"));
  window->set_transient_for(*mw);

  Gtk::ScrolledWindow *scrl =
    Gtk::make_managed<Gtk::ScrolledWindow> ();
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                   Gtk::PolicyType::AUTOMATIC);
  scrl->set_halign(Gtk::Align::CENTER);
  Gtk::Grid *grwin = Gtk::make_managed<Gtk::Grid> ();
  grwin->set_halign(Gtk::Align::CENTER);
  window->set_child(*grwin);
  grwin->attach(*scrl, 0, 0, 2, 1);

  Gtk::Box *bx;
  Gtk::ListBoxRow *lbr;
  Gtk::ListBox *listb = Gtk::make_managed<Gtk::ListBox> ();
  listb->set_halign(Gtk::Align::CENTER);
  listb->set_show_separators(true);
  listb->set_name("settingsLB");
  listb->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  scrl->set_child(*listb);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *themepathl = Gtk::make_managed<Gtk::Label> ();
  themepathl->set_halign(Gtk::Align::START);
  themepathl->set_margin(5);
  themepathl->set_wrap_mode(Pango::WrapMode::WORD);
  themepathl->set_wrap(true);
  themepathl->set_text(gettext("Theme directory path: "));
  themepathl->set_justify(Gtk::Justification::LEFT);
  bx->append(*themepathl);

  Gtk::Entry *themepathval = Gtk::make_managed<Gtk::Entry> ();
  themepathval->set_halign(Gtk::Align::END);
  themepathval->set_valign(Gtk::Align::CENTER);
  themepathval->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> themepathvalbuf =
    Gtk::EntryBuffer::create();
  themepathvalbuf->set_text(mw->Userthemepath);
  if(themepathvalbuf->get_length() > 0)
    {
      themepathval->set_width_chars(
        themepathvalbuf->get_length());
    }
  themepathval->set_buffer(themepathvalbuf);
  bx->append(*themepathval);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *thmlb = Gtk::make_managed<Gtk::Label> ();
  thmlb->set_halign(Gtk::Align::START);
  thmlb->set_justify(Gtk::Justification::LEFT);
  thmlb->set_margin(5);
  thmlb->set_wrap_mode(Pango::WrapMode::WORD);
  thmlb->set_text(gettext("Theme:"));
  bx->append(*thmlb);

  Gtk::ComboBoxText *cmbthm =
    Gtk::make_managed<Gtk::ComboBoxText> ();
  cmbthm->set_halign(Gtk::Align::END);
  cmbthm->set_margin(5);
  std::filesystem::path thmp = std::filesystem::u8path(
                                 mw->Userthemepath);
  if(std::filesystem::exists(thmp))
    {
      for(auto &dirit : std::filesystem::directory_iterator(thmp))
        {
          std::filesystem::path p = dirit.path();
          cmbthm->append(Glib::ustring(p.filename().u8string()));
        }
    }
  mw->prefvectmtx.lock();
  auto itprv = std::find_if(mw->prefvect.begin(),
                            mw->prefvect.end(), []
                            (auto & el)
  {
    return std::get<0>(el) == "Theme";
  });
  if(itprv != mw->prefvect.end())
    {
      std::string tmp = std::get<1> (*itprv);
      cmbthm->set_active_text(Glib::ustring(tmp));
    }
  else
    {
      if(cmbthm->get_has_entry())
        {
          cmbthm->set_active_text("default");
        }
    }
  mw->prefvectmtx.unlock();
  bx->append(*cmbthm);
  lbr->set_child(*bx);
  listb->append(*lbr);
  MainWindow *mwl = mw;
  cmbthm->signal_changed().connect([cmbthm, mwl]
  {
    std::string tmp(cmbthm->get_active_text());
    mwl->prefvectmtx.lock();
    auto itprv = std::find_if(mwl->prefvect.begin(), mwl->prefvect.end(), []
                              (auto & el)
    {
      return std::get<0>(el) == "Theme";
    });
    if(itprv != mwl->prefvect.end())
      {
        std::get<1> (*itprv) = tmp;
      }
    else
      {
        std::tuple<std::string, std::string> ttup;
        std::get<0> (ttup) = "Theme";
        std::get<1> (ttup) = tmp;
        mwl->prefvect.push_back(ttup);
      }
    mwl->prefvectmtx.unlock();
  });

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *langchenb = Gtk::make_managed<Gtk::Label> ();
  langchenb->set_halign(Gtk::Align::START);
  langchenb->set_justify(Gtk::Justification::LEFT);
  langchenb->set_margin(5);
  langchenb->set_wrap_mode(Pango::WrapMode::WORD);
  langchenb->set_text(gettext("Enable spell checking"));
  bx->append(*langchenb);

  Gtk::CheckButton *langchenbch =
    Gtk::make_managed<Gtk::CheckButton> ();
  langchenbch->set_margin(5);
  langchenbch->set_halign(Gtk::Align::CENTER);
  langchenbch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Spellcheck";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "1")
        {
          langchenbch->set_active(true);
        }
      else
        {
          langchenbch->set_active(false);
        }
    }
  else
    {
      langchenbch->set_active(true);
    }
  mw->prefvectmtx.unlock();
  bx->append(*langchenbch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *langl = Gtk::make_managed<Gtk::Label> ();
  langl->set_halign(Gtk::Align::START);
  langl->set_justify(Gtk::Justification::LEFT);
  langl->set_margin(5);
  langl->set_wrap_mode(Pango::WrapMode::WORD);
  langl->set_text(gettext("Language for spell checking:"));
  bx->append(*langl);

  Gtk::ComboBoxText *cmbtxt =
    Gtk::make_managed<Gtk::ComboBoxText> ();
  cmbtxt->set_halign(Gtk::Align::END);
  cmbtxt->set_margin(5);
  std::filesystem::path dictp = std::filesystem::u8path(
                                  std::string(mw->Sharepath + "/HunDict/languages"));
  std::vector<std::tuple<std::string, std::string>> langvect;
  std::fstream f;
  f.open(dictp, std::ios_base::in);
  if(f.is_open())
    {
      while(!f.eof())
        {
          std::string line;
          getline(f, line);
          if(line != "")
            {
              line.erase(0, line.find(" ") + std::string(" ").size());
              cmbtxt->append(Glib::ustring(line));
            }
        }
      f.close();
    }
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Language";
  });
  if(itprv != mw->prefvect.end())
    {
      std::string tmp = std::get<1> (*itprv);
      cmbtxt->set_active_text(Glib::ustring(tmp));
    }
  else
    {
      if(cmbtxt->get_has_entry())
        {
          cmbtxt->set_active(1);
        }
    }
  mw->prefvectmtx.unlock();
  bx->append(*cmbtxt);
  lbr->set_child(*bx);
  listb->append(*lbr);

  cmbtxt->signal_changed().connect([cmbtxt, mwl]
  {
    std::string tmp(cmbtxt->get_active_text());
    mwl->prefvectmtx.lock();
    auto itprv = std::find_if(mwl->prefvect.begin(), mwl->prefvect.end(), []
                              (auto & el)
    {
      return std::get<0>(el) == "Language";
    });
    if(itprv != mwl->prefvect.end())
      {
        std::get<1> (*itprv) = tmp;
      }
    else
      {
        std::tuple<std::string, std::string> ttup;
        std::get<0> (ttup) = "Language";
        std::get<1> (ttup) = tmp;
        mwl->prefvect.push_back(ttup);
      }
    mwl->prefvectmtx.unlock();
  });

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *netl = Gtk::make_managed<Gtk::Label> ();
  netl->set_halign(Gtk::Align::START);
  netl->set_justify(Gtk::Justification::LEFT);
  netl->set_margin(5);
  netl->set_wrap_mode(Pango::WrapMode::WORD);
  netl->set_text(gettext("Network mode:"));
  bx->append(*netl);

  Gtk::ComboBoxText *netcmb =
    Gtk::make_managed<Gtk::ComboBoxText> ();
  netcmb->set_halign(Gtk::Align::END);
  netcmb->set_margin(5);
  netcmb->append(gettext("Internet"));
  netcmb->append(gettext("Local"));
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Netmode";
  });
  if(itprv != mw->prefvect.end())
    {
      std::string nm = std::get<1> (*itprv);
      if(nm == "local")
        {
          netcmb->set_active(1);
        }
      else
        {
          netcmb->set_active(0);
        }
    }
  else
    {
      netcmb->set_active(0);
    }
  mw->prefvectmtx.unlock();
  netcmb->signal_changed().connect([netcmb, mwl]
  {
    int ch = netcmb->get_active_row_number();
    mwl->prefvectmtx.lock();
    auto itprv = std::find_if(mwl->prefvect.begin(), mwl->prefvect.end(), []
                              (auto & el)
    {
      return std::get<0>(el) == "Netmode";
    });
    if(itprv != mwl->prefvect.end())
      {
        if(ch == 1)
          {
            std::get<1> (*itprv) = "local";
          }
        else
          {
            std::get<1> (*itprv) = "internet";
          }
      }
    else
      {
        if(ch == 1)
          {
            mwl->prefvect.push_back(std::make_tuple("Netmode",
                                                    "local"));
          }
        else
          {
            mwl->prefvect.push_back(std::make_tuple("Netmode",
                                                    "internet"));
          }
      }
    mwl->prefvectmtx.unlock();
  });
  bx->append(*netcmb);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *locip4l = Gtk::make_managed<Gtk::Label> ();
  locip4l->set_halign(Gtk::Align::START);
  locip4l->set_margin(5);
  locip4l->set_wrap_mode(Pango::WrapMode::WORD);
  locip4l->set_wrap(true);
  locip4l->set_text(
    gettext("Local network group multicast ipv4 and port (ipv4:port)"));
  locip4l->set_justify(Gtk::Justification::LEFT);
  bx->append(*locip4l);

  Gtk::Entry *locip4val = Gtk::make_managed<Gtk::Entry> ();
  locip4val->set_halign(Gtk::Align::END);
  locip4val->set_valign(Gtk::Align::CENTER);
  locip4val->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> locip4valbuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Locip4port";
  });
  if(itprv != mw->prefvect.end())
    {
      locip4valbuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      locip4valbuf->set_text("239.192.150.8:48655");
    }
  mw->prefvectmtx.unlock();
  if(locip4valbuf->get_length() > 0)
    {
      locip4val->set_width_chars(locip4valbuf->get_length());
    }
  locip4val->set_buffer(locip4valbuf);
  bx->append(*locip4val);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *locip6l = Gtk::make_managed<Gtk::Label> ();
  locip6l->set_halign(Gtk::Align::START);
  locip6l->set_margin(5);
  locip6l->set_wrap_mode(Pango::WrapMode::WORD);
  locip6l->set_wrap(true);
  locip6l->set_text(
    gettext("Local network group multicast ipv6 and port ([ipv6]:port)"));
  locip6l->set_justify(Gtk::Justification::LEFT);
  bx->append(*locip6l);

  Gtk::Entry *locip6val = Gtk::make_managed<Gtk::Entry> ();
  locip6val->set_halign(Gtk::Align::END);
  locip6val->set_valign(Gtk::Align::CENTER);
  locip6val->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> locip6valbuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Locip6port";
  });
  if(itprv != mw->prefvect.end())
    {
      locip6valbuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      locip6valbuf->set_text("[ff15::13]:48666");
    }
  mw->prefvectmtx.unlock();
  if(locip6valbuf->get_length() > 0)
    {
      locip6val->set_width_chars(locip6valbuf->get_length());
    }
  locip6val->set_buffer(locip6valbuf);
  bx->append(*locip6val);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *autorem = Gtk::make_managed<Gtk::Label> ();
  autorem->set_halign(Gtk::Align::START);
  autorem->set_justify(Gtk::Justification::LEFT);
  autorem->set_margin(5);
  autorem->set_wrap_mode(Pango::WrapMode::WORD);
  autorem->set_text(gettext("Messages auto remove after:"));
  bx->append(*autorem);

  Gtk::ComboBoxText *autoremcmb =
    Gtk::make_managed<Gtk::ComboBoxText> ();
  autoremcmb->set_halign(Gtk::Align::END);
  autoremcmb->set_valign(Gtk::Align::CENTER);
  autoremcmb->set_margin(5);
  autoremcmb->append(gettext("never"));
  autoremcmb->append(gettext("1 day"));
  autoremcmb->append(gettext("1 week"));
  autoremcmb->append(gettext("1 month"));
  autoremcmb->append(gettext("1 year"));
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Autoremove";
  });
  if(itprv != mw->prefvect.end())
    {
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << std::get<1> (*itprv);
      int sel;
      strm >> sel;
      if(sel <= 4)
        {
          autoremcmb->set_active(sel);
        }
      else
        {
          autoremcmb->set_active(0);
        }
    }
  else
    {
      autoremcmb->set_active(0);
    }
  mw->prefvectmtx.unlock();
  autoremcmb->signal_changed().connect([autoremcmb, mwl]
  {
    std::stringstream strm;
    std::locale loc("C");
    strm.imbue(loc);
    strm << autoremcmb->get_active_row_number();
    std::string ch = strm.str();
    mwl->prefvectmtx.lock();
    auto itprv = std::find_if(mwl->prefvect.begin(), mwl->prefvect.end(), []
                              (auto & el)
    {
      return std::get<0>(el) == "Autoremove";
    });
    if(itprv != mwl->prefvect.end())
      {
        std::get<1> (*itprv) = ch;
      }
    else
      {
        mwl->prefvect.push_back(std::make_tuple("Autoremove", ch));
      }
    mwl->prefvectmtx.unlock();
  });
  bx->append(*autoremcmb);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *lifcs = Gtk::make_managed<Gtk::Label> ();
  lifcs->set_halign(Gtk::Align::START);
  lifcs->set_margin(5);
  lifcs->set_wrap_mode(Pango::WrapMode::WORD);
  lifcs->set_wrap(true);
  lifcs->set_text(
    gettext("DHT listen interfaces (ipv4:port,[ipv6]:port):"));
  lifcs->set_justify(Gtk::Justification::LEFT);
  bx->append(*lifcs);

  Gtk::Entry *lifcsval = Gtk::make_managed<Gtk::Entry> ();
  lifcsval->set_halign(Gtk::Align::END);
  lifcsval->set_valign(Gtk::Align::CENTER);
  lifcsval->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> lifcsvalbuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Listenifcs";
  });
  if(itprv != mw->prefvect.end())
    {
      lifcsvalbuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      lifcsvalbuf->set_text("0.0.0.0:0,[::]:0");
    }
  mw->prefvectmtx.unlock();
  if(lifcsvalbuf->get_length() > 0)
    {
      lifcsval->set_width_chars(lifcsvalbuf->get_length());
    }
  lifcsval->set_buffer(lifcsvalbuf);
  bx->append(*lifcsval);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *bootstrl = Gtk::make_managed<Gtk::Label> ();
  bootstrl->set_halign(Gtk::Align::START);
  bootstrl->set_margin(5);
  bootstrl->set_wrap_mode(Pango::WrapMode::WORD);
  bootstrl->set_wrap(true);
  bootstrl->set_text(
    gettext(
      "Comma separated DHT bootstrap nodes (address:port or ipv4:port or [ipv6]:port):"));
  bootstrl->set_justify(Gtk::Justification::LEFT);
  bx->append(*bootstrl);

  Gtk::Entry *bootstre = Gtk::make_managed<Gtk::Entry> ();
  bootstre->set_halign(Gtk::Align::END);
  bootstre->set_valign(Gtk::Align::CENTER);
  bootstre->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> bootstrebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Bootstrapdht";
  });
  if(itprv != mw->prefvect.end())
    {
      bootstrebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      bootstrebuf->set_text("router.bittorrent.com:6881");
    }
  mw->prefvectmtx.unlock();
  if(bootstrebuf->get_length() > 0)
    {
      bootstre->set_width_chars(bootstrebuf->get_length());
    }
  bootstre->set_buffer(bootstrebuf);
  bx->append(*bootstre);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *msgszl = Gtk::make_managed<Gtk::Label> ();
  msgszl->set_halign(Gtk::Align::START);
  msgszl->set_justify(Gtk::Justification::LEFT);
  msgszl->set_margin(5);
  msgszl->set_wrap_mode(Pango::WrapMode::WORD);
  msgszl->set_text(
    gettext("Maximum permitted incomming messages size (in bytes):"));
  bx->append(*msgszl);

  Gtk::Entry *msgsze = Gtk::make_managed<Gtk::Entry> ();
  msgsze->set_halign(Gtk::Align::END);
  msgsze->set_valign(Gtk::Align::CENTER);
  msgsze->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> msgszebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Maxmsgsz";
  });
  if(itprv != mw->prefvect.end())
    {
      msgszebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      msgszebuf->set_text("1048576");
    }
  mw->prefvectmtx.unlock();
  if(msgszebuf->get_length() > 0)
    {
      msgsze->set_width_chars(msgszebuf->get_length());
    }
  msgsze->set_buffer(msgszebuf);
  bx->append(*msgsze);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *partszl = Gtk::make_managed<Gtk::Label> ();
  partszl->set_halign(Gtk::Align::START);
  partszl->set_justify(Gtk::Justification::LEFT);
  partszl->set_margin(5);
  partszl->set_wrap_mode(Pango::WrapMode::WORD);
  partszl->set_text(gettext("Sending part size (in bytes):"));
  bx->append(*partszl);

  Gtk::Entry *partsze = Gtk::make_managed<Gtk::Entry> ();
  partsze->set_halign(Gtk::Align::END);
  partsze->set_valign(Gtk::Align::CENTER);
  partsze->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> partszebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Partsize";
  });
  if(itprv != mw->prefvect.end())
    {
      partszebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      partszebuf->set_text("1371");
    }
  mw->prefvectmtx.unlock();
  if(partszebuf->get_length() > 0)
    {
      partsze->set_width_chars(partszebuf->get_length());
    }
  partsze->set_buffer(partszebuf);
  bx->append(*partsze);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *shuttmtl = Gtk::make_managed<Gtk::Label> ();
  shuttmtl->set_halign(Gtk::Align::START);
  shuttmtl->set_justify(Gtk::Justification::LEFT);
  shuttmtl->set_margin(5);
  shuttmtl->set_wrap_mode(Pango::WrapMode::WORD);
  shuttmtl->set_text(
    gettext(
      "Contact connection maintenance messages shutdown timeout in seconds:"));
  bx->append(*shuttmtl);

  Gtk::Entry *shutmte = Gtk::make_managed<Gtk::Entry> ();
  shutmte->set_halign(Gtk::Align::END);
  shutmte->set_valign(Gtk::Align::CENTER);
  shutmte->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> shutmtebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "ShutTmt";
  });
  if(itprv != mw->prefvect.end())
    {
      shutmtebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      shutmtebuf->set_text("600");
    }
  mw->prefvectmtx.unlock();
  if(shutmtebuf->get_length() > 0)
    {
      shutmte->set_width_chars(shutmtebuf->get_length());
    }
  shutmte->set_buffer(shutmtebuf);
  bx->append(*shutmte);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *tmttearl = Gtk::make_managed<Gtk::Label> ();
  tmttearl->set_halign(Gtk::Align::START);
  tmttearl->set_justify(Gtk::Justification::LEFT);
  tmttearl->set_margin(5);
  tmttearl->set_wrap_mode(Pango::WrapMode::WORD);
  tmttearl->set_text(
    gettext("Connection broken timeout in seconds:"));
  bx->append(*tmttearl);

  Gtk::Entry *tmtteare = Gtk::make_managed<Gtk::Entry> ();
  tmtteare->set_halign(Gtk::Align::END);
  tmtteare->set_valign(Gtk::Align::CENTER);
  tmtteare->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> tmttearebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "TmtTear";
  });
  if(itprv != mw->prefvect.end())
    {
      tmttearebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      tmttearebuf->set_text("20");
    }
  mw->prefvectmtx.unlock();
  if(tmttearebuf->get_length() > 0)
    {
      tmtteare->set_width_chars(tmttearebuf->get_length());
    }
  tmtteare->set_buffer(tmttearebuf);
  bx->append(*tmtteare);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *mainttmtl = Gtk::make_managed<Gtk::Label> ();
  mainttmtl->set_halign(Gtk::Align::START);
  mainttmtl->set_justify(Gtk::Justification::LEFT);
  mainttmtl->set_margin(5);
  mainttmtl->set_wrap_mode(Pango::WrapMode::WORD);
  mainttmtl->set_text(
    gettext("Technical messages timeout in seconds:"));
  bx->append(*mainttmtl);

  Gtk::Entry *mainttmte = Gtk::make_managed<Gtk::Entry> ();
  mainttmte->set_halign(Gtk::Align::END);
  mainttmte->set_valign(Gtk::Align::CENTER);
  mainttmte->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> mainttmtebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Maintpause";
  });
  if(itprv != mw->prefvect.end())
    {
      mainttmtebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      mainttmtebuf->set_text("5");
    }
  mw->prefvectmtx.unlock();
  if(mainttmtebuf->get_length() > 0)
    {
      mainttmte->set_width_chars(mainttmtebuf->get_length());
    }
  mainttmte->set_buffer(mainttmtebuf);
  bx->append(*mainttmte);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *winszl = Gtk::make_managed<Gtk::Label> ();
  winszl->set_halign(Gtk::Align::START);
  winszl->set_justify(Gtk::Justification::LEFT);
  winszl->set_margin(5);
  winszl->set_wrap_mode(Pango::WrapMode::WORD);
  winszl->set_text(
    gettext("Save window size before closing"));
  bx->append(*winszl);

  Gtk::CheckButton *winszch =
    Gtk::make_managed<Gtk::CheckButton> ();
  winszch->set_margin(5);
  winszch->set_halign(Gtk::Align::CENTER);
  winszch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Winsizesv";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "active")
        {
          winszch->set_active(true);
        }
      else
        {
          winszch->set_active(false);
        }
    }
  else
    {
      winszch->set_active(true);
    }
  mw->prefvectmtx.unlock();
  bx->append(*winszch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *msgwchrl = Gtk::make_managed<Gtk::Label> ();
  msgwchrl->set_halign(Gtk::Align::START);
  msgwchrl->set_justify(Gtk::Justification::LEFT);
  msgwchrl->set_margin(5);
  msgwchrl->set_wrap_mode(Pango::WrapMode::WORD);
  msgwchrl->set_text(gettext("Message max width in chars:"));
  bx->append(*msgwchrl);

  Gtk::Entry *msgwchre = Gtk::make_managed<Gtk::Entry> ();
  msgwchre->set_halign(Gtk::Align::END);
  msgwchre->set_valign(Gtk::Align::CENTER);
  msgwchre->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> msgwchrebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "MsgWidth";
  });
  if(itprv != mw->prefvect.end())
    {
      msgwchrebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
    }
  else
    {
      msgwchrebuf->set_text("30");
    }
  mw->prefvectmtx.unlock();
  if(msgwchrebuf->get_length() > 0)
    {
      msgwchre->set_width_chars(msgwchrebuf->get_length());
    }
  msgwchre->set_buffer(msgwchrebuf);
  bx->append(*msgwchre);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *sendkeyl = Gtk::make_managed<Gtk::Label> ();
  sendkeyl->set_halign(Gtk::Align::START);
  sendkeyl->set_justify(Gtk::Justification::LEFT);
  sendkeyl->set_margin(5);
  sendkeyl->set_wrap_mode(Pango::WrapMode::WORD);
  sendkeyl->set_text(gettext("Send message by"));
  bx->append(*sendkeyl);

  Gtk::ComboBoxText *sendkeycmb =
    Gtk::make_managed<Gtk::ComboBoxText> ();
  sendkeycmb->set_halign(Gtk::Align::END);
  sendkeycmb->set_margin(5);
  sendkeycmb->append("Ctrl + Enter");
  sendkeycmb->append("Enter");
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "SendKey";
  });
  if(itprv != mw->prefvect.end())
    {
      std::string modes = std::get<1> (*itprv);
      if(modes != "")
        {
          std::stringstream strm;
          std::locale loc("C");
          strm.imbue(loc);
          strm << modes;
          int modei;
          strm >> modei;
          sendkeycmb->set_active(modei);
        }
      else
        {
          sendkeycmb->set_active(0);
        }
    }
  else
    {
      sendkeycmb->set_active(0);
    }
  mw->prefvectmtx.unlock();
  bx->append(*sendkeycmb);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *grnotificl = Gtk::make_managed<Gtk::Label> ();
  grnotificl->set_halign(Gtk::Align::START);
  grnotificl->set_justify(Gtk::Justification::LEFT);
  grnotificl->set_margin(5);
  grnotificl->set_wrap_mode(Pango::WrapMode::WORD);
  grnotificl->set_text(
    gettext("Enable notification on message receiving"));
  bx->append(*grnotificl);

  Gtk::CheckButton *grnotificch =
    Gtk::make_managed<Gtk::CheckButton> ();
  grnotificch->set_margin(5);
  grnotificch->set_halign(Gtk::Align::CENTER);
  grnotificch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "GrNotification";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "yes")
        {
          grnotificch->set_active(true);
        }
      else
        {
          grnotificch->set_active(false);
        }
    }
  else
    {
      grnotificch->set_active(true);
    }
  mw->prefvectmtx.unlock();
  bx->append(*grnotificch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *soundl = Gtk::make_managed<Gtk::Label> ();
  soundl->set_halign(Gtk::Align::START);
  soundl->set_justify(Gtk::Justification::LEFT);
  soundl->set_margin(5);
  soundl->set_wrap_mode(Pango::WrapMode::WORD);
  soundl->set_text(gettext("Enable message received sound"));
  bx->append(*soundl);

  Gtk::CheckButton *soundch =
    Gtk::make_managed<Gtk::CheckButton> ();
  soundch->set_margin(5);
  soundch->set_halign(Gtk::Align::CENTER);
  soundch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "SoundOn";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "active")
        {
          soundch->set_active(true);
        }
      else
        {
          soundch->set_active(false);
        }
    }
  else
    {
      soundch->set_active(true);
    }
  mw->prefvectmtx.unlock();
  bx->append(*soundch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *soundpl = Gtk::make_managed<Gtk::Label> ();
  soundpl->set_halign(Gtk::Align::START);
  soundpl->set_justify(Gtk::Justification::LEFT);
  soundpl->set_margin(5);
  soundpl->set_wrap_mode(Pango::WrapMode::WORD);
  soundpl->set_text(gettext("Message sound path:"));
  bx->append(*soundpl);

  Gtk::Entry *soundpe = Gtk::make_managed<Gtk::Entry> ();
  soundpe->set_halign(Gtk::Align::END);
  soundpe->set_valign(Gtk::Align::CENTER);
  soundpe->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> soundpebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "SoundPath";
  });
  if(itprv != mw->prefvect.end())
    {
      if(Glib::ustring(std::get<1> (*itprv)) != "")
        {
          soundpebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
          soundpe->set_buffer(soundpebuf);
        }
      else
        {
          soundpe->set_placeholder_text(gettext("default"));
        }
    }
  else
    {
      soundpe->set_placeholder_text(gettext("default"));
    }
  mw->prefvectmtx.unlock();
  soundpe->set_width_chars(50);
  bx->append(*soundpe);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *hole_punchl = Gtk::make_managed<Gtk::Label> ();
  hole_punchl->set_halign(Gtk::Align::START);
  hole_punchl->set_justify(Gtk::Justification::LEFT);
  hole_punchl->set_margin(5);
  hole_punchl->set_wrap_mode(Pango::WrapMode::WORD);
  hole_punchl->set_text(
    gettext("Enable hole punch mechanism for symmetric NAT"));
  bx->append(*hole_punchl);

  Gtk::CheckButton *hole_punchch =
    Gtk::make_managed<Gtk::CheckButton> ();
  hole_punchch->set_margin(5);
  hole_punchch->set_halign(Gtk::Align::CENTER);
  hole_punchch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "HolePunchMech";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "active")
        {
          hole_punchch->set_active(true);
        }
      else
        {
          hole_punchch->set_active(false);
        }
    }
  else
    {
      hole_punchch->set_active(false);
    }
  mw->prefvectmtx.unlock();
  bx->append(*hole_punchch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *enstunl = Gtk::make_managed<Gtk::Label> ();
  enstunl->set_halign(Gtk::Align::START);
  enstunl->set_justify(Gtk::Justification::LEFT);
  enstunl->set_margin(5);
  enstunl->set_wrap_mode(Pango::WrapMode::WORD);
  enstunl->set_text(gettext("Enable local STUN server"));
  bx->append(*enstunl);

  Gtk::CheckButton *enstunch =
    Gtk::make_managed<Gtk::CheckButton> ();
  enstunch->set_margin(5);
  enstunch->set_halign(Gtk::Align::CENTER);
  enstunch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Stun";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "active")
        {
          enstunch->set_active(true);
        }
      else
        {
          enstunch->set_active(false);
        }
    }
  else
    {
      enstunch->set_active(false);
    }
  mw->prefvectmtx.unlock();
  bx->append(*enstunch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *stunpl = Gtk::make_managed<Gtk::Label> ();
  stunpl->set_halign(Gtk::Align::START);
  stunpl->set_justify(Gtk::Justification::LEFT);
  stunpl->set_margin(5);
  stunpl->set_wrap_mode(Pango::WrapMode::WORD);
  stunpl->set_text(gettext("STUN port:"));
  bx->append(*stunpl);

  Gtk::Entry *stunpe = Gtk::make_managed<Gtk::Entry> ();
  stunpe->set_halign(Gtk::Align::END);
  stunpe->set_valign(Gtk::Align::CENTER);
  stunpe->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> stunpebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Stunport";
  });
  if(itprv != mw->prefvect.end())
    {
      if(Glib::ustring(std::get<1> (*itprv)) != "")
        {
          stunpebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
          stunpe->set_buffer(stunpebuf);
        }
      else
        {
          stunpebuf->set_text("3478");
          stunpe->set_buffer(stunpebuf);
        }
    }
  else
    {
      stunpebuf->set_text("3478");
      stunpe->set_buffer(stunpebuf);
    }
  mw->prefvectmtx.unlock();
  stunpe->set_width_chars(5);
  bx->append(*stunpe);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *enrellab = Gtk::make_managed<Gtk::Label> ();
  enrellab->set_halign(Gtk::Align::START);
  enrellab->set_justify(Gtk::Justification::LEFT);
  enrellab->set_margin(5);
  enrellab->set_wrap_mode(Pango::WrapMode::WORD);
  enrellab->set_text(gettext("Enable relay server"));
  bx->append(*enrellab);

  Gtk::CheckButton *enrelch =
    Gtk::make_managed<Gtk::CheckButton> ();
  enrelch->set_margin(5);
  enrelch->set_halign(Gtk::Align::CENTER);
  enrelch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "RelaySrv";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "enabled")
        {
          enrelch->set_active(true);
        }
      else
        {
          enrelch->set_active(false);
        }
    }
  else
    {
      enrelch->set_active(false);
    }
  mw->prefvectmtx.unlock();
  bx->append(*enrelch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *relaypl = Gtk::make_managed<Gtk::Label> ();
  relaypl->set_halign(Gtk::Align::START);
  relaypl->set_justify(Gtk::Justification::LEFT);
  relaypl->set_margin(5);
  relaypl->set_wrap_mode(Pango::WrapMode::WORD);
  relaypl->set_text(gettext("Relay port:"));
  bx->append(*relaypl);

  Gtk::Entry *relaype = Gtk::make_managed<Gtk::Entry> ();
  relaype->set_halign(Gtk::Align::END);
  relaype->set_valign(Gtk::Align::CENTER);
  relaype->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> relaypebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Relayport";
  });
  if(itprv != mw->prefvect.end())
    {
      if(Glib::ustring(std::get<1> (*itprv)) != "")
        {
          relaypebuf->set_text(Glib::ustring(std::get<1> (*itprv)));
          relaype->set_buffer(relaypebuf);
        }
      else
        {
          relaypebuf->set_text("3029");
          relaype->set_buffer(relaypebuf);
        }
    }
  else
    {
      relaypebuf->set_text("3029");
      relaype->set_buffer(relaypebuf);
    }
  mw->prefvectmtx.unlock();
  relaype->set_width_chars(5);
  bx->append(*relaype);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *relaypathl = Gtk::make_managed<Gtk::Label> ();
  relaypathl->set_halign(Gtk::Align::START);
  relaypathl->set_justify(Gtk::Justification::LEFT);
  relaypathl->set_margin(5);
  relaypathl->set_wrap_mode(Pango::WrapMode::WORD);
  relaypathl->set_text(gettext("Relay list path:"));
  bx->append(*relaypathl);

  Gtk::Entry *relaypathe = Gtk::make_managed<Gtk::Entry> ();
  relaypathe->set_halign(Gtk::Align::END);
  relaypathe->set_valign(Gtk::Align::CENTER);
  relaypathe->set_margin(5);
  Glib::RefPtr<Gtk::EntryBuffer> relaypathebuf =
    Gtk::EntryBuffer::create();
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "RelayListPath";
  });
  if(itprv != mw->prefvect.end())
    {
      if(Glib::ustring(std::get<1> (*itprv)) != "")
        {
          relaypathebuf->set_text(Glib::ustring(std::get<1>
                                                (*itprv)));
          relaypathe->set_buffer(relaypathebuf);
        }
      else
        {
          relaypathe->set_placeholder_text(gettext("default"));
        }
    }
  else
    {
      relaypathe->set_placeholder_text(gettext("default"));
    }
  mw->prefvectmtx.unlock();
  relaypathe->set_width_chars(50);
  bx->append(*relaypathe);
  lbr->set_child(*bx);
  listb->append(*lbr);

  bx = Gtk::make_managed<Gtk::Box>
       (Gtk::Orientation::HORIZONTAL);
  lbr = Gtk::make_managed<Gtk::ListBoxRow> ();
  lbr->set_selectable(false);
  Gtk::Label *directinetl = Gtk::make_managed<Gtk::Label> ();
  directinetl->set_halign(Gtk::Align::START);
  directinetl->set_justify(Gtk::Justification::LEFT);
  directinetl->set_margin(5);
  directinetl->set_wrap_mode(Pango::WrapMode::WORD);
  directinetl->set_text(
    gettext("Direct connection to Internet"));
  bx->append(*directinetl);

  Gtk::CheckButton *directinetch =
    Gtk::make_managed<Gtk::CheckButton> ();
  directinetch->set_margin(5);
  directinetch->set_halign(Gtk::Align::CENTER);
  directinetch->set_valign(Gtk::Align::CENTER);
  mw->prefvectmtx.lock();
  itprv = std::find_if(mw->prefvect.begin(),
                       mw->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "DirectInet";
  });
  if(itprv != mw->prefvect.end())
    {
      if(std::get<1> (*itprv) == "direct")
        {
          directinetch->set_active(true);
        }
      else
        {
          directinetch->set_active(false);
        }
    }
  else
    {
      directinetch->set_active(false);
    }
  mw->prefvectmtx.unlock();
  bx->append(*directinetch);
  lbr->set_child(*bx);
  listb->append(*lbr);

  Gtk::Button *apply = Gtk::make_managed<Gtk::Button> ();
  apply->set_halign(Gtk::Align::START);
  apply->set_margin(5);
  apply->set_name("applyButton");
  apply->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  apply->set_label(gettext("Apply"));
  grwin->attach(*apply, 0, 1, 1, 1);

  apply->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this,
                             &SettingsWindow::saveSettings), window,
               themepathval, lifcsval, locip6val, locip4val, bootstre,
               msgsze, partsze, winszch, msgwchre, sendkeycmb, grnotificch,
               soundch, soundpe, shutmte, tmtteare, mainttmte, langchenbch,
               hole_punchch, enstunch, stunpe, enrelch, relaype,
               relaypathe,
               directinetch, mwl));

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button> ();
  cancel->set_halign(Gtk::Align::END);
  cancel->set_margin(5);
  cancel->set_name("rejectButton");
  cancel->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  cancel->set_label(gettext("Cancel"));
  grwin->attach(*cancel, 1, 1, 1, 1);

  cancel->signal_clicked().connect(
    sigc::mem_fun(*window, &Gtk::Window::close));

  Gtk::Requisition rq1, rq2;
  listb->get_preferred_size(rq1, rq2);
  Gdk::Rectangle req = mw->screenRes();
  if(rq1.get_height() <= 0.7 * req.get_height())
    {
      scrl->set_min_content_height(rq1.get_height());
    }
  else
    {
      scrl->set_min_content_height(0.7 * req.get_height());
    }

  if(rq1.get_width() <= 0.7 * req.get_width())
    {
      scrl->set_min_content_width(rq1.get_width());
    }
  else
    {
      scrl->set_min_content_width(0.7 * req.get_width());
    }

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
SettingsWindow::saveSettings(Gtk::Window *window,
                             Gtk::Entry *themepathval,
                             Gtk::Entry *lifcsval, Gtk::Entry *locip6val,
                             Gtk::Entry *locip4val, Gtk::Entry *bootstre,
                             Gtk::Entry *msgsze, Gtk::Entry *partsze,
                             Gtk::CheckButton *winszch, Gtk::Entry *msgwchre,
                             Gtk::ComboBoxText *sendkeycmb,
                             Gtk::CheckButton *grnotificch,
                             Gtk::CheckButton *soundch, Gtk::Entry *soundpe,
                             Gtk::Entry *shutmte, Gtk::Entry *tmtteare,
                             Gtk::Entry *mainttmte,
                             Gtk::CheckButton *langchenbch,
                             Gtk::CheckButton *hole_punchch,
                             Gtk::CheckButton *enstunch, Gtk::Entry *stunpe,
                             Gtk::CheckButton *enrelch, Gtk::Entry *relaype,
                             Gtk::Entry *relaypathe,
                             Gtk::CheckButton *directinetch, MainWindow *mwl)
{
  std::string filename;
  std::filesystem::path filepath;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.config/Communist/Prefs";
  filepath = std::filesystem::u8path(filename);
  if(!std::filesystem::exists(filepath))
    {
      std::filesystem::create_directories(filepath.parent_path());
    }

  mwl->prefvectmtx.lock();
  auto itprv = std::find_if(mwl->prefvect.begin(),
                            mwl->prefvect.end(), []
                            (auto & el)
  {
    return std::get<0>(el) == "Listenifcs";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(lifcsval->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   lifcsval->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "0.0.0.0:0,[::]:0";
        }
    }
  else
    {
      if(lifcsval->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Listenifcs",
                            lifcsval->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(
            std::make_tuple("Listenifcs", "0.0.0.0:0,[::]:0"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Userthemepath";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(themepathval->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   themepathval->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = mwl->Sharepath + "/themes";
        }
    }
  else
    {
      if(themepathval->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Userthemepath",
                            themepathval->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(
            std::make_tuple("Userthemepath",
                            std::string(mwl->Sharepath + "/themes")));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Locip6port";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(locip6val->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   locip6val->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "[ff15::13]:48666";
        }
    }
  else
    {
      if(locip6val->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Locip6port",
                            locip6val->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(
            std::make_tuple("Locip6port", "[ff15::13]:48666"));
        }
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Locip4port";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(locip4val->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   locip4val->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "239.192.150.8:48655";
        }
    }
  else
    {
      if(locip4val->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Locip4port",
                            locip4val->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(
            std::make_tuple("Locip4port", "239.192.150.8:48655"));
        }
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Spellcheck";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(langchenbch->get_active())
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
      if(langchenbch->get_active())
        {
          std::get<1> (ttup) = "1";
        }
      else
        {
          std::get<1> (ttup) = "0";
        }
      mwl->prefvect.push_back(ttup);
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Bootstrapdht";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(bootstre->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   bootstre->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "router.bittorrent.com:6881";
        }
    }
  else
    {
      if(bootstre->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Bootstrapdht",
                            bootstre->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(
            std::make_tuple("Bootstrapdht",
                            "router.bittorrent.com:6881"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Maxmsgsz";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(msgsze->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   msgsze->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "1048576";
        }
    }
  else
    {
      if(msgsze->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Maxmsgsz",
                            msgsze->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("Maxmsgsz",
                                                  "1048576"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Partsize";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(partsze->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   partsze->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "1371";
        }
    }
  else
    {
      if(partsze->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Partsize",
                            partsze->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("Partsize",
                                                  "1371"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Winsizesv";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(winszch->get_active())
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
      if(winszch->get_active())
        {
          mwl->prefvect.push_back(std::make_tuple("Winsizesv",
                                                  "active"));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("Winsizesv",
                                                  "notactive"));
        }
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "SendKey";
  });
  if(itprv != mwl->prefvect.end())
    {
      int modei = sendkeycmb->get_active_row_number();
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << modei;
      std::get<1> (*itprv) = strm.str();
    }
  else
    {
      int modei = sendkeycmb->get_active_row_number();
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << modei;
      mwl->prefvect.push_back(std::make_tuple("SendKey", "0"));
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "GrNotification";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(grnotificch->get_active())
        {
          std::get<1> (*itprv) = "yes";
        }
      else
        {
          std::get<1> (*itprv) = "no";
        }
    }
  else
    {
      if(grnotificch->get_active())
        {
          mwl->prefvect.push_back(std::make_tuple("GrNotification",
                                                  "yes"));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("GrNotification",
                                                  "no"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "SoundOn";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(soundch->get_active())
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
      if(soundch->get_active())
        {
          mwl->prefvect.push_back(std::make_tuple("SoundOn",
                                                  "active"));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("SoundOn",
                                                  "notactive"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "SoundPath";
  });
  if(itprv != mwl->prefvect.end())
    {
      std::get<1> (*itprv) = std::string(
                               soundpe->get_buffer()->get_text());
    }
  else
    {
      mwl->prefvect.push_back(
        std::make_tuple("SoundPath",
                        soundpe->get_buffer()->get_text()));
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "ShutTmt";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(shutmte->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   shutmte->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "600";
        }
    }
  else
    {
      if(shutmte->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("ShutTmt",
                            shutmte->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("ShutTmt", "600"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "TmtTear";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(tmtteare->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   tmtteare->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "20";
        }
    }
  else
    {
      if(tmtteare->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("TmtTear",
                            tmtteare->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("TmtTear", "20"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Maintpause";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(mainttmte->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   mainttmte->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "5";
        }
    }
  else
    {
      if(mainttmte->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Maintpause",
                            mainttmte->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("Maintpause", "5"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "HolePunchMech";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(hole_punchch->get_active())
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
      if(hole_punchch->get_active())
        {
          mwl->prefvect.push_back(std::make_tuple("HolePunchMech",
                                                  "active"));
        }
      else
        {
          mwl->prefvect.push_back(
            std::make_tuple("HolePunchMech", "notactive"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Stun";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(enstunch->get_active())
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
      if(enstunch->get_active())
        {
          mwl->prefvect.push_back(std::make_tuple("Stun", "active"));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("Stun",
                                                  "notactive"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Stunport";
  });
  if(itprv != mwl->prefvect.end())
    {
      std::get<1> (*itprv) = std::string(
                               stunpe->get_buffer()->get_text());
    }
  else
    {
      mwl->prefvect.push_back(
        std::make_tuple("Stunport",
                        stunpe->get_buffer()->get_text()));
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "DirectInet";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(directinetch->get_active())
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
      if(directinetch->get_active())
        {
          mwl->prefvect.push_back(std::make_tuple("DirectInet",
                                                  "direct"));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("DirectInet",
                                                  "notdirect"));
        }
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "MsgWidth";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(msgwchre->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   msgwchre->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "30";
        }
    }
  else
    {
      if(msgwchre->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("MsgWidth",
                            msgwchre->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("MsgWidth", "30"));
        }
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "RelaySrv";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(enrelch->get_active())
        {
          std::get<1> (*itprv) = "enabled";
        }
      else
        {
          std::get<1> (*itprv) = "disabled";
        }
    }
  else
    {
      std::tuple<std::string, std::string> ttup;
      std::get<0> (ttup) = "RelaySrv";
      if(enrelch->get_active())
        {
          std::get<1> (ttup) = "enabled";
        }
      else
        {
          std::get<1> (ttup) = "disabled";
        }
      mwl->prefvect.push_back(ttup);
    }
  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "Relayport";
  });
  if(itprv != mwl->prefvect.end())
    {
      if(relaype->get_buffer()->get_text() != "")
        {
          std::get<1> (*itprv) = std::string(
                                   relaype->get_buffer()->get_text());
        }
      else
        {
          std::get<1> (*itprv) = "3029";
        }
    }
  else
    {
      if(relaype->get_buffer()->get_text() != "")
        {
          mwl->prefvect.push_back(
            std::make_tuple("Relayport",
                            relaype->get_buffer()->get_text()));
        }
      else
        {
          mwl->prefvect.push_back(std::make_tuple("Relayport",
                                                  "3029"));
        }
    }

  itprv = std::find_if(mwl->prefvect.begin(),
                       mwl->prefvect.end(), []
                       (auto & el)
  {
    return std::get<0>(el) == "RelayListPath";
  });
  if(itprv != mwl->prefvect.end())
    {
      std::get<1> (*itprv) = std::string(
                               relaypathe->get_buffer()->get_text());
    }
  else
    {
      mwl->prefvect.push_back(
        std::make_tuple("RelayListPath",
                        relaypathe->get_buffer()->get_text()));
    }

  std::fstream f;
  f.open(filepath, std::ios_base::out |
         std::ios_base::binary);
  for(size_t i = 0; i < mwl->prefvect.size(); i++)
    {
      std::string line = std::get<0> (mwl->prefvect.at(i));
      line = line + ": ";
      line = line + std::get<1> (mwl->prefvect.at(i)) + "\n";
      f.write(line.c_str(), line.size());
    }
  mwl->prefvectmtx.unlock();
  f.close();

  window->close();
  Gtk::Window *windowinfo = new Gtk::Window;
  windowinfo->set_application(mwl->get_application());
  windowinfo->set_modal(true);
  windowinfo->set_transient_for(*mwl);
  windowinfo->set_name("settingsWindow");
  windowinfo->get_style_context()->add_provider(
    mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  windowinfo->set_title(gettext("Info"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  windowinfo->set_child(*grid);

  Gtk::Label *info = Gtk::make_managed<Gtk::Label> ();
  info->set_halign(Gtk::Align::CENTER);
  info->set_margin(5);
  info->set_text(gettext("Application will be closed now. "
                         "Start it again to apply changes."));
  info->set_justify(Gtk::Justification::CENTER);
  info->set_max_width_chars(40);
  info->set_wrap(true);
  info->set_wrap_mode(Pango::WrapMode::WORD);
  grid->attach(*info, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button> ();
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_name("applyButton");
  close->get_style_context()->add_provider(mwl->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  close->set_label(gettext("Close"));
  grid->attach(*close, 0, 1, 1, 1);

  Gtk::Requisition rq1, rq2;
  grid->get_preferred_size(rq1, rq2);
  windowinfo->set_size_request(rq2.get_width(),
                               rq2.get_height());

  close->signal_clicked().connect([windowinfo]
  {
    windowinfo->close();
  });

  windowinfo->signal_close_request().connect([mwl, windowinfo]
  {
    windowinfo->hide();
    delete windowinfo;
    mwl->close();
    return true;
  },
  false);

  windowinfo->show();
}
