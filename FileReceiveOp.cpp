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

#include "FileReceiveOp.h"

FileReceiveOp::FileReceiveOp (NetworkOperations *No)
{
  no = No;
}

FileReceiveOp::~FileReceiveOp ()
{
  // TODO Auto-generated destructor stub
}

void
FileReceiveOp::fileProcessing (std::string msgtype, std::string key,
			       int ip6check, int sockipv, sockaddr_in *from,
			       sockaddr_in6 *from6)
{
  std::string chkey = key;
  int rcvip6 = ip6check;
  AuxFunc af;
  no->contmtx.lock ();
  auto contit = std::find_if (no->contacts.begin (), no->contacts.end (),
			      [&chkey]
			      (auto &el)
				{
				  return std::get<1>(el) == chkey;
				});
  if (contit != no->contacts.end ())
    {
      std::string key = std::get<1> (*contit);
      std::string index;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << std::get<0> (*contit);
      index = strm.str ();
      if (msgtype == "Fp")
	{
	  no->filepartrcvmtx.lock ();
	  for (;;)
	    {
	      auto itfpr = std::find_if (no->filepartrcv.begin (),
					 no->filepartrcv.end (), [&key]
					 (auto &el)
					   {
					     return std::get<0>(el) == key;
					   });
	      if (itfpr != no->filepartrcv.end ())
		{
		  uint64_t tint = std::get<1> (*itfpr);
		  uint64_t rpnum = std::get<2> (*itfpr);
		  no->filepartrlogmtx.lock ();
		  auto itfprl = std::find_if (
		      no->filepartrlog.begin (), no->filepartrlog.end (),
		      [&key, &tint, &rpnum]
		      (auto &el)
			{
			  if (std::get<0>(el) == key && std::get<1>(el) == tint
			      && int (rpnum) == std::get<2>(el) + 1)
			    {
			      return true;
			    }
			  else
			    {
			      return false;
			    }
			});
		  if (itfprl != no->filepartrlog.end ())
		    {
		      no->currentpartmtx.lock ();
		      auto itcpv =
			  std::find_if (
			      no->currentpart.begin (),
			      no->currentpart.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == tint)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				});
		      std::vector<char> part = std::get<3> (*itfpr);
		      if (itcpv == no->currentpart.end ())
			{
			  no->currentpart.push_back (
			      std::make_tuple (key, tint, part));
			}
		      else
			{
			  std::vector<char> tv = std::get<2> (*itcpv);
			  std::copy (part.begin (), part.end (),
				     std::back_inserter (tv));
			  std::get<2> (*itcpv) = tv;
			}
		      no->currentpartmtx.unlock ();
		      no->filepartrcv.erase (itfpr);
		      std::get<2> (*itfprl) = std::get<2> (*itfprl) + 1;
		    }
		  else
		    {
		      no->filepartrlogmtx.unlock ();
		      break;
		    }
		  no->filepartrlogmtx.unlock ();
		}
	      else
		{
		  break;
		}
	    }
	  no->filepartrcvmtx.unlock ();
	}

      if (msgtype == "Fe")
	{
	  no->filepartendmtx.lock ();
	  for (;;)
	    {
	      auto itfpe = std::find_if (no->filepartend.begin (),
					 no->filepartend.end (), [&key]
					 (auto &el)
					   {
					     return std::get<0>(el) == key;
					   });
	      if (itfpe == no->filepartend.end ())
		{
		  break;
		}
	      else
		{
		  uint64_t tint = std::get<1> (*itfpe);
		  no->fileparthashmtx.lock ();
		  auto itfph = std::find_if (
		      no->fileparthash.begin (), no->fileparthash.end (),
		      [&key, &tint]
		      (auto &el)
			{
			  time_t tm = tint;
			  if (std::get<0>(el) == key && std::get<1>(el) == tm)
			    {
			      return true;
			    }
			  else
			    {
			      return false;
			    }
			});
		  if (itfph == no->fileparthash.end ())
		    {
		      no->filepartend.erase (itfpe);
		      no->fileparthashmtx.unlock ();
		      break;
		    }
		  else
		    {
		      std::vector<char> hash = std::get<3> (*itfph);
		      no->currentpartmtx.lock ();
		      auto itcp =
			  std::find_if (
			      no->currentpart.begin (),
			      no->currentpart.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if(std::get<0>(el) == key && std::get<1>(el) == tint)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				});
		      if (itcp == no->currentpart.end ())
			{
			  no->filepartend.erase (itfpe);
			  no->fileparthash.erase (itfph);
			  no->currentpartmtx.unlock ();
			  no->fileparthashmtx.unlock ();
			  break;
			}
		      else
			{
			  std::vector<char> part = std::get<2> (*itcp);
			  std::vector<char> chhash;
			  chhash = af.strhash (part, 2);
			  if (chhash == hash)
			    {
			      no->filehashvectmtx.lock ();
			      auto itfhv =
				  std::find_if (
				      no->filehashvect.begin (),
				      no->filehashvect.end (),
				      [&key, &tint]
				      (auto &el)
					{
					  time_t tm = tint;
					  if (std::get<0>(el) == key && std::get<1>(el) == tm)
					    {
					      return true;
					    }
					  else
					    {
					      return false;
					    }
					});
			      if (itfhv == no->filehashvect.end ())
				{
				  no->filepartend.erase (itfpe);
				  no->fileparthash.erase (itfph);
				  no->currentpart.erase (itcp);
				  no->currentpartmtx.unlock ();
				  no->fileparthashmtx.unlock ();
				  no->filehashvectmtx.unlock ();
				  break;
				}
			      else
				{
				  if (int (std::get<2> (*itfph))
				      > std::get<4> (*itfhv))
				    {
				      std::filesystem::path p = std::get<3> (
					  *itfhv);
				      std::fstream f;
				      f.open (
					  p,
					  std::ios_base::out
					      | std::ios_base::app
					      | std::ios_base::binary);
				      f.write (&part[0], part.size ());
				      f.close ();
				      uint64_t fcsz =
					  std::filesystem::file_size (p);
				      if (no->filepartrcvdsig)
					{
					  no->filepartrcvdsig (key, p, fcsz);
					}
				      std::get<4> (*itfhv) = std::get<4> (
					  *itfhv) + 1;
				    }

				  std::vector<char> msg;
				  std::tuple<lt::dht::public_key,
				      lt::dht::secret_key> okp;
				  okp = lt::dht::ed25519_create_keypair (
				      *(no->seed));
				  std::array<char, 32> okarr =
				      std::get<0> (okp).bytes;
				  std::copy (okarr.begin (), okarr.end (),
					     std::back_inserter (msg));
				  std::string mtype = "Fr";
				  std::copy (mtype.begin (), mtype.end (),
					     std::back_inserter (msg));
				  msg.resize (msg.size () + sizeof(tint));
				  std::memcpy (&msg[34], &tint, sizeof(tint));
				  uint64_t numb = std::get<2> (*itfph);
				  msg.resize (msg.size () + sizeof(numb));
				  std::memcpy (&msg[42], &numb, sizeof(numb));
				  std::string uname = key;
				  lt::dht::public_key othpk;
				  lt::aux::from_hex (key, othpk.bytes.data ());
				  std::array<char, 32> scalar;
				  scalar = lt::dht::ed25519_key_exchange (
				      othpk, std::get<1> (okp));
				  othpk = lt::dht::ed25519_add_scalar (othpk,
								       scalar);
				  std::string passwd = lt::aux::to_hex (
				      othpk.bytes);
				  msg = af.cryptStrm (uname, passwd, msg);
				  if (rcvip6 == 0)
				    {
				      no->sendMsg (sockipv,
						   from->sin_addr.s_addr,
						   from->sin_port, msg);
				    }
				  else
				    {
				      std::vector<char> ip6ad;
				      ip6ad.resize (INET6_ADDRSTRLEN);
				      std::string ip6 = inet_ntop (
					  AF_INET6, &from6->sin6_addr,
					  ip6ad.data (), ip6ad.size ());
				      no->sockipv6mtx.lock ();
				      no->sendMsg6 (sockipv, ip6,
						    from6->sin6_port, msg);
				      no->sockipv6mtx.unlock ();
				    }
				  no->filepartend.erase (itfpe);
				  no->fileparthash.erase (itfph);
				  no->currentpart.erase (itcp);
				}
			      no->filehashvectmtx.unlock ();
			    }
			  else
			    {
			      std::cerr << "File part hash error!" << std::endl;
			      no->filepartend.erase (itfpe);
			      no->fileparthash.erase (itfph);
			      no->currentpart.erase (itcp);
			    }
			}
		      no->currentpartmtx.unlock ();
		    }
		  no->fileparthashmtx.unlock ();
		}
	    }
	  no->filepartendmtx.unlock ();
	}

      if (msgtype == "FE")
	{
	  no->fileendmtx.lock ();
	  for (;;)
	    {
	      auto itfev = std::find_if (no->fileend.begin (),
					 no->fileend.end (), [&key]
					 (auto &el)
					   {
					     return std::get<0>(el) == key;
					   });
	      if (itfev == no->fileend.end ())
		{
		  break;
		}
	      else
		{
		  uint64_t tint = std::get<1> (*itfev);
		  no->filehashvectmtx.lock ();
		  auto itfhv = std::find_if (
		      no->filehashvect.begin (), no->filehashvect.end (),
		      [&key, &tint]
		      (auto &el)
			{
			  time_t tm = tint;
			  if (std::get<0>(el) == key && std::get<1>(el) == tm)
			    {
			      return true;
			    }
			  else
			    {
			      return false;
			    }
			});
		  if (itfhv == no->filehashvect.end ())
		    {
		      std::vector<char> msg;
		      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		      okp = lt::dht::ed25519_create_keypair (*(no->seed));
		      std::array<char, 32> okarr = std::get<0> (okp).bytes;
		      std::copy (okarr.begin (), okarr.end (),
				 std::back_inserter (msg));
		      std::string mtype = "FR";
		      std::copy (mtype.begin (), mtype.end (),
				 std::back_inserter (msg));
		      msg.resize (msg.size () + sizeof(tint));
		      std::memcpy (&msg[34], &tint, sizeof(tint));
		      uint64_t numb = 0;
		      msg.resize (msg.size () + sizeof(numb));
		      std::memcpy (&msg[42], &numb, sizeof(numb));
		      std::string uname = key;
		      lt::dht::public_key othpk;
		      lt::aux::from_hex (key, othpk.bytes.data ());
		      std::array<char, 32> scalar;
		      scalar = lt::dht::ed25519_key_exchange (
			  othpk, std::get<1> (okp));
		      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		      std::string passwd = lt::aux::to_hex (othpk.bytes);
		      msg = af.cryptStrm (uname, passwd, msg);
		      if (rcvip6 == 0)
			{
			  no->sendMsg (sockipv, from->sin_addr.s_addr,
				       from->sin_port, msg);
			}
		      else
			{
			  std::vector<char> ip6ad;
			  ip6ad.resize (INET6_ADDRSTRLEN);
			  std::string ip6 = inet_ntop (AF_INET6,
						       &from6->sin6_addr,
						       ip6ad.data (),
						       ip6ad.size ());
			  no->sockipv6mtx.lock ();
			  no->sendMsg6 (sockipv, ip6, from6->sin6_port, msg);
			  no->sockipv6mtx.unlock ();
			}
		      no->fileend.erase (itfev);
		      no->filehashvectmtx.unlock ();

		      no->fqrcvdmtx.lock ();
		      no->fqrcvd.erase (
			  std::remove_if (
			      no->fqrcvd.begin (),
			      no->fqrcvd.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == time_t(tint))
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->fqrcvd.end ());
		      no->fqrcvdmtx.unlock ();
		      break;
		    }
		  else
		    {
		      std::vector<char> hash = std::get<2> (*itfhv);
		      std::filesystem::path p = std::get<3> (*itfhv);
		      std::vector<char> chhash = af.filehash (p);
		      if (hash == chhash)
			{
			  std::vector<char> msg;
			  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
			  okp = lt::dht::ed25519_create_keypair (*(no->seed));
			  std::array<char, 32> okarr = std::get<0> (okp).bytes;
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
			  std::string mtype = "FR";
			  std::copy (mtype.begin (), mtype.end (),
				     std::back_inserter (msg));
			  msg.resize (msg.size () + sizeof(tint));
			  std::memcpy (&msg[34], &tint, sizeof(tint));
			  uint64_t numb = 0;
			  msg.resize (msg.size () + sizeof(numb));
			  std::memcpy (&msg[42], &numb, sizeof(numb));
			  std::string uname = key;
			  lt::dht::public_key othpk;
			  lt::aux::from_hex (key, othpk.bytes.data ());
			  std::array<char, 32> scalar;
			  scalar = lt::dht::ed25519_key_exchange (
			      othpk, std::get<1> (okp));
			  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
			  std::string passwd = lt::aux::to_hex (othpk.bytes);
			  msg = af.cryptStrm (uname, passwd, msg);
			  if (rcvip6 == 0)
			    {
			      no->sendMsg (sockipv, from->sin_addr.s_addr,
					   from->sin_port, msg);
			    }
			  else
			    {
			      std::vector<char> ip6ad;
			      ip6ad.resize (INET6_ADDRSTRLEN);
			      std::string ip6 = inet_ntop (AF_INET6,
							   &from6->sin6_addr,
							   ip6ad.data (),
							   ip6ad.size ());
			      no->sockipv6mtx.lock ();
			      no->sendMsg6 (sockipv, ip6, from6->sin6_port,
					    msg);
			      no->sockipv6mtx.unlock ();
			    }
			  if (no->filercvd)
			    {
			      no->filercvd (key, p.filename ().u8string ());
			    }
			  std::string filename;
			  time_t curtime = time (NULL);
			  strm.clear ();
			  strm.str ("");
			  strm.imbue (loc);
			  strm << curtime;
#ifdef __linux
			  filename =
			      std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  			      filename =
  				  std::filesystem::temp_directory_path ()
  				  .parent_path ().u8string ();
#endif
			  filename = filename + "/" + strm.str ();
			  std::filesystem::path filepath =
			      std::filesystem::u8path (filename);
			  std::fstream f;
			  std::string line;
			  f.open (filepath,
				  std::ios_base::out | std::ios_base::binary);
			  line = key + "\n";
			  f.write (line.c_str (), line.size ());
			  line = lt::aux::to_hex (std::get<0> (okp).bytes)
			      + "\n";
			  f.write (line.c_str (), line.size ());
			  time_t ctmutc = curtime;
			  strm.clear ();
			  strm.str ("");
			  strm.imbue (loc);
			  strm << ctmutc;
			  line = strm.str () + "\n";
			  f.write (line.c_str (), line.size ());
			  line = "1\n";
			  f.write (line.c_str (), line.size ());
			  line = "r\n";
			  f.write (line.c_str (), line.size ());
			  line = p.u8string ();
			  f.write (line.c_str (), line.size ());
			  f.close ();
			  lt::aux::from_hex (key, othpk.bytes.data ());
			  scalar = lt::dht::ed25519_key_exchange (
			      othpk, std::get<1> (okp));
			  othpk = lt::dht::ed25519_add_scalar (
			      std::get<0> (okp), scalar);
			  std::string unm = lt::aux::to_hex (
			      std::get<0> (okp).bytes);
			  passwd = lt::aux::to_hex (othpk.bytes);
			  af.homePath (&filename);
			  filename = filename + "/.Communist/" + index;
			  std::filesystem::path outpath;
			  outpath = std::filesystem::u8path (filename);
			  std::vector<int> fnmv;
			  for (auto &dit : std::filesystem::directory_iterator (
			      outpath))
			    {
			      std::filesystem::path tp = dit.path ();
			      if (tp.filename ().u8string () != "Yes"
				  && tp.filename ().u8string () != "Profile")
				{
				  filename = tp.filename ().u8string ();
				  std::string::size_type n;
				  n = filename.find ("f");
				  if (n != std::string::npos)
				    {
				      filename.erase (
					  n, n + std::string ("f").size ());
				    }
				  strm.clear ();
				  strm.str ("");
				  strm.imbue (loc);
				  strm << filename;
				  int fnmi;
				  strm >> fnmi;
				  fnmv.push_back (fnmi);
				}
			    }
			  std::string fnm;
			  if (fnmv.size () > 0)
			    {
			      std::sort (fnmv.begin (), fnmv.end ());
			      int fnmi = fnmv[fnmv.size () - 1];
			      fnmi = fnmi + 1;
			      strm.clear ();
			      strm.str ("");
			      strm.imbue (loc);
			      strm << fnmi;
			      fnm = strm.str () + "f";
			    }
			  else
			    {
			      fnm = "0f";
			    }
			  af.homePath (&filename);
			  filename = filename + "/.Communist/" + index + "/"
			      + fnm;
			  outpath = std::filesystem::u8path (filename);
			  if (!std::filesystem::exists (outpath.parent_path ()))
			    {
			      std::filesystem::create_directories (
				  outpath.parent_path ());
			    }
			  af.cryptFile (unm, passwd, filepath.u8string (),
					outpath.u8string ());
			  std::filesystem::remove (filepath);

			  no->fqrcvdmtx.lock ();
			  no->fqrcvd.erase (
			      std::remove_if (
				  no->fqrcvd.begin (),
				  no->fqrcvd.end (),
				  [&key, &tint]
				  (auto &el)
				    {
				      if (std::get<0>(el) == key && std::get<1>(el) == time_t(tint))
					{
					  return true;
					}
				      else
					{
					  return false;
					}
				    }),
			      no->fqrcvd.end ());
			  no->fqrcvdmtx.unlock ();
			  if (no->messageReceived)
			    {
			      no->messageReceived (key, outpath);
			    }
			}
		      else
			{
			  std::filesystem::remove (p);
			  std::vector<char> msg;
			  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
			  okp = lt::dht::ed25519_create_keypair (*(no->seed));
			  std::array<char, 32> okarr = std::get<0> (okp).bytes;
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
			  std::string mtype = "FI";
			  std::copy (mtype.begin (), mtype.end (),
				     std::back_inserter (msg));
			  msg.resize (msg.size () + sizeof(tint));
			  std::memcpy (&msg[34], &tint, sizeof(tint));
			  uint64_t numb = 0;
			  msg.resize (msg.size () + sizeof(numb));
			  std::memcpy (&msg[42], &numb, sizeof(numb));
			  std::string uname = key;
			  lt::dht::public_key othpk;
			  lt::aux::from_hex (key, othpk.bytes.data ());
			  std::array<char, 32> scalar;
			  scalar = lt::dht::ed25519_key_exchange (
			      othpk, std::get<1> (okp));
			  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
			  std::string passwd = lt::aux::to_hex (othpk.bytes);
			  msg = af.cryptStrm (uname, passwd, msg);
			  if (rcvip6 == 0)
			    {
			      no->sendMsg (sockipv, from->sin_addr.s_addr,
					   from->sin_port, msg);
			    }
			  else
			    {
			      std::vector<char> ip6ad;
			      ip6ad.resize (INET6_ADDRSTRLEN);
			      std::string ip6 = inet_ntop (AF_INET6,
							   &from6->sin6_addr,
							   ip6ad.data (),
							   ip6ad.size ());
			      no->sockipv6mtx.lock ();
			      no->sendMsg6 (sockipv, ip6, from6->sin6_port,
					    msg);
			      no->sockipv6mtx.unlock ();
			    }
			  if (no->filehasherr)
			    {
			      no->filehasherr (key, p.filename ().u8string ());
			    }
			}
		      no->filehashvect.erase (itfhv);

		      no->filepartrcvmtx.lock ();
		      no->filepartrcv.erase (
			  std::remove_if (
			      no->filepartrcv.begin (),
			      no->filepartrcv.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == tint)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->filepartrcv.end ());
		      no->filepartrcvmtx.unlock ();

		      no->fileparthashmtx.lock ();
		      no->fileparthash.erase (
			  std::remove_if (
			      no->fileparthash.begin (),
			      no->fileparthash.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  time_t tm = tint;
				  if (std::get<0>(el) == key && std::get<1>(el) == tm)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->fileparthash.end ());
		      no->fileparthashmtx.unlock ();

		      no->filepartrlogmtx.lock ();
		      no->filepartrlog.erase (
			  std::remove_if (
			      no->filepartrlog.begin (),
			      no->filepartrlog.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == tint)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->filepartrlog.end ());
		      no->filepartrlogmtx.unlock ();

		      no->currentpartmtx.lock ();
		      no->currentpart.erase (
			  std::remove_if (
			      no->currentpart.begin (),
			      no->currentpart.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == tint)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->currentpart.end ());
		      no->currentpartmtx.unlock ();

		      no->filepartendmtx.lock ();
		      no->filepartend.erase (
			  std::remove_if (
			      no->filepartend.begin (),
			      no->filepartend.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == tint)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->filepartend.end ());
		      no->filepartendmtx.unlock ();

		      no->fqrcvdmtx.lock ();
		      no->fqrcvd.erase (
			  std::remove_if (
			      no->fqrcvd.begin (),
			      no->fqrcvd.end (),
			      [&key, &tint]
			      (auto &el)
				{
				  if (std::get<0>(el) == key && std::get<1>(el) == time_t(tint))
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				}),
			  no->fqrcvd.end ());
		      no->fqrcvdmtx.unlock ();

		      no->fileend.erase (itfev);
		    }
		  no->filehashvectmtx.unlock ();
		}
	    }
	  no->fileendmtx.unlock ();
	}
    }
  no->contmtx.unlock ();
}

