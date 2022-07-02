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

#include "FileSending.h"

FileSending::FileSending (NetworkOperations *No)
{
  no = No;
}

FileSending::~FileSending ()
{
  // TODO Auto-generated destructor stub
}
int
FileSending::fileSending (std::filesystem::path pvect, std::string key,
			  int variant, int sock, uint32_t ip, uint16_t port,
			  int *totalsent)
{
  int result = 0;
  AuxFunc af;
  std::filesystem::path ptos = pvect;
  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
  okp = lt::dht::ed25519_create_keypair (*(no->seed));
  std::array<char, 32> okarr = std::get<0> (okp).bytes;
  lt::dht::public_key othpk;
  lt::aux::from_hex (key, othpk.bytes.data ());
  std::array<char, 32> scalar = lt::dht::ed25519_key_exchange (
      othpk, std::get<1> (okp));
  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
  std::string unm = key;
  std::string passwd = lt::aux::to_hex (othpk.bytes);
  std::filesystem::path source = ptos;
#ifdef __linux
  std::string filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  std::string filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/CommunistSF/file";
  std::filesystem::path outpath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (outpath.parent_path ()))
    {
      std::filesystem::remove_all (outpath.parent_path ());
    }
  std::filesystem::create_directories (outpath.parent_path ());
  af.decryptFile (unm, passwd, source.u8string (), outpath.u8string ());
  std::fstream f;
  std::string line;
  int count = 0;
  f.open (outpath, std::ios_base::in);
  while (!f.eof ())
    {
      getline (f, line);
      count++;
      if (count == 6)
	{
	  break;
	}
    }
  f.close ();
  std::filesystem::remove_all (outpath.parent_path ());
  ptos = std::filesystem::u8path (line);
  no->filesendreqmtx.lock ();
  auto itfsr = std::find_if (
      no->filesendreq.begin (), no->filesendreq.end (), [key, &ptos]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == ptos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfsr == no->filesendreq.end ())
    {
      time_t curtime = time (NULL);
      int status = 0;
      no->filesendreq.push_back (
	  std::make_tuple (key, ptos, curtime, status, pvect));
      std::vector<char> msg;
      std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
      std::string type = "FQ";
      std::copy (type.begin (), type.end (), std::back_inserter (msg));
      uint64_t tmt = curtime;
      msg.resize (msg.size () + sizeof(tmt));
      std::memcpy (&msg[34], &tmt, sizeof(tmt));
      uint64_t fsize = std::filesystem::file_size (ptos);
      msg.resize (msg.size () + sizeof(fsize));
      std::memcpy (&msg[42], &fsize, sizeof(fsize));
      std::vector<char> input;
      std::string fnm = ptos.filename ().u8string ();
      std::copy (fnm.begin (), fnm.end (), std::back_inserter (input));
      if (input.size () < 16)
	{
	  std::string add = "\\12356789012345";
	  std::copy (add.begin (), add.end (), std::back_inserter (input));
	}
      std::vector<char> output;
      output = af.cryptStrm (unm, passwd, input);
      std::copy (output.begin (), output.end (), std::back_inserter (msg));
      std::string unm = key;
      lt::dht::public_key othpk;
      lt::aux::from_hex (unm, othpk.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (okp));
      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
      std::string passwd = lt::aux::to_hex (othpk.bytes);
      msg = af.cryptStrm (unm, passwd, msg);
      int sent = 0;
      no->ipv6lrmtx.lock ();
      auto itlr6 = std::find_if (no->ipv6lr.begin (), no->ipv6lr.end (), [key]
      (auto &el)
	{
	  return std::get<0>(el) == key;
	});
      if (itlr6 != no->ipv6lr.end ())
	{
	  if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
	    {
	      no->ipv6contmtx.lock ();
	      auto itip6 = std::find_if (no->ipv6cont.begin (),
					 no->ipv6cont.end (), [key]
					 (auto &el)
					   {
					     return std::get<0>(el) == key;
					   });
	      if (itip6 != no->ipv6cont.end ())
		{
		  std::string ipv6 = std::get<1> (*itip6);
		  uint16_t port = std::get<2> (*itip6);
		  no->sockipv6mtx.lock ();
		  sent = no->sendMsg6 (no->sockipv6, ipv6, port, msg);
		  no->sockipv6mtx.unlock ();
		}
	      no->ipv6contmtx.unlock ();
	    }
	}
      no->ipv6lrmtx.unlock ();
      if (sent <= 0)
	{
	  sent = no->sendMsg (sock, ip, port, msg);
	}
    }
  else
    {
      int status = std::get<3> (*itfsr);
      if (status == 0)
	{
	  time_t curtime = time (NULL);
	  std::vector<char> msg;
	  std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
	  std::string type = "FQ";
	  std::copy (type.begin (), type.end (), std::back_inserter (msg));
	  uint64_t tmt = std::get<2> (*itfsr);
	  msg.resize (msg.size () + sizeof(tmt));
	  std::memcpy (&msg[34], &tmt, sizeof(tmt));
	  uint64_t fsize = std::filesystem::file_size (ptos);
	  msg.resize (msg.size () + sizeof(fsize));
	  std::memcpy (&msg[42], &fsize, sizeof(fsize));
	  std::vector<char> input;
	  std::string fnm = ptos.filename ().u8string ();
	  std::copy (fnm.begin (), fnm.end (), std::back_inserter (input));
	  if (input.size () < 16)
	    {
	      std::string add = "\\12356789012345";
	      std::copy (add.begin (), add.end (), std::back_inserter (input));
	    }
	  std::vector<char> output;
	  output = af.cryptStrm (unm, passwd, input);
	  std::copy (output.begin (), output.end (), std::back_inserter (msg));
	  std::string unm = key;
	  lt::dht::public_key othpk;
	  lt::aux::from_hex (unm, othpk.bytes.data ());
	  std::array<char, 32> scalar;
	  scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (okp));
	  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
	  std::string passwd = lt::aux::to_hex (othpk.bytes);
	  msg = af.cryptStrm (unm, passwd, msg);
	  int sent = 0;
	  no->ipv6lrmtx.lock ();
	  auto itlr6 = std::find_if (no->ipv6lr.begin (), no->ipv6lr.end (),
				     [key]
				     (auto &el)
				       {
					 return std::get<0>(el) == key;
				       });
	  if (itlr6 != no->ipv6lr.end ())
	    {
	      if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
		{
		  no->ipv6contmtx.lock ();
		  auto itip6 = std::find_if (no->ipv6cont.begin (),
					     no->ipv6cont.end (), [key]
					     (auto &el)
					       {
						 return std::get<0>(el) == key;
					       });
		  if (itip6 != no->ipv6cont.end ())
		    {
		      std::string ipv6 = std::get<1> (*itip6);
		      uint16_t port = std::get<2> (*itip6);
		      no->sockipv6mtx.lock ();
		      sent = no->sendMsg6 (no->sockipv6, ipv6, port, msg);
		      no->sockipv6mtx.unlock ();
		      result = 1;
		    }
		  no->ipv6contmtx.unlock ();
		}
	    }
	  no->ipv6lrmtx.unlock ();
	  if (sent <= 0)
	    {
	      sent = no->sendMsg (sock, ip, port, msg);
	      result = 1;
	    }
	}
      if (status == 1)
	{
	  time_t curtime = time (NULL);
	  std::string key = std::get<0> (*itfsr);
	  std::filesystem::path sp = std::get<1> (*itfsr);
	  no->filepartbufmtx.lock ();
	  auto itfpb = std::find_if (
	      no->filepartbuf.begin (), no->filepartbuf.end (), [key, &sp]
	      (auto &el)
		{
		  if (std::get<0>(el) == key && std::get<2>(el) == sp)
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
	      std::string fbrkey = std::get<0> (*itfpb);
	      time_t fbrtm = std::get<1> (*itfpb);
	      no->fbrvectmtx.lock ();
	      auto itfbrv = std::find_if (
		  no->fbrvect.begin (), no->fbrvect.end (), [&fbrkey, &fbrtm]
		  (auto &el)
		    {
		      if (std::get<0>(el) == fbrkey && std::get<1>(el) == fbrtm)
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
		  std::vector<char> msg;
		  std::copy (okarr.begin (), okarr.end (),
			     std::back_inserter (msg));
		  std::string msgtype = "FB";
		  std::copy (msgtype.begin (), msgtype.end (),
			     std::back_inserter (msg));
		  uint64_t tmfb = std::get<1> (*itfpb);
		  msg.resize (msg.size () + sizeof(tmfb));
		  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
		  uint64_t msgn = 0;
		  msg.resize (msg.size () + sizeof(msgn));
		  std::memcpy (&msg[42], &msgn, sizeof(msgn));
		  std::vector<char> fh = af.filehash (sp);
		  std::copy (fh.begin (), fh.end (), std::back_inserter (msg));
		  msg = af.cryptStrm (unm, passwd, msg);
		  int sent = 0;
		  no->ipv6lrmtx.lock ();
		  auto itlr6 = std::find_if (no->ipv6lr.begin (),
					     no->ipv6lr.end (), [key]
					     (auto &el)
					       {
						 return std::get<0>(el) == key;
					       });
		  if (itlr6 != no->ipv6lr.end ())
		    {
		      if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
			{
			  no->ipv6contmtx.lock ();
			  auto itip6 = std::find_if (
			      no->ipv6cont.begin (), no->ipv6cont.end (), [key]
			      (auto &el)
				{
				  return std::get<0>(el) == key;
				});
			  if (itip6 != no->ipv6cont.end ())
			    {
			      std::string ipv6 = std::get<1> (*itip6);
			      uint16_t port = std::get<2> (*itip6);
			      no->sockipv6mtx.lock ();
			      sent = no->sendMsg6 (no->sockipv6, ipv6, port,
						   msg);
			      no->sockipv6mtx.unlock ();
			      result = 1;
			    }
			  no->ipv6contmtx.unlock ();
			}
		    }
		  no->ipv6lrmtx.unlock ();
		  if (sent <= 0)
		    {
		      sent = no->sendMsg (sock, ip, port, msg);
		      result = 1;
		    }
		}
	      else
		{
		  int fsz = std::filesystem::file_size (std::get<2> (*itfpb));
		  if (std::get<6> (*itfpb) == 1)
		    {
		      int bytefmf = std::get<3> (*itfpb);
		      std::vector<char> fpv;
		      if (fsz - bytefmf > no->Partsize)
			{
			  fpv.resize (no->Partsize);
			}
		      else
			{
			  if (fsz - bytefmf > 0)
			    {
			      fpv.resize (fsz - bytefmf);
			    }
			}
		      if (fpv.size () > 0)
			{
			  f.open (std::get<2> (*itfpb),
				  std::ios_base::in | std::ios_base::binary);
			  f.seekg (bytefmf, std::ios_base::beg);
			  f.read (&fpv[0], fpv.size ());
			  f.close ();
			  if (std::get<3> (*itfpb) > 0)
			    {
			      std::get<4> (*itfpb) = std::get<4> (*itfpb) + 1;
			    }
			  std::get<3> (*itfpb) = std::get<3> (*itfpb)
			      + fpv.size ();
			  std::get<5> (*itfpb) = fpv;

			  std::get<6> (*itfpb) = 0;
			}
		      else
			{
			  std::vector<char> msg;
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
			  std::string msgtype = "FE";
			  std::copy (msgtype.begin (), msgtype.end (),
				     std::back_inserter (msg));
			  uint64_t tmfb = std::get<1> (*itfpb);
			  msg.resize (msg.size () + sizeof(tmfb));
			  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
			  uint64_t msgn = 0;
			  msg.resize (msg.size () + sizeof(msgn));
			  std::memcpy (&msg[42], &msgn, sizeof(msgn));
			  msg = af.cryptStrm (unm, passwd, msg);
			  int sent = 0;
			  no->ipv6lrmtx.lock ();
			  auto itlr6 = std::find_if (
			      no->ipv6lr.begin (), no->ipv6lr.end (), [key]
			      (auto &el)
				{
				  return std::get<0>(el) == key;
				});
			  if (itlr6 != no->ipv6lr.end ())
			    {
			      if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
				{
				  no->ipv6contmtx.lock ();
				  auto itip6 = std::find_if (
				      no->ipv6cont.begin (),
				      no->ipv6cont.end (), [key]
				      (auto &el)
					{
					  return std::get<0>(el) == key;
					});
				  if (itip6 != no->ipv6cont.end ())
				    {
				      std::string ipv6 = std::get<1> (*itip6);
				      uint16_t port = std::get<2> (*itip6);
				      no->sockipv6mtx.lock ();
				      sent = no->sendMsg6 (no->sockipv6, ipv6,
							   port, msg);
				      no->sockipv6mtx.unlock ();
				      result = 1;
				    }
				  no->ipv6contmtx.unlock ();
				}
			    }
			  no->ipv6lrmtx.unlock ();
			  if (sent <= 0)
			    {
			      sent = no->sendMsg (sock, ip, port, msg);
			      result = 1;
			    }
			}
		    }
		  if (std::get<6> (*itfpb) == 0)
		    {
		      std::vector<char> msg;
		      std::copy (okarr.begin (), okarr.end (),
				 std::back_inserter (msg));
		      std::string msgtype = "Fb";
		      std::copy (msgtype.begin (), msgtype.end (),
				 std::back_inserter (msg));
		      uint64_t tmfb = std::get<1> (*itfpb);
		      msg.resize (msg.size () + sizeof(tmfb));
		      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
		      uint64_t msgn = std::get<4> (*itfpb);
		      msg.resize (msg.size () + sizeof(msgn));
		      std::memcpy (&msg[42], &msgn, sizeof(msgn));
		      std::vector<char> fpv = std::get<5> (*itfpb);
		      std::vector<char> fh = af.strhash (fpv, 2);
		      std::copy (fh.begin (), fh.end (),
				 std::back_inserter (msg));
		      msg = af.cryptStrm (unm, passwd, msg);
		      int sent = 0;
		      no->ipv6lrmtx.lock ();
		      auto itlr6 = std::find_if (
			  no->ipv6lr.begin (), no->ipv6lr.end (), [key]
			  (auto &el)
			    {
			      return std::get<0>(el) == key;
			    });
		      if (itlr6 != no->ipv6lr.end ())
			{
			  if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
			    {
			      no->ipv6contmtx.lock ();
			      auto itip6 = std::find_if (
				  no->ipv6cont.begin (), no->ipv6cont.end (),
				  [key]
				  (auto &el)
				    {
				      return std::get<0>(el) == key;
				    });
			      if (itip6 != no->ipv6cont.end ())
				{
				  std::string ipv6 = std::get<1> (*itip6);
				  uint16_t port = std::get<2> (*itip6);
				  no->sockipv6mtx.lock ();
				  sent = no->sendMsg6 (no->sockipv6, ipv6, port,
						       msg);
				  no->sockipv6mtx.unlock ();
				  result = 1;
				}
			      no->ipv6contmtx.unlock ();
			    }
			}
		      no->ipv6lrmtx.unlock ();
		      if (sent <= 0)
			{
			  sent = no->sendMsg (sock, ip, port, msg);
			  result = 1;
			}
		      int sentb = 0;
		      uint64_t dtgmnmb = 0;
		      for (;;)
			{
			  int fpsz = fpv.size ();
			  if (sentb >= fpsz)
			    {
			      break;
			    }
			  msg.clear ();
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
			  std::string msgtype = "Fp";
			  std::copy (msgtype.begin (), msgtype.end (),
				     std::back_inserter (msg));
			  uint64_t tmfb = std::get<1> (*itfpb);
			  msg.resize (msg.size () + sizeof(tmfb));
			  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
			  msg.resize (msg.size () + sizeof(dtgmnmb));
			  std::memcpy (&msg[42], &dtgmnmb, sizeof(dtgmnmb));
			  dtgmnmb = dtgmnmb + 1;
			  std::vector<char> dtgm;
			  if (fpsz - sentb > 457)
			    {
			      std::copy (fpv.begin () + sentb,
					 fpv.begin () + sentb + 457,
					 std::back_inserter (dtgm));
			      sentb = sentb + 457;
			    }
			  else
			    {
			      int dif = fpsz - sentb;
			      std::copy (fpv.begin () + sentb,
					 fpv.begin () + sentb + dif,
					 std::back_inserter (dtgm));
			      sentb = sentb + dif;
			    }
			  std::copy (dtgm.begin (), dtgm.end (),
				     std::back_inserter (msg));
			  msg = af.cryptStrm (unm, passwd, msg);
			  int sent = 0;
			  no->ipv6lrmtx.lock ();
			  auto itlr6 = std::find_if (
			      no->ipv6lr.begin (), no->ipv6lr.end (), [key]
			      (auto &el)
				{
				  return std::get<0>(el) == key;
				});
			  if (itlr6 != no->ipv6lr.end ())
			    {
			      if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
				{
				  no->ipv6contmtx.lock ();
				  auto itip6 = std::find_if (
				      no->ipv6cont.begin (),
				      no->ipv6cont.end (), [key]
				      (auto &el)
					{
					  return std::get<0>(el) == key;
					});
				  if (itip6 != no->ipv6cont.end ())
				    {
				      std::string ipv6 = std::get<1> (*itip6);
				      uint16_t port = std::get<2> (*itip6);
				      no->sockipv6mtx.lock ();
				      sent = no->sendMsg6 (no->sockipv6, ipv6,
							   port, msg);
				      no->sockipv6mtx.unlock ();
				      result = 1;
				    }
				  no->ipv6contmtx.unlock ();
				}
			    }
			  no->ipv6lrmtx.unlock ();
			  if (sent <= 0)
			    {
			      sent = no->sendMsg (sock, ip, port, msg);
			      result = 1;
			    }

			  *totalsent = *totalsent + msg.size ();
			  if (sent < 0)
			    {
			      no->fbrvectmtx.unlock ();
			      no->filepartbufmtx.unlock ();
			      no->filesendreqmtx.unlock ();
			      no->sendbufmtx.unlock ();
			      no->contmtx.unlock ();
			      return result;
			    }
			}
		      msg.clear ();
		      std::copy (okarr.begin (), okarr.end (),
				 std::back_inserter (msg));
		      msgtype = "Fe";
		      std::copy (msgtype.begin (), msgtype.end (),
				 std::back_inserter (msg));
		      tmfb = std::get<1> (*itfpb);
		      msg.resize (msg.size () + sizeof(tmfb));
		      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
		      msgn = 0;
		      msg.resize (msg.size () + sizeof(msgn));
		      std::memcpy (&msg[42], &msgn, sizeof(msgn));
		      msg = af.cryptStrm (unm, passwd, msg);
		      sent = 0;
		      no->ipv6lrmtx.lock ();
		      itlr6 = std::find_if (no->ipv6lr.begin (),
					    no->ipv6lr.end (), [key]
					    (auto &el)
					      {
						return std::get<0>(el) == key;
					      });
		      if (itlr6 != no->ipv6lr.end ())
			{
			  if (curtime - std::get<1> (*itlr6) <= no->Tmttear)
			    {
			      no->ipv6contmtx.lock ();
			      auto itip6 = std::find_if (
				  no->ipv6cont.begin (), no->ipv6cont.end (),
				  [key]
				  (auto &el)
				    {
				      return std::get<0>(el) == key;
				    });
			      if (itip6 != no->ipv6cont.end ())
				{
				  std::string ipv6 = std::get<1> (*itip6);
				  uint16_t port = std::get<2> (*itip6);
				  no->sockipv6mtx.lock ();
				  sent = no->sendMsg6 (no->sockipv6, ipv6, port,
						       msg);
				  no->sockipv6mtx.unlock ();
				  result = 1;
				}
			      no->ipv6contmtx.unlock ();
			    }
			}
		      no->ipv6lrmtx.unlock ();
		      if (sent <= 0)
			{
			  sent = no->sendMsg (sock, ip, port, msg);
			  result = 1;
			}
		    }
		}
	      no->fbrvectmtx.unlock ();
	    }
	  no->filepartbufmtx.unlock ();
	}
    }
  no->filesendreqmtx.unlock ();

  return result;
}
