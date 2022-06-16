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
#include "AuxFunc.h"

AuxFunc::AuxFunc ()
{
  // TODO Auto-generated constructor stub

}

AuxFunc::~AuxFunc ()
{
  // TODO Auto-generated destructor stub
}

void
AuxFunc::homePath (std::string *filename)
{
  char *fnm = getenv ("USERPROFILE");
  if (fnm)
    {
      *filename = std::string (getenv ("USERPROFILE"));
    }
  else
    {
      fnm = getenv ("HOMEDRIVE");
      if (fnm)
	{
	  *filename = std::string (getenv ("HOMEDRIVE"));
	}
      else
	{
	  fnm = getenv ("HOMEPATH");
	  if (fnm)
	    {
	      *filename = std::string (getenv ("HOMEPATH"));
	    }
	  else
	    {
	      fnm = getenv ("HOME");
	      if (fnm)
		{
		  *filename = std::string (getenv ("HOME"));
		}
	      else
		{
		  fnm = getenv ("SystemDrive");
		  if (fnm)
		    {
		      *filename = std::string (getenv ("SystemDrive"));
		    }
		  else
		    {
		      std::cerr << "Cannot find user home folder" << std::endl;
		      exit (1);
		    }
		}
	    }
	}
    }
  toutf8 (*filename);
}

