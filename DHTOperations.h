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

#ifndef DHTOPERATIONS_H_
#define DHTOPERATIONS_H_
#include <vector>
#include <mutex>
#include <thread>
#include <filesystem>
#include <string>
#include <sstream>
#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/session_handle.hpp>
#include <libtorrent/session_params.hpp>
#include "AuxFunc.h"
#include "NetworkOperations.h"

class NetworkOperations;

class DHTOperations
{
public:
  DHTOperations (NetworkOperations *No);
  virtual
  ~DHTOperations ();
  void
  processDHT ();
private:
  std::vector<std::string>
  getFrVect ();
  std::vector<std::tuple<std::string, uint32_t, uint16_t>>
  putVect ();
  std::string
  getSes (std::string key, lt::session *ses);
  std::string
  getSes6 (std::string key, lt::session *ses);
  std::string
  putSes (std::string otherkey, uint32_t ip, uint16_t port, lt::session *ses);
  std::string
  putSes6 (std::string otherkey, lt::session *ses);
  void
  getvResult (std::string key, uint32_t ip, uint16_t port, int seq);
  void
  getvResult6 (std::string key, std::string ip, uint16_t port, int seq);
  NetworkOperations *no = nullptr;
};

#endif /* DHTOPERATIONS_H_ */
