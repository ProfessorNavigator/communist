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
#include "AuxFunc.h"

AuxFunc::AuxFunc()
{
  // TODO Auto-generated constructor stub

}

AuxFunc::~AuxFunc()
{
  // TODO Auto-generated destructor stub
}

void
AuxFunc::homePath(std::string *filename)
{
  char *fnm = getenv("USERPROFILE");
  if(fnm)
    {
      *filename = std::string(getenv("USERPROFILE"));
    }
  else
    {
      fnm = getenv("HOMEDRIVE");
      if(fnm)
	{
	  *filename = std::string(getenv("HOMEDRIVE"));
	}
      else
	{
	  fnm = getenv("HOMEPATH");
	  if(fnm)
	    {
	      *filename = std::string(getenv("HOMEPATH"));
	    }
	  else
	    {
	      fnm = getenv("HOME");
	      if(fnm)
		{
		  *filename = std::string(getenv("HOME"));
		}
	      else
		{
		  fnm = getenv("SystemDrive");
		  if(fnm)
		    {
		      *filename = std::string(getenv("SystemDrive"));
		    }
		  else
		    {
		      std::cerr << "Cannot find user home folder" << std::endl;
		      exit(1);
		    }
		}
	    }
	}
    }
  toutf8(*filename);
}

std::string
AuxFunc::get_selfpath()
{
  std::filesystem::path p;
#ifdef __linux
  p = std::filesystem::u8path("/proc/self/exe");
  return std::filesystem::read_symlink(p).u8string();
#endif
#ifdef __WIN32
  char pth [MAX_PATH];
  GetModuleFileNameA(NULL, pth, MAX_PATH);
  p = std::filesystem::path(pth);
  return p.u8string();
#endif
}

void
AuxFunc::toutf8(std::string &line)
{
  const std::string::size_type srclen = line.size();
  std::vector<UChar> target(srclen);
  UErrorCode status = U_ZERO_ERROR;
  UConverter *conv = ucnv_open(NULL, &status);
  int32_t len = ucnv_toUChars(conv, target.data(), srclen, line.c_str(), srclen,
			      &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  icu::UnicodeString ustr(target.data(), len);
  line.clear();
  ustr.toUTF8String(line);
  ucnv_close(conv);
}

std::string
AuxFunc::utf8to(std::string line)
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr;
  UConverter *c = ucnv_open(NULL, &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  status = U_ZERO_ERROR;
  std::vector<char> target2;
  ustr.remove();
  ustr = icu::UnicodeString::fromUTF8(line.c_str());
  target2.resize(ustr.length());
  char16_t data[ustr.length()];
  for(int i = 0; i < ustr.length(); i++)
    {
      data[i] = ustr.charAt(i);
    }
  size_t cb = ucnv_fromUChars(c, target2.data(), ustr.length(), data,
			      ustr.length(), &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  if(status == U_BUFFER_OVERFLOW_ERROR)
    {
      status = U_ZERO_ERROR;
      target2.clear();
      target2.resize(cb);
      ucnv_fromUChars(c, target2.data(), cb, data, ustr.length(), &status);
      if(!U_SUCCESS(status))
	{
	  std::cerr << u_errorName(status) << std::endl;
	}
    }
  line.clear();
  line = std::string(target2.begin(), target2.end());
  ucnv_close(c);

  return line;
}

std::string
AuxFunc::stringToLower(std::string line)
{
  std::string innerline = line;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(innerline.c_str());
  ustr.toLower();
  line.clear();
  ustr.toUTF8String(line);
  return line;
}
