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

#include "ContactOperations.h"

ContactOperations::ContactOperations(MainWindow *Mw)
{
  mw = Mw;
}

ContactOperations::~ContactOperations()
{
  // TODO Auto-generated destructor stub
}

void
ContactOperations::selectContact(Gtk::ScrolledWindow *scr,
                                 std::string selkey,
                                 std::string selnick)
{
  if(mw->Scrwinovershot.connected())
    {
      mw->Scrwinovershot.disconnect();
    }
  mw->usermsgadj = -1;
  mw->msgwinadj = -1;
  Gtk::Requisition rq1, rq2;
  mw->Rightgrid->get_preferred_size(rq1, rq2);
  mw->sentstatmtx.lock();
  mw->sentstatus.clear();
  mw->sentstatmtx.unlock();
  mw->msg_grid_vectmtx.lock();
  mw->msg_grid_vect.clear();
  mw->msg_grid_vectmtx.unlock();
  if(mw->selectedc != nullptr)
    {
      mw->selectedc->set_name("inactiveButton");
    }
  Gtk::Widget *widg = scr->get_child();
  if(widg != nullptr)
    {
      if(mw->msgovllab != nullptr)
        {
          widg = mw->msgovllab->get_parent();
          if(widg != nullptr)
            {
              widg = widg->get_parent();
              if(widg != nullptr)
                {
                  mw->msgovl->remove_overlay(*widg);
                  mw->msgovllab = nullptr;
                }
            }
        }
      scr->unset_child();
    }
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
  mw->msg_win_gr = grid;
  scr->set_child(*grid);

  std::string filename;
  AuxFunc af;
  filename = selkey;
  mw->frvectmtx.lock();
  auto itfrv = std::find_if(
                 mw->friendvect.begin(), mw->friendvect.end(), [selkey]
                 (auto & el)
  {
    return std::get<2>(el)->get_text() == Glib::ustring(selkey);
  });
  if(itfrv != mw->friendvect.end())
    {
      Gtk::Button *button = std::get<0> (*itfrv);
      button->set_name("activeButton");
      mw->selectedc = button;
      Gtk::Label *l = std::get<3> (*itfrv);
      if(l != nullptr)
        {
          Gtk::Grid *gr = std::get<1> (*itfrv);
          gr->remove(*l);
          std::get<3> (*itfrv) = nullptr;
        }
    }
  mw->frvectmtx.unlock();
  int index = -1;
  mw->contmtx.lock();
  auto iter = std::find_if(mw->contacts.begin(),
                           mw->contacts.end(),
                           [&filename]
                           (auto & el)
  {
    return filename == std::get<1>(el);
  });
  if(iter != mw->contacts.end())
    {
      index = std::get<0> (*iter);
      std::string key = mw->fordelkey;
      mw->fordelkey = std::get<1> (*iter);
      if(key != mw->fordelkey)
        {
          Glib::RefPtr<Gtk::TextBuffer> buf =
            mw->msgfrm->get_buffer();
          buf->set_text("");
        }
    }
  if(index >= 0)
    {
      af.homePath(&filename);
      std::vector<std::filesystem::path> msg;
      LibCommunist Lc;
      msg = Lc.listMessages(filename, selkey, mw->Username,
                            mw->Password);
      if(msg.size() > 0)
        {
          std::sort(msg.begin(), msg.end(), []
                    (auto & el1, auto & el2)
          {
            std::string corr = el1.filename().u8string();
            corr = corr.substr(0, corr.find("f"));
            std::stringstream strm;
            std::locale loc("C");
            strm.imbue(loc);
            strm << corr;
            int one;
            strm >> one;
            corr = el2.filename().u8string();
            corr = corr.substr(0, corr.find("f"));
            strm.str("");
            strm.clear();
            strm.imbue(loc);
            strm << corr;
            int two;
            strm >> two;

            return one < two;
          });

          MainWindow *mwl = mw;
          mw->Scrwinovershot =
            scr->signal_edge_reached().connect(
              [mwl, grid, selkey, selnick, index, scr]
              (Gtk::PositionType pos)
          {
            std::vector<std::filesystem::path> msgl;
            AuxFunc af;
            std::string filename;
            af.homePath(&filename);
            mwl->contmtx.lock();
            LibCommunist Lc;
            msgl = Lc.listMessages(filename, selkey, mwl->Username,
                                   mwl->Password);
            mwl->contmtx.unlock();
            std::sort(msgl.begin(), msgl.end(), []
                      (auto & el1, auto & el2)
            {
              std::string corr = el1.filename().u8string();
              corr = corr.substr(0, corr.find("f"));
              std::stringstream strm;
              std::locale loc("C");
              strm.imbue(loc);
              strm << corr;
              int one;
              strm >> one;
              corr = el2.filename().u8string();
              corr = corr.substr(0, corr.find("f"));
              strm.str("");
              strm.clear();
              strm.imbue(loc);
              strm << corr;
              int two;
              strm >> two;

              return one > two;
            });
            if(pos == Gtk::PositionType::TOP)
              {
                if(msgl.size() > 20)
                  {
                    mwl->msg_grid_vectmtx.lock();
                    std::vector<std::filesystem::path> tmp;
                    size_t end;
                    if(msgl.size() - mwl->msg_grid_vect.size() > 20)
                      {
                        end = mwl->msg_grid_vect.size() + 20;
                      }
                    else
                      {
                        end = msgl.size();
                      }
                    for(size_t i = mwl->msg_grid_vect.size(); i < end; i++)
                      {
                        tmp.push_back(msgl[i]);
                      }
                    mwl->msg_grid_vectmtx.unlock();
                    std::sort(tmp.begin(), tmp.end(), []
                              (auto & el1, auto & el2)
                    {
                      std::string corr = el1.filename().u8string();
                      corr = corr.substr(0, corr.find("f"));
                      std::stringstream strm;
                      std::locale loc("C");
                      strm.imbue(loc);
                      strm << corr;
                      int one;
                      strm >> one;
                      corr = el2.filename().u8string();
                      strm.str("");
                      strm.clear();
                      strm.imbue(loc);
                      strm << corr;
                      int two;
                      strm >> two;

                      return one > two;
                    });
                    Glib::RefPtr<Gtk::Adjustment> adj;
                    adj = scr->get_vadjustment();
                    mwl->msgwinadj = adj->get_upper();
                    mwl->formMsgWinGrid(tmp, 0, tmp.size(), grid, selkey,
                                        selnick, index, 1);
                  }
              }
            if(pos == Gtk::PositionType::BOTTOM)
              {
                mwl->msgwinadj = -1;
                mwl->msg_grid_vectmtx.lock();
                while(mwl->msg_grid_vect.size() > 20)
                  {
                    Gtk::Widget *widg = std::get<0>(mwl->msg_grid_vect[0]);
                    int column, row, width, height;
                    grid->query_child(*widg, column, row, width, height);
                    grid->remove_row(row);
                    mwl->msg_grid_vect.erase(mwl->msg_grid_vect.begin());
                  }
                mwl->msg_grid_vectmtx.unlock();
              }
          });
          size_t begin;
          if(msg.size() > 20)
            {
              begin = msg.size() - 20;
            }
          else
            {
              begin = 0;
            }
          mw->formMsgWinGrid(msg, begin, msg.size(), grid, selkey,
                             selnick,
                             index, 0);
        }
    }
  mw->contmtx.unlock();
  Glib::RefPtr<Glib::MainContext> mc =
    Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  Glib::RefPtr<Gtk::Adjustment> adj;
  adj = scr->get_vadjustment();
  adj->set_value(adj->get_upper() - adj->get_page_size());
}

