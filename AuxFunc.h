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
#ifndef AUXFUNC_H_
#define AUXFUNC_H_

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <gcrypt.h>
#include <zip.h>
#include <libtorrent/kademlia/ed25519.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/kademlia/item.hpp>
#include <libtorrent/entry.hpp>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#endif

class AuxFunc
{
public:
  AuxFunc ();
  virtual
  ~AuxFunc ();
  void
  homePath (std::string *filename);
  void
  cryptFile (std::string Username, std::string Password, std::string infile,
	     std::string outfile);
  std::vector<char>
  cryptStrm (std::string Username, std::string Password,
	     std::vector<char> &input);
  void
  decryptFile (std::string Username, std::string Password, std::string infile,
	       std::string outfile);
  std::vector<char>
  decryptStrm (std::string Username, std::string Password,
	       std::vector<char> &input);
  void
  packing (std::string source, std::string out);
  void
  unpacking (std::string archadress, std::string outfolder);
  std::string
  get_selfpath ();
  void
  put_string (lt::entry &e, std::array<char, 64> &sig, std::int64_t &seq,
	      std::string const &salt, std::array<char, 32> const &pk,
	      std::array<char, 64> const &sk, std::string str);
  std::vector<char>
  filehash (std::filesystem::path filepath);
  void
  toutf8 (std::string &line);
  std::string
  utf8to (std::string line);
  std::string
  stringToLower (std::string line);
  std::vector<char>
  strhash (std::vector<char> &th, int type);
  std::string
  string_to_hex (const std::string &input);
  std::string
  hex_to_string (const std::string &input);

private:
  int
  fileNames (std::string adress, std::vector<std::string> &filenames);
  int
  hex_value (unsigned char hex_digit);
};

#endif /* AUXFUNC_H_ */