void
FileReceiveOp::fileFQ (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  AuxFunc af;
  std::cout << msgtype << std::endl;
  uint64_t time;
  std::memcpy (&time, &buf[34], sizeof(time));
  uint64_t fsize;
  std::memcpy (&fsize, &buf[42], sizeof(fsize));
  std::vector<char> fnmv;
  std::copy (buf.begin () + 50, buf.end (), std::back_inserter (fnmv));
  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
  okp = lt::dht::ed25519_create_keypair (*(no->seed));
  lt::dht::public_key othk;
  lt::aux::from_hex (key, othk.bytes.data ());
  std::string unm = lt::aux::to_hex (std::get<0> (okp).bytes);
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (othk, std::get<1> (okp));
  othk = lt::dht::ed25519_add_scalar (std::get<0> (okp), scalar);
  std::string passwd = lt::aux::to_hex (othk.bytes);
  fnmv = af.decryptStrm (unm, passwd, fnmv);
  std::string fnm (fnmv.begin (), fnmv.end ());
  std::string::size_type n;
  n = fnm.find ("\\");
  if (n != std::string::npos)
    {
      fnm.erase (n, std::string::npos);
    }
  no->fqrcvdmtx.lock ();
  std::tuple<std::string, time_t, std::string> fqtup;
  fqtup = std::make_tuple (key, time, fnm);
  auto itfqrcvd = std::find (no->fqrcvd.begin (), no->fqrcvd.end (), fqtup);
  if (itfqrcvd == no->fqrcvd.end ())
    {
      std::cout << key << " " << time << " " << fsize << " " << fnm
	  << std::endl;
      no->fqrcvd.push_back (fqtup);
      if (no->filerequest)
	{
	  no->filerequest (key, time, fsize, fnm);
	}
    }
  no->fqrcvdmtx.unlock ();
}

