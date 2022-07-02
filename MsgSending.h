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

#ifndef SRC_MSGSENDING_H_
#define SRC_MSGSENDING_H_

#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <filesystem>
#include "NetworkOperations.h"
#include "AuxFunc.h"

class NetworkOperations;

class MsgSending
{
public:
  MsgSending (NetworkOperations *No);
  virtual
  ~MsgSending ();
  int
  sendMsg (std::filesystem::path pvect, std::string key, int variant, int sock,
	   uint32_t ip, uint16_t port);
private:
  NetworkOperations *no = nullptr;
};

#endif /* SRC_MSGSENDING_H_ */
