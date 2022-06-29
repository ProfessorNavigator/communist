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

#ifndef SRC_FILERECEIVEOP_H_
#define SRC_FILERECEIVEOP_H_

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include "NetworkOperations.h"
#include "AuxFunc.h"

class NetworkOperations;

class FileReceiveOp
{
public:
  FileReceiveOp (NetworkOperations *No);
  virtual
  ~FileReceiveOp ();
  void
  fileProcessing (std::string msgtype, std::string key, int ip6check,
		  int sockipv, sockaddr_in *from, sockaddr_in6 *from6);
  void
  fileFQ (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFJ (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFA (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFr (std::string msgtype, std::string key, int rcvip6, sockaddr_in6 *from6,
	  sockaddr_in *from, int sockipv4, std::vector<char> &buf);
  void
  fileFRFI (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFB (std::string msgtype, std::string key, int rcvip6, sockaddr_in6 *from6,
	  sockaddr_in *from, int sockipv4, std::vector<char> &buf);
  void
  fileFH (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFb (std::string msgtype, std::string key, int rcvip6, sockaddr_in6 *from6,
	  sockaddr_in *from, int sockipv4, std::vector<char> &buf);
  void
  fileFp (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFe (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFE (std::string msgtype, std::string key, std::vector<char> &buf);
  void
  fileFF (std::string msgtype, std::string key, std::vector<char> &buf);
private:
  NetworkOperations *no = nullptr;
};

#endif /* SRC_FILERECEIVEOP_H_ */