void
AuxFunc::cryptFile (std::string Username, std::string Password,
		    std::string infile, std::string outfile)
{
  std::vector<char> F;
  std::vector<char> out;
  std::vector<char> tmpv;
  std::copy (Username.begin (), Username.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> saltv = strhash (tmpv, 1);
  tmpv.clear ();
  std::copy (Password.begin (), Password.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> passv = strhash (tmpv, 2);
  tmpv.clear ();
  size_t textlength;
  std::string temp;
  std::filesystem::path inpath = std::filesystem::u8path (infile);
  std::filesystem::path outpath = std::filesystem::u8path (outfile);

  const char *pass = passv.data ();
  size_t passlength = passv.size ();
  const char *salt = saltv.data ();
  size_t saltlength = saltv.size ();
  if (std::filesystem::exists (inpath))
    {
      int filesize = std::filesystem::file_size (inpath);
      if (filesize < 16)
	{
	  std::cerr << "File you\'ve try to crypt is too small" << std::endl;
	}
      else
	{
	  std::fstream file (inpath, std::ios::in | std::ios_base::binary);
	  std::fstream file2 (outpath, std::ios::out | std::ios_base::binary);

	  int partsize = 16;
	  int readbytes = 0;

	  for (;;)
	    {
	      F.clear ();
	      if (filesize - readbytes < partsize * 2)
		{
		  partsize = filesize - readbytes;
		  F.resize (partsize);
		  file.read (&F[0], partsize);
		  readbytes = readbytes + partsize;
		}
	      else
		{
		  F.resize (partsize);
		  file.read (&F[0], partsize);
		  readbytes = readbytes + partsize;
		}
	      textlength = F.size ();
	      gcry_error_t err;
	      gcry_cipher_hd_t hd;
	      err = gcry_cipher_open (&hd, GCRY_CIPHER_AES256,
				      GCRY_CIPHER_MODE_CBC,
				      GCRY_CIPHER_SECURE | GCRY_CIPHER_CBC_CTS);
	      if (err != 0)
		{
		  std::cerr << gcry_strerror (err) << std::endl;
		}
	      err = gcry_cipher_setkey (hd, pass, passlength);
	      if (err != 0)
		{
		  std::cerr << gcry_strerror (err) << std::endl;
		}
	      err = gcry_cipher_setiv (hd, salt, saltlength);
	      if (err != 0)
		{
		  std::cerr << gcry_strerror (err) << std::endl;
		}
	      out.resize (textlength);
	      err = gcry_cipher_encrypt (hd, out.data (), textlength, F.data (),
					 textlength);
	      if (err != 0)
		{
		  std::cerr << gcry_strerror (err) << std::endl;
		}
	      gcry_cipher_close (hd);
	      file2.write (reinterpret_cast<const char*> (&out[0]),
			   out.size ());
	      if (readbytes >= filesize)
		{
		  break;
		}
	    }
	  file2.close ();
	  file.close ();
	}
    }
  else
    {
      std::cerr << "File for crypting not found" << std::endl;
    }
}

std::vector<char>
AuxFunc::cryptStrm (std::string Username, std::string Password,
		    std::vector<char> &input)
{

  std::vector<char> inner;
  std::vector<char> result;
  std::vector<char> tmpv;
  std::copy (Username.begin (), Username.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> saltv = strhash (tmpv, 1);
  tmpv.clear ();
  std::copy (Password.begin (), Password.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> passv = strhash (tmpv, 2);
  tmpv.clear ();
  inner = input;
  if (inner.size () < 16)
    {
      std::cerr << "Crypting error: too short message" << std::endl;
    }
  else
    {
      size_t textlength;
      std::string temp;
      const char *pass = passv.data ();
      size_t passlength = passv.size ();
      const char *salt = saltv.data ();
      size_t saltlength = saltv.size ();
      textlength = inner.size ();
      gcry_error_t err;
      gcry_cipher_hd_t hd;
      err = gcry_cipher_open (&hd, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
			      GCRY_CIPHER_SECURE | GCRY_CIPHER_CBC_CTS);
      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      err = gcry_cipher_setkey (hd, pass, passlength);
      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      err = gcry_cipher_setiv (hd, salt, saltlength);
      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      result.resize (textlength);
      err = gcry_cipher_encrypt (hd, result.data (), textlength, inner.data (),
				 textlength);
      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      gcry_cipher_close (hd);
    }

  return result;
}

void
AuxFunc::packing (std::string source, std::string out)
{
  int er = 0;
  std::filesystem::path dir;
  dir = std::filesystem::u8path (source);
  source = dir.generic_u8string ();
  if (std::filesystem::exists (dir))
    {
      if (std::filesystem::is_directory (dir))
	{
	  zip_t *z;
	  zip_error_t err;
	  z = zip_open (out.c_str (), ZIP_TRUNCATE | ZIP_CREATE, &er);
	  if (er < 1)
	    {
	      std::vector<std::filesystem::path> listf;
	      std::vector<std::filesystem::path> listd;
	      std::string line;
	      if (!std::filesystem::is_empty (dir))
		{
		  for (auto &iter : std::filesystem::recursive_directory_iterator (
		      dir))
		    {
		      std::filesystem::path path = iter.path ();
		      path = std::filesystem::u8path (path.generic_u8string ());
		      if (std::filesystem::is_directory (path))
			{
			  listd.push_back (path);
			}
		      else
			{
			  listf.push_back (path);
			}
		    }
		  std::sort (listd.begin (), listd.end (), []
		  (auto &el1, auto &el2)
		    {
		      return el1.string ().size () < el2.string ().size ();
		    });

		  std::string pardir = dir.filename ().u8string ();
		  zip_dir_add (z, pardir.c_str (), ZIP_FL_ENC_UTF_8);

		  for (size_t i = 0; i < listd.size (); i++)
		    {
		      line = listd[i].u8string ();
		      std::string::size_type n;
		      n = line.find (source, 0);
		      line.erase (n, source.size ());
		      line = pardir + line;
		      if (!std::filesystem::is_empty (listd[i]))
			{
			  zip_dir_add (z, line.c_str (), ZIP_FL_ENC_UTF_8);
			}
		    }
		  for (size_t i = 0; i < listf.size (); i++)
		    {
		      line = listf[i].u8string ();
		      std::string::size_type n;
		      n = line.find (source, 0);
		      line.erase (n, source.size ());
		      zip_source_t *zsource;
		      zsource = zip_source_file_create (
			  listf[i].u8string ().c_str (), 0, 0, &err);
		      line = pardir + line;
		      zip_file_add (z, line.c_str (), zsource,
		      ZIP_FL_ENC_UTF_8);
		    }
		}

	      zip_close (z);
	    }
	  else
	    {
	      std::cerr << "Error on packaing: " << strerror (er) << std::endl;
	    }
	}
      else
	{
	  zip_t *z;
	  zip_error_t err;
	  std::string line = dir.filename ().u8string ();
	  z = zip_open (out.c_str (), ZIP_TRUNCATE | ZIP_CREATE, &er);
	  if (er >= 1)
	    {
	      std::cerr << "Packing (file) error: " << strerror (er)
		  << std::endl;
	    }
	  else
	    {
	      zip_source_t *zsource;
	      zsource = zip_source_file_create (source.c_str (), 0, 0, &err);
	      if (zsource == nullptr)
		{
		  std::cerr << "Error on open file while packing" << std::endl;
		}
	      else
		{
		  zip_file_add (z, line.c_str (), zsource, ZIP_FL_ENC_UTF_8);
		}
	      zip_close (z);
	    }
	}
    }
}

void
AuxFunc::decryptFile (std::string Username, std::string Password,
		      std::string infile, std::string outfile)
{
  std::vector<char> F;
  std::vector<char> out;
  std::vector<char> tmpv;
  std::copy (Username.begin (), Username.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> saltv = strhash (tmpv, 1);
  tmpv.clear ();
  std::copy (Password.begin (), Password.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> passv = strhash (tmpv, 2);
  tmpv.clear ();
  size_t textlength;
  std::string temp;
  std::filesystem::path inpath = std::filesystem::u8path (infile);
  std::filesystem::path outpath = std::filesystem::u8path (outfile);

  const char *pass = passv.data ();
  size_t passlength = passv.size ();
  const char *salt = saltv.data ();
  size_t saltlength = saltv.size ();
  if (std::filesystem::exists (inpath))
    {
      std::fstream file (inpath, std::ios::binary | std::ios::in);
      std::fstream file2 (outpath, std::ios::binary | std::ios::out);

      int filesize = std::filesystem::file_size (inpath);
      int partsize = 16;
      int readbytes = 0;

      for (;;)
	{
	  F.clear ();
	  if (filesize - readbytes < partsize * 2)
	    {
	      partsize = filesize - readbytes;
	      F.resize (partsize);
	      file.read (&F[0], partsize);
	      readbytes = readbytes + partsize;
	    }
	  else
	    {
	      F.resize (partsize);
	      file.read (&F[0], partsize);
	      readbytes = readbytes + partsize;
	    }
	  textlength = F.size ();
	  gcry_error_t err;
	  gcry_cipher_hd_t hd;
	  err = gcry_cipher_open (&hd, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
				  GCRY_CIPHER_SECURE | GCRY_CIPHER_CBC_CTS);
	  if (err != 0)
	    {
	      std::cout << gcry_strerror (err) << std::endl;
	    }
	  err = gcry_cipher_setkey (hd, pass, passlength);
	  if (err != 0)
	    {
	      std::cout << gcry_strerror (err) << std::endl;
	    }
	  err = gcry_cipher_setiv (hd, salt, saltlength);
	  if (err != 0)
	    {
	      std::cout << gcry_strerror (err) << std::endl;
	    }
	  out.resize (textlength);
	  err = gcry_cipher_decrypt (hd, out.data (), textlength, F.data (),
				     textlength);
	  if (err != 0)
	    {
	      std::cout << gcry_strerror (err) << std::endl;
	    }
	  gcry_cipher_close (hd);
	  file2.write (reinterpret_cast<const char*> (&out[0]), out.size ());
	  if (readbytes >= filesize)
	    {
	      break;
	    }
	}
      file.close ();
      file2.close ();
    }
  else
    {
      std::cerr << "File for decrypting not found" << std::endl;
    }
}

std::vector<char>
AuxFunc::decryptStrm (std::string Username, std::string Password,
		      std::vector<char> &input)
{
  std::vector<char> inner;
  std::vector<char> result;
  std::vector<char> tmpv;
  std::copy (Username.begin (), Username.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> saltv = strhash (tmpv, 1);
  tmpv.clear ();
  std::copy (Password.begin (), Password.end (), std::back_inserter (tmpv));
  while (tmpv.size () > 64)
    {
      tmpv.pop_back ();
    }
  std::vector<char> passv = strhash (tmpv, 2);
  tmpv.clear ();
  inner = input;
  size_t textlength;
  std::string temp;
  if (inner.size () < 16)
    {
      std::cerr << "Decrypting error: too short message" << std::endl;
    }
  else
    {
      const char *pass = passv.data ();
      size_t passlength = passv.size ();
      const char *salt = saltv.data ();
      size_t saltlength = saltv.size ();

      textlength = inner.size ();
      gcry_error_t err;
      gcry_cipher_hd_t hd;
      err = gcry_cipher_open (&hd, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC,
			      GCRY_CIPHER_SECURE | GCRY_CIPHER_CBC_CTS);
      if (err != 0)
	{
	  std::cout << gcry_strerror (err) << std::endl;
	}
      err = gcry_cipher_setkey (hd, pass, passlength);
      if (err != 0)
	{
	  std::cout << gcry_strerror (err) << std::endl;
	}
      err = gcry_cipher_setiv (hd, salt, saltlength);
      if (err != 0)
	{
	  std::cout << gcry_strerror (err) << std::endl;
	}
      result.resize (textlength);
      err = gcry_cipher_decrypt (hd, result.data (), textlength, inner.data (),
				 textlength);
      if (err != 0)
	{
	  std::cout << gcry_strerror (err) << std::endl;
	}
      gcry_cipher_close (hd);
    }

  return result;
}

int
AuxFunc::fileNames (std::string adress, std::vector<std::string> &filenames)
{
  zip_t *z;
  std::string flname;
  int er = 0;
  int num;
  z = zip_open (adress.c_str (), ZIP_RDONLY, &er);
  if (er < 1)
    {
      num = zip_get_num_files (z);

      for (int i = 0; i < num; i++)
	{
	  flname = zip_get_name (z, i, ZIP_FL_ENC_UTF_8);
	  filenames.push_back (flname);
	}
      zip_close (z);
    }

  return er;
}

void
AuxFunc::unpacking (std::string archadress, std::string outfolder)
{
  std::vector<std::string> filenames;
  std::string line, archnm;
  int er = 0;
  er = fileNames (archadress, filenames);
  if (er < 1)
    {
      for (size_t i = 0; i < filenames.size (); i++)
	{
	  line = filenames[i];
	  std::filesystem::path path;
	  if (line.substr (line.size () - 1, line.size () - 1) == "/")
	    {
	      line = outfolder + "/" + line;
	      path = std::filesystem::u8path (line);
	      std::filesystem::create_directories (path);
	    }
	}
      zip_t *z;
      zip_file_t *file;
      zip_stat_t st;
      z = zip_open (archadress.c_str (), ZIP_RDONLY, &er);
      if (er < 1)
	{
	  for (size_t i = 0; i < filenames.size (); i++)
	    {
	      line = filenames[i];
	      if (line.substr (line.size () - 1, line.size () - 1) != "/")
		{
		  file = zip_fopen (z, line.c_str (), ZIP_FL_ENC_UTF_8);
		  zip_stat (z, line.c_str (), ZIP_STAT_NAME | ZIP_FL_ENC_UTF_8,
			    &st);
		  std::vector<char> content;
		  std::filesystem::path path;
		  line = outfolder + "/" + line;
		  path = std::filesystem::u8path (line);
		  std::fstream f;
		  f.open (path, std::ios_base::out | std::ios_base::binary);
		  for (zip_uint64_t i = 0; i < st.size; i++)
		    {
		      content.clear ();
		      content.resize (1);
		      zip_fread (file, content.data (), 1);
		      f.write (content.data (), content.size ());
		    }
		  f.close ();
		  zip_fclose (file);
		}
	    }

	  zip_close (z);
	}
      else
	{

	  if (er == ZIP_ER_EXISTS)
	    {
	      std::cerr
		  << "The file specified by path exists and ZIP_EXCL is set"
		  << std::endl;
	    }
	  if (er == ZIP_ER_INCONS)
	    {
	      std::cerr
		  << "Inconsistencies were found in the file specified by path"
		  << std::endl;
	    }
	  if (er == ZIP_ER_INVAL)
	    {
	      std::cerr << "The path argument is NULL" << std::endl;
	    }
	  if (er == ZIP_ER_MEMORY)
	    {
	      std::cerr << "Required memory could not be allocated"
		  << std::endl;
	    }
	  if (er == ZIP_ER_NOENT)
	    {
	      std::cerr
		  << "The file specified by path does not exist and ZIP_CREATE is not set"
		  << std::endl;
	    }
	  if (er == ZIP_ER_NOZIP)
	    {
	      std::cerr << "The file specified by path is not a zip archive"
		  << std::endl;
	    }
	  if (er == ZIP_ER_OPEN)
	    {
	      std::cerr << "The file specified by path could not be opened"
		  << std::endl;
	    }
	  if (er == ZIP_ER_READ)
	    {
	      std::cerr << "A read error occurred" << std::endl;
	    }
	  if (er == ZIP_ER_SEEK)
	    {
	      std::cerr << "The file specified by path does not allow seeks"
		  << std::endl;
	    }
	}
    }
  else
    {
      if (er == ZIP_ER_EXISTS)
	{
	  std::cerr << "The file specified by path exists and ZIP_EXCL is set"
	      << std::endl;
	}
      if (er == ZIP_ER_INCONS)
	{
	  std::cerr
	      << "Inconsistencies were found in the file specified by path"
	      << std::endl;
	}
      if (er == ZIP_ER_INVAL)
	{
	  std::cerr << "The path argument is NULL" << std::endl;
	}
      if (er == ZIP_ER_MEMORY)
	{
	  std::cerr << "Required memory could not be allocated" << std::endl;
	}
      if (er == ZIP_ER_NOENT)
	{
	  std::cerr
	      << "The file specified by path does not exist and ZIP_CREATE is not set"
	      << std::endl;
	}
      if (er == ZIP_ER_NOZIP)
	{
	  std::cerr << "The file specified by path is not a zip archive"
	      << std::endl;
	}
      if (er == ZIP_ER_OPEN)
	{
	  std::cerr << "The file specified by path could not be opened"
	      << std::endl;
	}
      if (er == ZIP_ER_READ)
	{
	  std::cerr << "A read error occurred" << std::endl;
	}
      if (er == ZIP_ER_SEEK)
	{
	  std::cerr << "The file specified by path does not allow seeks"
	      << std::endl;
	}
    }
}

std::string
AuxFunc::get_selfpath ()
{
  std::filesystem::path p;
#ifdef __linux
  p = std::filesystem::u8path ("/proc/self/exe");
  return std::filesystem::read_symlink (p).u8string ();
#endif
#ifdef __WIN32
  char pth [MAX_PATH];
  GetModuleFileNameA (NULL, pth, MAX_PATH);
  p = std::filesystem::path (pth);
  return p.u8string ();
#endif
}

void
AuxFunc::put_string (lt::entry &e, std::array<char, 64> &sig, std::int64_t &seq,
		     std::string const &salt, std::array<char, 32> const &pk,
		     std::array<char, 64> const &sk, std::string str)
{

  e = str;
  std::vector<char> buf;
  lt::bencode (std::back_inserter (buf), e);
  lt::dht::signature sign;
  ++seq;
  sign = lt::dht::sign_mutable_item (buf, salt, lt::dht::sequence_number (seq),
				     lt::dht::public_key (pk.data ()),
				     lt::dht::secret_key (sk.data ()));
  sig = sign.bytes;
}

void
AuxFunc::toutf8 (std::string &line)
{
  const std::string::size_type srclen = line.size ();
  std::vector<UChar> target (srclen);
  UErrorCode status = U_ZERO_ERROR;
  UConverter *conv = ucnv_open (NULL, &status);
  int32_t len = ucnv_toUChars (conv, target.data (), srclen, line.c_str (),
			       srclen, &status);
  if (!U_SUCCESS (status))
    {
      std::cerr << u_errorName (status) << std::endl;
    }
  icu::UnicodeString ustr (target.data (), len);
  line.clear ();
  ustr.toUTF8String (line);
  ucnv_close (conv);
}

std::string
AuxFunc::utf8to (std::string line)
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr;
  UConverter *c = ucnv_open (NULL, &status);
  if (!U_SUCCESS (status))
    {
      std::cerr << u_errorName (status) << std::endl;
    }
  status = U_ZERO_ERROR;
  std::vector<char> target2;
  ustr.remove ();
  ustr = icu::UnicodeString::fromUTF8 (line.c_str ());
  target2.resize (ustr.length ());
  char16_t data[ustr.length ()];
  for (int i = 0; i < ustr.length (); i++)
    {
      data[i] = ustr.charAt (i);
    }
  size_t cb = ucnv_fromUChars (c, target2.data (), ustr.length (), data,
			       ustr.length (), &status);
  if (!U_SUCCESS (status))
    {
      std::cerr << u_errorName (status) << std::endl;
    }
  if (status == U_BUFFER_OVERFLOW_ERROR)
    {
      status = U_ZERO_ERROR;
      target2.clear ();
      target2.resize (cb);
      ucnv_fromUChars (c, target2.data (), cb, data, ustr.length (), &status);
      if (!U_SUCCESS (status))
	{
	  std::cerr << u_errorName (status) << std::endl;
	}
    }
  line.clear ();
  line = std::string (target2.begin (), target2.end ());
  ucnv_close (c);

  return line;
}

std::string
AuxFunc::stringToLower (std::string line)
{
  std::string innerline = line;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8 (innerline.c_str ());
  ustr.toLower ();
  line.clear ();
  ustr.toUTF8String (line);
  return line;
}

std::vector<char>
AuxFunc::filehash (std::filesystem::path filepath)
{
  if (std::filesystem::exists (filepath))
    {
      int fsz = std::filesystem::file_size (filepath);
      std::fstream f;
      std::vector<char> F;
      int readb = 0;
      gcry_error_t err;
      gcry_md_hd_t hd;
      err = gcry_md_open (&hd, GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE);
      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      f.open (filepath, std::ios_base::in | std::ios_base::binary);
      for (;;)
	{
	  if (readb + 50 < fsz)
	    {
	      F.resize (50);
	      f.read (&F[0], 50);
	      readb = readb + 50;
	      gcry_md_write (hd, &F[0], F.size ());
	    }
	  else
	    {
	      int left = fsz - readb;
	      F.resize (left);
	      f.read (&F[0], left);
	      readb = readb + left;
	      gcry_md_write (hd, &F[0], F.size ());
	    }
	  if (readb >= fsz)
	    {
	      break;
	    }
	}
      f.close ();
      size_t len = gcry_md_get_algo_dlen (GCRY_MD_SHA256);
      char *buf = reinterpret_cast<char*> (gcry_md_read (hd, GCRY_MD_SHA256));
      std::vector<char> result;
      result.insert (result.begin (), buf, buf + len);
      gcry_md_close (hd);
      return result;
    }
  else
    {
      std::cerr << "File for hashing not exists" << std::endl;
      std::vector<char> result
	{ 'e', 'r', 'r', 'o', 'r' };
      return result;
    }
}

std::vector<char>
AuxFunc::strhash (std::vector<char> &th, int type)
{
  std::vector<char> hv (th.begin (), th.end ());
  gcry_error_t err;
  gcry_md_hd_t hd;
  std::vector<char> result;
  if (type == 1)
    {
      err = gcry_md_open (&hd, GCRY_MD_BLAKE2S_128, GCRY_MD_FLAG_SECURE);

      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      gcry_md_write (hd, hv.data (), hv.size ());
      size_t len = gcry_md_get_algo_dlen (GCRY_MD_BLAKE2S_128);
      char *buf = reinterpret_cast<char*> (gcry_md_read (hd,
							 GCRY_MD_BLAKE2S_128));
      result.insert (result.begin (), buf, buf + len);
      gcry_md_close (hd);
    }
  if (type == 2)
    {
      err = gcry_md_open (&hd, GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE);

      if (err != 0)
	{
	  std::cerr << gcry_strerror (err) << std::endl;
	}
      size_t len = gcry_md_get_algo_dlen (GCRY_MD_SHA256);
      gcry_md_write (hd, hv.data (), hv.size ());
      char *buf = reinterpret_cast<char*> (gcry_md_read (hd, GCRY_MD_SHA256));
      result.insert (result.begin (), buf, buf + len);
      gcry_md_close (hd);
    }

  return result;
}

std::string
AuxFunc::string_to_hex (const std::string &input)
{
  static const char hex_digits[] = "0123456789ABCDEF";

  std::string output;
  output.reserve (input.length () * 2);
  for (unsigned char c : input)
    {
      output.push_back (hex_digits[c >> 4]);
      output.push_back (hex_digits[c & 15]);
    }
  return output;
}

int
AuxFunc::hex_value (unsigned char hex_digit)
{
  static const signed char hex_values[256] =
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, };
  int value = hex_values[hex_digit];
  if (value == -1)
    {
      throw std::invalid_argument ("invalid hex digit");
    }
  return value;
}

std::string
AuxFunc::hex_to_string (const std::string &input)
{
  const auto len = input.length ();
  if (len & 1)
    {
      throw std::invalid_argument ("odd length");
    }

  std::string output;
  output.reserve (len / 2);
  for (auto it = input.begin (); it != input.end ();)
    {
      int hi = hex_value (*it++);
      int lo = hex_value (*it++);
      output.push_back (hi << 4 | lo);
    }
  return output;
}
