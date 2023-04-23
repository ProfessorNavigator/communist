/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

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
#ifndef AUXFUNC_H_
#define AUXFUNC_H_

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

class AuxFunc
{
public:
  AuxFunc();
  virtual
  ~AuxFunc();
  void
  homePath(std::string *filename);
  std::string
  get_selfpath();
  void
  toutf8(std::string &line);
  std::string
  utf8to(std::string line);
  std::string
  stringToLower(std::string line);
};

#endif /* AUXFUNC_H_ */
