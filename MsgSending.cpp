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

#include "MsgSending.h"

MsgSending::MsgSending (NetworkOperations *No)
{
  no = No;
}

MsgSending::~MsgSending ()
{
  // TODO Auto-generated destructor stub
}

int
MsgSending::sendMsg (std::filesystem::path pvect, std::string key, int variant,
		     int sock, uint32_t ip, uint16_t port)
{
  int result = 0;
  AuxFunc af;
  std::filesystem::path ptos = pvect;
  size_t vind;
  no->msgpartbufmtx.lock ();
  auto itmpb = std::find_if (
      no->msgpartbuf.begin (), no->msgpartbuf.end (), [key, &ptos]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<3>(el) == ptos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itmpb == no->msgpartbuf.end ())
    {
      std::tuple<std::string, uint64_t, int, std::filesystem::path, int,
	  uint64_t, std::vector<char>> ttup;
      std::get<0> (ttup) = key;
      time_t tmt = time (NULL);
      for (;;)
	{
	  auto ittmt = std::find_if (no->msgpartbuf.begin (),
				     no->msgpartbuf.end (), [key, &tmt]
				     (auto &el)
				       {
					 if (std::get<0>(el) == key &&
					     std::get<1>(el) == uint64_t (tmt))
					   {
					     return true;
					   }
					 else
					   {
					     return false;
					   }
				       });
	  if (ittmt != no->msgpartbuf.end ())
	    {
	      tmt = tmt + 1;
	    }
	  else
	    {
	      break;
	    }
	}
      std::get<1> (ttup) = uint64_t (tmt);
      std::get<2> (ttup) = -1;
      std::get<3> (ttup) = ptos;
      std::get<4> (ttup) = 0;
      std::get<5> (ttup) = 0;
      no->msgpartbuf.push_back (ttup);
      vind = no->msgpartbuf.size () - 1;
    }
  else
    {
      vind = std::distance (no->msgpartbuf.begin (), itmpb);
    }
  int sentstat = std::get<2> (no->msgpartbuf[vind]);
  int byteread = std::get<4> (no->msgpartbuf[vind]);
  int fsz = std::filesystem::file_size (ptos);
  if (sentstat == -1)
    {
      std::vector<char> msg;
      std::array<char, 32> okarr;
      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
      okp = lt::dht::ed25519_create_keypair (*(no->seed));
      lt::dht::public_key othpk;
      lt::aux::from_hex (key, othpk.bytes.data ());
      std::array<char, 32> scalar = lt::dht::ed25519_key_exchange (
	  othpk, std::get<1> (okp));
      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
      std::string unm = key;
      std::string passwd = lt::aux::to_hex (othpk.bytes);
      okarr = std::get<0> (okp).bytes;
      std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
      std::string msgtype;
      if (variant == 1)
	{
	  msgtype = "PB";
	}
      if (variant == 2)
	{
	  msgtype = "MB";
	}

      std::copy (msgtype.begin (), msgtype.end (), std::back_inserter (msg));
      uint64_t tmfb = std::get<1> (no->msgpartbuf[vind]);
      msg.resize (msg.size () + sizeof(tmfb));
      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
      uint64_t msgn = uint64_t (
	  std::filesystem::file_size (std::get<3> (no->msgpartbuf[vind])));
      msg.resize (msg.size () + sizeof(msgn));
      std::memcpy (&msg[42], &msgn, sizeof(msgn));
      std::vector<char> fh = af.filehash (std::get<3> (no->msgpartbuf[vind]));
      std::copy (fh.begin (), fh.end (), std::back_inserter (msg));
      msg = af.cryptStrm (unm, passwd, msg);
      time_t curtime = time (NULL);
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
  std::vector<char> msgpart;
  if (sentstat == 0 || sentstat == 2)
    {
      if (byteread < fsz)
	{
	  if ((fsz - byteread) > no->Partsize)
	    {
	      msgpart.resize (no->Partsize);
	    }
	  else
	    {
	      msgpart.resize (fsz - byteread);
	    }

	  std::fstream f;
	  f.open (ptos, std::ios_base::in | std::ios_base::binary);
	  f.seekg (byteread, std::ios_base::beg);
	  f.read (&msgpart[0], msgpart.size ());
	  f.close ();
	  std::get<6> (no->msgpartbuf[vind]) = msgpart;
	  byteread = byteread + msgpart.size ();
	  std::get<4> (no->msgpartbuf[vind]) = byteread;
	  std::get<2> (no->msgpartbuf[vind]) = 1;
	  if (sentstat == 2)
	    {
	      std::get<5> (no->msgpartbuf[vind]) = std::get<5> (
		  no->msgpartbuf[vind]) + 1;
	    }
	  sentstat = 1;
	}
      else
	{
	  std::get<2> (no->msgpartbuf[vind]) = 2;
	  sentstat = 2;
	}

    }

  if (sentstat == 1)
    {
      msgpart.clear ();
      msgpart = std::get<6> (no->msgpartbuf[vind]);
      if (msgpart.size () > 0)
	{
	  std::vector<char> msg;
	  std::array<char, 32> okarr;
	  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	  okp = lt::dht::ed25519_create_keypair (*(no->seed));
	  lt::dht::public_key othpk;
	  lt::aux::from_hex (key, othpk.bytes.data ());
	  std::array<char, 32> scalar = lt::dht::ed25519_key_exchange (
	      othpk, std::get<1> (okp));
	  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
	  std::string unm = key;
	  std::string passwd = lt::aux::to_hex (othpk.bytes);
	  okarr = std::get<0> (okp).bytes;
	  std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
	  std::string msgtype;
	  if (variant == 1)
	    {
	      msgtype = "Pb";
	    }
	  if (variant == 2)
	    {
	      msgtype = "Mb";
	    }
	  std::copy (msgtype.begin (), msgtype.end (),
		     std::back_inserter (msg));
	  uint64_t tmfb = std::get<1> (no->msgpartbuf[vind]);
	  msg.resize (msg.size () + sizeof(tmfb));
	  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
	  uint64_t msgn = std::get<5> (no->msgpartbuf[vind]);
	  msg.resize (msg.size () + sizeof(msgn));
	  std::memcpy (&msg[42], &msgn, sizeof(msgn));
	  std::vector<char> fh = af.strhash (msgpart, 2);
	  std::copy (fh.begin (), fh.end (), std::back_inserter (msg));
	  msg = af.cryptStrm (unm, passwd, msg);
	  time_t curtime = time (NULL);
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

	  size_t mcount = 0;
	  msgn = 0;
	  for (;;)
	    {
	      msg.clear ();
	      std::copy (okarr.begin (), okarr.end (),
			 std::back_inserter (msg));
	      if (variant == 1)
		{
		  msgtype = "Pp";
		}
	      if (variant == 2)
		{
		  msgtype = "Mp";
		}
	      std::copy (msgtype.begin (), msgtype.end (),
			 std::back_inserter (msg));
	      msg.resize (msg.size () + sizeof(tmfb));
	      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
	      msg.resize (msg.size () + sizeof(msgn));
	      std::memcpy (&msg[42], &msgn, sizeof(msgn));
	      if (mcount >= msgpart.size ())
		{
		  break;
		}

	      if (msgpart.size () - mcount >= 457)
		{
		  std::copy (msgpart.begin () + mcount,
			     msgpart.begin () + mcount + 457,
			     std::back_inserter (msg));
		  mcount = mcount + 457;
		}
	      else
		{
		  std::copy (msgpart.begin () + mcount, msgpart.end (),
			     std::back_inserter (msg));
		  mcount = msgpart.size ();
		}
	      msg = af.cryptStrm (unm, passwd, msg);
	      sent = 0;
	      no->ipv6lrmtx.lock ();
	      itlr6 = std::find_if (no->ipv6lr.begin (), no->ipv6lr.end (),
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

	      msgn = msgn + 1;
	    }
	  msg.clear ();
	  std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
	  if (variant == 1)
	    {
	      msgtype = "Pe";
	    }
	  if (variant == 2)
	    {
	      msgtype = "Me";
	    }
	  std::copy (msgtype.begin (), msgtype.end (),
		     std::back_inserter (msg));
	  msg.resize (msg.size () + sizeof(tmfb));
	  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
	  msgn = std::get<5> (no->msgpartbuf[vind]);
	  msg.resize (msg.size () + sizeof(msgn));
	  std::memcpy (&msg[42], &msgn, sizeof(msgn));
	  msg = af.cryptStrm (unm, passwd, msg);
	  sent = 0;
	  no->ipv6lrmtx.lock ();
	  itlr6 = std::find_if (no->ipv6lr.begin (), no->ipv6lr.end (), [key]
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
	  std::get<2> (no->msgpartbuf[vind]) = 1;
	}
    }
  if (byteread >= fsz && sentstat == 2)
    {
      std::vector<char> msg;
      std::array<char, 32> okarr;
      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
      okp = lt::dht::ed25519_create_keypair (*(no->seed));
      lt::dht::public_key othpk;
      lt::aux::from_hex (key, othpk.bytes.data ());
      std::array<char, 32> scalar = lt::dht::ed25519_key_exchange (
	  othpk, std::get<1> (okp));
      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
      std::string unm = key;
      std::string passwd = lt::aux::to_hex (othpk.bytes);
      okarr = std::get<0> (okp).bytes;
      std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
      std::string msgtype;
      if (variant == 1)
	{
	  msgtype = "PE";
	}
      if (variant == 2)
	{
	  msgtype = "ME";
	}
      std::copy (msgtype.begin (), msgtype.end (), std::back_inserter (msg));
      uint64_t tmfb = std::get<1> (no->msgpartbuf[vind]);
      msg.resize (msg.size () + sizeof(tmfb));
      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
      uint64_t msgn = 0;
      msg.resize (msg.size () + sizeof(msgn));
      std::memcpy (&msg[42], &msgn, sizeof(msgn));
      msg = af.cryptStrm (unm, passwd, msg);
      time_t curtime = time (NULL);
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

  no->msgpartbufmtx.unlock ();

  return result;
}
