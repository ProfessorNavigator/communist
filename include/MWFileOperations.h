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

#ifndef SRC_GUI_INCLUDE_MWFILEOPERATIONS_H_
#define SRC_GUI_INCLUDE_MWFILEOPERATIONS_H_

#include "MainWindow.h"

class MainWindow;

class MWFileOperations
{
public:
  MWFileOperations(MainWindow *mw);
  virtual
  ~MWFileOperations();
  void
  fileDownloadProg(std::string *keyg,
                   std::filesystem::path *filepathg,
                   uint64_t *cursizeg, std::mutex *mtx);
  void
  fileSendProg(std::string *keyg,
               std::filesystem::path *filepathg,
               uint64_t *cursizeg, std::mutex *mtx);
  void
  fileRcvdStatus(std::string *key,
                 std::filesystem::path *filename,
                 std::mutex *dispNmtx, int variant);
  void
  fileRejectedSlot(std::string *keyr,
                   std::filesystem::path *filer,
                   std::mutex *disp5mtx);
  void
  fileRequestSlot(std::string *key, uint64_t *tm,
                  uint64_t *filesize,
                  std::string *filename, std::mutex *disp4mtx);
private:
  void
  fileSendCncl(MainWindow *mwl, std::string key,
               std::filesystem::path filepath);
  void
  acceptButtonSlot(Gtk::Window *window, MainWindow *mwl,
                   std::string keyloc,
                   uint64_t tml, std::string fnmloc, uint64_t fs);
  void
  rejectButtonSlot(Gtk::Window *window, MainWindow *mwl,
                   std::string keyloc,
                   uint64_t tml);
  void
  openButtonSlot(int rid,
                 Glib::RefPtr<Gtk::FileChooserNative> fcd,
                 MainWindow *mwl, std::string keyloc, uint64_t tml,
                 std::string fnmloc, uint64_t fs);
  void
  warningAcceptButton(std::filesystem::path fp,
                      MainWindow *mwl,
                      std::string keyloc, uint64_t tml, uint64_t fs);
  void
  cancelRcvFile(MainWindow *mwl, std::string keyloc,
                std::filesystem::path fp);

  MainWindow *mw = nullptr;
};

#endif /* SRC_GUI_INCLUDE_MWFILEOPERATIONS_H_ */