void
FileReceiveOp::fileFJ (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  no->sendbufmtx.lock ();
  no->filesendreqmtx.lock ();
  auto itfsr = std::find_if (
      no->filesendreq.begin (), no->filesendreq.end (), [key, &tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<2>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfsr != no->filesendreq.end ())
    {
      std::filesystem::path p = std::get<4> (*itfsr);
      if (std::filesystem::exists (p))
	{
	  std::filesystem::remove (p);
	}
      std::string pstr = p.filename ().u8string ();
      pstr = pstr.substr (0, pstr.find ("f"));
      int msgind;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << pstr;
      strm >> msgind;
      if (no->msgSent)
	{
	  no->msgSent (key, msgind);
	}
      if (no->fileRejected)
	{
	  no->fileRejected (key);
	}
      no->filesendreq.erase (itfsr);

    }
  no->filesendreqmtx.unlock ();
  no->sendbufmtx.unlock ();
}

void
FileReceiveOp::fileFA (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  no->filesendreqmtx.lock ();
  auto itfsr = std::find_if (
      no->filesendreq.begin (), no->filesendreq.end (), [key, &tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<2>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfsr != no->filesendreq.end ())
    {
      std::filesystem::path p = std::get<1> (*itfsr);
      std::vector<char> frbuf;
      no->filepartbufmtx.lock ();
      auto it = std::find_if (
	  no->filepartbuf.begin (), no->filepartbuf.end (), [key, &tm]
	  (auto &el)
	    {
	      if (std::get<0>(el) == key && std::get<1>(el) == tm)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    });
      if (it == no->filepartbuf.end ())
	{
	  no->filepartbuf.push_back (
	      std::make_tuple (key, tm, p, 0, 0, frbuf, 1));
	}
      no->filepartbufmtx.unlock ();
      std::get<3> (*itfsr) = 1;
    }
  no->filesendreqmtx.unlock ();
}

void
FileReceiveOp::fileFr (std::string msgtype, std::string key, int rcvip6,
		       sockaddr_in6 *from6, sockaddr_in *from, int sockipv4,
		       std::vector<char> &buf)
{
  AuxFunc af;
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*(no->seed));
  std::string unm = lt::aux::to_hex (std::get<0> (ownkey).bytes);
  lt::dht::public_key othpk;
  lt::aux::from_hex (key, othpk.bytes.data ());
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (ownkey));
  othpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  uint64_t partnum;
  std::memcpy (&partnum, &buf[42], sizeof(partnum));
  no->filepartbufmtx.lock ();
  auto itfpb = std::find_if (
      no->filepartbuf.begin (), no->filepartbuf.end (), [key, &tm, &partnum]
      (auto &el)
	{
	  if (std::get<0>(el) == key &&
	      std::get<1>(el) == tm && std::get<4>(el) == partnum)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfpb != no->filepartbuf.end ())
    {
      std::get<6> (*itfpb) = 1;
      std::filesystem::path sentp = std::get<2> (*itfpb);
      if (int (std::filesystem::file_size (sentp)) == std::get<3> (*itfpb))
	{
	  std::vector<char> msg;
	  std::array<char, 32> okarr = std::get<0> (ownkey).bytes;
	  std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
	  std::string msgtype = "FE";
	  std::copy (msgtype.begin (), msgtype.end (),
		     std::back_inserter (msg));
	  uint64_t tmfb = std::get<1> (*itfpb);
	  msg.resize (msg.size () + sizeof(tmfb));
	  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
	  uint64_t msgn = 0;
	  msg.resize (msg.size () + sizeof(msgn));
	  std::memcpy (&msg[42], &msgn, sizeof(msgn));
	  lt::aux::from_hex (key, othpk.bytes.data ());
	  std::string unms, passwds;
	  unms = key;
	  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
	  passwds = lt::aux::to_hex (othpk.bytes);
	  msg = af.cryptStrm (unms, passwds, msg);
	  std::vector<char> tmpmsg;
	  if (rcvip6 > 0)
	    {
	      tmpmsg.resize (INET6_ADDRSTRLEN);
	      std::string sentadr = inet_ntop (AF_INET6, &from6->sin6_addr,
					       tmpmsg.data (), tmpmsg.size ());
	      no->sockipv6mtx.lock ();
	      no->sendMsg6 (no->sockipv6, sentadr, from6->sin6_port, msg);
	      no->sockipv6mtx.unlock ();
	    }
	  else
	    {
	      no->sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			   msg);
	    }
	}
      else
	{
	  if (no->filepartsendsig)
	    {
	      no->filepartsendsig (key, std::get<2> (*itfpb),
				   uint64_t (std::get<3> (*itfpb)));
	    }
	}
    }
  no->filepartbufmtx.unlock ();
}
void
FileReceiveOp::fileFRFI (std::string msgtype, std::string key,
			 std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  no->sendbufmtx.lock ();
  no->filesendreqmtx.lock ();
  auto itfsr = std::find_if (
      no->filesendreq.begin (), no->filesendreq.end (), [key, &tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<2>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfsr != no->filesendreq.end ())
    {
      std::filesystem::path p = std::get<4> (*itfsr);
      std::string pstr = p.filename ().u8string ();
      pstr = pstr.substr (0, pstr.find ("f"));
      int msgind;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << pstr;
      strm >> msgind;
      std::filesystem::remove (p);
      if (no->msgSent)
	{
	  no->msgSent (key, msgind);
	}
      no->filesendreq.erase (itfsr);
    }
  no->filesendreqmtx.unlock ();
  no->sendbufmtx.unlock ();

  no->filepartbufmtx.lock ();
  auto itfpb = std::find_if (
      no->filepartbuf.begin (), no->filepartbuf.end (), [key, &tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfpb != no->filepartbuf.end ())
    {
      if (msgtype == "FR")
	{
	  no->filecanceledmtx.lock ();
	  no->filecanceled.erase (
	      std::remove_if (
		  no->filecanceled.begin (), no->filecanceled.end (), [key, &tm]
		  (auto &el)
		    {
		      if (std::get<0>(el) == key && std::get<1>(el) == tm)
			{
			  return true;
			}
		      else
			{
			  return false;
			}
		    }),
	      no->filecanceled.end ());
	  no->filecanceledmtx.unlock ();
	  if (no->filesentsig)
	    {
	      no->filesentsig (key,
			       std::get<2> (*itfpb).filename ().u8string ());
	    }
	}
      if (msgtype == "FI")
	{
	  no->filecanceledmtx.lock ();
	  no->filecanceled.erase (
	      std::remove_if (
		  no->filecanceled.begin (), no->filecanceled.end (), [key, &tm]
		  (auto &el)
		    {
		      if (std::get<0>(el) == key && std::get<1>(el) == tm)
			{
			  return true;
			}
		      else
			{
			  return false;
			}
		    }),
	      no->filecanceled.end ());
	  no->filecanceledmtx.unlock ();
	  std::string toemit = std::get<2> (*itfpb).filename ().u8string ();
	  if (no->filesenterror)
	    {
	      no->filesenterror (key, toemit);
	    }
	}
      no->filepartbuf.erase (itfpb);
    }
  no->filepartbufmtx.unlock ();
}

void
FileReceiveOp::fileFB (std::string msgtype, std::string key, int rcvip6,
		       sockaddr_in6 *from6, sockaddr_in *from, int sockipv4,
		       std::vector<char> &buf)
{
  AuxFunc af;
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  no->filehashvectmtx.lock ();
  auto itfhv = std::find_if (
      no->filehashvect.begin (), no->filehashvect.end (), [key, &tm]
      (auto &el)
	{
	  if(std::get<0>(el) == key && std::get<1>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfhv != no->filehashvect.end ())
    {
      std::vector<char> fh;
      std::copy (buf.begin () + 50, buf.end (), std::back_inserter (fh));
      std::get<2> (*itfhv) = fh;
      std::array<char, 32> okarr;
      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
      okp = lt::dht::ed25519_create_keypair (*(no->seed));
      okarr = std::get<0> (okp).bytes;
      std::vector<char> msg;
      std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
      std::string type = "FH";
      std::copy (type.begin (), type.end (), std::back_inserter (msg));
      uint64_t lct = uint64_t (tm);
      msg.resize (msg.size () + sizeof(lct));
      std::memcpy (&msg[34], &lct, sizeof(lct));
      uint64_t numb = 0;
      msg.resize (msg.size () + sizeof(numb));
      std::memcpy (&msg[42], &numb, sizeof(numb));
      std::string unm = key;
      lt::dht::public_key passkey;
      lt::aux::from_hex (unm, passkey.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (passkey, std::get<1> (okp));
      passkey = lt::dht::ed25519_add_scalar (passkey, scalar);
      std::string passwd = lt::aux::to_hex (passkey.bytes);
      msg = af.cryptStrm (unm, passwd, msg);
      std::vector<char> tmpmsg;
      if (rcvip6 > 0)
	{
	  tmpmsg.resize (INET6_ADDRSTRLEN);
	  std::string ip6 = inet_ntop (AF_INET6, &from6->sin6_addr,
				       tmpmsg.data (), tmpmsg.size ());
	  no->sendMsg6 (sockipv4, ip6, from6->sin6_port, msg);
	}
      else
	{
	  no->sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port, msg);
	}
    }
  no->filehashvectmtx.unlock ();
}

void
FileReceiveOp::fileFH (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  no->filepartbufmtx.lock ();
  auto itfpb = std::find_if (
      no->filepartbuf.begin (), no->filepartbuf.end (), [key, &tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfpb != no->filepartbuf.end ())
    {
      no->fbrvectmtx.lock ();
      auto itfbrv = std::find_if (
	  no->fbrvect.begin (), no->fbrvect.end (), [key, &tm]
	  (auto &el)
	    {
	      if (std::get<0>(el) == key && std::get<1>(el) == tm)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    });
      if (itfbrv == no->fbrvect.end ())
	{
	  no->fbrvect.push_back (std::make_tuple (key, tm));
	}
      no->fbrvectmtx.unlock ();
    }
  no->filepartbufmtx.unlock ();
}

void
FileReceiveOp::fileFb (std::string msgtype, std::string key, int rcvip6,
		       sockaddr_in6 *from6, sockaddr_in *from, int sockipv4,
		       std::vector<char> &buf)
{
  AuxFunc af;
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  time_t tm = tint;
  uint64_t partnum;
  std::memcpy (&partnum, &buf[42], sizeof(partnum));
  std::vector<char> hash;
  std::copy (buf.begin () + 50, buf.end (), std::back_inserter (hash));
  no->filehashvectmtx.lock ();
  auto itfhv = std::find_if (
      no->filehashvect.begin (), no->filehashvect.end (), [key, &tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == tm)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfhv != no->filehashvect.end ())
    {
      no->fileparthashmtx.lock ();
      no->fileparthash.erase (
	  std::remove_if (
	      no->fileparthash.begin (), no->fileparthash.end (), [key, &tm]
	      (auto &el)
		{
		  if (std::get<0>(el) == key && std::get<1>(el) == tm)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		}),
	  no->fileparthash.end ());
      no->fileparthash.push_back (std::make_tuple (key, tm, partnum, hash));
      no->fileparthashmtx.unlock ();

      no->filepartrlogmtx.lock ();
      no->filepartrlog.erase (
	  std::remove_if (
	      no->filepartrlog.begin (), no->filepartrlog.end (), [key, &tint]
	      (auto &el)
		{
		  if (std::get<0>(el) == key && std::get<1>(el) == tint)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		}),
	  no->filepartrlog.end ());
      no->filepartrlog.push_back (std::make_tuple (key, tint, -1));
      no->filepartrlogmtx.unlock ();

      no->filepartrcvmtx.lock ();
      no->filepartrcv.erase (
	  std::remove_if (
	      no->filepartrcv.begin (), no->filepartrcv.end (), [key, &tint]
	      (auto &el)
		{
		  if (std::get<0>(el) == key && std::get<1>(el) == tint)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		}),
	  no->filepartrcv.end ());
      no->filepartrcvmtx.unlock ();

      no->currentpartmtx.lock ();
      no->currentpart.erase (
	  std::remove_if (
	      no->currentpart.begin (), no->currentpart.end (), [key, &tint]
	      (auto &el)
		{
		  if (std::get<0>(el) == key && std::get<1>(el) == tint)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		}),
	  no->currentpart.end ());
      no->currentpartmtx.unlock ();

      no->filepartendmtx.lock ();
      no->filepartend.erase (
	  std::remove_if (
	      no->filepartend.begin (), no->filepartend.end (), [key, &tint]
	      (auto &el)
		{
		  if (std::get<0>(el) == key && std::get<1>(el) == tint)
		    {
		      return true;
		    }
		  else
		    {
		      return false;
		    }
		}),
	  no->filepartend.end ());
      no->filepartendmtx.unlock ();
    }
  else
    {
      std::string type;
      no->fqrcvdmtx.lock ();
      auto itfqrcv = std::find_if (
	  no->fqrcvd.begin (), no->fqrcvd.end (), [key, &tm]
	  (auto &el)
	    {
	      if (std::get<0>(el) == key && std::get<1>(el) == tm)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    });
      if (itfqrcv != no->fqrcvd.end ())
	{
	  type = "FF";
	}
      else
	{
	  type = "FI";
	}
      no->fqrcvdmtx.unlock ();
      std::array<char, 32> okarr;
      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
      okp = lt::dht::ed25519_create_keypair (*(no->seed));
      okarr = std::get<0> (okp).bytes;
      std::vector<char> msg;
      std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
      std::copy (type.begin (), type.end (), std::back_inserter (msg));
      uint64_t lct = uint64_t (tm);
      msg.resize (msg.size () + sizeof(lct));
      std::memcpy (&msg[34], &lct, sizeof(lct));
      std::string unm = key;
      lt::dht::public_key passkey;
      lt::aux::from_hex (unm, passkey.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (passkey, std::get<1> (okp));
      passkey = lt::dht::ed25519_add_scalar (passkey, scalar);
      std::string passwd = lt::aux::to_hex (passkey.bytes);
      msg = af.cryptStrm (unm, passwd, msg);
      std::vector<char> tmpmsg;
      if (rcvip6 > 0)
	{
	  tmpmsg.resize (INET6_ADDRSTRLEN);
	  std::string ip6 = inet_ntop (AF_INET6, &from6->sin6_addr,
				       tmpmsg.data (), tmpmsg.size ());
	  no->sendMsg6 (sockipv4, ip6, from6->sin6_port, msg);
	}
      else
	{
	  no->sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port, msg);
	}
    }
  no->filehashvectmtx.unlock ();
}

void
FileReceiveOp::fileFp (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  uint64_t numb;
  std::memcpy (&numb, &buf[42], sizeof(numb));
  std::vector<char> data;
  std::copy (buf.begin () + 50, buf.end (), std::back_inserter (data));
  no->filehashvectmtx.lock ();
  auto itfhv = std::find_if (
      no->filehashvect.begin (), no->filehashvect.end (), [key, &tint]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == time_t (tint))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfhv != no->filehashvect.end ())
    {
      std::tuple<std::string, uint64_t, uint64_t, std::vector<char>> tempt;
      tempt = std::make_tuple (key, tint, numb, data);
      no->filepartrcvmtx.lock ();
      auto itfpr = std::find (no->filepartrcv.begin (), no->filepartrcv.end (),
			      tempt);
      if (itfpr == no->filepartrcv.end ())
	{
	  no->filepartrcv.push_back (tempt);
	}
      no->filepartrcvmtx.unlock ();
    }
  no->filehashvectmtx.unlock ();
}

void
FileReceiveOp::fileFe (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  no->filehashvectmtx.lock ();
  auto itfhv = std::find_if (
      no->filehashvect.begin (), no->filehashvect.end (), [key, &tint]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == time_t (tint))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfhv != no->filehashvect.end ())
    {
      std::tuple<std::string, uint64_t> tempt;
      tempt = std::make_tuple (key, tint);
      no->filepartendmtx.lock ();
      auto itfpe = std::find (no->filepartend.begin (), no->filepartend.end (),
			      tempt);
      if (itfpe == no->filepartend.end ())
	{
	  no->filepartend.push_back (tempt);
	}
      no->filepartendmtx.unlock ();
    }
  no->filehashvectmtx.unlock ();
}

void
FileReceiveOp::fileFE (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  no->fileendmtx.lock ();
  auto itfev = std::find_if (
      no->fileend.begin (), no->fileend.end (), [key, &tint]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == tint)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfev == no->fileend.end ())
    {
      no->fileend.push_back (std::make_tuple (key, tint));
    }
  no->fileendmtx.unlock ();
}

void
FileReceiveOp::fileFF (std::string msgtype, std::string key,
		       std::vector<char> &buf)
{
  std::cout << msgtype << std::endl;
  uint64_t tint;
  std::memcpy (&tint, &buf[34], sizeof(tint));
  no->filesendreqmtx.lock ();
  no->filesendreq.erase (
      std::remove_if (
	  no->filesendreq.begin (), no->filesendreq.end (), [key, &tint]
	  (auto &el)
	    {
	      if (std::get<0>(el) == key && std::get<2>(el) == time_t (tint))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }),
      no->filesendreq.end ());
  no->filesendreqmtx.unlock ();

  no->filepartbufmtx.lock ();
  no->filepartbuf.erase (
      std::remove_if (
	  no->filepartbuf.begin (), no->filepartbuf.end (), [key, &tint]
	  (auto &el)
	    {
	      if (std::get<0>(el) == key && std::get<1>(el) == time_t (tint))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }),
      no->filepartbuf.end ());
  no->filepartbufmtx.unlock ();

  no->fbrvectmtx.lock ();
  no->fbrvect.erase (
      std::remove_if (no->fbrvect.begin (), no->fbrvect.end (), [key, &tint]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == time_t (tint))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
      no->fbrvect.end ());
  no->fbrvectmtx.unlock ();
}