void
ContactOperations::formMsgWinGrid(
  std::vector<std::filesystem::path> &msg,
  size_t begin, size_t end, Gtk::Grid *grid,
  std::string key, std::string nick, int index,
  int varform)
{
  AuxFunc af;
  std::string filename;
  std::filesystem::path filepath;
  Gtk::Requisition rq1, rq2;
  std::stringstream strm;
  std::locale loc("C");
  std::string otherstr(key);
  int count = 0;
  LibCommunist Lc;
  grid->set_column_homogeneous(true);
  for(size_t i = begin; i < end; i++)
    {
      std::string line = msg[i].u8string();

      filename = Lc.openMessage(line, key, mw->Username,
                                mw->Password);
      std::fstream f;
      std::string::size_type n;
      std::filesystem::path tmpp = std::filesystem::u8path(
                                     filename);

      Gtk::Grid *grmsg = Gtk::make_managed<Gtk::Grid> ();
      Gtk::Frame *fr = Gtk::make_managed<Gtk::Frame> ();
      Glib::RefPtr<Gtk::GestureClick> clck =
        Gtk::GestureClick::create();
      clck->set_button(0);

      fr->set_child(*grmsg);
      fr->add_controller(clck);
      fr->get_style_context()->add_provider(mw->css_provider,
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);
      fr->set_margin(2);

      Gtk::Label *date = Gtk::make_managed<Gtk::Label> ();
      date->set_halign(Gtk::Align::START);
      date->set_margin(2);
      grmsg->attach(*date, 0, 0, 1, 1);

      Gtk::Label *from = Gtk::make_managed<Gtk::Label> ();
      from->set_halign(Gtk::Align::START);
      from->set_margin(2);
      grmsg->attach(*from, 0, 1, 1, 1);

      Gtk::Frame *repfr = Gtk::make_managed<Gtk::Frame> ();
      repfr->set_halign(Gtk::Align::START);
      repfr->set_name("repFrame");
      repfr->get_style_context()->add_provider(mw->css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      repfr->set_margin(2);
      Gtk::Label *repl = Gtk::make_managed<Gtk::Label> ();
      repl->set_halign(Gtk::Align::START);
      repl->set_margin(5);
      repl->set_use_markup(true);
      repl->set_max_width_chars(20);
      repl->set_ellipsize(Pango::EllipsizeMode::END);
      repfr->set_child(*repl);
      grmsg->attach(*repfr, 0, 2, 1, 1);

      int count2 = 0;
      int type;
      int own = 0;
      f.open(tmpp, std::ios_base::in);
      std::string fromkey = "";
      while(!f.eof())
        {
          std::string gl;
          getline(f, gl);
          if(gl != "")
            {
              if(count2 == 0)
                {
                  fromkey = gl;
                  n = fromkey.find("From: ");
                  if(n != std::string::npos)
                    {
                      fromkey.erase(n, std::string("From: ").size());
                    }
                }
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
                      Glib::ustring txt(gl);
                      Glib::ustring trnstxt;
                      trnstxt = gettext("Forwarded from: ");
                      txt = trnstxt + txt;
                      from->set_text(txt);
                      if(fromkey == Lc.getKeyFmSeed(mw->seed))
                        {
                          own = 0;
                          mw->sentstatmtx.lock();
                          mw->sentstatus.push_back(
                            std::make_tuple(mw->fordelkey, msg[i], from));
                          mw->sentstatmtx.unlock();
                          if(mw->oper != nullptr)
                            {
                              bool check = true;
                              check = mw->oper->checkIfMsgSent(msg[i]);
                              if(!check)
                                {
                                  from->set_name("notSentMsg");
                                }
                              else
                                {
                                  from->set_name("SentMsg");
                                }
                            }

                          from->get_style_context()->add_provider(
                            mw->css_provider,
                            GTK_STYLE_PROVIDER_PRIORITY_USER);
                          fr->set_name("myMsg");
                        }
                      if(fromkey == std::string(key))
                        {
                          own = 1;
                          fr->set_name("frMsg");
                        }
                    }
                  else
                    {
                      if(fromkey == Lc.getKeyFmSeed(mw->seed))
                        {
                          own = 0;
                          from->set_text(gettext("Me:"));
                          mw->sentstatmtx.lock();
                          mw->sentstatus.push_back(
                            std::make_tuple(mw->fordelkey, msg[i], from));
                          mw->sentstatmtx.unlock();
                          if(mw->oper != nullptr)
                            {
                              bool check = true;
                              check = mw->oper->checkIfMsgSent(msg[i]);
                              if(!check)
                                {
                                  from->set_name("notSentMsg");
                                }
                              else
                                {
                                  from->set_name("SentMsg");
                                }
                            }

                          from->get_style_context()->add_provider(
                            mw->css_provider,
                            GTK_STYLE_PROVIDER_PRIORITY_USER);
                          fr->set_name("myMsg");
                        }
                      if(fromkey == std::string(key))
                        {
                          own = 1;
                          from->set_text(Glib::ustring(nick) + ":");
                          fr->set_name("frMsg");
                        }
                    }
                }
              if(count2 == 3)
                {
                  n = gl.find("Creation time: ");
                  if(n != std::string::npos)
                    {
                      gl.erase(n, std::string("Creation time: ").size());
                    }
                  time_t timet;
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
                      gl.erase(0, std::string("Reply to: ").size());
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
                      Gtk::Label *message = Gtk::make_managed<Gtk::Label> ();
                      message->set_halign(Gtk::Align::START);
                      message->set_margin(5);
                      message->set_justify(Gtk::Justification::LEFT);
                      message->set_wrap(true);
                      message->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
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
                      message->set_margin_bottom(10);
                      message->set_selectable(true);
                      message->set_max_width_chars(mw->msg_width_chars);
                      grmsg->attach(*message, 0, rownum, 1, 1);
                      MWMsgOperations mop(mw);
                      clck->signal_pressed().connect(
                        sigc::bind(
                          sigc::mem_fun(mop, &MWMsgOperations::creatReply),
                          fr, message, clck, mw));
                    }
                }
            }
          count2++;
        }
      f.close();
      std::filesystem::remove_all(tmpp);
      fr->get_preferred_size(rq1, rq2);
      if(own == 0)
        {
          fr->set_halign(Gtk::Align::FILL);
          if(varform == 0)
            {
              grid->attach(*fr, 0, count, 1, 1);
              Gtk::Frame *empty = Gtk::make_managed<Gtk::Frame> ();
              empty->set_size_request(rq2.get_width(), rq2.get_height());
              empty->set_opacity(0);
              grid->attach(*empty, 1, count, 1, 1);
            }
          if(varform == 1)
            {
              grid->insert_row(0);
              grid->attach(*fr, 0, 0, 1, 1);
              Gtk::Frame *empty = Gtk::make_managed<Gtk::Frame> ();
              empty->set_size_request(rq2.get_width(), rq2.get_height());
              empty->set_opacity(0);
              grid->attach(*empty, 1, 0, 1, 1);
            }
        }
      else
        {
          fr->set_halign(Gtk::Align::FILL);
          if(varform == 0)
            {
              grid->attach(*fr, 1, count, 1, 1);
              Gtk::Frame *empty = Gtk::make_managed<Gtk::Frame> ();
              fr->get_preferred_size(rq1, rq2);
              empty->set_size_request(rq2.get_width(), rq2.get_height());
              empty->set_opacity(0);
              grid->attach(*empty, 0, count, 1, 1);
            }
          if(varform == 1)
            {
              grid->insert_row(0);
              grid->attach(*fr, 1, 0, 1, 1);
              Gtk::Frame *empty = Gtk::make_managed<Gtk::Frame> ();
              fr->get_preferred_size(rq1, rq2);
              empty->set_size_request(rq2.get_width(), rq2.get_height());
              empty->set_opacity(0);
              grid->attach(*empty, 0, 0, 1, 1);
            }
        }
      mw->msg_grid_vectmtx.lock();
      if(varform == 0)
        {
          mw->msg_grid_vect.push_back(std::make_tuple(fr, msg[i],
                                      nullptr));
        }
      if(varform == 1)
        {
          mw->msg_grid_vect.insert(mw->msg_grid_vect.begin(),
                                   std::make_tuple(fr, msg[i], nullptr));
        }
      mw->msg_grid_vectmtx.unlock();
      count++;
    }
}

void
ContactOperations::deleteContact()
{
  if(mw->fordelkey != "")
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(mw->get_application());
      window->set_name("settingsWindow");
      window->get_style_context()->add_provider(mw->css_provider,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
      window->set_title(gettext("Remove contact"));
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid> ();
      window->set_child(*grid);

      Gtk::Label *label = Gtk::make_managed<Gtk::Label> ();
      label->set_text(
        gettext(
          "Remove contact? This action will erase all messages forever."));
      label->set_max_width_chars(40);
      label->set_justify(Gtk::Justification::CENTER);
      label->set_wrap(true);
      label->set_wrap_mode(Pango::WrapMode::WORD);
      label->set_margin(5);
      grid->attach(*label, 0, 0, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button> ();
      yes->set_label(gettext("Yes"));
      yes->set_margin(5);
      yes->set_halign(Gtk::Align::CENTER);
      yes->set_name("clearButton");
      yes->get_style_context()->add_provider(mw->css_provider,
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
      MainWindow *mwl = mw;
      yes->signal_clicked().connect(
        sigc::bind(
          sigc::mem_fun(*this, &ContactOperations::deleteContactFunc),
          mwl));
      yes->signal_clicked().connect(
        sigc::mem_fun(*window, &Gtk::Window::close));
      grid->attach(*yes, 0, 1, 1, 1);

      Gtk::Button *no = Gtk::make_managed<Gtk::Button> ();
      no->set_label(gettext("No"));
      no->set_margin(5);
      no->set_halign(Gtk::Align::CENTER);
      no->set_name("rejectButton");
      no->get_style_context()->add_provider(mw->css_provider,
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);
      no->signal_clicked().connect(
        sigc::mem_fun(*window, &Gtk::Window::close));
      grid->attach(*no, 1, 1, 1, 1);

      Gtk::Requisition rq1, rq2;
      grid->get_preferred_size(rq1, rq2);
      window->set_size_request(rq2.get_width(), rq2.get_height());

      window->signal_close_request().connect([window]
      {
        window->hide();
        delete window;
        return true;
      },
      false);

      window->show();
    }
}

void
ContactOperations::deleteContactFunc(MainWindow *mwl)
{
  if(mwl->fordelgrid != nullptr)
    {
      if(mwl->selectedc != nullptr)
        {
          if(mwl->fordelkey != "")
            {
              Gtk::Widget *widg = mwl->Winright->get_child();
              if(widg != nullptr)
                {
                  mwl->Winright->unset_child();
                }

              mwl->frvectmtx.lock();
              mwl->friendvect.erase(
                std::remove_if(
                  mwl->friendvect.begin(), mwl->friendvect.end(), [mwl]
                  (auto & el)
              {
                if(std::get<0>(el) == mwl->selectedc)
                  {
                    mwl->fordelgrid->remove(*(mwl->selectedc));
                    mwl->selectedc = nullptr;
                    return true;
                  }
                else
                  {
                    return false;
                  }

              }),
              mwl->friendvect.end());
              mwl->frvectmtx.unlock();

              mwl->conavmtx.lock();
              if(mwl->conavatars.size() > 0)
                {
                  mwl->conavatars.erase(
                    std::remove_if(
                      mwl->conavatars.begin(),
                      mwl->conavatars.end(),
                      [mwl]
                      (auto & el)
                  {
                    return el.first == Glib::ustring(mwl->fordelkey);
                  }));
                }
              mwl->conavmtx.unlock();
              mwl->fileprogrvectmtx.lock();
              mwl->fileprogrvect.erase(
                std::remove_if(
                  mwl->fileprogrvect.begin(), mwl->fileprogrvect.end(),
                  [mwl]
                  (auto & el)
              {
                if(std::get<0>(el) == mwl->fordelkey)
                  {
                    mwl->dld_grid->remove(*(std::get<3>(el)));
                    mwl->dld_grid->remove(*(std::get<4>(el)));
                    mwl->dld_grid->remove(*(std::get<5>(el)));
                    return true;
                  }
                else
                  {
                    return false;
                  }
              }),
              mwl->fileprogrvect.end());
              if(mwl->fileprogrvect.size() == 0
                  && mwl->dld_win != nullptr)
                {
                  mwl->dld_win->hide();
                  delete mwl->dld_win;
                  mwl->dld_win = nullptr;
                  mwl->dld_grid = nullptr;
                }
              mwl->fileprogrvectmtx.unlock();
              if(mwl->oper != nullptr)
                {
                  mwl->Contdelwin = new Gtk::Window;
                  mwl->Contdelwin->set_application(mwl->get_application());
                  mwl->Contdelwin->set_title(gettext("Contact removing"));
                  mwl->Contdelwin->set_modal(true);
                  mwl->Contdelwin->set_transient_for(*mwl);
                  mwl->Contdelwin->set_name("settingsWindow");
                  mwl->Contdelwin->get_style_context()->add_provider(
                    mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
                  Gtk::Grid *rmwimgr = Gtk::make_managed<Gtk::Grid> ();
                  mwl->Contdelwin->set_child(*rmwimgr);
                  rmwimgr->set_halign(Gtk::Align::CENTER);
                  Gtk::ProgressBar *bar =
                    Gtk::make_managed<Gtk::ProgressBar> ();
                  bar->set_name("clBar");
                  bar->get_style_context()->add_provider(
                    mwl->css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
                  bar->set_halign(Gtk::Align::CENTER);
                  bar->set_valign(Gtk::Align::CENTER);
                  bar->set_text(gettext("Contact is being removed"));
                  bar->set_show_text(true);
                  bar->set_margin(5);
                  rmwimgr->attach(*bar, 0, 0, 1, 1);

                  Glib::Dispatcher *disp = new Glib::Dispatcher;
                  disp->connect([bar]
                  {
                    bar->pulse();
                  });

                  mwl->oper->friendDelPulse_signal = [disp]
                  {
                    disp->emit();
                  };

                  mwl->Contdelwin->signal_close_request().connect([mwl, disp]
                  {
                    delete disp;
                    mwl->Contdelwin->hide();
                    delete mwl->Contdelwin;
                    mwl->Contdelwin = nullptr;
                    return true;
                  },
                  false);
                  mwl->Contdelwin->show();
                  mwl->oper->removeFriend(mwl->fordelkey);
                }
              mwl->selectedc = nullptr;
              mwl->fordelkey = "";
            }
        }
    }
}

void
ContactOperations::friendRemoved(std::string *key,
                                 std::mutex *mtx)
{
  std::string keyloc = *key;
  std::cout << "Deleted: " << keyloc << std::endl;
  mtx->unlock();
  AuxFunc af;
  std::string homepath;
  af.homePath(&homepath);
  LibCommunist Lc;
  mw->proffopmtx.lock();
  mw->contmtx.lock();
  mw->contacts = Lc.readContacts(homepath, mw->Username,
                                 mw->Password);
  mw->contmtx.unlock();
  mw->addfrmtx.lock();
  mw->Addfriends = Lc.readRequestList(homepath, mw->Username,
                                      mw->Password);
  mw->addfrmtx.unlock();
  mw->proffopmtx.unlock();
  if(mw->Contdelwin != nullptr)
    {
      mw->Contdelwin->close();
    }
  mw->mainWindow();
}

