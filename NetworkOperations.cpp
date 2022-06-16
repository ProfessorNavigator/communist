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
#include "NetworkOperations.h"

NetworkOperations::NetworkOperations (
    std::string username, std::string password,
    std::vector<std::tuple<int, std::string>> *Contacts,
    std::array<char, 32> *Seed, std::vector<std::string> *addfriends,
    std::vector<std::tuple<std::string, std::string>> *Prefvect,
    std::string sharepath)
{
  Username = username;
  Password = password;
  contacts = *Contacts;
  contactsfull = *Contacts;
  sockipv6 = 0;
  for (size_t i = 0; i < contactsfull.size (); i++)
    {
      std::tuple<std::string, int, std::shared_ptr<std::mutex>, time_t,
	  std::shared_ptr<std::mutex>> ttup;
      std::shared_ptr<std::mutex> mtx (new std::mutex);
      std::shared_ptr<std::mutex> mtxgip (new std::mutex);
      std::get<0> (ttup) = std::get<1> (contactsfull[i]);
      std::get<1> (ttup) = 0;
      std::get<2> (ttup) = mtx;
      std::get<3> (ttup) = 0;
      std::get<4> (ttup) = mtxgip;
      sockets4.push_back (ttup);
    }
  contsizech = contacts.size () - 1;
  seed = Seed;
  Addfriends = *addfriends;
  prefvect = *Prefvect;
  Sharepath = sharepath;
  auto itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Netmode";
    });
  if (itprv != prefvect.end ())
    {
      std::string nm = std::get<1> (*itprv);
      if (nm == "0")
	{
	  Netmode = "0";
	}
      else
	{
	  Netmode = "1";
	}
    }
  else
    {
      Netmode = "0";
    }
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Maxmsgsz";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << ms;
	  strm >> Maxmsgsz;
	}
    }
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Partsize";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << ms;
	  strm >> Partsize;
	}
    }

  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "ShutTmt";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << ms;
	  strm >> Shuttmt;
	}
    }

  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "TmtTear";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << ms;
	  strm >> Tmttear;
	}
    }
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Stunport";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  std::stringstream strm;
	  std::locale loc ("C");
	  strm.imbue (loc);
	  strm << ms;
	  strm >> stunport;
	}
    }
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Stun";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  Enablestun = ms;
	}
    }
  itprv = std::find_if (prefvect.begin (), prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "DirectInet";
    });
  if (itprv != prefvect.end ())
    {
      std::string ms = std::get<1> (*itprv);
      if (ms != "")
	{
	  Directinet = ms;
	}
    }
}

NetworkOperations::~NetworkOperations ()
{

}

void
NetworkOperations::mainFunc ()
{
  std::string filename;
  AuxFunc af;
  af.homePath (&filename);
  filename = filename + "/.Communist/Bufer";
  std::filesystem::path filepath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (filepath))
    {
      std::filesystem::remove_all (filepath);
    }
  std::filesystem::create_directories (filepath);

  dnsfinished.connect ( [this]
  {
    std::mutex *thrmtx = new std::mutex;
    thrmtx->lock ();
    this->threadvectmtx.lock ();
    this->threadvect.push_back (std::make_tuple (thrmtx, "Dns finished"));
    this->threadvectmtx.unlock ();
    std::thread *dnsfinthr = new std::thread ( [this, thrmtx]
    {
      this->sockipv6mtx.lock ();
#ifdef __linux
      this->sockipv6 = socket (AF_INET6, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
      this->sockipv6 = socket (AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
      u_long nonblocking_enabled = TRUE;
      ioctlsocket (this->sockipv6, FIONBIO, &nonblocking_enabled);
#endif
      sockaddr_in6 addripv6 =
	{ };
      addripv6.sin6_family = AF_INET6;
      addripv6.sin6_addr = in6addr_any;
      addripv6.sin6_port = 0;
      int addrlen2 = sizeof(addripv6);
      bind (this->sockipv6, (const sockaddr*) &addripv6, addrlen2);
#ifdef __linux
      ifaddrs *ifap, *ifa;

      sockaddr_in *ipv4;
      sockaddr_in6 *ipv6;
      char addr6[INET6_ADDRSTRLEN];

      if (getifaddrs (&ifap) == -1)
#endif
#ifdef __WIN32
	WSADATA WsaData;
      WSAStartup (MAKEWORD (2, 2), &WsaData);
      ULONG outBufLen = 15000;
      PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*) malloc (outBufLen);
      if (GetAdaptersAddresses (AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX,
				NULL, pAddresses, &outBufLen) != NO_ERROR)














#endif
    {
      std::cerr << "Error on getting ipv6" << std::endl;
    }
  else
    {
      std::string line;
      std::vector<std::string> ipv6tmp;
      std::vector<std::string> ipv4tmp;
#ifdef __linux
      for (ifa = ifap; ifa; ifa = ifa->ifa_next)
	{

	  if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6)
	    {
	      ipv6 = (struct sockaddr_in6*) ifa->ifa_addr;
	      if (!IN6_IS_ADDR_LOOPBACK(&ipv6->sin6_addr))
		{
		  if (this->Netmode == "0")
		    {
		      if (!IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr))
			{
			  line = std::string (ifa->ifa_name) + " ";
			  line = line
			  + inet_ntop (AF_INET6, &ipv6->sin6_addr,
			      addr6, sizeof(addr6));
			  ipv6tmp.push_back (line);
			}
		    }
		  if (this->Netmode == "1")
		    {
		      line = std::string (ifa->ifa_name) + " ";
		      line = line
		      + +inet_ntop (AF_INET6, &ipv6->sin6_addr, addr6,
			  sizeof(addr6));
		      if (IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr))
			{
			  ipv6tmp.insert (ipv6tmp.begin (), line);
			}
		      else
			{
			  ipv6tmp.push_back (line);
			}
		    }
		}

	    }
	  if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
	    {
	      ipv4 = (struct sockaddr_in*) ifa->ifa_addr;
	      if (ipv4->sin_addr.s_addr != htonl (INADDR_LOOPBACK))
		{
		  std::vector<char> addr4;
		  addr4.resize (INET_ADDRSTRLEN);
		  line = std::string (ifa->ifa_name) + " ";
		  line = line
		  + inet_ntop (AF_INET, &ipv4->sin_addr.s_addr,
		      addr4.data (), addr4.size ());
		  ipv4tmp.push_back (line);
		}
	    }
	}
#endif
#ifdef _WIN32
      while (pAddresses)
	{
	  PIP_ADAPTER_UNICAST_ADDRESS pUnicast =
	  pAddresses->FirstUnicastAddress;
	  while (pUnicast != NULL)
	    {
	      if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
		{
		  sockaddr_in *sa_in =
		  (sockaddr_in*) pUnicast->Address.lpSockaddr;
		  std::vector<char> buff;
		  buff.resize (INET_ADDRSTRLEN);
		  if (sa_in->sin_addr.s_addr != htonl (INADDR_LOOPBACK))
		    {
		      std::string ip = inet_ntop (AF_INET,
			  &(sa_in->sin_addr),
			  buff.data (),
			  buff.size ());
		      ipv4tmp.push_back (ip);
		    }
		}
	      if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
		{
		  sockaddr_in6 *sa_in =
		  (sockaddr_in6*) pUnicast->Address.lpSockaddr;
		  std::vector<char> buff;
		  buff.resize (INET6_ADDRSTRLEN);
		  if (!IN6_IS_ADDR_LINKLOCAL (&sa_in->sin6_addr)
		      && !IN6_IS_ADDR_LOOPBACK (&sa_in->sin6_addr))
		    {
		      std::string ip = inet_ntop (AF_INET6,
			  &(sa_in->sin6_addr),
			  buff.data (),
			  buff.size ());
		      ipv6tmp.push_back (ip);
		    }
		}
	      pUnicast = pUnicast->Next;
	    }

	  pAddresses = pAddresses->Next;
	}
#endif
      if (ipv4tmp.size () > 1)
	{
	  for (size_t i = 0; i < ipv4tmp.size (); i++)
	    {
	      line = ipv4tmp[i];
	      this->ipv4signal.emit (line);
	    }
	  this->ipv4signalfinished.emit ();
	  for (;;)
	    {
	      this->IPV4mtx.lock ();
	      if (this->IPV4 != "")
		{
		  std::cout << "Own ipv4 " << this->IPV4 << std::endl;
		  this->IPV4mtx.unlock ();
		  break;
		}
	      this->IPV4mtx.unlock ();

	      usleep (100000);
	    }
	}
      else
	{
	  if (ipv4tmp.size () > 0)
	    {
	      std::string tmp = ipv4tmp[0];
	      tmp.erase (0, tmp.find ((" ")) + std::string (" ").size ());
	      this->IPV4mtx.lock ();
	      this->IPV4 = tmp;
	      std::cout << "Own ipv4 " << this->IPV4 << std::endl;
	      this->IPV4mtx.unlock ();
	    }
	}
      if (ipv6tmp.size () > 1)
	{
	  for (size_t i = 0; i < ipv6tmp.size (); i++)
	    {
	      line = ipv6tmp[i];
	      this->ipv6signal.emit (line);
	    }
	  this->ipv6signalfinished.emit ();
	  for (;;)
	    {
	      this->ownipv6mtx.lock ();
	      if (this->ownipv6 != "")
		{
		  sockaddr_in6 addressp1 =
		    {};
#ifdef __linux
		  unsigned int len = sizeof(addressp1);
#endif
#ifdef _WIN32
		  int len = sizeof(addressp1);
#endif
		  getsockname (this->sockipv6, (sockaddr*) &addressp1,
		      &len);
		  this->ownipv6port = addressp1.sin6_port;
		  std::cout << "Own ipv6 " << this->ownipv6 << " ";
		  std::cout << ntohs (addressp1.sin6_port) << std::endl;
		  this->ownipv6mtx.unlock ();
		  break;
		}
	      this->ownipv6mtx.unlock ();
	      usleep (100000);
	    }
	}
      else
	{
	  if (ipv6tmp.size () > 0)
	    {
	      sockaddr_in6 addressp1 =
		{};
#ifdef __linux
	      unsigned int len = sizeof(addressp1);
#endif
#ifdef _WIN32
	      int len = sizeof(addressp1);
#endif
	      getsockname (this->sockipv6, (sockaddr*) &addressp1, &len);
	      this->ownipv6mtx.lock ();
	      this->ownipv6 = ipv6tmp[0];
	      this->ownipv6.erase (
		  0, this->ownipv6.find (" ") + std::string (" ").size ());
	      this->ownipv6port = addressp1.sin6_port;
	      std::cout << "Own ipv6 " << this->ownipv6 << " ";
	      std::cout << ntohs (addressp1.sin6_port) << std::endl;
	      this->ownipv6mtx.unlock ();
	    }
	}
#ifdef __linux
      freeifaddrs (ifap);
#endif
    }
      this->sockipv6mtx.unlock ();
      this->contmtx.lock ();
      for (size_t i = 0; i < this->contacts.size (); i++)
	{
#ifdef __linux
	  int sock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
	  int sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	  u_long nonblocking_enabled = TRUE;
	  ioctlsocket (sock, FIONBIO, &nonblocking_enabled);
#endif
	  sockaddr_in addripv4 =
	    { };
	  addripv4.sin_family = AF_INET;
	  this->IPV4mtx.lock ();
	  inet_pton (AF_INET, this->IPV4.c_str (), &addripv4.sin_addr.s_addr);
	  this->IPV4mtx.unlock ();
	  addripv4.sin_port = 0;
	  int addrlen1 = sizeof(addripv4);
	  bind (sock, (const sockaddr*) &addripv4, addrlen1);

	  this->sockmtx.lock ();
	  time_t tm = time (NULL);
	  std::string keysearch = std::get<1> (this->contacts[i]);
	  auto itsock = std::find_if (this->sockets4.begin (),
				      this->sockets4.end (), [&keysearch]
				      (auto &el)
					{
					  return std::get<0>(el) == keysearch;
					});
	  if (itsock != this->sockets4.end ())
	    {
	      std::get<1> (*itsock) = sock;
	      std::get<3> (*itsock) = tm;
	    }
	  this->sockmtx.unlock ();
	}
      this->contmtx.unlock ();

      if (this->Netmode == "0")
	{
	  this->processDHT ();
	  this->stunSrv ();
	}

      std::mutex *thrmtxc = new std::mutex;
      thrmtxc->lock ();
      this->threadvectmtx.lock ();
      this->threadvect.push_back (std::make_tuple (thrmtxc, "Commops main"));
      this->threadvectmtx.unlock ();
      std::thread *thr = new std::thread ( [this, thrmtxc]
      {
	if (this->copsrun.try_lock ())
	  {
	    this->commOps ();
	    this->copsrun.unlock ();
	  }
	thrmtxc->unlock ();
      });
      thr->detach ();
      delete thr;
      thrmtx->unlock ();
      this->addfrmtx.lock ();
      for (size_t i = 0; i < this->Addfriends.size (); i++)
	{
	  this->getNewFriends (this->Addfriends[i]);
	}
      this->addfrmtx.unlock ();
    });
    dnsfinthr->detach ();
    delete dnsfinthr;
  });

  if (Netmode == "0")
    {
      dnsFunc ();
    }
  else
    {
      dnsfinished.emit ();
    }
}

void
NetworkOperations::putOwnIps (std::string otherkey, uint32_t ip, uint16_t port)
{
  putipmtx.lock ();
  std::tuple<std::string, uint32_t, uint16_t> ttup;
  ttup = std::make_tuple (otherkey, ip, port);
  auto it = std::find (putipv.begin (), putipv.end (), ttup);
  if (it == putipv.end ())
    {
      putipv.push_back (ttup);
    }
  else
    {
      *it = ttup;
    }
  putipmtx.unlock ();
}

std::pair<uint32_t, uint16_t>
NetworkOperations::getOwnIps (int udpsock,
			      std::pair<struct in_addr, int> stunsv)
{
  std::pair<uint32_t, uint16_t> result;
  if (Netmode == "0" && Directinet == "notdirect")
    {
      std::vector<char> msgv;
      msgv.resize (20);
      uint16_t tt = htons (1);
      std::memcpy (&msgv[0], &tt, sizeof(tt));
      tt = htons (0);
      std::memcpy (&msgv[2], &tt, sizeof(tt));
      uint32_t ttt = htonl (554869826);
      std::memcpy (&msgv[4], &ttt, sizeof(ttt));

      sockaddr_in stun =
	{ };
      stun.sin_family = AF_INET;
      stun.sin_port = stunsv.second;
      result.first = 0;
      result.second = 0;
      stun.sin_addr = stunsv.first;
      sendto (udpsock, msgv.data (), msgv.size (), 0, (struct sockaddr*) &stun,
	      sizeof(stun));
      std::vector<char> buf;
      buf.resize (200);
      int count = 0;
      while (count < 3)
	{
	  if (cancel > 0)
	    {
	      result.first = 0;
	      result.second = 0;
	      return result;
	    }
	  sockaddr_in from =
	    { };
	  socklen_t sizefrom = sizeof(from);
	  int n = 0;
	  pollfd fdsl[1];
	  fdsl[0].fd = udpsock;
	  fdsl[0].events = POLLRDNORM;
	  int respol = poll (fdsl, 1, 3000);
	  if (respol > 0)
	    {
	      n = recvfrom (udpsock, buf.data (), buf.size (), MSG_PEEK,
			    (struct sockaddr*) &from, &sizefrom);
	      if (n > 0 && n <= 576)
		{
		  if (from.sin_addr.s_addr == stun.sin_addr.s_addr
		      && from.sin_port == stun.sin_port)
		    {
		      buf.resize (n);
		      recvfrom (udpsock, buf.data (), buf.size (), 0,
				(struct sockaddr*) &from, &sizefrom);
		      break;
		    }
		}
	      else
		{
		  std::cout << "Stun error!" << std::endl;
		  buf.resize (576);
		  recvfrom (udpsock, buf.data (), buf.size (), 0,
			    (struct sockaddr*) &from, &sizefrom);
		  result = std::make_pair (0, 0);
		  return result;
		}
	    }
	  else
	    {
	      std::cout << "Stun polling error!" << std::endl;
	      result = std::make_pair (0, 0);
	      return result;
	    }
	  count++;
	}
      if (count > 2)
	{
	  std::cout << "Stun error!" << std::endl;
	  result = std::make_pair (0, 0);
	  return result;
	}

      std::pair<uint32_t, uint16_t> xored;
      std::pair<uint32_t, uint16_t> notxored;
      int ch = 0;

      for (size_t i = 0; i < buf.size (); i++)
	{
	  uint16_t chk;
	  std::memcpy (&chk, &buf[i], sizeof(chk));
	  if (chk == htons (1))
	    {
	      if (buf[i + 5] == 1)
		{
		  uint32_t ip;
		  uint16_t port;
		  std::memcpy (&ip, &buf[i + 8], sizeof(ip));
		  std::memcpy (&port, &buf[i + 6], sizeof(port));
		  port = ntohs (port);
		  notxored = std::make_pair (ip, htons (port));
		  ch = 1;
		}
	    }

	  if (chk == htons (32))
	    {
	      if (buf[i + 5] == 1)
		{
		  uint16_t port;
		  std::memcpy (&port, &buf[i + 6], sizeof(port));
		  port = ntohs (port);
		  port ^= 8466;

		  uint32_t iptemp;
		  std::memcpy (&iptemp, &buf[i + 8], sizeof(iptemp));
		  iptemp = ntohl (iptemp);
		  iptemp ^= 554869826;
		  xored = std::make_pair (htonl (iptemp), htons (port));
		  ch = 2;
		}
	    }
	}
      if (ch == 1)
	{
	  result = notxored;
	  uint32_t ip = notxored.first;
	  uint16_t port = notxored.second;
	  std::vector<char> dat;
	  dat.resize (INET_ADDRSTRLEN);
	  std::cout << "Own ipv4:port "
	      << inet_ntop (AF_INET, &ip, dat.data (), dat.size ()) << ":"
	      << ntohs (port) << std::endl;
	}
      if (ch == 2)
	{
	  result = xored;
	  uint32_t ip = xored.first;
	  uint16_t port = xored.second;
	  std::vector<char> dat;
	  dat.resize (INET_ADDRSTRLEN);
	  std::cout << "Own x ipv4:port "
	      << inet_ntop (AF_INET, &ip, dat.data (), dat.size ()) << ":"
	      << ntohs (port) << std::endl;
	}
    }

  if (Netmode == "1" || Directinet == "direct")
    {
      uint32_t ip4;
      IPV4mtx.lock ();
      int ch = inet_pton (AF_INET, IPV4.c_str (), &ip4);
      IPV4mtx.unlock ();
      if (ch > 0)
	{
	  result.first = ip4;
	  sockaddr_in sin =
	    { };
	  socklen_t len = sizeof(sin);
	  if (getsockname (udpsock, (struct sockaddr*) &sin, &len) == -1)
	    {
	      result.second = 0;
	    }
	  else
	    {
	      result.second = sin.sin_port;
	    }
	}
      std::vector<char> dat;
      dat.resize (INET_ADDRSTRLEN);
      std::cout << "Own ipv4:port "
	  << inet_ntop (AF_INET, &ip4, dat.data (), dat.size ()) << ":"
	  << ntohs (result.second) << std::endl;
    }

  return result;
}

void
NetworkOperations::holePunch (int sock, uint32_t ip, std::string otherkey)
{
  holepunchstopmtx.lock ();
  int firstport = 1024;
  auto hpsit = std::find_if (holepunchstop.begin (), holepunchstop.end (),
			     [otherkey]
			     (auto &el)
			       {
				 return std::get<0>(el) == otherkey;
			       });
  if (hpsit != holepunchstop.end ())
    {
      firstport = std::get<1> (*hpsit);
      if (firstport >= 65535)
	{
	  firstport = 1024;
	  std::get<1> (*hpsit) = firstport;
	}
    }
  else
    {
      firstport = 1024;
      holepunchstop.push_back (std::make_tuple (otherkey, firstport));
    }
  holepunchstopmtx.unlock ();
  std::vector<char> ipv;
  ipv.resize (INET_ADDRSTRLEN);
  uint32_t lip = ip;
  std::cout << "HP to " << inet_ntop (AF_INET, &lip, ipv.data (), ipv.size ())
      << std::endl;
  std::tuple<lt::dht::public_key, lt::dht::secret_key> keypair;
  keypair = lt::dht::ed25519_create_keypair (*seed);
  std::array<char, 32> key = std::get<0> (keypair).bytes;
  std::string msg = "TT";
  std::vector<char> msgv (key.begin (), key.end ());
  std::copy (msg.begin (), msg.end (), std::back_inserter (msgv));
  std::string unm = otherkey;
  lt::dht::public_key othpk;
  AuxFunc af;
  lt::aux::from_hex (unm, othpk.bytes.data ());
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (keypair));
  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
  std::string passwd = lt::aux::to_hex (othpk.bytes);
  msgv = af.cryptStrm (unm, passwd, msgv);

  for (int i = firstport; i < 65536; i = i + 1)
    {
      if (cancel > 0)
	{
	  break;
	}
      sockaddr_in op =
	{ };
      op.sin_family = AF_INET;
      op.sin_port = htons (i);
      op.sin_addr.s_addr = ip;
      int count = 0;
      int ch = -1;
      while (ch < 0)
	{
	  ch = sendto (sock, msgv.data (), msgv.size (), 0,
		       (struct sockaddr*) &op, sizeof(op));
	  if (ch < 0)
	    {
#ifdef __linux
	      std::cerr << "Hole punch error: " << strerror (errno)
		  << std::endl;
#endif
#ifdef _WIN32
	      ch = WSAGetLastError ();
	      std::cerr << "Hole punch error: " << ch << std::endl;
#endif
	    }
	  count++;
	  if (count > 10)
	    {
	      std::cout << i << std::endl;
	      holepunchstopmtx.lock ();
	      hpsit = std::find_if (holepunchstop.begin (),
				    holepunchstop.end (), [otherkey]
				    (auto &el)
				      {
					return std::get<0>(el) == otherkey;
				      });
	      if (hpsit != holepunchstop.end ())
		{
		  std::get<1> (*hpsit) = i;
		}
	      else
		{
		  holepunchstop.push_back (std::make_tuple (otherkey, i));
		}
	      holepunchstopmtx.unlock ();
	      return void ();
	    }
	  usleep (100);
	}
    }
  holepunchstopmtx.lock ();
  hpsit = std::find_if (holepunchstop.begin (), holepunchstop.end (), [otherkey]
  (auto &el)
    {
      return std::get<0>(el) == otherkey;
    });
  if (hpsit != holepunchstop.end ())
    {
      std::get<1> (*hpsit) = 1024;
    }
  else
    {
      holepunchstop.push_back (std::make_tuple (otherkey, 1024));
    }
  holepunchstopmtx.unlock ();
}

int
NetworkOperations::receiveMsg (int sockipv4, sockaddr_in *from)
{
  int result = 0;
  AuxFunc af;
  int n;
  std::vector<char> buf;
  std::string chkey;
  int rcvip6 = 0;
  sockaddr_in6 from6 =
    { };
  socklen_t sizeoffrom6 = sizeof(from6);
  std::string msgtype = "";
  auto itsock = std::find_if (sockets4.begin (), sockets4.end (), [sockipv4]
  (auto &el)
    { return std::get<1>(el) == sockipv4;});
  if (itsock != sockets4.end ())
    {
      chkey = std::get<0> (*itsock);
    }
  buf.clear ();
  n = 0;
  buf.resize (507);
  socklen_t sizefrom = sizeof(*from);

  if (sockipv4 == sockipv6)
    {
      sockipv6mtx.lock ();
      n = recvfrom (sockipv6, buf.data (), buf.size (), MSG_PEEK,
		    (struct sockaddr*) &from6, &sizeoffrom6);
      sockipv6mtx.unlock ();
      if (n >= 0 && n <= 576)
	{
	  buf.clear ();
	  buf.resize (n);
	  recvfrom (sockipv6, buf.data (), buf.size (), MSG_PEEK,
		    (struct sockaddr*) &from6, &sizeoffrom6);
	  rcvip6 = 1;
	  std::vector<char> tmpmsg;
	  tmpmsg.resize (INET6_ADDRSTRLEN);
	  std::string chip = inet_ntop (AF_INET6, &from6.sin6_addr,
					tmpmsg.data (), tmpmsg.size ());
	  ipv6contmtx.lock ();
	  auto itip6 = std::find_if (ipv6cont.begin (), ipv6cont.end (), [&chip]
	  (auto &el)
	    {
	      return std::get<1>(el) == chip;
	    });
	  if (itip6 != ipv6cont.end ())
	    {
	      chkey = std::get<0> (*itip6);
	      std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
	      ownkey = lt::dht::ed25519_create_keypair (*seed);
	      std::string unm = lt::aux::to_hex (std::get<0> (ownkey).bytes);
	      lt::dht::public_key othpk;
	      lt::aux::from_hex (chkey, othpk.bytes.data ());
	      std::array<char, 32> scalar;
	      scalar = lt::dht::ed25519_key_exchange (othpk,
						      std::get<1> (ownkey));
	      othpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey),
						   scalar);
	      std::string passwd = lt::aux::to_hex (othpk.bytes);
	      buf = af.decryptStrm (unm, passwd, buf);
	      std::array<char, 32> keyarr;
	      if (buf.size () >= 32)
		{
		  std::copy_n (buf.begin (), 32, keyarr.begin ());
		}
	      std::string key = lt::aux::to_hex (keyarr);
	      if (chkey != key)
		{
		  recvfrom (sockipv6, buf.data (), buf.size (), 0,
			    (struct sockaddr*) &from6, &sizeoffrom6);
		  n = 0;
		  std::cout << "Wrong packet from " << chip << " "
		      << ntohs (from6.sin6_port) << std::endl;
		}
	    }
	  else
	    {
	      recvfrom (sockipv6, buf.data (), buf.size (), 0,
			(struct sockaddr*) &from6, &sizeoffrom6);
	      n = 0;
	    }
	  ipv6contmtx.unlock ();
	}
    }
  else
    {
      n = recvfrom (sockipv4, buf.data (), buf.size (), MSG_PEEK,
		    (struct sockaddr*) from, &sizefrom);
      if (n > 0)
	{
	  buf.clear ();
	  buf.resize (n);
	  recvfrom (sockipv4, buf.data (), buf.size (), MSG_PEEK,
		    (struct sockaddr*) from, &sizefrom);
	}
      std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
      ownkey = lt::dht::ed25519_create_keypair (*seed);
      std::string unm = lt::aux::to_hex (std::get<0> (ownkey).bytes);
      lt::dht::public_key othpk;
      lt::aux::from_hex (chkey, othpk.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (ownkey));
      othpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
      std::string passwd = lt::aux::to_hex (othpk.bytes);
      buf = af.decryptStrm (unm, passwd, buf);
      std::array<char, 32> keyarr;
      if (buf.size () >= 32)
	{
	  std::copy_n (buf.begin (), 32, keyarr.begin ());
	}
      std::string key = lt::aux::to_hex (keyarr);
      if (chkey != key)
	{
	  sockaddr_in delfrom =
	    { };
	  std::vector<char> tmpmsg;
	  tmpmsg.resize (INET_ADDRSTRLEN);
	  recvfrom (sockipv4, buf.data (), buf.size (), 0,
		    (struct sockaddr*) &delfrom, &sizefrom);
	  std::cerr << "Wrong packet from "
	      << inet_ntop (AF_INET, &delfrom.sin_addr.s_addr, tmpmsg.data (),
			    tmpmsg.size ()) << ":" << ntohs (delfrom.sin_port)
	      << std::endl;
	  n = 0;
	}
      else
	{
	  result = 1;
	}
    }
  std::vector<char> tmpmsg;

  if (n > 0 && n <= 576)
    {
      buf.clear ();
      buf.resize (n);
      if (rcvip6 == 0)
	{
	  recvfrom (sockipv4, buf.data (), buf.size (), 0,
		    (struct sockaddr*) from, &sizefrom);
	  uint32_t iptmp = from->sin_addr.s_addr;
	  uint16_t tmpp = from->sin_port;
	  tmpmsg.resize (INET_ADDRSTRLEN);
	  std::cout << "Rcvd fm "
	      << inet_ntop (AF_INET, &iptmp, tmpmsg.data (), tmpmsg.size ())
	      << ":" << ntohs (tmpp) << " Type ";
	}
      else
	{
	  tmpmsg.resize (INET6_ADDRSTRLEN);
	  sockipv6mtx.lock ();
	  recvfrom (sockipv6, buf.data (), buf.size (), 0,
		    (struct sockaddr*) &from6, &sizeoffrom6);
	  sockipv6mtx.unlock ();

	  std::cout << "Rcvd fm "
	      << inet_ntop (AF_INET6, &from6.sin6_addr, tmpmsg.data (),
			    tmpmsg.size ()) << " " << ntohs (from6.sin6_port)
	      << " Type ";
	}
      std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
      ownkey = lt::dht::ed25519_create_keypair (*seed);
      std::string unm = lt::aux::to_hex (std::get<0> (ownkey).bytes);
      lt::dht::public_key othpk;
      lt::aux::from_hex (chkey, othpk.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (othpk, std::get<1> (ownkey));
      othpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
      std::string passwd = lt::aux::to_hex (othpk.bytes);
      buf = af.decryptStrm (unm, passwd, buf);
      std::array<char, 32> keyarr;
      if (buf.size () >= 32)
	{
	  std::copy_n (buf.begin (), 32, keyarr.begin ());
	}
      std::string key = lt::aux::to_hex (keyarr);
      if (buf.size () >= 34)
	{
	  msgtype = std::string (buf.begin () + 32, buf.begin () + 34);
	}
      if (key == chkey)
	{
	  time_t crtm = time (NULL);
	  maintblockmtx.lock ();
	  auto itmnt = std::find_if (maintblock.begin (), maintblock.end (),
				     [key]
				     (auto &el)
				       {
					 return std::get<0>(el) == key;
				       });
	  if (itmnt != maintblock.end ())
	    {
	      std::get<1> (*itmnt) = crtm;
	    }
	  else
	    {
	      maintblock.push_back (std::make_tuple (key, crtm));
	    }
	  maintblockmtx.unlock ();
	  std::get<3> (*itsock) = crtm;
	  if (rcvip6 > 0)
	    {
	      ipv6contmtx.lock ();
	      auto ipv6it = std::find_if (ipv6cont.begin (), ipv6cont.end (),
					  [&key]
					  (auto &el)
					    {
					      return std::get<0>(el) == key;
					    });
	      if (ipv6it != ipv6cont.end ())
		{
		  std::vector<char> ipv6ad;
		  ipv6ad.resize (INET6_ADDRSTRLEN);
		  std::get<1> (*ipv6it) = inet_ntop (AF_INET6, &from6.sin6_addr,
						     ipv6ad.data (),
						     ipv6ad.size ());
		  std::get<2> (*ipv6it) = from6.sin6_port;
		}
	      else
		{
		  std::tuple<std::string, std::string, uint16_t, int> ttup;
		  std::vector<char> ipv6ad;
		  ipv6ad.resize (INET6_ADDRSTRLEN);
		  std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
					       ipv6ad.data (), ipv6ad.size ());
		  uint16_t port6 = from6.sin6_port;
		  ttup = std::make_tuple (key, ip6, port6, 1);
		  ipv6cont.push_back (ttup);
		}
	      ipv6contmtx.unlock ();
	      time_t ctm = time (NULL);
	      ipv6lrmtx.lock ();
	      auto itlr = std::find_if (ipv6lr.begin (), ipv6lr.end (), [&key]
	      (auto &el)
		{
		  return std::get<0>(el) == key;
		});
	      if (itlr != ipv6lr.end ())
		{
		  std::get<1> (*itlr) = ctm;
		}
	      else
		{
		  ipv6lr.push_back (std::make_tuple (key, ctm));
		}
	      ipv6lrmtx.unlock ();
	    }

	  result = 1;
	  if (msgtype == "TT")
	    {
	      std::cout << msgtype << std::endl;
	      smthrcvdsig.emit (key, crtm);
	    }
	  if (buf.size () >= 50 && (msgtype == "MB" || msgtype == "PB"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      uint64_t msgsz;
	      std::memcpy (&msgsz, &buf[42], sizeof(msgsz));
	      int chprexists = 0;
	      contmtx.lock ();
	      auto contit = std::find_if (contacts.begin (), contacts.end (),
					  [key]
					  (auto &el)
					    {
					      return std::get<1>(el) == key;
					    });
	      if (contit != contacts.end ())
		{
		  std::string indstr;
		  std::stringstream strm;
		  std::locale loc ("C");
		  strm.imbue (loc);
		  strm << std::get<0> (*contit);
		  indstr = strm.str ();
		  std::string filename;
		  std::filesystem::path filepath;
		  af.homePath (&filename);
		  if (msgtype == "PB")
		    {
		      filename = filename + "/.Communist/" + indstr
			  + "/Profile";
		    }
		  else
		    {
		      filename = filename + "/.Communist/" + indstr;
		      filepath = std::filesystem::u8path (filename);
		      std::vector<int> indv;
		      for (auto &dit : std::filesystem::directory_iterator (
			  filepath))
			{
			  std::filesystem::path tp = dit.path ();
			  if (tp.filename ().u8string () != "Profile"
			      && tp.filename ().u8string () != "Yes")
			    {
			      int vint;
			      strm.clear ();
			      strm.str ("");
			      strm.imbue (loc);
			      std::string fnm = tp.filename ().u8string ();
			      std::string::size_type n;
			      n = fnm.find ("f");
			      if (n != std::string::npos)
				{
				  fnm = fnm.substr (0, n);
				}
			      strm << fnm;
			      strm >> vint;
			      indv.push_back (vint);
			    }
			}
		      std::sort (indv.begin (), indv.end ());
		      if (indv.size () > 0)
			{
			  strm.clear ();
			  strm.str ("");
			  strm.imbue (loc);
			  strm << indv[indv.size () - 1];
			  filename = filename + "/" + strm.str ();
			}
		    }
		  filepath = std::filesystem::u8path (filename);
		  if (std::filesystem::exists (filepath)
		      && !std::filesystem::is_directory (filepath))
		    {
		      std::vector<char> hash;
		      std::copy (buf.begin () + 50, buf.end (),
				 std::back_inserter (hash));
		      std::vector<char> chhash = af.filehash (filepath);
		      if (hash == chhash)
			{
			  chprexists = 1;
			  std::vector<char> rpmsg;
			  std::array<char, 32> keyarr;
			  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
			  okp = lt::dht::ed25519_create_keypair (*seed);
			  keyarr = std::get<0> (okp).bytes;
			  std::copy (keyarr.begin (), keyarr.end (),
				     std::back_inserter (rpmsg));
			  std::string mt;
			  if (msgtype == "MB")
			    {
			      mt = "MR";
			    }
			  if (msgtype == "PB")
			    {
			      mt = "PR";
			    }
			  std::copy (mt.begin (), mt.end (),
				     std::back_inserter (rpmsg));
			  rpmsg.resize (rpmsg.size () + sizeof(tm));
			  std::memcpy (&rpmsg[34], &tm, sizeof(tm));
			  lt::dht::public_key othpk;
			  lt::aux::from_hex (key, othpk.bytes.data ());
			  std::array<char, 32> scalar;
			  scalar = lt::dht::ed25519_key_exchange (
			      othpk, std::get<1> (okp));
			  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
			  std::string passwd = lt::aux::to_hex (othpk.bytes);
			  rpmsg = af.cryptStrm (key, passwd, rpmsg);
			  if (rcvip6 == 0)
			    {
			      sendMsg (sockipv4, from->sin_addr.s_addr,
				       from->sin_port, rpmsg);
			    }
			  else
			    {
			      std::vector<char> ip6ad;
			      ip6ad.resize (INET6_ADDRSTRLEN);
			      std::string ip6 = inet_ntop (AF_INET6,
							   &from6.sin6_addr,
							   ip6ad.data (),
							   ip6ad.size ());
			      sockipv6mtx.lock ();
			      sendMsg6 (sockipv6, ip6, from6.sin6_port, rpmsg);
			      sockipv6mtx.unlock ();
			    }
			}
		    }
		}
	      contmtx.unlock ();
	      if (chprexists == 0)
		{
		  msghashmtx.lock ();
		  auto itmh = std::find_if (
		      msghash.begin (), msghash.end (), [&key, &tm]
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
		  if (itmh == msghash.end () && msgsz <= uint64_t (Maxmsgsz))
		    {
		      std::tuple<std::string, uint64_t, uint64_t,
			  std::vector<char>> ttup;
		      std::get<0> (ttup) = key;
		      std::get<1> (ttup) = tm;
		      std::get<2> (ttup) = msgsz;
		      std::vector<char> hash;
		      std::copy (buf.begin () + 50, buf.end (),
				 std::back_inserter (hash));
		      std::get<3> (ttup) = hash;
		      msghash.push_back (ttup);
		    }
		  msghashmtx.unlock ();

		  msgparthashmtx.lock ();
		  msgparthash.erase (
		      std::remove_if (
			  msgparthash.begin (),
			  msgparthash.end (),
			  [&key, &tm]
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
		      msgparthash.end ());
		  msgparthashmtx.unlock ();

		  msgpartrcvmtx.lock ();
		  msgpartrcv.erase (
		      std::remove_if (
			  msgpartrcv.begin (),
			  msgpartrcv.end (),
			  [&key, &tm]
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
		      msgpartrcv.end ());
		  msgpartrcvmtx.unlock ();

		  msgrcvdpnummtx.lock ();
		  msgrcvdpnum.erase (
		      std::remove_if (
			  msgrcvdpnum.begin (),
			  msgrcvdpnum.end (),
			  [key, tm]
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
		      msgrcvdpnum.end ());
		  msgrcvdpnummtx.unlock ();

		  std::vector<char> rpmsg;
		  std::array<char, 32> keyarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  keyarr = std::get<0> (okp).bytes;
		  std::copy (keyarr.begin (), keyarr.end (),
			     std::back_inserter (rpmsg));
		  std::string mt;
		  if (msgtype == "MB")
		    {
		      mt = "MA";
		    }
		  if (msgtype == "PB")
		    {
		      mt = "PA";
		    }

		  std::copy (mt.begin (), mt.end (),
			     std::back_inserter (rpmsg));
		  rpmsg.resize (rpmsg.size () + sizeof(tm));
		  std::memcpy (&rpmsg[34], &tm, sizeof(tm));
		  lt::dht::public_key othpk;
		  lt::aux::from_hex (key, othpk.bytes.data ());
		  std::array<char, 32> scalar;
		  scalar = lt::dht::ed25519_key_exchange (othpk,
							  std::get<1> (okp));
		  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		  std::string passwd = lt::aux::to_hex (othpk.bytes);
		  rpmsg = af.cryptStrm (key, passwd, rpmsg);
		  if (rcvip6 == 0)
		    {
		      sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			       rpmsg);
		    }
		  else
		    {
		      std::vector<char> ip6ad;
		      ip6ad.resize (INET6_ADDRSTRLEN);
		      std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
						   ip6ad.data (),
						   ip6ad.size ());
		      sockipv6mtx.lock ();
		      sendMsg6 (sockipv6, ip6, from6.sin6_port, rpmsg);
		      sockipv6mtx.unlock ();
		    }
		}
	    }

	  if (buf.size () >= 50 && (msgtype == "Mb" || msgtype == "Pb"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      uint64_t partnum;
	      std::memcpy (&partnum, &buf[42], sizeof(partnum));
	      msghashmtx.lock ();
	      auto itmh = std::find_if (
		  msghash.begin (), msghash.end (), [&key, &tm]
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
	      if (itmh != msghash.end ())
		{
		  msgparthashmtx.lock ();
		  auto itmph = std::find_if (
		      msgparthash.begin (), msgparthash.end (), [&key, &tm]
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
		  std::vector<char> hash;
		  std::copy (buf.begin () + 50, buf.end (),
			     std::back_inserter (hash));
		  if (itmph == msgparthash.end ())
		    {
		      std::tuple<std::string, uint64_t, std::vector<char>> ttup;
		      std::get<0> (ttup) = key;
		      std::get<1> (ttup) = tm;
		      std::get<2> (ttup) = hash;
		      msgparthash.push_back (ttup);
		    }
		  else
		    {
		      std::get<2> (*itmph) = hash;
		    }
		  msgparthashmtx.unlock ();
		}
	      msghashmtx.unlock ();
	    }

	  if (buf.size () >= 50 && (msgtype == "Mp" || msgtype == "Pp"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      uint64_t partnum;
	      std::memcpy (&partnum, &buf[42], sizeof(partnum));
	      msgparthashmtx.lock ();
	      auto itmph = std::find_if (
		  msgparthash.begin (), msgparthash.end (), [&key, &tm]
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
	      if (itmph != msgparthash.end ())
		{
		  msgpartrcvmtx.lock ();
		  auto itmpr = std::find_if (msgpartrcv.begin (),
					     msgpartrcv.end (),
					     [&key, &tm, &partnum]
					     (auto &el)
					       {
						 if (std::get<0>(el) == key &&
						     std::get<1>(el) == tm &&
						     std::get<2>(el) == partnum)
						   {
						     return true;
						   }
						 else
						   {
						     return false;
						   }
					       });
		  if (itmpr == msgpartrcv.end ())
		    {
		      std::vector<char> part;
		      std::copy (buf.begin () + 50, buf.end (),
				 std::back_inserter (part));
		      msgpartrcv.push_back (
			  std::make_tuple (key, tm, partnum, part));
		    }

		  msgpartrcvmtx.unlock ();
		}
	      msgparthashmtx.unlock ();
	    }

	  if (buf.size () >= 50 && (msgtype == "Me" || msgtype == "Pe"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      uint64_t partnum;
	      std::memcpy (&partnum, &buf[42], sizeof(partnum));

	      int check = 0;
	      if (msgtype == "Me")
		{
		  check = msgMe (key, tm, partnum);
		}
	      if (msgtype == "Pe")
		{
		  check = msgPe (key, tm, partnum);
		}
	      if (check == 1)
		{
		  std::vector<char> rpmsg;
		  std::array<char, 32> keyarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  keyarr = std::get<0> (okp).bytes;
		  std::copy (keyarr.begin (), keyarr.end (),
			     std::back_inserter (rpmsg));
		  std::string mt;
		  if (msgtype == "Me")
		    {
		      mt = "Mr";
		    }
		  if (msgtype == "Pe")
		    {
		      mt = "Pr";
		    }
		  std::copy (mt.begin (), mt.end (),
			     std::back_inserter (rpmsg));
		  rpmsg.resize (rpmsg.size () + sizeof(tm));
		  std::memcpy (&rpmsg[34], &tm, sizeof(tm));
		  rpmsg.resize (rpmsg.size () + sizeof(partnum));
		  std::memcpy (&rpmsg[42], &partnum, sizeof(partnum));
		  lt::dht::public_key othpk;
		  lt::aux::from_hex (key, othpk.bytes.data ());
		  std::array<char, 32> scalar;
		  scalar = lt::dht::ed25519_key_exchange (othpk,
							  std::get<1> (okp));
		  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		  std::string passwd = lt::aux::to_hex (othpk.bytes);
		  rpmsg = af.cryptStrm (key, passwd, rpmsg);
		  if (rcvip6 == 0)
		    {
		      sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			       rpmsg);
		    }
		  else
		    {
		      std::vector<char> ip6ad;
		      ip6ad.resize (INET6_ADDRSTRLEN);
		      std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
						   ip6ad.data (),
						   ip6ad.size ());
		      sockipv6mtx.lock ();
		      sendMsg6 (sockipv6, ip6, from6.sin6_port, rpmsg);
		      sockipv6mtx.unlock ();
		    }
		}
	    }

	  if (buf.size () >= 42 && (msgtype == "ME" || msgtype == "PE"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      int checkms = 0;
	      if (msgtype == "ME")
		{
		  checkms = msgME (key, tm);
		}
	      if (msgtype == "PE")
		{
		  checkms = msgPE (key, tm);
		}
	      if (checkms == 1)
		{
		  std::vector<char> rpmsg;
		  std::array<char, 32> keyarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  keyarr = std::get<0> (okp).bytes;
		  std::copy (keyarr.begin (), keyarr.end (),
			     std::back_inserter (rpmsg));
		  std::string mt;
		  if (msgtype == "ME")
		    {
		      mt = "MR";
		    }
		  if (msgtype == "PE")
		    {
		      mt = "PR";
		    }
		  std::copy (mt.begin (), mt.end (),
			     std::back_inserter (rpmsg));
		  rpmsg.resize (rpmsg.size () + sizeof(tm));
		  std::memcpy (&rpmsg[34], &tm, sizeof(tm));
		  lt::dht::public_key othpk;
		  lt::aux::from_hex (key, othpk.bytes.data ());
		  std::array<char, 32> scalar;
		  scalar = lt::dht::ed25519_key_exchange (othpk,
							  std::get<1> (okp));
		  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		  std::string passwd = lt::aux::to_hex (othpk.bytes);
		  rpmsg = af.cryptStrm (key, passwd, rpmsg);
		  if (rcvip6 == 0)
		    {
		      sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			       rpmsg);
		    }
		  else
		    {
		      std::vector<char> ip6ad;
		      ip6ad.resize (INET6_ADDRSTRLEN);
		      std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
						   ip6ad.data (),
						   ip6ad.size ());
		      sockipv6mtx.lock ();
		      sendMsg6 (sockipv6, ip6, from6.sin6_port, rpmsg);
		      sockipv6mtx.unlock ();
		    }
		}
	      else
		{
		  std::vector<char> rpmsg;
		  std::array<char, 32> keyarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  keyarr = std::get<0> (okp).bytes;
		  std::copy (keyarr.begin (), keyarr.end (),
			     std::back_inserter (rpmsg));
		  std::string mt;
		  if (msgtype == "ME")
		    {
		      mt = "MI";
		    }
		  if (msgtype == "PE")
		    {
		      mt = "PI";
		    }
		  std::copy (mt.begin (), mt.end (),
			     std::back_inserter (rpmsg));
		  rpmsg.resize (rpmsg.size () + sizeof(tm));
		  std::memcpy (&rpmsg[34], &tm, sizeof(tm));
		  lt::dht::public_key othpk;
		  lt::aux::from_hex (key, othpk.bytes.data ());
		  std::array<char, 32> scalar;
		  scalar = lt::dht::ed25519_key_exchange (othpk,
							  std::get<1> (okp));
		  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		  std::string passwd = lt::aux::to_hex (othpk.bytes);
		  rpmsg = af.cryptStrm (key, passwd, rpmsg);
		  if (rcvip6 == 0)
		    {
		      sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			       rpmsg);
		    }
		  else
		    {
		      std::vector<char> ip6ad;
		      ip6ad.resize (INET6_ADDRSTRLEN);
		      std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
						   ip6ad.data (),
						   ip6ad.size ());
		      sockipv6mtx.lock ();
		      sendMsg6 (sockipv6, ip6, from6.sin6_port, rpmsg);
		      sockipv6mtx.unlock ();
		    }
		}
	      msghashmtx.lock ();
	      msghash.erase (
		  std::remove_if (msghash.begin (), msghash.end (), [key, tm]
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
		  msghash.end ());
	      msghashmtx.unlock ();

	      msgparthashmtx.lock ();
	      msgparthash.erase (
		  std::remove_if (
		      msgparthash.begin (), msgparthash.end (), [key, tm]
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
		  msgparthash.end ());
	      msgparthashmtx.unlock ();

	      msgpartrcvmtx.lock ();
	      msgpartrcv.erase (
		  std::remove_if (
		      msgpartrcv.begin (), msgpartrcv.end (), [key, tm]
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
		  msgpartrcv.end ());
	      msgpartrcvmtx.unlock ();

	      msgrcvdpnummtx.lock ();
	      msgrcvdpnum.erase (
		  std::remove_if (
		      msgrcvdpnum.begin (), msgrcvdpnum.end (), [key, tm]
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
		  msgrcvdpnum.end ());
	      msgrcvdpnummtx.unlock ();
	    }
	  if (buf.size () >= 42 && (msgtype == "MA" || msgtype == "PA"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      msgpartbufmtx.lock ();
	      auto itmpb = std::find_if (
		  msgpartbuf.begin (), msgpartbuf.end (), [key, tm]
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
	      if (itmpb != msgpartbuf.end ())
		{
		  if (std::get<2> (*itmpb) == -1)
		    {
		      std::get<2> (*itmpb) = 0;
		    }
		}
	      msgpartbufmtx.unlock ();
	    }
	  if (buf.size () >= 50 && (msgtype == "Mr" || msgtype == "Pr"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      uint64_t partnum;
	      std::memcpy (&partnum, &buf[42], sizeof(partnum));
	      msgpartbufmtx.lock ();
	      auto itmpb = std::find_if (msgpartbuf.begin (), msgpartbuf.end (),
					 [key, tm, partnum]
					 (auto &el)
					   {
					     if (std::get<0>(el) == key
						 && std::get<1>(el) == tm
						 && std::get<5>(el) == partnum)
					       {
						 return true;
					       }
					     else
					       {
						 return false;
					       }
					   });
	      if (itmpb != msgpartbuf.end ())
		{
		  std::get<2> (*itmpb) = 2;
		}
	      msgpartbufmtx.unlock ();
	    }
	  if (buf.size () >= 42 && (msgtype == "MR" || msgtype == "PR"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      sendbufmtx.lock ();
	      msgpartbufmtx.lock ();
	      auto itmpb = std::find_if (
		  msgpartbuf.begin (), msgpartbuf.end (), [key, tm]
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
	      if (itmpb != msgpartbuf.end ())
		{
		  std::filesystem::path p = std::get<3> (*itmpb);
		  if (std::filesystem::exists (p))
		    {
		      std::filesystem::remove (p);
		    }
		  int ind;
		  std::string indstr = p.filename ().u8string ();
		  std::stringstream strm;
		  std::locale loc ("C");
		  strm.imbue (loc);
		  strm << indstr;
		  strm >> ind;
		  if (msgtype == "MR")
		    {
		      msgSent.emit (key, ind);
		    }
		  msgpartbuf.erase (itmpb);
		}
	      msgpartbufmtx.unlock ();
	      sendbufmtx.unlock ();
	    }
	  if (msgtype == "MI" || msgtype == "PI")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tm;
	      std::memcpy (&tm, &buf[34], sizeof(tm));
	      msgpartbufmtx.lock ();
	      msgpartbuf.erase (
		  std::remove_if (
		      msgpartbuf.begin (), msgpartbuf.end (), [key, tm]
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
		  msgpartbuf.end ());
	      msgpartbufmtx.unlock ();
	    }
	  if (buf.size () >= 50 && msgtype == "FQ")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t time;
	      std::memcpy (&time, &buf[34], sizeof(time));
	      uint64_t fsize;
	      std::memcpy (&fsize, &buf[42], sizeof(fsize));
	      std::vector<char> fnmv;
	      std::copy (buf.begin () + 50, buf.end (),
			 std::back_inserter (fnmv));
	      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	      okp = lt::dht::ed25519_create_keypair (*seed);
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
	      fqrcvdmtx.lock ();
	      std::tuple<std::string, time_t, std::string> fqtup;
	      fqtup = std::make_tuple (key, time, fnm);
	      auto itfqrcvd = std::find (fqrcvd.begin (), fqrcvd.end (), fqtup);
	      if (itfqrcvd == fqrcvd.end ())
		{
		  fqrcvd.push_back (fqtup);
		}
	      filerequest.emit (key, time, fsize, fnm);
	      fqrcvdmtx.unlock ();
	    }
	  if (buf.size () >= 42 && msgtype == "FJ")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      sendbufmtx.lock ();
	      filesendreqmtx.lock ();
	      auto itfsr = std::find_if (
		  filesendreq.begin (), filesendreq.end (), [&key, &tm]
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
	      if (itfsr != filesendreq.end ())
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
		  msgSent.emit (key, msgind);
		  fileRejected.emit (key);
		  filesendreq.erase (itfsr);
		}
	      filesendreqmtx.unlock ();
	      sendbufmtx.unlock ();
	    }
	  if (buf.size () >= 42 && msgtype == "FA")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      filesendreqmtx.lock ();
	      auto itfsr = std::find_if (
		  filesendreq.begin (), filesendreq.end (), [&key, &tm]
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
	      if (itfsr != filesendreq.end ())
		{
		  std::filesystem::path p = std::get<1> (*itfsr);
		  std::vector<char> frbuf;
		  filepartbufmtx.lock ();
		  auto it = std::find_if (
		      filepartbuf.begin (), filepartbuf.end (), [&key, &tm]
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
		  if (it == filepartbuf.end ())
		    {
		      filepartbuf.push_back (
			  std::make_tuple (key, tm, p, 0, 0, frbuf, 1));
		    }
		  filepartbufmtx.unlock ();
		  std::get<3> (*itfsr) = 1;
		}
	      filesendreqmtx.unlock ();
	    }
	  if (buf.size () >= 50 && msgtype == "Fr")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      uint64_t partnum;
	      std::memcpy (&partnum, &buf[42], sizeof(partnum));
	      filepartbufmtx.lock ();
	      auto itfpb = std::find_if (
		  filepartbuf.begin (), filepartbuf.end (),
		  [&key, &tm, &partnum]
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
	      if (itfpb != filepartbuf.end ())
		{
		  std::get<6> (*itfpb) = 1;
		  std::filesystem::path sentp = std::get<2> (*itfpb);
		  if (int (std::filesystem::file_size (sentp))
		      == std::get<3> (*itfpb))
		    {
		      std::vector<char> msg;
		      std::array<char, 32> okarr = std::get<0> (ownkey).bytes;
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
		      lt::aux::from_hex (chkey, othpk.bytes.data ());
		      std::string unms, passwds;
		      unms = chkey;
		      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		      passwds = lt::aux::to_hex (othpk.bytes);
		      msg = af.cryptStrm (unms, passwds, msg);
		      if (rcvip6 > 0)
			{
			  tmpmsg.resize (INET6_ADDRSTRLEN);
			  std::string sentadr = inet_ntop (AF_INET6,
							   &from6.sin6_addr,
							   tmpmsg.data (),
							   tmpmsg.size ());
			  sockipv6mtx.lock ();
			  sendMsg6 (sockipv6, sentadr, from6.sin6_port, msg);
			  sockipv6mtx.unlock ();
			}
		      else
			{
			  sendMsg (sockipv4, from->sin_addr.s_addr,
				   from->sin_port, msg);
			}
		    }
		  else
		    {
		      filepartsendsig.emit (key, std::get<2> (*itfpb),
					    uint64_t (std::get<3> (*itfpb)));
		    }
		}
	      filepartbufmtx.unlock ();
	    }
	  if (buf.size () >= 42 && (msgtype == "FR" || msgtype == "FI"))
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      sendbufmtx.lock ();
	      filesendreqmtx.lock ();
	      auto itfsr = std::find_if (
		  filesendreq.begin (), filesendreq.end (), [&key, &tm]
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
	      if (itfsr != filesendreq.end ())
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
		  msgSent.emit (key, msgind);
		  filesendreq.erase (itfsr);
		}
	      filesendreqmtx.unlock ();
	      sendbufmtx.unlock ();

	      filepartbufmtx.lock ();
	      auto itfpb = std::find_if (
		  filepartbuf.begin (), filepartbuf.end (), [&key, &tm]
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
	      if (itfpb != filepartbuf.end ())
		{
		  if (msgtype == "FR")
		    {
		      filecanceledmtx.lock ();
		      filecanceled.erase (
			  std::remove_if (
			      filecanceled.begin (),
			      filecanceled.end (),
			      [&key, &tm]
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
			  filecanceled.end ());
		      filecanceledmtx.unlock ();
		      filesentsig.emit (
			  key, std::get<2> (*itfpb).filename ().u8string ());
		    }
		  if (msgtype == "FI")
		    {
		      filecanceledmtx.lock ();
		      filecanceled.erase (
			  std::remove_if (
			      filecanceled.begin (),
			      filecanceled.end (),
			      [&key, &tm]
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
			  filecanceled.end ());
		      filecanceledmtx.unlock ();
		      std::string toemit =
			  std::get<2> (*itfpb).filename ().u8string ();
		      filesenterror.emit (key, toemit);
		    }
		  filepartbuf.erase (itfpb);
		}
	      filepartbufmtx.unlock ();
	    }
	  if (buf.size () >= 42 && msgtype == "FB")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      filehashvectmtx.lock ();
	      auto itfhv = std::find_if (
		  filehashvect.begin (), filehashvect.end (), [&key, &tm]
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
	      if (itfhv != filehashvect.end ())
		{
		  std::vector<char> fh;
		  std::copy (buf.begin () + 50, buf.end (),
			     std::back_inserter (fh));
		  std::get<2> (*itfhv) = fh;
		  std::array<char, 32> okarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  okarr = std::get<0> (okp).bytes;
		  std::vector<char> msg;
		  std::copy (okarr.begin (), okarr.end (),
			     std::back_inserter (msg));
		  std::string type = "FH";
		  std::copy (type.begin (), type.end (),
			     std::back_inserter (msg));
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
		  scalar = lt::dht::ed25519_key_exchange (passkey,
							  std::get<1> (okp));
		  passkey = lt::dht::ed25519_add_scalar (passkey, scalar);
		  std::string passwd = lt::aux::to_hex (passkey.bytes);
		  msg = af.cryptStrm (unm, passwd, msg);
		  if (rcvip6 > 0)
		    {
		      tmpmsg.clear ();
		      tmpmsg.resize (INET6_ADDRSTRLEN);
		      std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
						   tmpmsg.data (),
						   tmpmsg.size ());
		      sendMsg6 (sockipv4, ip6, from6.sin6_port, msg);
		    }
		  else
		    {
		      sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			       msg);
		    }
		}
	      filehashvectmtx.unlock ();
	    }
	  if (buf.size () >= 42 && msgtype == "FH")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      filepartbufmtx.lock ();
	      auto itfpb = std::find_if (
		  filepartbuf.begin (), filepartbuf.end (), [&key, &tm]
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
	      if (itfpb != filepartbuf.end ())
		{
		  fbrvectmtx.lock ();
		  auto itfbrv = std::find_if (
		      fbrvect.begin (), fbrvect.end (), [&key, &tm]
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
		  if (itfbrv == fbrvect.end ())
		    {
		      fbrvect.push_back (std::make_tuple (key, tm));
		    }
		  fbrvectmtx.unlock ();
		}
	      filepartbufmtx.unlock ();
	    }
	  if (buf.size () >= 50 && msgtype == "Fb")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      time_t tm = tint;
	      uint64_t partnum;
	      std::memcpy (&partnum, &buf[42], sizeof(partnum));
	      std::vector<char> hash;
	      std::copy (buf.begin () + 50, buf.end (),
			 std::back_inserter (hash));
	      filehashvectmtx.lock ();
	      auto itfhv = std::find_if (
		  filehashvect.begin (), filehashvect.end (), [&key, &tm]
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
	      if (itfhv != filehashvect.end ())
		{
		  fileparthashmtx.lock ();
		  fileparthash.erase (
		      std::remove_if (
			  fileparthash.begin (),
			  fileparthash.end (),
			  [&key, &tm]
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
		      fileparthash.end ());
		  fileparthash.push_back (
		      std::make_tuple (key, tm, partnum, hash));
		  fileparthashmtx.unlock ();

		  filepartrlogmtx.lock ();
		  filepartrlog.erase (
		      std::remove_if (
			  filepartrlog.begin (),
			  filepartrlog.end (),
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
		      filepartrlog.end ());
		  filepartrlog.push_back (std::make_tuple (key, tint, -1));
		  filepartrlogmtx.unlock ();

		  filepartrcvmtx.lock ();
		  filepartrcv.erase (
		      std::remove_if (
			  filepartrcv.begin (),
			  filepartrcv.end (),
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
		      filepartrcv.end ());
		  filepartrcvmtx.unlock ();

		  currentpartmtx.lock ();
		  currentpart.erase (
		      std::remove_if (
			  currentpart.begin (),
			  currentpart.end (),
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
		      currentpart.end ());
		  currentpartmtx.unlock ();

		  filepartendmtx.lock ();
		  filepartend.erase (
		      std::remove_if (
			  filepartend.begin (),
			  filepartend.end (),
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
		      filepartend.end ());
		  filepartendmtx.unlock ();
		}
	      else
		{
		  std::string type;
		  fqrcvdmtx.lock ();
		  auto itfqrcv = std::find_if (
		      fqrcvd.begin (), fqrcvd.end (), [&key, &tm]
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
		  if (itfqrcv != fqrcvd.end ())
		    {
		      type = "FF";
		    }
		  else
		    {
		      type = "FI";
		    }
		  fqrcvdmtx.unlock ();
		  std::array<char, 32> okarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  okarr = std::get<0> (okp).bytes;
		  std::vector<char> msg;
		  std::copy (okarr.begin (), okarr.end (),
			     std::back_inserter (msg));
		  std::copy (type.begin (), type.end (),
			     std::back_inserter (msg));
		  uint64_t lct = uint64_t (tm);
		  msg.resize (msg.size () + sizeof(lct));
		  std::memcpy (&msg[34], &lct, sizeof(lct));
		  std::string unm = key;
		  lt::dht::public_key passkey;
		  lt::aux::from_hex (unm, passkey.bytes.data ());
		  std::array<char, 32> scalar;
		  scalar = lt::dht::ed25519_key_exchange (passkey,
							  std::get<1> (okp));
		  passkey = lt::dht::ed25519_add_scalar (passkey, scalar);
		  std::string passwd = lt::aux::to_hex (passkey.bytes);
		  msg = af.cryptStrm (unm, passwd, msg);
		  if (rcvip6 > 0)
		    {
		      tmpmsg.clear ();
		      tmpmsg.resize (INET6_ADDRSTRLEN);
		      std::string ip6 = inet_ntop (AF_INET6, &from6.sin6_addr,
						   tmpmsg.data (),
						   tmpmsg.size ());
		      sendMsg6 (sockipv4, ip6, from6.sin6_port, msg);
		    }
		  else
		    {
		      sendMsg (sockipv4, from->sin_addr.s_addr, from->sin_port,
			       msg);
		    }
		}
	      filehashvectmtx.unlock ();
	    }
	  if (buf.size () >= 50 && msgtype == "Fp")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      uint64_t numb;
	      std::memcpy (&numb, &buf[42], sizeof(numb));
	      std::vector<char> data;
	      std::copy (buf.begin () + 50, buf.end (),
			 std::back_inserter (data));
	      filehashvectmtx.lock ();
	      auto itfhv =
		  std::find_if (
		      filehashvect.begin (),
		      filehashvect.end (),
		      [&key, &tint]
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
	      if (itfhv != filehashvect.end ())
		{
		  std::tuple<std::string, uint64_t, uint64_t, std::vector<char>> tempt;
		  tempt = std::make_tuple (key, tint, numb, data);
		  filepartrcvmtx.lock ();
		  auto itfpr = std::find (filepartrcv.begin (),
					  filepartrcv.end (), tempt);
		  if (itfpr == filepartrcv.end ())
		    {
		      filepartrcv.push_back (tempt);
		    }
		  filepartrcvmtx.unlock ();
		}
	      filehashvectmtx.unlock ();

	    }
	  if (buf.size () >= 42 && msgtype == "Fe")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      filehashvectmtx.lock ();
	      auto itfhv =
		  std::find_if (
		      filehashvect.begin (),
		      filehashvect.end (),
		      [&key, &tint]
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
	      if (itfhv != filehashvect.end ())
		{
		  std::tuple<std::string, uint64_t> tempt;
		  tempt = std::make_tuple (key, tint);
		  filepartendmtx.lock ();
		  auto itfpe = std::find (filepartend.begin (),
					  filepartend.end (), tempt);
		  if (itfpe == filepartend.end ())
		    {
		      filepartend.push_back (tempt);
		    }
		  filepartendmtx.unlock ();
		}
	      filehashvectmtx.unlock ();
	    }
	  if (buf.size () >= 42 && msgtype == "FE")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      fileendmtx.lock ();
	      auto itfev = std::find_if (
		  fileend.begin (), fileend.end (), [&key, &tint]
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
	      if (itfev == fileend.end ())
		{
		  fileend.push_back (std::make_tuple (key, tint));
		}
	      fileendmtx.unlock ();
	    }
	  if (buf.size () >= 42 && msgtype == "FF")
	    {
	      std::cout << msgtype << std::endl;
	      uint64_t tint;
	      std::memcpy (&tint, &buf[34], sizeof(tint));
	      filesendreqmtx.lock ();
	      filesendreq.erase (
		  std::remove_if (
		      filesendreq.begin (),
		      filesendreq.end (),
		      [&key, &tint]
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
		  filesendreq.end ());
	      filesendreqmtx.unlock ();

	      filepartbufmtx.lock ();
	      filepartbuf.erase (
		  std::remove_if (
		      filepartbuf.begin (),
		      filepartbuf.end (),
		      [&key, &tint]
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
		  filepartbuf.end ());
	      filepartbufmtx.unlock ();

	      fbrvectmtx.lock ();
	      fbrvect.erase (
		  std::remove_if (
		      fbrvect.begin (),
		      fbrvect.end (),
		      [&key, &tint]
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
		  fbrvect.end ());
	      fbrvectmtx.unlock ();
	    }
	}
    }

  if (msgtype == "Fp" || msgtype == "Fe" || msgtype == "FE")
    {

      contmtx.lock ();
      auto contit = std::find_if (contacts.begin (), contacts.end (), [&chkey]
      (auto &el)
	{
	  return std::get<1>(el) == chkey;
	});
      if (contit != contacts.end ())
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
	      filepartrcvmtx.lock ();
	      for (;;)
		{
		  auto itfpr = std::find_if (filepartrcv.begin (),
					     filepartrcv.end (), [&key]
					     (auto &el)
					       {
						 return std::get<0>(el) == key;
					       });
		  if (itfpr != filepartrcv.end ())
		    {
		      uint64_t tint = std::get<1> (*itfpr);
		      uint64_t rpnum = std::get<2> (*itfpr);
		      filepartrlogmtx.lock ();
		      auto itfprl =
			  std::find_if (
			      filepartrlog.begin (),
			      filepartrlog.end (),
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
		      if (itfprl != filepartrlog.end ())
			{
			  currentpartmtx.lock ();
			  auto itcpv =
			      std::find_if (
				  currentpart.begin (),
				  currentpart.end (),
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
			  if (itcpv == currentpart.end ())
			    {
			      currentpart.push_back (
				  std::make_tuple (key, tint, part));
			    }
			  else
			    {
			      std::vector<char> tv = std::get<2> (*itcpv);
			      std::copy (part.begin (), part.end (),
					 std::back_inserter (tv));
			      std::get<2> (*itcpv) = tv;
			    }
			  currentpartmtx.unlock ();
			  filepartrcv.erase (itfpr);
			  std::get<2> (*itfprl) = std::get<2> (*itfprl) + 1;
			}
		      else
			{
			  filepartrlogmtx.unlock ();
			  break;
			}
		      filepartrlogmtx.unlock ();
		    }
		  else
		    {
		      break;
		    }
		}
	      filepartrcvmtx.unlock ();
	    }

	  if (msgtype == "Fe")
	    {
	      filepartendmtx.lock ();
	      for (;;)
		{
		  auto itfpe = std::find_if (filepartend.begin (),
					     filepartend.end (), [&key]
					     (auto &el)
					       {
						 return std::get<0>(el) == key;
					       });
		  if (itfpe == filepartend.end ())
		    {
		      break;
		    }
		  else
		    {
		      uint64_t tint = std::get<1> (*itfpe);
		      fileparthashmtx.lock ();
		      auto itfph =
			  std::find_if (
			      fileparthash.begin (),
			      fileparthash.end (),
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
		      if (itfph == fileparthash.end ())
			{
			  filepartend.erase (itfpe);
			  fileparthashmtx.unlock ();
			  break;
			}
		      else
			{
			  std::vector<char> hash = std::get<3> (*itfph);
			  currentpartmtx.lock ();
			  auto itcp =
			      std::find_if (
				  currentpart.begin (),
				  currentpart.end (),
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
			  if (itcp == currentpart.end ())
			    {
			      filepartend.erase (itfpe);
			      fileparthash.erase (itfph);
			      currentpartmtx.unlock ();
			      fileparthashmtx.unlock ();
			      break;
			    }
			  else
			    {
			      std::vector<char> part = std::get<2> (*itcp);
			      std::vector<char> chhash;
			      chhash = af.strhash (part, 2);
			      if (chhash == hash)
				{
				  filehashvectmtx.lock ();
				  auto itfhv =
				      std::find_if (
					  filehashvect.begin (),
					  filehashvect.end (),
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
				  if (itfhv == filehashvect.end ())
				    {
				      filepartend.erase (itfpe);
				      fileparthash.erase (itfph);
				      currentpart.erase (itcp);
				      currentpartmtx.unlock ();
				      fileparthashmtx.unlock ();
				      filehashvectmtx.unlock ();
				      break;
				    }
				  else
				    {
				      if (int (std::get<2> (*itfph))
					  > std::get<4> (*itfhv))
					{
					  std::filesystem::path p =
					      std::get<3> (*itfhv);
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
					  filepartrcvdsig.emit (key, p, fcsz);
					  std::get<4> (*itfhv) = std::get<4> (
					      *itfhv) + 1;
					}

				      std::vector<char> msg;
				      std::tuple<lt::dht::public_key,
					  lt::dht::secret_key> okp;
				      okp = lt::dht::ed25519_create_keypair (
					  *seed);
				      std::array<char, 32> okarr = std::get<0> (
					  okp).bytes;
				      std::copy (okarr.begin (), okarr.end (),
						 std::back_inserter (msg));
				      std::string mtype = "Fr";
				      std::copy (mtype.begin (), mtype.end (),
						 std::back_inserter (msg));
				      msg.resize (msg.size () + sizeof(tint));
				      std::memcpy (&msg[34], &tint,
						   sizeof(tint));
				      uint64_t numb = std::get<2> (*itfph);
				      msg.resize (msg.size () + sizeof(numb));
				      std::memcpy (&msg[42], &numb,
						   sizeof(numb));
				      std::string uname = key;
				      lt::dht::public_key othpk;
				      lt::aux::from_hex (key,
							 othpk.bytes.data ());
				      std::array<char, 32> scalar;
				      scalar = lt::dht::ed25519_key_exchange (
					  othpk, std::get<1> (okp));
				      othpk = lt::dht::ed25519_add_scalar (
					  othpk, scalar);
				      std::string passwd = lt::aux::to_hex (
					  othpk.bytes);
				      msg = af.cryptStrm (uname, passwd, msg);
				      if (rcvip6 == 0)
					{
					  sendMsg (sockipv4,
						   from->sin_addr.s_addr,
						   from->sin_port, msg);
					}
				      else
					{
					  std::vector<char> ip6ad;
					  ip6ad.resize (INET6_ADDRSTRLEN);
					  std::string ip6 = inet_ntop (
					      AF_INET6, &from6.sin6_addr,
					      ip6ad.data (), ip6ad.size ());
					  sockipv6mtx.lock ();
					  sendMsg6 (sockipv6, ip6,
						    from6.sin6_port, msg);
					  sockipv6mtx.unlock ();
					}
				      filepartend.erase (itfpe);
				      fileparthash.erase (itfph);
				      currentpart.erase (itcp);
				    }
				  filehashvectmtx.unlock ();
				}
			      else
				{
				  std::cerr << "File part hash error!"
				      << std::endl;
				  filepartend.erase (itfpe);
				  fileparthash.erase (itfph);
				  currentpart.erase (itcp);
				}
			    }
			  currentpartmtx.unlock ();
			}
		      fileparthashmtx.unlock ();
		    }
		}
	      filepartendmtx.unlock ();
	    }

	  if (msgtype == "FE")
	    {
	      fileendmtx.lock ();
	      for (;;)
		{
		  auto itfev = std::find_if (fileend.begin (), fileend.end (),
					     [&key]
					     (auto &el)
					       {
						 return std::get<0>(el) == key;
					       });
		  if (itfev == fileend.end ())
		    {
		      break;
		    }
		  else
		    {
		      uint64_t tint = std::get<1> (*itfev);
		      filehashvectmtx.lock ();
		      auto itfhv =
			  std::find_if (
			      filehashvect.begin (),
			      filehashvect.end (),
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
		      if (itfhv == filehashvect.end ())
			{
			  std::vector<char> msg;
			  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
			  okp = lt::dht::ed25519_create_keypair (*seed);
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
			      sendMsg (sockipv4, from->sin_addr.s_addr,
				       from->sin_port, msg);
			    }
			  else
			    {
			      std::vector<char> ip6ad;
			      ip6ad.resize (INET6_ADDRSTRLEN);
			      std::string ip6 = inet_ntop (AF_INET6,
							   &from6.sin6_addr,
							   ip6ad.data (),
							   ip6ad.size ());
			      sockipv6mtx.lock ();
			      sendMsg6 (sockipv6, ip6, from6.sin6_port, msg);
			      sockipv6mtx.unlock ();
			    }
			  fileend.erase (itfev);
			  filehashvectmtx.unlock ();

			  fqrcvdmtx.lock ();
			  fqrcvd.erase (
			      std::remove_if (
				  fqrcvd.begin (),
				  fqrcvd.end (),
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
			      fqrcvd.end ());
			  fqrcvdmtx.unlock ();
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
			      std::tuple<lt::dht::public_key,
				  lt::dht::secret_key> okp;
			      okp = lt::dht::ed25519_create_keypair (*seed);
			      std::array<char, 32> okarr =
				  std::get<0> (okp).bytes;
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
			      othpk = lt::dht::ed25519_add_scalar (othpk,
								   scalar);
			      std::string passwd = lt::aux::to_hex (
				  othpk.bytes);
			      msg = af.cryptStrm (uname, passwd, msg);
			      if (rcvip6 == 0)
				{
				  sendMsg (sockipv4, from->sin_addr.s_addr,
					   from->sin_port, msg);
				}
			      else
				{
				  std::vector<char> ip6ad;
				  ip6ad.resize (INET6_ADDRSTRLEN);
				  std::string ip6 = inet_ntop (AF_INET6,
							       &from6.sin6_addr,
							       ip6ad.data (),
							       ip6ad.size ());
				  sockipv6mtx.lock ();
				  sendMsg6 (sockipv6, ip6, from6.sin6_port,
					    msg);
				  sockipv6mtx.unlock ();
				}
			      filercvd.emit (key, p.filename ().u8string ());
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
			      f.open (
				  filepath,
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
				      && tp.filename ().u8string ()
					  != "Profile")
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
			      if (!std::filesystem::exists (
				  outpath.parent_path ()))
				{
				  std::filesystem::create_directories (
				      outpath.parent_path ());
				}
			      af.cryptFile (unm, passwd, filepath.u8string (),
					    outpath.u8string ());
			      std::filesystem::remove (filepath);

			      fqrcvdmtx.lock ();
			      fqrcvd.erase (
				  std::remove_if (
				      fqrcvd.begin (),
				      fqrcvd.end (),
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
				  fqrcvd.end ());
			      fqrcvdmtx.unlock ();
			      messageReceived.emit (key, outpath);
			    }
			  else
			    {
			      std::filesystem::remove (p);
			      std::vector<char> msg;
			      std::tuple<lt::dht::public_key,
				  lt::dht::secret_key> okp;
			      okp = lt::dht::ed25519_create_keypair (*seed);
			      std::array<char, 32> okarr =
				  std::get<0> (okp).bytes;
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
			      othpk = lt::dht::ed25519_add_scalar (othpk,
								   scalar);
			      std::string passwd = lt::aux::to_hex (
				  othpk.bytes);
			      msg = af.cryptStrm (uname, passwd, msg);
			      if (rcvip6 == 0)
				{
				  sendMsg (sockipv4, from->sin_addr.s_addr,
					   from->sin_port, msg);
				}
			      else
				{
				  std::vector<char> ip6ad;
				  ip6ad.resize (INET6_ADDRSTRLEN);
				  std::string ip6 = inet_ntop (AF_INET6,
							       &from6.sin6_addr,
							       ip6ad.data (),
							       ip6ad.size ());
				  sockipv6mtx.lock ();
				  sendMsg6 (sockipv6, ip6, from6.sin6_port,
					    msg);
				  sockipv6mtx.unlock ();
				}
			      filehasherr.emit (key, p.filename ().u8string ());
			    }
			  filehashvect.erase (itfhv);

			  filepartrcvmtx.lock ();
			  filepartrcv.erase (
			      std::remove_if (
				  filepartrcv.begin (),
				  filepartrcv.end (),
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
			      filepartrcv.end ());
			  filepartrcvmtx.unlock ();

			  fileparthashmtx.lock ();
			  fileparthash.erase (
			      std::remove_if (
				  fileparthash.begin (),
				  fileparthash.end (),
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
			      fileparthash.end ());
			  fileparthashmtx.unlock ();

			  filepartrlogmtx.lock ();
			  filepartrlog.erase (
			      std::remove_if (
				  filepartrlog.begin (),
				  filepartrlog.end (),
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
			      filepartrlog.end ());
			  filepartrlogmtx.unlock ();

			  currentpartmtx.lock ();
			  currentpart.erase (
			      std::remove_if (
				  currentpart.begin (),
				  currentpart.end (),
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
			      currentpart.end ());
			  currentpartmtx.unlock ();

			  filepartendmtx.lock ();
			  filepartend.erase (
			      std::remove_if (
				  filepartend.begin (),
				  filepartend.end (),
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
			      filepartend.end ());
			  filepartendmtx.unlock ();

			  fqrcvdmtx.lock ();
			  fqrcvd.erase (
			      std::remove_if (
				  fqrcvd.begin (),
				  fqrcvd.end (),
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
			      fqrcvd.end ());
			  fqrcvdmtx.unlock ();

			  fileend.erase (itfev);
			}
		      filehashvectmtx.unlock ();
		    }
		}
	      fileendmtx.unlock ();
	    }
	}
      contmtx.unlock ();
    }
  return result;
}

int
NetworkOperations::sendMsg (int sockipv4, uint32_t ip, uint16_t port,
			    std::vector<char> &msg)
{
  sockaddr_in op =
    { };
  op.sin_family = AF_INET;
  op.sin_port = port;
  op.sin_addr.s_addr = ip;
  if (ip != 0 && port != 0)
    {
      int ch = sendto (sockipv4, msg.data (), msg.size (), 0,
		       (struct sockaddr*) &op, sizeof(op));
      if (ch < 0)
	{
#ifdef __linux
	  std::cerr << "ipv4 send error: " << strerror (errno) << std::endl;
#endif
#ifdef _WIN32
	  ch = WSAGetLastError ();
	  std::cerr << "ipv4 send error: " << ch << std::endl;
#endif
	}
      return ch;
    }
  else
    {
      return -1;
    }
}

int
NetworkOperations::sendMsg6 (int sock, std::string ip6, uint16_t port,
			     std::vector<char> &msg)
{
  sockaddr_in6 op =
    { };
  std::string ip6l = ip6;
  op.sin6_family = AF_INET6;
  op.sin6_port = port;
  inet_pton (AF_INET6, ip6l.c_str (), &op.sin6_addr);
  if (ip6 != "" && port != 0)
    {
      int ch = sendto (sock, msg.data (), msg.size (), 0,
		       (struct sockaddr*) &op, sizeof(op));
      if (ch < 0)
	{
#ifdef __linux
	  std::cerr << "ipv6 send error: " << strerror (errno) << std::endl;
#endif
#ifdef _WIN32
	  ch = WSAGetLastError ();
	  std::cerr << "ipv6 send error: " << ch << std::endl;
#endif
	}
      return ch;
    }
  else
    {
      return -1;
    }
}

void
NetworkOperations::getNewFriends (std::string key)
{
  AuxFunc af;
  std::string filename;
  af.homePath (&filename);
  filename = filename + "/.Communist/Profile";
  std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
  filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
  filename = filename + "/CommunistAddFr/Profile.zip";
  std::filesystem::path outpath = std::filesystem::u8path (filename);
  if (std::filesystem::exists (outpath.parent_path ()))
    {
      std::filesystem::remove_all (outpath.parent_path ());
    }
  std::filesystem::create_directories (outpath.parent_path ());
  af.decryptFile (Username, Password, source.u8string (), outpath.u8string ());
  af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
  std::filesystem::remove (outpath);
  filename = outpath.parent_path ().u8string ();
  filename = filename + "/Profile";
  source = std::filesystem::u8path (filename);
  if (std::filesystem::exists (source))
    {
      std::vector<std::filesystem::path> pv;
      for (auto &itdir : std::filesystem::directory_iterator (source))
	{
	  std::filesystem::path p = itdir.path ();
	  if (p.filename ().u8string () != "Avatar.jpeg"
	      && p.filename ().u8string () != "Profile")
	    {
	      pv.push_back (p);
	    }
	}
      for (size_t i = 0; i < pv.size (); i++)
	{
	  std::filesystem::remove (pv[i]);
	}
    }
  af.packing (source.u8string (), outpath.u8string ());
  source = outpath;
  af.homePath (&filename);
  filename = filename + "/.Communist/SendBufer/";
  contfullmtx.lock ();
  auto itcont = std::find_if (contactsfull.begin (), contactsfull.end (), [key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itcont == contactsfull.end ())
    {
      std::tuple<int, std::string> p;
      contsizech = contsizech + 1;
      std::get<0> (p) = contsizech;
      std::get<1> (p) = key;
      contactsfull.push_back (p);
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << std::get<0> (p);
      filename = filename + strm.str () + "/Profile";
      outpath = std::filesystem::u8path (filename);
      sendbufmtx.lock ();
      if (!std::filesystem::exists (outpath.parent_path ()))
	{
	  std::filesystem::create_directories (outpath.parent_path ());
	}
      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
      okp = lt::dht::ed25519_create_keypair (*seed);
      lt::dht::public_key opk;
      std::string unm = key;
      lt::aux::from_hex (unm, opk.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (opk, std::get<1> (okp));
      opk = lt::dht::ed25519_add_scalar (opk, scalar);
      std::string passwd = lt::aux::to_hex (opk.bytes);
      af.cryptFile (unm, passwd, source.u8string (), outpath.u8string ());
      sendbufmtx.unlock ();
      contmtx.lock ();
      auto itcontn = std::find_if (contacts.begin (), contacts.end (), [key]
      (auto &el)
	{
	  return std::get<1>(el) == key;
	});
      if (itcontn == contacts.end ())
	{
	  contacts.push_back (p);
	}
      contmtx.unlock ();
    }
  std::filesystem::remove_all (source.parent_path ());
  contfullmtx.unlock ();

  getfrmtx.lock ();
  auto gfrit = std::find (getfr.begin (), getfr.end (), key);
  if (gfrit == getfr.end ())
    {
      getfr.push_back (key);
    }
  getfrmtx.unlock ();
  sockmtx.lock ();
  int ss = 0;
  auto itsock = std::find_if (sockets4.begin (), sockets4.end (), [key]
  (auto &el)
    { return std::get<0>(el) == key;});
  if (itsock == sockets4.end ())
    {
#ifdef __linux
      int sock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
      int sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      u_long nonblocking_enabled = TRUE;
      ioctlsocket (sock, FIONBIO, &nonblocking_enabled);
#endif

      sockaddr_in addripv4 =
	{ };
      addripv4.sin_family = AF_INET;
      addripv4.sin_addr.s_addr = INADDR_ANY;
      addripv4.sin_port = 0;
      int addrlen1 = sizeof(addripv4);
      bind (sock, (const sockaddr*) &addripv4, addrlen1);
      std::shared_ptr<std::mutex> mtx (new std::mutex);
      std::shared_ptr<std::mutex> mtxgip (new std::mutex);
      time_t tm = time (NULL);
      sockets4.push_back (std::make_tuple (key, sock, mtx, tm, mtxgip));
    }
  if (sockets4.size () == 1)
    {
      ss = 1;
    }
  sockmtx.unlock ();
  if (ss == 1)
    {
      std::mutex *thrmtx = new std::mutex;
      thrmtx->lock ();
      threadvectmtx.lock ();
      threadvect.push_back (std::make_tuple (thrmtx, "Get new friend"));
      threadvectmtx.unlock ();
      std::thread *thr = new std::thread ( [this, thrmtx]
      {
	if (this->copsrun.try_lock ())
	  {
	    this->commOps ();
	    this->copsrun.unlock ();
	  }
	thrmtx->unlock ();
      });
      thr->detach ();
      delete thr;
    }
}

int
NetworkOperations::sendMsgGlob (int sock, std::string keytos, uint32_t ip,
				uint16_t port)
{
  int result = 0;
  AuxFunc af;
  std::string key = keytos;
  contmtx.lock ();
  auto itcont = std::find_if (contacts.begin (), contacts.end (), [&key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itcont != contacts.end ())
    {
      std::string index;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << std::get<0> (*itcont);
      index = strm.str ();
      std::string filename;
      af.homePath (&filename);
      filename = filename + "/.Communist/SendBufer/" + index;
      std::filesystem::path filepath = std::filesystem::u8path (filename);

      if (std::filesystem::exists (filepath)
	  && std::filesystem::is_directory (filepath))
	{
	  filecanceledmtx.lock ();
	  for (size_t i = 0; i < filecanceled.size (); i++)
	    {
	      if (std::get<0> (filecanceled[i]) == key)
		{
		  std::vector<char> msg;
		  std::array<char, 32> okarr;
		  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		  okp = lt::dht::ed25519_create_keypair (*seed);
		  lt::dht::public_key othpk;
		  lt::aux::from_hex (keytos, othpk.bytes.data ());
		  std::array<char, 32> scalar = lt::dht::ed25519_key_exchange (
		      othpk, std::get<1> (okp));
		  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		  std::string unm = keytos;
		  std::string passwd = lt::aux::to_hex (othpk.bytes);
		  okarr = std::get<0> (okp).bytes;
		  std::copy (okarr.begin (), okarr.end (),
			     std::back_inserter (msg));
		  std::string msgtype;
		  msgtype = "FE";
		  std::copy (msgtype.begin (), msgtype.end (),
			     std::back_inserter (msg));
		  uint64_t tmfb = std::get<1> (filecanceled[i]);
		  msg.resize (msg.size () + sizeof(tmfb));
		  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
		  msg = af.cryptStrm (unm, passwd, msg);
		  time_t curtime = time (NULL);
		  int sent = 0;
		  ipv6lrmtx.lock ();
		  auto itlr6 = std::find_if (
		      ipv6lr.begin (), ipv6lr.end (), [keytos]
		      (auto &el)
			{
			  return std::get<0>(el) == keytos;
			});
		  if (itlr6 != ipv6lr.end ())
		    {
		      if (curtime - std::get<1> (*itlr6) <= Tmttear)
			{
			  ipv6contmtx.lock ();
			  auto itip6 = std::find_if (
			      ipv6cont.begin (), ipv6cont.end (), [keytos]
			      (auto &el)
				{
				  return std::get<0>(el) == keytos;
				});
			  if (itip6 != ipv6cont.end ())
			    {
			      std::string ipv6 = std::get<1> (*itip6);
			      uint16_t port = std::get<2> (*itip6);
			      sockipv6mtx.lock ();
			      sent = sendMsg6 (sockipv6, ipv6, port, msg);
			      sockipv6mtx.unlock ();
			      result = 1;
			    }
			  ipv6contmtx.unlock ();
			}
		    }
		  ipv6lrmtx.unlock ();
		  if (sent <= 0)
		    {
		      sent = sendMsg (sock, ip, port, msg);
		      result = 1;
		    }
		}
	    }
	  filecanceledmtx.unlock ();
	  std::vector<std::filesystem::path> pvect;
	  sendbufmtx.lock ();
	  for (auto &dirit : std::filesystem::directory_iterator (filepath))
	    {
	      std::filesystem::path p = dirit.path ();
	      pvect.push_back (p);
	    }
	  int totalsent = 0;
	  for (size_t i = 0; i < pvect.size (); i++)
	    {
	      int variant = 0;
	      if (pvect[i].filename ().u8string () == "Profile")
		{
		  variant = 1; //Profile
		}
	      else
		{
		  std::string pathstr = pvect[i].filename ().u8string ();
		  std::string::size_type n;
		  n = pathstr.find ("f");
		  if (n == std::string::npos)
		    {
		      variant = 2; //msg
		    }
		  else
		    {
		      variant = 3; //file
		    }
		}
	      if (variant == 1 || variant == 2)
		{
		  std::filesystem::path ptos = pvect[i];
		  size_t vind;
		  msgpartbufmtx.lock ();
		  auto itmpb = std::find_if (
		      msgpartbuf.begin (), msgpartbuf.end (), [&key, &ptos]
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
		  if (itmpb == msgpartbuf.end ())
		    {
		      std::tuple<std::string, uint64_t, int,
			  std::filesystem::path, int, uint64_t,
			  std::vector<char>> ttup;
		      std::get<0> (ttup) = key;
		      time_t tmt = time (NULL);
		      for (;;)
			{
			  auto ittmt = std::find_if (
			      msgpartbuf.begin (), msgpartbuf.end (),
			      [&key, &tmt]
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
			  if (ittmt != msgpartbuf.end ())
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
		      msgpartbuf.push_back (ttup);
		      vind = msgpartbuf.size () - 1;
		    }
		  else
		    {
		      vind = std::distance (msgpartbuf.begin (), itmpb);
		    }
		  int sentstat = std::get<2> (msgpartbuf[vind]);
		  int byteread = std::get<4> (msgpartbuf[vind]);
		  int fsz = std::filesystem::file_size (ptos);
		  if (sentstat == -1)
		    {
		      std::vector<char> msg;
		      std::array<char, 32> okarr;
		      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		      okp = lt::dht::ed25519_create_keypair (*seed);
		      lt::dht::public_key othpk;
		      lt::aux::from_hex (keytos, othpk.bytes.data ());
		      std::array<char, 32> scalar =
			  lt::dht::ed25519_key_exchange (othpk,
							 std::get<1> (okp));
		      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		      std::string unm = keytos;
		      std::string passwd = lt::aux::to_hex (othpk.bytes);
		      okarr = std::get<0> (okp).bytes;
		      std::copy (okarr.begin (), okarr.end (),
				 std::back_inserter (msg));
		      std::string msgtype;
		      if (variant == 1)
			{
			  msgtype = "PB";
			}
		      if (variant == 2)
			{
			  msgtype = "MB";
			}

		      std::copy (msgtype.begin (), msgtype.end (),
				 std::back_inserter (msg));
		      uint64_t tmfb = std::get<1> (msgpartbuf[vind]);
		      msg.resize (msg.size () + sizeof(tmfb));
		      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
		      uint64_t msgn = uint64_t (
			  std::filesystem::file_size (
			      std::get<3> (msgpartbuf[vind])));
		      msg.resize (msg.size () + sizeof(msgn));
		      std::memcpy (&msg[42], &msgn, sizeof(msgn));
		      std::vector<char> fh = af.filehash (
			  std::get<3> (msgpartbuf[vind]));
		      std::copy (fh.begin (), fh.end (),
				 std::back_inserter (msg));
		      msg = af.cryptStrm (unm, passwd, msg);
		      time_t curtime = time (NULL);
		      int sent = 0;
		      ipv6lrmtx.lock ();
		      auto itlr6 = std::find_if (
			  ipv6lr.begin (), ipv6lr.end (), [keytos]
			  (auto &el)
			    {
			      return std::get<0>(el) == keytos;
			    });
		      if (itlr6 != ipv6lr.end ())
			{
			  if (curtime - std::get<1> (*itlr6) <= Tmttear)
			    {
			      ipv6contmtx.lock ();
			      auto itip6 = std::find_if (
				  ipv6cont.begin (), ipv6cont.end (), [keytos]
				  (auto &el)
				    {
				      return std::get<0>(el) == keytos;
				    });
			      if (itip6 != ipv6cont.end ())
				{
				  std::string ipv6 = std::get<1> (*itip6);
				  uint16_t port = std::get<2> (*itip6);
				  sockipv6mtx.lock ();
				  sent = sendMsg6 (sockipv6, ipv6, port, msg);
				  sockipv6mtx.unlock ();
				  result = 1;
				}
			      ipv6contmtx.unlock ();
			    }
			}
		      ipv6lrmtx.unlock ();
		      if (sent <= 0)
			{
			  sent = sendMsg (sock, ip, port, msg);
			  result = 1;
			}
		    }
		  std::vector<char> msgpart;
		  if (sentstat == 0 || sentstat == 2)
		    {
		      if (byteread < fsz)
			{
			  if ((fsz - byteread) > Partsize)
			    {
			      msgpart.resize (Partsize);
			    }
			  else
			    {
			      msgpart.resize (fsz - byteread);
			    }

			  std::fstream f;
			  f.open (ptos,
				  std::ios_base::in | std::ios_base::binary);
			  f.seekg (byteread, std::ios_base::beg);
			  f.read (&msgpart[0], msgpart.size ());
			  f.close ();
			  std::get<6> (msgpartbuf[vind]) = msgpart;
			  byteread = byteread + msgpart.size ();
			  std::get<4> (msgpartbuf[vind]) = byteread;
			  std::get<2> (msgpartbuf[vind]) = 1;
			  if (sentstat == 2)
			    {
			      std::get<5> (msgpartbuf[vind]) = std::get<5> (
				  msgpartbuf[vind]) + 1;
			    }
			  sentstat = 1;
			}
		      else
			{
			  std::get<2> (msgpartbuf[vind]) = 2;
			  sentstat = 2;
			}

		    }

		  if (sentstat == 1)
		    {
		      msgpart.clear ();
		      msgpart = std::get<6> (msgpartbuf[vind]);
		      if (msgpart.size () > 0)
			{
			  std::vector<char> msg;
			  std::array<char, 32> okarr;
			  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
			  okp = lt::dht::ed25519_create_keypair (*seed);
			  lt::dht::public_key othpk;
			  lt::aux::from_hex (keytos, othpk.bytes.data ());
			  std::array<char, 32> scalar =
			      lt::dht::ed25519_key_exchange (othpk,
							     std::get<1> (okp));
			  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
			  std::string unm = keytos;
			  std::string passwd = lt::aux::to_hex (othpk.bytes);
			  okarr = std::get<0> (okp).bytes;
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
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
			  uint64_t tmfb = std::get<1> (msgpartbuf[vind]);
			  msg.resize (msg.size () + sizeof(tmfb));
			  std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
			  uint64_t msgn = std::get<5> (msgpartbuf[vind]);
			  msg.resize (msg.size () + sizeof(msgn));
			  std::memcpy (&msg[42], &msgn, sizeof(msgn));
			  std::vector<char> fh = af.strhash (msgpart, 2);
			  std::copy (fh.begin (), fh.end (),
				     std::back_inserter (msg));
			  msg = af.cryptStrm (unm, passwd, msg);
			  time_t curtime = time (NULL);
			  int sent = 0;
			  ipv6lrmtx.lock ();
			  auto itlr6 = std::find_if (
			      ipv6lr.begin (), ipv6lr.end (), [keytos]
			      (auto &el)
				{
				  return std::get<0>(el) == keytos;
				});
			  if (itlr6 != ipv6lr.end ())
			    {
			      if (curtime - std::get<1> (*itlr6) <= Tmttear)
				{
				  ipv6contmtx.lock ();
				  auto itip6 = std::find_if (
				      ipv6cont.begin (), ipv6cont.end (),
				      [keytos]
				      (auto &el)
					{
					  return std::get<0>(el) == keytos;
					});
				  if (itip6 != ipv6cont.end ())
				    {
				      std::string ipv6 = std::get<1> (*itip6);
				      uint16_t port = std::get<2> (*itip6);
				      sockipv6mtx.lock ();
				      sent = sendMsg6 (sockipv6, ipv6, port,
						       msg);
				      sockipv6mtx.unlock ();
				      result = 1;
				    }
				  ipv6contmtx.unlock ();
				}
			    }
			  ipv6lrmtx.unlock ();
			  if (sent <= 0)
			    {
			      sent = sendMsg (sock, ip, port, msg);
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
				  std::copy (msgpart.begin () + mcount,
					     msgpart.end (),
					     std::back_inserter (msg));
				  mcount = msgpart.size ();
				}
			      msg = af.cryptStrm (unm, passwd, msg);
			      sent = 0;
			      ipv6lrmtx.lock ();
			      itlr6 = std::find_if (
				  ipv6lr.begin (), ipv6lr.end (), [keytos]
				  (auto &el)
				    {
				      return std::get<0>(el) == keytos;
				    });
			      if (itlr6 != ipv6lr.end ())
				{
				  if (curtime - std::get<1> (*itlr6) <= Tmttear)
				    {
				      ipv6contmtx.lock ();
				      auto itip6 = std::find_if (
					  ipv6cont.begin (), ipv6cont.end (),
					  [keytos]
					  (auto &el)
					    {
					      return std::get<0>(el) == keytos;
					    });
				      if (itip6 != ipv6cont.end ())
					{
					  std::string ipv6 = std::get<1> (
					      *itip6);
					  uint16_t port = std::get<2> (*itip6);
					  sockipv6mtx.lock ();
					  sent = sendMsg6 (sockipv6, ipv6, port,
							   msg);
					  sockipv6mtx.unlock ();
					  result = 1;
					}
				      ipv6contmtx.unlock ();
				    }
				}
			      ipv6lrmtx.unlock ();
			      if (sent <= 0)
				{
				  sent = sendMsg (sock, ip, port, msg);
				  result = 1;
				}

			      msgn = msgn + 1;
			    }
			  msg.clear ();
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
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
			  msgn = std::get<5> (msgpartbuf[vind]);
			  msg.resize (msg.size () + sizeof(msgn));
			  std::memcpy (&msg[42], &msgn, sizeof(msgn));
			  msg = af.cryptStrm (unm, passwd, msg);
			  sent = 0;
			  ipv6lrmtx.lock ();
			  itlr6 = std::find_if (
			      ipv6lr.begin (), ipv6lr.end (), [keytos]
			      (auto &el)
				{
				  return std::get<0>(el) == keytos;
				});
			  if (itlr6 != ipv6lr.end ())
			    {
			      if (curtime - std::get<1> (*itlr6) <= Tmttear)
				{
				  ipv6contmtx.lock ();
				  auto itip6 = std::find_if (
				      ipv6cont.begin (), ipv6cont.end (),
				      [keytos]
				      (auto &el)
					{
					  return std::get<0>(el) == keytos;
					});
				  if (itip6 != ipv6cont.end ())
				    {
				      std::string ipv6 = std::get<1> (*itip6);
				      uint16_t port = std::get<2> (*itip6);
				      sockipv6mtx.lock ();
				      sent = sendMsg6 (sockipv6, ipv6, port,
						       msg);
				      sockipv6mtx.unlock ();
				      result = 1;
				    }
				  ipv6contmtx.unlock ();
				}
			    }
			  ipv6lrmtx.unlock ();
			  if (sent <= 0)
			    {
			      sent = sendMsg (sock, ip, port, msg);
			      result = 1;
			    }
			  std::get<2> (msgpartbuf[vind]) = 1;
			}
		    }
		  if (byteread >= fsz && sentstat == 2)
		    {
		      std::vector<char> msg;
		      std::array<char, 32> okarr;
		      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		      okp = lt::dht::ed25519_create_keypair (*seed);
		      lt::dht::public_key othpk;
		      lt::aux::from_hex (keytos, othpk.bytes.data ());
		      std::array<char, 32> scalar =
			  lt::dht::ed25519_key_exchange (othpk,
							 std::get<1> (okp));
		      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		      std::string unm = keytos;
		      std::string passwd = lt::aux::to_hex (othpk.bytes);
		      okarr = std::get<0> (okp).bytes;
		      std::copy (okarr.begin (), okarr.end (),
				 std::back_inserter (msg));
		      std::string msgtype;
		      if (variant == 1)
			{
			  msgtype = "PE";
			}
		      if (variant == 2)
			{
			  msgtype = "ME";
			}
		      std::copy (msgtype.begin (), msgtype.end (),
				 std::back_inserter (msg));
		      uint64_t tmfb = std::get<1> (msgpartbuf[vind]);
		      msg.resize (msg.size () + sizeof(tmfb));
		      std::memcpy (&msg[34], &tmfb, sizeof(tmfb));
		      uint64_t msgn = 0;
		      msg.resize (msg.size () + sizeof(msgn));
		      std::memcpy (&msg[42], &msgn, sizeof(msgn));
		      msg = af.cryptStrm (unm, passwd, msg);
		      time_t curtime = time (NULL);
		      int sent = 0;
		      ipv6lrmtx.lock ();
		      auto itlr6 = std::find_if (
			  ipv6lr.begin (), ipv6lr.end (), [keytos]
			  (auto &el)
			    {
			      return std::get<0>(el) == keytos;
			    });
		      if (itlr6 != ipv6lr.end ())
			{
			  if (curtime - std::get<1> (*itlr6) <= Tmttear)
			    {
			      ipv6contmtx.lock ();
			      auto itip6 = std::find_if (
				  ipv6cont.begin (), ipv6cont.end (), [keytos]
				  (auto &el)
				    {
				      return std::get<0>(el) == keytos;
				    });
			      if (itip6 != ipv6cont.end ())
				{
				  std::string ipv6 = std::get<1> (*itip6);
				  uint16_t port = std::get<2> (*itip6);
				  sockipv6mtx.lock ();
				  sent = sendMsg6 (sockipv6, ipv6, port, msg);
				  sockipv6mtx.unlock ();
				  result = 1;
				}
			      ipv6contmtx.unlock ();
			    }
			}
		      ipv6lrmtx.unlock ();
		      if (sent <= 0)
			{
			  sent = sendMsg (sock, ip, port, msg);
			  result = 1;
			}
		    }

		  msgpartbufmtx.unlock ();
		}
	      else
		{
		  if (variant == 3)
		    {
		      std::filesystem::path ptos = pvect[i];
		      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
		      okp = lt::dht::ed25519_create_keypair (*seed);
		      std::array<char, 32> okarr = std::get<0> (okp).bytes;
		      lt::dht::public_key othpk;
		      lt::aux::from_hex (keytos, othpk.bytes.data ());
		      std::array<char, 32> scalar =
			  lt::dht::ed25519_key_exchange (othpk,
							 std::get<1> (okp));
		      othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
		      std::string unm = keytos;
		      std::string passwd = lt::aux::to_hex (othpk.bytes);
		      std::filesystem::path source = ptos;
#ifdef __linux
		      std::string filename =
			  std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
		      std::string filename =
			  std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
		      filename = filename + "/CommunistSF/file";
		      std::filesystem::path outpath = std::filesystem::u8path (
			  filename);
		      if (std::filesystem::exists (outpath.parent_path ()))
			{
			  std::filesystem::remove_all (outpath.parent_path ());
			}
		      std::filesystem::create_directories (
			  outpath.parent_path ());
		      af.decryptFile (unm, passwd, source.u8string (),
				      outpath.u8string ());
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
		      filesendreqmtx.lock ();
		      auto itfsr =
			  std::find_if (
			      filesendreq.begin (),
			      filesendreq.end (),
			      [keytos, &ptos]
			      (auto &el)
				{
				  if (std::get<0>(el) == keytos && std::get<1>(el) == ptos)
				    {
				      return true;
				    }
				  else
				    {
				      return false;
				    }
				});
		      if (itfsr == filesendreq.end ())
			{
			  time_t curtime = time (NULL);
			  int status = 0;
			  filesendreq.push_back (
			      std::make_tuple (keytos, ptos, curtime, status,
					       pvect[i]));
			  std::vector<char> msg;
			  std::copy (okarr.begin (), okarr.end (),
				     std::back_inserter (msg));
			  std::string type = "FQ";
			  std::copy (type.begin (), type.end (),
				     std::back_inserter (msg));
			  uint64_t tmt = curtime;
			  msg.resize (msg.size () + sizeof(tmt));
			  std::memcpy (&msg[34], &tmt, sizeof(tmt));
			  uint64_t fsize = std::filesystem::file_size (ptos);
			  msg.resize (msg.size () + sizeof(fsize));
			  std::memcpy (&msg[42], &fsize, sizeof(fsize));
			  std::vector<char> input;
			  std::string fnm = ptos.filename ().u8string ();
			  std::copy (fnm.begin (), fnm.end (),
				     std::back_inserter (input));
			  if (input.size () < 16)
			    {
			      std::string add = "\\12356789012345";
			      std::copy (add.begin (), add.end (),
					 std::back_inserter (input));
			    }
			  std::vector<char> output;
			  output = af.cryptStrm (unm, passwd, input);
			  std::copy (output.begin (), output.end (),
				     std::back_inserter (msg));
			  std::string unm = key;
			  lt::dht::public_key othpk;
			  lt::aux::from_hex (unm, othpk.bytes.data ());
			  std::array<char, 32> scalar;
			  scalar = lt::dht::ed25519_key_exchange (
			      othpk, std::get<1> (okp));
			  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
			  std::string passwd = lt::aux::to_hex (othpk.bytes);
			  msg = af.cryptStrm (unm, passwd, msg);
			  int sent = 0;
			  ipv6lrmtx.lock ();
			  auto itlr6 = std::find_if (
			      ipv6lr.begin (), ipv6lr.end (), [keytos]
			      (auto &el)
				{
				  return std::get<0>(el) == keytos;
				});
			  if (itlr6 != ipv6lr.end ())
			    {
			      if (curtime - std::get<1> (*itlr6) <= Tmttear)
				{
				  ipv6contmtx.lock ();
				  auto itip6 = std::find_if (
				      ipv6cont.begin (), ipv6cont.end (),
				      [keytos]
				      (auto &el)
					{
					  return std::get<0>(el) == keytos;
					});
				  if (itip6 != ipv6cont.end ())
				    {
				      std::string ipv6 = std::get<1> (*itip6);
				      uint16_t port = std::get<2> (*itip6);
				      sockipv6mtx.lock ();
				      sent = sendMsg6 (sockipv6, ipv6, port,
						       msg);
				      sockipv6mtx.unlock ();
				    }
				  ipv6contmtx.unlock ();
				}
			    }
			  ipv6lrmtx.unlock ();
			  if (sent <= 0)
			    {
			      sent = sendMsg (sock, ip, port, msg);
			    }
			}
		      else
			{
			  int status = std::get<3> (*itfsr);
			  if (status == 0)
			    {
			      time_t curtime = time (NULL);
			      std::vector<char> msg;
			      std::copy (okarr.begin (), okarr.end (),
					 std::back_inserter (msg));
			      std::string type = "FQ";
			      std::copy (type.begin (), type.end (),
					 std::back_inserter (msg));
			      uint64_t tmt = std::get<2> (*itfsr);
			      msg.resize (msg.size () + sizeof(tmt));
			      std::memcpy (&msg[34], &tmt, sizeof(tmt));
			      uint64_t fsize = std::filesystem::file_size (
				  ptos);
			      msg.resize (msg.size () + sizeof(fsize));
			      std::memcpy (&msg[42], &fsize, sizeof(fsize));
			      std::vector<char> input;
			      std::string fnm = ptos.filename ().u8string ();
			      std::copy (fnm.begin (), fnm.end (),
					 std::back_inserter (input));
			      if (input.size () < 16)
				{
				  std::string add = "\\12356789012345";
				  std::copy (add.begin (), add.end (),
					     std::back_inserter (input));
				}
			      std::vector<char> output;
			      output = af.cryptStrm (unm, passwd, input);
			      std::copy (output.begin (), output.end (),
					 std::back_inserter (msg));
			      std::string unm = key;
			      lt::dht::public_key othpk;
			      lt::aux::from_hex (unm, othpk.bytes.data ());
			      std::array<char, 32> scalar;
			      scalar = lt::dht::ed25519_key_exchange (
				  othpk, std::get<1> (okp));
			      othpk = lt::dht::ed25519_add_scalar (othpk,
								   scalar);
			      std::string passwd = lt::aux::to_hex (
				  othpk.bytes);
			      msg = af.cryptStrm (unm, passwd, msg);
			      int sent = 0;
			      ipv6lrmtx.lock ();
			      auto itlr6 = std::find_if (
				  ipv6lr.begin (), ipv6lr.end (), [keytos]
				  (auto &el)
				    {
				      return std::get<0>(el) == keytos;
				    });
			      if (itlr6 != ipv6lr.end ())
				{
				  if (curtime - std::get<1> (*itlr6) <= Tmttear)
				    {
				      ipv6contmtx.lock ();
				      auto itip6 = std::find_if (
					  ipv6cont.begin (), ipv6cont.end (),
					  [keytos]
					  (auto &el)
					    {
					      return std::get<0>(el) == keytos;
					    });
				      if (itip6 != ipv6cont.end ())
					{
					  std::string ipv6 = std::get<1> (
					      *itip6);
					  uint16_t port = std::get<2> (*itip6);
					  sockipv6mtx.lock ();
					  sent = sendMsg6 (sockipv6, ipv6, port,
							   msg);
					  sockipv6mtx.unlock ();
					  result = 1;
					}
				      ipv6contmtx.unlock ();
				    }
				}
			      ipv6lrmtx.unlock ();
			      if (sent <= 0)
				{
				  sent = sendMsg (sock, ip, port, msg);
				  result = 1;
				}
			    }
			  if (status == 1)
			    {
			      time_t curtime = time (NULL);
			      std::string key = std::get<0> (*itfsr);
			      std::filesystem::path sp = std::get<1> (*itfsr);
			      filepartbufmtx.lock ();
			      auto itfpb =
				  std::find_if (
				      filepartbuf.begin (),
				      filepartbuf.end (),
				      [&key, &sp]
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
			      if (itfpb != filepartbuf.end ())
				{
				  std::string fbrkey = std::get<0> (*itfpb);
				  time_t fbrtm = std::get<1> (*itfpb);
				  fbrvectmtx.lock ();
				  auto itfbrv =
				      std::find_if (
					  fbrvect.begin (),
					  fbrvect.end (),
					  [&fbrkey, &fbrtm]
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
				  if (itfbrv == fbrvect.end ())
				    {
				      std::vector<char> msg;
				      std::copy (okarr.begin (), okarr.end (),
						 std::back_inserter (msg));
				      std::string msgtype = "FB";
				      std::copy (msgtype.begin (),
						 msgtype.end (),
						 std::back_inserter (msg));
				      uint64_t tmfb = std::get<1> (*itfpb);
				      msg.resize (msg.size () + sizeof(tmfb));
				      std::memcpy (&msg[34], &tmfb,
						   sizeof(tmfb));
				      uint64_t msgn = 0;
				      msg.resize (msg.size () + sizeof(msgn));
				      std::memcpy (&msg[42], &msgn,
						   sizeof(msgn));
				      std::vector<char> fh = af.filehash (sp);
				      std::copy (fh.begin (), fh.end (),
						 std::back_inserter (msg));
				      msg = af.cryptStrm (unm, passwd, msg);
				      int sent = 0;
				      ipv6lrmtx.lock ();
				      auto itlr6 = std::find_if (
					  ipv6lr.begin (), ipv6lr.end (),
					  [keytos]
					  (auto &el)
					    {
					      return std::get<0>(el) == keytos;
					    });
				      if (itlr6 != ipv6lr.end ())
					{
					  if (curtime - std::get<1> (*itlr6)
					      <= Tmttear)
					    {
					      ipv6contmtx.lock ();
					      auto itip6 =
						  std::find_if (
						      ipv6cont.begin (),
						      ipv6cont.end (),
						      [keytos]
						      (auto &el)
							{
							  return std::get<0>(el) == keytos;
							});
					      if (itip6 != ipv6cont.end ())
						{
						  std::string ipv6 =
						      std::get<1> (*itip6);
						  uint16_t port = std::get<2> (
						      *itip6);
						  sockipv6mtx.lock ();
						  sent = sendMsg6 (sockipv6,
								   ipv6, port,
								   msg);
						  sockipv6mtx.unlock ();
						  result = 1;
						}
					      ipv6contmtx.unlock ();
					    }
					}
				      ipv6lrmtx.unlock ();
				      if (sent <= 0)
					{
					  sent = sendMsg (sock, ip, port, msg);
					  result = 1;
					}
				    }
				  else
				    {
				      int fsz = std::filesystem::file_size (
					  std::get<2> (*itfpb));
				      if (std::get<6> (*itfpb) == 1)
					{
					  int bytefmf = std::get<3> (*itfpb);
					  std::vector<char> fpv;
					  if (fsz - bytefmf > Partsize)
					    {
					      fpv.resize (Partsize);
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
					      f.open (
						  std::get<2> (*itfpb),
						  std::ios_base::in
						      | std::ios_base::binary);
					      f.seekg (bytefmf,
						       std::ios_base::beg);
					      f.read (&fpv[0], fpv.size ());
					      f.close ();
					      if (std::get<3> (*itfpb) > 0)
						{
						  std::get<4> (*itfpb) =
						      std::get<4> (*itfpb) + 1;
						}
					      std::get<3> (*itfpb) =
						  std::get<3> (*itfpb)
						      + fpv.size ();
					      std::get<5> (*itfpb) = fpv;

					      std::get<6> (*itfpb) = 0;
					    }
					  else
					    {
					      std::vector<char> msg;
					      std::copy (
						  okarr.begin (), okarr.end (),
						  std::back_inserter (msg));
					      std::string msgtype = "FE";
					      std::copy (
						  msgtype.begin (),
						  msgtype.end (),
						  std::back_inserter (msg));
					      uint64_t tmfb = std::get<1> (
						  *itfpb);
					      msg.resize (
						  msg.size () + sizeof(tmfb));
					      std::memcpy (&msg[34], &tmfb,
							   sizeof(tmfb));
					      uint64_t msgn = 0;
					      msg.resize (
						  msg.size () + sizeof(msgn));
					      std::memcpy (&msg[42], &msgn,
							   sizeof(msgn));
					      msg = af.cryptStrm (unm, passwd,
								  msg);
					      int sent = 0;
					      ipv6lrmtx.lock ();
					      auto itlr6 =
						  std::find_if (
						      ipv6lr.begin (),
						      ipv6lr.end (),
						      [keytos]
						      (auto &el)
							{
							  return std::get<0>(el) == keytos;
							});
					      if (itlr6 != ipv6lr.end ())
						{
						  if (curtime
						      - std::get<1> (*itlr6)
						      <= Tmttear)
						    {
						      ipv6contmtx.lock ();
						      auto itip6 =
							  std::find_if (
							      ipv6cont.begin (),
							      ipv6cont.end (),
							      [keytos]
							      (auto &el)
								{
								  return std::get<0>(el) == keytos;
								});
						      if (itip6
							  != ipv6cont.end ())
							{
							  std::string ipv6 =
							      std::get<1> (
								  *itip6);
							  uint16_t port =
							      std::get<2> (
								  *itip6);
							  sockipv6mtx.lock ();
							  sent = sendMsg6 (
							      sockipv6, ipv6,
							      port, msg);
							  sockipv6mtx.unlock ();
							  result = 1;
							}
						      ipv6contmtx.unlock ();
						    }
						}
					      ipv6lrmtx.unlock ();
					      if (sent <= 0)
						{
						  sent = sendMsg (sock, ip,
								  port, msg);
						  result = 1;
						}
					    }
					}
				      if (std::get<6> (*itfpb) == 0)
					{
					  std::vector<char> msg;
					  std::copy (okarr.begin (),
						     okarr.end (),
						     std::back_inserter (msg));
					  std::string msgtype = "Fb";
					  std::copy (msgtype.begin (),
						     msgtype.end (),
						     std::back_inserter (msg));
					  uint64_t tmfb = std::get<1> (*itfpb);
					  msg.resize (
					      msg.size () + sizeof(tmfb));
					  std::memcpy (&msg[34], &tmfb,
						       sizeof(tmfb));
					  uint64_t msgn = std::get<4> (*itfpb);
					  msg.resize (
					      msg.size () + sizeof(msgn));
					  std::memcpy (&msg[42], &msgn,
						       sizeof(msgn));
					  std::vector<char> fpv = std::get<5> (
					      *itfpb);
					  std::vector<char> fh = af.strhash (
					      fpv, 2);
					  std::copy (fh.begin (), fh.end (),
						     std::back_inserter (msg));
					  msg = af.cryptStrm (unm, passwd, msg);
					  int sent = 0;
					  ipv6lrmtx.lock ();
					  auto itlr6 =
					      std::find_if (
						  ipv6lr.begin (),
						  ipv6lr.end (),
						  [keytos]
						  (auto &el)
						    {
						      return std::get<0>(el) == keytos;
						    });
					  if (itlr6 != ipv6lr.end ())
					    {
					      if (curtime - std::get<1> (*itlr6)
						  <= Tmttear)
						{
						  ipv6contmtx.lock ();
						  auto itip6 =
						      std::find_if (
							  ipv6cont.begin (),
							  ipv6cont.end (),
							  [keytos]
							  (auto &el)
							    {
							      return std::get<0>(el) == keytos;
							    });
						  if (itip6 != ipv6cont.end ())
						    {
						      std::string ipv6 =
							  std::get<1> (*itip6);
						      uint16_t port =
							  std::get<2> (*itip6);
						      sockipv6mtx.lock ();
						      sent = sendMsg6 (sockipv6,
								       ipv6,
								       port,
								       msg);
						      sockipv6mtx.unlock ();
						      result = 1;
						    }
						  ipv6contmtx.unlock ();
						}
					    }
					  ipv6lrmtx.unlock ();
					  if (sent <= 0)
					    {
					      sent = sendMsg (sock, ip, port,
							      msg);
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
					      std::copy (
						  okarr.begin (), okarr.end (),
						  std::back_inserter (msg));
					      std::string msgtype = "Fp";
					      std::copy (
						  msgtype.begin (),
						  msgtype.end (),
						  std::back_inserter (msg));
					      uint64_t tmfb = std::get<1> (
						  *itfpb);
					      msg.resize (
						  msg.size () + sizeof(tmfb));
					      std::memcpy (&msg[34], &tmfb,
							   sizeof(tmfb));
					      msg.resize (
						  msg.size ()
						      + sizeof(dtgmnmb));
					      std::memcpy (&msg[42], &dtgmnmb,
							   sizeof(dtgmnmb));
					      dtgmnmb = dtgmnmb + 1;
					      std::vector<char> dtgm;
					      if (fpsz - sentb > 457)
						{
						  std::copy (
						      fpv.begin () + sentb,
						      fpv.begin () + sentb
							  + 457,
						      std::back_inserter (
							  dtgm));
						  sentb = sentb + 457;
						}
					      else
						{
						  int dif = fpsz - sentb;
						  std::copy (
						      fpv.begin () + sentb,
						      fpv.begin () + sentb
							  + dif,
						      std::back_inserter (
							  dtgm));
						  sentb = sentb + dif;
						}
					      std::copy (
						  dtgm.begin (), dtgm.end (),
						  std::back_inserter (msg));
					      msg = af.cryptStrm (unm, passwd,
								  msg);
					      int sent = 0;
					      ipv6lrmtx.lock ();
					      auto itlr6 =
						  std::find_if (
						      ipv6lr.begin (),
						      ipv6lr.end (),
						      [keytos]
						      (auto &el)
							{
							  return std::get<0>(el) == keytos;
							});
					      if (itlr6 != ipv6lr.end ())
						{
						  if (curtime
						      - std::get<1> (*itlr6)
						      <= Tmttear)
						    {
						      ipv6contmtx.lock ();
						      auto itip6 =
							  std::find_if (
							      ipv6cont.begin (),
							      ipv6cont.end (),
							      [keytos]
							      (auto &el)
								{
								  return std::get<0>(el) == keytos;
								});
						      if (itip6
							  != ipv6cont.end ())
							{
							  std::string ipv6 =
							      std::get<1> (
								  *itip6);
							  uint16_t port =
							      std::get<2> (
								  *itip6);
							  sockipv6mtx.lock ();
							  sent = sendMsg6 (
							      sockipv6, ipv6,
							      port, msg);
							  sockipv6mtx.unlock ();
							  result = 1;
							}
						      ipv6contmtx.unlock ();
						    }
						}
					      ipv6lrmtx.unlock ();
					      if (sent <= 0)
						{
						  sent = sendMsg (sock, ip,
								  port, msg);
						  result = 1;
						}

					      totalsent = totalsent
						  + msg.size ();
					      if (sent < 0)
						{
						  fbrvectmtx.unlock ();
						  filepartbufmtx.unlock ();
						  filesendreqmtx.unlock ();
						  sendbufmtx.unlock ();
						  contmtx.unlock ();
						  return result;
						}
					    }
					  msg.clear ();
					  std::copy (okarr.begin (),
						     okarr.end (),
						     std::back_inserter (msg));
					  msgtype = "Fe";
					  std::copy (msgtype.begin (),
						     msgtype.end (),
						     std::back_inserter (msg));
					  tmfb = std::get<1> (*itfpb);
					  msg.resize (
					      msg.size () + sizeof(tmfb));
					  std::memcpy (&msg[34], &tmfb,
						       sizeof(tmfb));
					  msgn = 0;
					  msg.resize (
					      msg.size () + sizeof(msgn));
					  std::memcpy (&msg[42], &msgn,
						       sizeof(msgn));
					  msg = af.cryptStrm (unm, passwd, msg);
					  sent = 0;
					  ipv6lrmtx.lock ();
					  itlr6 =
					      std::find_if (
						  ipv6lr.begin (),
						  ipv6lr.end (),
						  [keytos]
						  (auto &el)
						    {
						      return std::get<0>(el) == keytos;
						    });
					  if (itlr6 != ipv6lr.end ())
					    {
					      if (curtime - std::get<1> (*itlr6)
						  <= Tmttear)
						{
						  ipv6contmtx.lock ();
						  auto itip6 =
						      std::find_if (
							  ipv6cont.begin (),
							  ipv6cont.end (),
							  [keytos]
							  (auto &el)
							    {
							      return std::get<0>(el) == keytos;
							    });
						  if (itip6 != ipv6cont.end ())
						    {
						      std::string ipv6 =
							  std::get<1> (*itip6);
						      uint16_t port =
							  std::get<2> (*itip6);
						      sockipv6mtx.lock ();
						      sent = sendMsg6 (sockipv6,
								       ipv6,
								       port,
								       msg);
						      sockipv6mtx.unlock ();
						      result = 1;
						    }
						  ipv6contmtx.unlock ();
						}
					    }
					  ipv6lrmtx.unlock ();
					  if (sent <= 0)
					    {
					      sent = sendMsg (sock, ip, port,
							      msg);
					      result = 1;
					    }
					}
				    }
				  fbrvectmtx.unlock ();
				}
			      filepartbufmtx.unlock ();
			    }
			}
		      filesendreqmtx.unlock ();
		    }
		}
	      if (totalsent > 1521)
		{
		  break;
		}
	    }
	  sendbufmtx.unlock ();
	}
    }
  contmtx.unlock ();
  return result;
}

void
NetworkOperations::removeFriend (std::string key)
{
  std::string keyloc = key;
  if (sockmtx.try_lock ())
    {
      auto itsock = std::find_if (sockets4.begin (), sockets4.end (), [&keyloc]
      (auto &el)
	{
	  if (std::get<0>(el) == keyloc)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
      if (itsock != sockets4.end ())
	{
	  int sock = std::get<1> (*itsock);
	  std::shared_ptr<std::mutex> mtx = std::get<2> (*itsock);
	  std::shared_ptr<std::mutex> mtx2 = std::get<4> (*itsock);
	  if (mtx->try_lock ())
	    {
	      if (sock >= 0)
		{
#ifdef __linux
		  close (sock);
#endif
#ifdef _WIN32
  	      closesocket (sock);
  #endif
		}
	      mtx->unlock ();
	      sockets4.erase (itsock);
	    }
	  else
	    {
	      std::mutex *thrmtx = new std::mutex;
	      thrmtx->lock ();
	      threadvectmtx.lock ();
	      threadvect.push_back (std::make_tuple (thrmtx, "deleteFriend"));
	      threadvectmtx.unlock ();
	      std::thread *thr = new std::thread ( [keyloc, this, thrmtx]
	      {
		usleep (100000);
		this->removeFriend (keyloc);
		thrmtx->unlock ();
	      });
	      thr->detach ();
	      delete thr;
	      sockmtx.unlock ();
	      return void ();
	    }
	}
      sockmtx.unlock ();
    }
  else
    {
      std::mutex *thrmtx = new std::mutex;
      thrmtx->lock ();
      threadvectmtx.lock ();
      threadvect.push_back (std::make_tuple (thrmtx, "deleteFriend"));
      threadvectmtx.unlock ();
      std::thread *thr = new std::thread ( [keyloc, this, thrmtx]
      {
	usleep (100000);
	this->removeFriend (keyloc);
	thrmtx->unlock ();
      });
      thr->detach ();
      delete thr;
      return void ();
    }

  addfrmtx.lock ();
  Addfriends.erase (
      std::remove (Addfriends.begin (), Addfriends.end (), keyloc),
      Addfriends.end ());
  addfrmtx.unlock ();

  contfullmtx.lock ();
  auto contit = std::find_if (contactsfull.begin (), contactsfull.end (),
			      [&keyloc]
			      (auto &el)
				{
				  return std::get<1>(el) == keyloc;
				});
  if (contit != contactsfull.end ())
    {
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      std::string index;
      strm << std::get<0> (*contit);
      index = strm.str ();
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist/SendBufer/" + index;
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
	  sendbufmtx.lock ();
	  std::filesystem::remove_all (filepath);
	  sendbufmtx.unlock ();
	}

      std::string line;
      int indep = std::get<0> (*contit);
      af.homePath (&filename);
      filename = filename + "/.Communist/SendBufer";
      std::filesystem::path folderpath = std::filesystem::u8path (filename);
      std::vector<std::filesystem::path> pathvect;
      if (std::filesystem::exists (folderpath))
	{
	  for (auto &dir : std::filesystem::directory_iterator (folderpath))
	    {
	      std::filesystem::path old = dir.path ();
	      pathvect.push_back (old);
	    }
	  std::sort (pathvect.begin (), pathvect.end (), []
	  (auto &el1, auto el2)
	    {
	      std::string line1 = el1.filename().u8string();
	      std::string line2 = el2.filename().u8string();
	      return std::stoi(line1) < std::stoi(line2);
	    });
	  for (size_t i = 0; i < pathvect.size (); i++)
	    {
	      line = pathvect[i].filename ().u8string ();
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << line;
	      int tint;
	      strm >> tint;
	      if (tint > indep)
		{
		  tint = tint - 1;
		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << tint;
		  line = pathvect[i].parent_path ().u8string ();
		  line = line + "/" + strm.str ();
		  std::filesystem::path newpath (
		      std::filesystem::u8path (line));
		  std::filesystem::rename (pathvect[i], newpath);
		}
	    }
	}

      af.homePath (&filename);
      filename = filename + "/.Communist/Bufer/" + index;
      filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
	  std::filesystem::remove_all (filepath);
	}

      af.homePath (&filename);
      filename = filename + "/.Communist/Bufer";
      folderpath = std::filesystem::u8path (filename);
      pathvect.clear ();
      if (std::filesystem::exists (folderpath))
	{
	  for (auto &dir : std::filesystem::directory_iterator (folderpath))
	    {
	      std::filesystem::path old = dir.path ();
	      pathvect.push_back (old);
	    }
	  std::sort (pathvect.begin (), pathvect.end (), []
	  (auto &el1, auto el2)
	    {
	      std::string line1 = el1.filename().u8string();
	      std::string line2 = el2.filename().u8string();
	      return std::stoi(line1) < std::stoi(line2);
	    });
	  for (size_t i = 0; i < pathvect.size (); i++)
	    {
	      line = pathvect[i].filename ().u8string ();
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << line;
	      int tint;
	      strm >> tint;
	      if (tint > indep)
		{
		  tint = tint - 1;
		  strm.str ("");
		  strm.clear ();
		  strm.imbue (loc);
		  strm << tint;
		  line = pathvect[i].parent_path ().u8string ();
		  line = line + "/" + strm.str ();
		  std::filesystem::path newpath (
		      std::filesystem::u8path (line));
		  std::filesystem::rename (pathvect[i], newpath);
		}
	    }
	}

      af.homePath (&filename);
      filename = filename + "/.Communist/" + index;
      filepath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (filepath))
	{
	  std::filesystem::remove_all (filepath);
	}

      contactsfull.erase (contit);
      af.homePath (&filename);
      filename = filename + "/.Communist";
      folderpath = std::filesystem::u8path (filename);
      pathvect.clear ();
      for (auto &dir : std::filesystem::directory_iterator (folderpath))
	{
	  std::filesystem::path old = dir.path ();
	  if (std::filesystem::is_directory (old)
	      && old.filename ().u8string () != "Bufer"
	      && old.filename ().u8string () != "SendBufer")
	    {
	      pathvect.push_back (old);
	    }
	}
      std::sort (pathvect.begin (), pathvect.end (), []
      (auto &el1, auto el2)
	{
	  std::string line1 = el1.filename().u8string();
	  std::string line2 = el2.filename().u8string();
	  return std::stoi(line1) < std::stoi(line2);
	});
      for (size_t i = 0; i < pathvect.size (); i++)
	{
	  line = pathvect[i].filename ().u8string ();
	  strm.str ("");
	  strm.clear ();
	  strm.imbue (loc);
	  strm << line;
	  int tint;
	  strm >> tint;
	  if (tint > indep)
	    {
	      tint = tint - 1;
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << tint;
	      line = pathvect[i].parent_path ().u8string ();
	      line = line + "/" + strm.str ();
	      std::filesystem::path newpath (std::filesystem::u8path (line));
	      std::filesystem::rename (pathvect[i], newpath);
	    }
	}
      for (size_t i = 0; i < contactsfull.size (); i++)
	{
	  if (std::get<0> (contactsfull[i]) > indep)
	    {
	      std::get<0> (contactsfull[i]) = std::get<0> (contactsfull[i]) - 1;
	    }
	}
      contsizech = contsizech - 1;
      contmtx.lock ();
      contacts.erase (
	  std::remove_if (contacts.begin (), contacts.end (), [keyloc]
	  (auto &el)
	    {
	      return std::get<1>(el) == keyloc;
	    }),
	  contacts.end ());
      for (size_t i = 0; i < contacts.size (); i++)
	{
	  if (std::get<0> (contacts[i]) > indep)
	    {
	      std::get<0> (contacts[i]) = std::get<0> (contacts[i]) - 1;
	    }
	}
      contmtx.unlock ();
    }
  contfullmtx.unlock ();
  friendDeleted.emit (keyloc);

  msgpartbufmtx.lock ();
  msgpartbuf.erase (
      std::remove_if (msgpartbuf.begin (), msgpartbuf.end (), [&keyloc]
      (auto &el)
	{ return std::get<0>(el) == keyloc;}),
      msgpartbuf.end ());
  msgpartbufmtx.unlock ();

  msghashmtx.lock ();
  msghash.erase (std::remove_if (msghash.begin (), msghash.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		 msghash.end ());
  msghashmtx.unlock ();

  msgparthashmtx.lock ();
  msgparthash.erase (
      std::remove_if (msgparthash.begin (), msgparthash.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgparthash.end ());
  msgparthashmtx.unlock ();

  msgpartrcvmtx.lock ();
  msgpartrcv.erase (
      std::remove_if (msgpartrcv.begin (), msgpartrcv.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgpartrcv.end ());
  msgpartrcvmtx.unlock ();

  msgpartbufmtx.lock ();
  msgpartbuf.erase (
      std::remove_if (msgpartbuf.begin (), msgpartbuf.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgpartbuf.end ());
  msgpartbufmtx.unlock ();

  msgrcvdpnummtx.lock ();
  msgrcvdpnum.erase (
      std::remove_if (msgrcvdpnum.begin (), msgrcvdpnum.end (), [keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgrcvdpnum.end ());
  msgrcvdpnummtx.unlock ();

  getfrmtx.lock ();
  getfr.erase (std::remove_if (getfr.begin (), getfr.end (), [&keyloc]
  (auto &el)
    {
      return el == keyloc;
    }),
	       getfr.end ());
  getfrmtx.unlock ();

  getfrresmtx.lock ();
  getfrres.erase (std::remove_if (getfrres.begin (), getfrres.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		  getfrres.end ());
  getfrresmtx.unlock ();

  putipmtx.lock ();
  putipv.erase (std::remove_if (putipv.begin (), putipv.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		putipv.end ());
  putipmtx.unlock ();

  ipv6contmtx.lock ();
  ipv6cont.erase (std::remove_if (ipv6cont.begin (), ipv6cont.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		  ipv6cont.end ());
  ipv6contmtx.unlock ();

  ipv6lrmtx.lock ();
  ipv6lr.erase (std::remove_if (ipv6lr.begin (), ipv6lr.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		ipv6lr.end ());
  ipv6lrmtx.unlock ();

  filesendreqmtx.lock ();
  filesendreq.erase (
      std::remove_if (filesendreq.begin (), filesendreq.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filesendreq.end ());
  filesendreqmtx.unlock ();

  fqrcvdmtx.lock ();
  fqrcvd.erase (std::remove_if (fqrcvd.begin (), fqrcvd.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		fqrcvd.end ());
  fqrcvdmtx.unlock ();

  filepartbufmtx.lock ();
  filepartbuf.erase (
      std::remove_if (filepartbuf.begin (), filepartbuf.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartbuf.end ());
  filepartbufmtx.unlock ();

  filehashvectmtx.lock ();
  filehashvect.erase (
      std::remove_if (filehashvect.begin (), filehashvect.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filehashvect.end ());
  filehashvectmtx.unlock ();

  fileparthashmtx.lock ();
  fileparthash.erase (
      std::remove_if (fileparthash.begin (), fileparthash.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      fileparthash.end ());
  fileparthashmtx.unlock ();

  filepartrcvmtx.lock ();
  filepartrcv.erase (
      std::remove_if (filepartrcv.begin (), filepartrcv.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartrcv.end ());
  filepartrcvmtx.unlock ();

  filepartrlogmtx.lock ();
  filepartrlog.erase (
      std::remove_if (filepartrlog.begin (), filepartrlog.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartrlog.end ());
  filepartrlogmtx.unlock ();

  currentpartmtx.lock ();
  currentpart.erase (
      std::remove_if (currentpart.begin (), currentpart.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      currentpart.end ());
  currentpartmtx.unlock ();

  fbrvectmtx.lock ();
  fbrvect.erase (std::remove_if (fbrvect.begin (), fbrvect.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		 fbrvect.end ());
  fbrvectmtx.unlock ();

  filepartendmtx.lock ();
  filepartend.erase (
      std::remove_if (filepartend.begin (), filepartend.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartend.end ());
  filepartendmtx.unlock ();

  fileendmtx.lock ();
  fileend.erase (std::remove_if (fileend.begin (), fileend.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		 fileend.end ());
  fileendmtx.unlock ();

  maintblockmtx.lock ();
  maintblock.erase (
      std::remove_if (maintblock.begin (), maintblock.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      maintblock.end ());
  maintblockmtx.unlock ();

  holepunchstopmtx.lock ();
  holepunchstop.erase (
      std::remove_if (holepunchstop.begin (), holepunchstop.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      holepunchstop.end ());
  holepunchstopmtx.unlock ();
}

void
NetworkOperations::processDHT ()
{
  std::mutex *thrmtx = new std::mutex;
  thrmtx->lock ();
  threadvectmtx.lock ();
  threadvect.push_back (std::make_tuple (thrmtx, "Process DHT"));
  threadvectmtx.unlock ();
  std::thread *dhtthr = new std::thread ( [this, thrmtx]
  {
    AuxFunc af;
    std::vector<char> sesbuf;
    lt::session_params sespar;
    std::string filename;
    af.homePath (&filename);
    filename = filename + "/.cache/Communist/dhtstate";
    std::filesystem::path filepath = std::filesystem::u8path (filename);
    std::fstream f;
    if (std::filesystem::exists (filepath))
      {
	sesbuf.resize (std::filesystem::file_size (filepath));
	f.open (filepath, std::ios_base::in | std::ios_base::binary);
	f.read (&sesbuf[0], sesbuf.size ());
	f.close ();
	sespar = lt::read_session_params (sesbuf, lt::session_handle::save_dht_state);
}
    sesbuf.clear ();
    lt::session ses (sespar);
    lt::settings_pack p;
    p.set_bool (lt::settings_pack::enable_dht, true);
    p.set_int (lt::settings_pack::alert_mask,
	       lt::alert_category::dht | lt::alert_category::dht_operation);
    auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (),
			       []
			       (auto &el)
				 {
				   return std::get<0>(el) == "Listenifcs";
				 });
    if (itprv != this->prefvect.end ())
      {
	if (std::get<1> (*itprv) != "")
	  {
	    if (std::get<1> (*itprv) == "0.0.0.0:0,[::]:0")
	      {
		p.set_str (lt::settings_pack::listen_interfaces,
			   "0.0.0.0:0,[::]:0");
	      }
	    else
	      {
		p.set_str (lt::settings_pack::listen_interfaces,
			   std::get<1> (*itprv));
	      }
	  }
	else
	  {
	    p.set_str (lt::settings_pack::listen_interfaces,
		       "0.0.0.0:0,[::]:0");
	  }
      }
    else
      {
	p.set_str (lt::settings_pack::listen_interfaces, "0.0.0.0:0,[::]:0");
      }

    itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
    (auto &el)
      {
	return std::get<0>(el) == "Bootstrapdht";
      });
    if (itprv != this->prefvect.end ())
      {
	if (std::get<1> (*itprv) != "")
	  {
	    p.set_str (lt::setting_by_name ("dht_bootstrap_nodes"),
		       std::get<1> (*itprv));
	  }
	else
	  {
	    p.set_str (lt::setting_by_name ("dht_bootstrap_nodes"),
		       "router.bittorrent.com:6881");
	  }
      }
    else
      {
	p.set_str (lt::setting_by_name ("dht_bootstrap_nodes"),
		   "router.bittorrent.com:6881");
      }
    ses.apply_settings (p);

    int count = 0;
    int errcount = 0;
    std::vector<std::string> tlv;

    for (;;)
      {
	if (ses.is_dht_running () || (this->cancel) != 0)
	  {
	    std::cout << "DHT started" << std::endl;
	    break;
	  }
	if (count > 2000)
	  {
	    break;
	  }
	count++;
	usleep (10000);
      }
    if (count <= 2000 && (this->cancel) == 0)
      {
	std::vector<std::string> getv;
	std::vector<std::tuple<std::string, std::string>> getvinner;
	std::vector<std::tuple<std::string, std::string>> getvinner6;
	std::vector<std::tuple<std::string, uint32_t, uint16_t>> putv;
	std::vector<std::tuple<std::string, uint32_t, uint16_t>> putfault;
	std::vector<std::string> putfault6;
	std::vector<std::tuple<std::string, std::string, uint32_t, uint16_t>> putvinner;
	std::vector<std::tuple<std::string, std::string>> putvinner6; //other key, put key
	std::vector<lt::alert*> alerts;
	for (;;)
	  {
	    if ((this->cancel) > 0)
	      {
		break;
	      }
	    getv = getFrVect ();
	    putv = putVect ();

	    for (size_t i = 0; i < getv.size (); i++)
	      {
		std::string getkey = getv[i];

		auto it = std::find_if (getvinner.begin (), getvinner.end (),
					[&getkey]
					(auto &el)
					  { return std::get<0>(el) == getkey;});
		if (it == getvinner.end ())
		  {
		    getkey = getSes (getv[i], &ses);
		    std::tuple<std::string, std::string> ttup;
		    ttup = std::make_tuple (getv[i], getkey);
		    getvinner.push_back (ttup);
		  }

		getkey = getv[i];
		auto it6 = std::find_if (getvinner6.begin (), getvinner6.end (),
					 [&getkey]
					 (auto &el)
					   {
					     return std::get<0>(el) == getkey;
					   });
		if (it6 == getvinner6.end ())
		  {
		    getkey = getSes6 (getv[i], &ses);
		    getvinner6.push_back (std::make_tuple (getv[i], getkey));
		  }
	      }
	    getv.clear ();

	    for (size_t i = 0; i < putv.size (); i++)
	      {
		std::string otherkey = std::get<0> (putv[i]);
		putfault.erase (
		    std::remove_if (putfault.begin (), putfault.end (),
				    [&otherkey]
				    (auto &el)
				      { return std::get<0>(el) == otherkey;}),
		    putfault.end ());
		auto it = std::find_if (
		    putvinner.begin (), putvinner.end (), [&otherkey]
		    (auto &el)
		      { return std::get<0>(el) == otherkey;});
		if (it == putvinner.end ())
		  {
		    std::string putkey;
		    putkey = putSes (otherkey, std::get<1> (putv[i]),
				     std::get<2> (putv[i]), &ses);
		    std::tuple<std::string, std::string, uint32_t, uint16_t> ttup;
		    ttup = std::make_tuple (otherkey, putkey,
					    std::get<1> (putv[i]),
					    std::get<2> (putv[i]));
		    putvinner.push_back (ttup);

		  }
		auto itp6 = std::find_if (
		    putvinner6.begin (), putvinner6.end (), [&otherkey]
		    (auto &el)
		      {
			return std::get<0>(el) == otherkey;
		      });
		if (itp6 == putvinner6.end ())
		  {
		    std::string putkey = putSes6 (otherkey, &ses);
		    putvinner6.push_back (std::make_tuple (otherkey, putkey));
		  }
	      }
	    putv.clear ();

	    for (size_t i = 0; i < putfault6.size (); i++)
	      {
		std::string otherkey = putfault6[i];
		putSes6 (otherkey, &ses);
	      }
	    putfault6.clear ();

	    for (size_t i = 0; i < putfault.size (); i++)
	      {
		std::string otherkey = std::get<0> (putfault[i]);
		putSes (otherkey, std::get<1> (putfault[i]),
			std::get<2> (putfault[i]), &ses);
	      }
	    putfault.clear ();
	    alerts.clear ();
	    std::vector<std::tuple<std::string, uint32_t, uint16_t, int>> rcvd;
	    std::vector<std::tuple<std::string, std::string, uint16_t, int>> rcvd6;
	    ses.pop_alerts (&alerts);
	    for (size_t i = 0; i < alerts.size (); i++)
	      {
		if (alerts[i]->type () == lt::dht_bootstrap_alert::alert_type)
		  {
		    std::cout << alerts[i]->message () << std::endl;
		    break;
		  }
		if (alerts[i]->type ()
		    == lt::dht_mutable_item_alert::alert_type)
		  {
		    std::string msg = alerts[i]->message ();
		    std::string::size_type n;
		    n = msg.find ("<uninitialized>");
		    if (n != std::string::npos)
		      {
			std::string key = msg;
			key.erase (
			    0,
			    key.find ("key=") + std::string ("key=").size ());
			key = key.substr (0, key.find (" "));
			std::string seqstr = msg;
			seqstr.erase (
			    0,
			    seqstr.find ("seq=")
				+ std::string ("seq=").size ());
			seqstr = seqstr.substr (0, seqstr.find (" "));
			n = msg.find ("ipv4");
			if (n != std::string::npos)
			  {
			    std::stringstream strm;
			    std::locale loc ("C");
			    strm.imbue (loc);
			    strm << seqstr;
			    int seq;
			    strm >> seq;
			    std::string othk;
			    auto it = std::find_if (
				getvinner.begin (), getvinner.end (), [&key]
				(auto &el)
				  { return std::get<1>(el) == key;});
			      {
				if (it != getvinner.end ())
				  {
				    othk = std::get<0> (*it);
				    auto itr = std::find_if (
					rcvd.begin (), rcvd.end (), [&othk]
					(auto &el)
					  { return std::get<0>(el) == othk;});
				    if (itr != rcvd.end ())
				      {
					*itr = std::make_tuple (othk, 0, 0,
								seq);
				      }
				    else
				      {
					rcvd.push_back (
					    std::make_tuple (othk, 0, 0, seq));
				      }
				  }
			      }
			  }
			n = msg.find ("ipv6");
			if (n != std::string::npos)
			  {
			    std::stringstream strm;
			    std::locale loc ("C");
			    strm.imbue (loc);
			    strm << seqstr;
			    int seq;
			    strm >> seq;
			    std::string othk;
			    auto it = std::find_if (
				getvinner6.begin (), getvinner6.end (), [&key]
				(auto &el)
				  { return std::get<1>(el) == key;});
			      {
				if (it != getvinner6.end ())
				  {
				    othk = std::get<0> (*it);
				    auto itr = std::find_if (
					rcvd6.begin (), rcvd6.end (), [&othk]
					(auto &el)
					  { return std::get<0>(el) == othk;});
				    if (itr != rcvd6.end ())
				      {
					*itr = std::make_tuple (othk, "", 0,
								seq);
				      }
				    else
				      {
					rcvd6.push_back (
					    std::make_tuple (othk, "", 0, seq));
				      }
				  }
			      }
			  }
		      }
		    else
		      {
			std::string key = msg;
			key.erase (
			    0,
			    key.find ("key=") + std::string ("key=").size ());
			key = key.substr (0, key.find (" "));
			std::string seqstr = msg;
			seqstr.erase (
			    0,
			    seqstr.find ("seq=")
				+ std::string ("seq=").size ());
			seqstr = seqstr.substr (0, seqstr.find (" "));
			n = msg.find ("ipv4");
			if (n != std::string::npos)
			  {
			    std::stringstream strm;
			    std::locale loc ("C");
			    strm.imbue (loc);
			    strm << seqstr;
			    int seq;
			    strm >> seq;
			    std::string othk;
			    auto it = std::find_if (
				getvinner.begin (), getvinner.end (), [&key]
				(auto &el)
				  { return std::get<1>(el) == key;});
			      {
				if (it != getvinner.end () && seq > 0)
				  {
				    othk = std::get<0> (*it);
				    std::string ipst = msg;
				    ipst.erase (
					0,
					ipst.find ("[")
					    + std::string ("[").size ());
				    ipst.erase (
					0,
					ipst.find ("\'")
					    + std::string ("\'").size ());
				    ipst = ipst.substr (0, ipst.find ("\'"));
				    ipst = af.hex_to_string (ipst);
				    std::vector<char> crv (ipst.begin (),
							   ipst.end ());
				    lt::dht::public_key othpk;
				    lt::aux::from_hex (std::get<0> (*it),
						       othpk.bytes.data ());
				    std::array<char, 32> scalar;
				    std::tuple<lt::dht::public_key,
					lt::dht::secret_key> okp;
				    okp = lt::dht::ed25519_create_keypair (
					*seed);
				    scalar = lt::dht::ed25519_key_exchange (
					othpk, std::get<1> (okp));
				    othpk = lt::dht::ed25519_add_scalar (
					othpk, scalar);

				    std::string unm = lt::aux::to_hex (
					std::get<0> (okp).bytes);
				    std::string passwd = lt::aux::to_hex (
					othpk.bytes);
				    crv = af.decryptStrm (unm, passwd, crv);
				    ipst = std::string (crv.begin (),
							crv.end ());
				    ipst = ipst.substr (0, ipst.find (":"));
				    uint32_t ip;

				    int chip = inet_pton (AF_INET,
							  ipst.c_str (), &ip);
				    strm.str ("");
				    strm.clear ();
				    strm.imbue (loc);
				    std::string portst = std::string (
					crv.begin (), crv.end ());
				    portst.erase (
					0,
					portst.find (":")
					    + std::string (":").size ());
				    std::string::size_type n;
				    n = portst.find ("1234567890123456");
				    if (n != std::string::npos)
				      {
					portst.erase (
					    n,
					    n
						+ std::string (
						    "1234567890123456").size ());
				      }

				    uint16_t port;
				    if (portst == "sym")
				      {
					port = 0;
				      }
				    else
				      {
					strm << portst;
					strm >> port;
					port = htons (port);
				      }
				    auto itr = std::find_if (
					rcvd.begin (), rcvd.end (), [&othk]
					(auto &el)
					  { return std::get<0>(el) == othk;});
				    if (itr != rcvd.end ())
				      {
					if (chip > 0 && seq >= 0)
					  {
					    *itr = make_tuple (othk, ip, port,
							       seq);
					  }

				      }
				    else
				      {
					if (chip > 0 && seq >= 0)
					  {
					    rcvd.push_back (
						std::make_tuple (othk, ip, port,
								 seq));
					  }
				      }
				  }
			      }
			  }
			n = msg.find ("ipv6");
			if (n != std::string::npos)
			  {
			    std::stringstream strm;
			    std::locale loc ("C");
			    strm.imbue (loc);
			    strm << seqstr;
			    int seq;
			    strm >> seq;
			    std::string othk;
			    auto it = std::find_if (
				getvinner6.begin (), getvinner6.end (), [&key]
				(auto &el)
				  { return std::get<1>(el) == key;});
			      {
				if (it != getvinner6.end () && seq > 0)
				  {
				    othk = std::get<0> (*it);
				    std::string ipst = msg;
				    ipst.erase (
					0,
					ipst.find ("[")
					    + std::string ("[").size ());
				    ipst.erase (
					0,
					ipst.find ("\'")
					    + std::string ("\'").size ());
				    ipst = ipst.substr (0, ipst.find ("\'"));
				    std::vector<char> crv (ipst.begin (),
							   ipst.end ());
				    if (crv.size () > 3)
				      {
					ipst = af.hex_to_string (ipst);
					crv.clear ();
					std::copy (ipst.begin (), ipst.end (),
						   std::back_inserter (crv));
					lt::dht::public_key othpk;
					lt::aux::from_hex (std::get<0> (*it),
							   othpk.bytes.data ());
					std::array<char, 32> scalar;
					std::tuple<lt::dht::public_key,
					    lt::dht::secret_key> okp;
					okp = lt::dht::ed25519_create_keypair (
					    *seed);
					scalar = lt::dht::ed25519_key_exchange (
					    othpk, std::get<1> (okp));
					othpk = lt::dht::ed25519_add_scalar (
					    othpk, scalar);
					std::string unm = lt::aux::to_hex (
					    std::get<0> (okp).bytes);
					std::string passwd = lt::aux::to_hex (
					    othpk.bytes);
					crv = af.decryptStrm (unm, passwd, crv);
				      }
				    ipst = std::string (crv.begin (),
							crv.end ());
				    ipst = ipst.substr (0, ipst.find ("-"));
				    strm.str ("");
				    strm.clear ();
				    strm.imbue (loc);
				    std::string portst = std::string (
					crv.begin (), crv.end ());
				    portst.erase (
					0,
					portst.find ("-")
					    + std::string ("-").size ());
				    uint16_t port;
				    strm << portst;
				    strm >> port;
				    port = htons (port);
				    auto itr = std::find_if (
					rcvd6.begin (), rcvd6.end (), [&othk]
					(auto &el)
					  { return std::get<0>(el) == othk;});
				    if (itr != rcvd6.end ())
				      {
					*itr = make_tuple (othk, ipst, port,
							   seq);
				      }
				    else
				      {
					rcvd6.push_back (
					    std::make_tuple (othk, ipst, port,
							     seq));
				      }
				  }
			      }
			  }
		      }
		  }
		if (alerts[i]->type () == lt::dht_put_alert::alert_type)
		  {
		    std::string msg = alerts[i]->message ();
		    std::cout << msg << std::endl;
		    std::string::size_type n;
		    n = msg.find ("success=0");
		    if (n != std::string::npos)
		      {
			errcount++;
			std::string key = msg;
			key.erase (
			    0,
			    key.find ("key=") + std::string ("key=").size ());
			key = key.substr (0, key.find (" "));
			n = msg.find ("ipv4");
			if (n != std::string::npos)
			  {
			    auto it = std::find_if (
				putvinner.begin (), putvinner.end (), [&key]
				(auto &el)
				  { return std::get<1>(el) == key;});
			    if (it != putvinner.end ())
			      {
				std::string othk = std::get<0> (*it);
				uint32_t ip = std::get<2> (*it);
				uint16_t port = std::get<3> (*it);
				std::tuple<std::string, uint32_t, uint16_t> ttup;
				ttup = std::make_tuple (othk, ip, port);
				putfault.push_back (ttup);
			      }
			  }
			n = msg.find ("ipv6");
			if (n != std::string::npos)
			  {
			    auto it = std::find_if (
				putvinner6.begin (), putvinner6.end (), [&key]
				(auto &el)
				  {
				    return std::get<1>(el) == key;
				  });
			    if (it != putvinner6.end ())
			      {
				std::string othk = std::get<0> (*it);
				putfault6.push_back (othk);
			      }
			  }
		      }
		    else
		      {
			errcount = 0;
			std::string key = msg;
			key.erase (
			    0,
			    key.find ("key=") + std::string ("key=").size ());
			key = key.substr (0, key.find (" "));
			std::string::size_type n;
			n = msg.find ("ipv4");
			if (n != std::string::npos)
			  {
			    putvinner.erase (
				std::remove_if (
				    putvinner.begin (), putvinner.end (), [&key]
				    (auto &el)
				      { return std::get<1>(el) == key;}),
				putvinner.end ());
			  }
			n = msg.find ("ipv6");
			if (n != std::string::npos)
			  {
			    putvinner6.erase (
				std::remove_if (
				    putvinner6.begin (), putvinner6.end (),
				    [&key]
				    (auto &el)
				      {
					return std::get<1>(el) == key;
				      }),
				putvinner6.end ());
			  }
		      }
		  }
	      }
	    for (size_t i = 0; i < rcvd.size (); i++)
	      {
		std::string key = std::get<0> (rcvd[i]);
		getvResult (key, std::get<1> (rcvd[i]), std::get<2> (rcvd[i]),
			    std::get<3> (rcvd[i]));
		getvinner.erase (
		    std::remove_if (getvinner.begin (), getvinner.end (), [&key]
		    (auto &el)
		      { return std::get<0>(el) == key;}),
		    getvinner.end ());
	      }
	    for (size_t i = 0; i < rcvd6.size (); i++)
	      {
		std::string key = std::get<0> (rcvd6[i]);
		getvResult6 (key, std::get<1> (rcvd6[i]),
			     std::get<2> (rcvd6[i]), std::get<3> (rcvd6[i]));
		getvinner6.erase (
		    std::remove_if (getvinner6.begin (), getvinner6.end (),
				    [&key]
				    (auto &el)
				      { return std::get<0>(el) == key;}),
		    getvinner6.end ());
	      }
	    if (errcount > 50)
	      {
		errcount = 0;
		std::cerr
		    << "Network error! Check connection and restart application"
		    << std::endl;
		sleep (60);
	      }
	    else
	      {
		usleep (100000);
	      }
	  }
      }
    else
      {
	std::cerr << "DHT not started, check connection!" << std::endl;
      }

    sespar = ses.session_state (lt::save_state_flags_t::all ());
    sesbuf = lt::write_session_params_buf (sespar, lt::session::save_dht_state);
    if (!std::filesystem::exists (filepath.parent_path ()))
      {
	std::filesystem::create_directories (filepath.parent_path ());
      }

    f.open (filepath, std::ios_base::out | std::ios_base::binary);
    f.write (&sesbuf[0], sesbuf.size ());
    f.close ();
    ses.abort ();
    thrmtx->unlock ();
  });
  dhtthr->detach ();
  delete dhtthr;
}

std::vector<std::string>
NetworkOperations::getFrVect ()
{
  std::vector<std::string> result;
  getfrmtx.lock ();
  result = getfr;
  getfr.clear ();
  getfrmtx.unlock ();
  return result;
}

std::vector<std::tuple<std::string, uint32_t, uint16_t>>
NetworkOperations::putVect ()
{
  std::vector<std::tuple<std::string, uint32_t, uint16_t>> result;
  putipmtx.lock ();
  result = putipv;
  putipv.clear ();
  putipmtx.unlock ();
  return result;
}

std::string
NetworkOperations::getSes (std::string key, lt::session *ses)
{
  AuxFunc af;
  time_t lt = time (NULL);
  std::stringstream strm;
  std::locale loc ("C");
  strm.imbue (loc);
  std::string salt;
  std::string salt6;
  strm << lt / (3600 * 24);
  salt = strm.str ();
  salt = salt + "ipv4";
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*(this->seed));
  lt::dht::public_key otherkey;
  lt::aux::from_hex (key, otherkey.bytes.data ());
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (otherkey, std::get<1> (ownkey));
  otherkey = lt::dht::ed25519_add_scalar (otherkey, scalar);
  otherkey = lt::dht::ed25519_add_scalar (otherkey, scalar);
  std::tuple<lt::dht::public_key, lt::dht::secret_key> newkp;
  newkp = lt::dht::ed25519_create_keypair (otherkey.bytes);
  otherkey = std::get<0> (newkp);
  ses->dht_get_item (otherkey.bytes, salt);
  std::string result = lt::aux::to_hex (otherkey.bytes);
  return result;
}

std::string
NetworkOperations::getSes6 (std::string key, lt::session *ses)
{
  AuxFunc af;
  time_t lt = time (NULL);
  std::stringstream strm;
  std::locale loc ("C");
  strm.imbue (loc);
  std::string salt;
  std::string salt6;
  strm << lt / (3600 * 24);
  salt = strm.str ();
  salt = salt + "ipv6";
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*(this->seed));
  lt::dht::public_key otherkey;
  lt::aux::from_hex (key, otherkey.bytes.data ());
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (otherkey, std::get<1> (ownkey));
  otherkey = lt::dht::ed25519_add_scalar (otherkey, scalar);
  otherkey = lt::dht::ed25519_add_scalar (otherkey, scalar);
  std::tuple<lt::dht::public_key, lt::dht::secret_key> newkp;
  newkp = lt::dht::ed25519_create_keypair (otherkey.bytes);
  otherkey = std::get<0> (newkp);
  ses->dht_get_item (otherkey.bytes, salt);
  std::string result = lt::aux::to_hex (otherkey.bytes);
  return result;
}

std::string
NetworkOperations::putSes (std::string otherkey, uint32_t ip, uint16_t port,
			   lt::session *ses)
{
  AuxFunc af;
  lt::dht::public_key otherpk;
  lt::aux::from_hex (otherkey, otherpk.bytes.data ());
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*seed);
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (otherpk, std::get<1> (ownkey));
  otherpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
  otherpk = lt::dht::ed25519_add_scalar (otherpk, scalar);
  std::tuple<lt::dht::public_key, lt::dht::secret_key> newpk;
  newpk = lt::dht::ed25519_create_keypair (otherpk.bytes);
  time_t curt = time (NULL);
  std::string salt;
  std::stringstream strm;
  std::locale loc ("C");
  strm.imbue (loc);
  strm << curt / (3600 * 24);
  salt = strm.str ();
  salt = salt + "ipv4";
  std::vector<char> dst;
  dst.resize (INET_ADDRSTRLEN);

  std::string putstring = inet_ntop (AF_INET, &ip, dst.data (), dst.size ());
  strm.str ("");
  strm.clear ();
  strm.imbue (loc);
  strm << ntohs (port);

  if (port == 0)
    {
      putstring = putstring + ":sym1234567890123456";
      lt::aux::from_hex (otherkey, otherpk.bytes.data ());
      otherpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
      std::string passwd = lt::aux::to_hex (otherpk.bytes);
      std::vector<char> crv (putstring.begin (), putstring.end ());
      crv = af.cryptStrm (otherkey, passwd, crv);
      putstring = std::string (crv.begin (), crv.end ());
      putstring = af.string_to_hex (putstring);
    }
  else
    {
      putstring = putstring + ":" + strm.str () + "1234567890123456";
      lt::aux::from_hex (otherkey, otherpk.bytes.data ());
      otherpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
      std::string passwd = lt::aux::to_hex (otherpk.bytes);
      std::vector<char> crv (putstring.begin (), putstring.end ());
      crv = af.cryptStrm (otherkey, passwd, crv);
      putstring = std::string (crv.begin (), crv.end ());
      putstring = af.string_to_hex (putstring);
    }
  std::string putkey = lt::aux::to_hex (std::get<0> (newpk).bytes);
  ses->dht_put_item (
      std::get<0> (newpk).bytes,
      std::bind (&AuxFunc::put_string, af, std::placeholders::_1,
		 std::placeholders::_2, std::placeholders::_3,
		 std::placeholders::_4, std::get<0> (newpk).bytes,
		 std::get<1> (newpk).bytes, putstring),
      salt);
  return putkey;
}

std::string
NetworkOperations::putSes6 (std::string otherkey, lt::session *ses)
{
  AuxFunc af;
  lt::dht::public_key otherpk;
  lt::aux::from_hex (otherkey, otherpk.bytes.data ());
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*(this->seed));
  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (otherpk, std::get<1> (ownkey));
  otherpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
  otherpk = lt::dht::ed25519_add_scalar (otherpk, scalar);
  std::tuple<lt::dht::public_key, lt::dht::secret_key> newpk;
  newpk = lt::dht::ed25519_create_keypair (otherpk.bytes);
  time_t curt = time (NULL);
  std::string salt;
  std::stringstream strm;
  std::locale loc ("C");
  strm.imbue (loc);
  strm << curt / (3600 * 24);
  salt = strm.str ();
  salt = salt + "ipv6";
  std::string putstring;
  ownipv6mtx.lock ();
  if (ownipv6 != "" && ownipv6port != 0)
    {
      putstring = ownipv6;
      strm.str ("");
      strm.clear ();
      strm.imbue (loc);
      strm << ntohs (ownipv6port);
      putstring = putstring + "-" + strm.str ();
      lt::aux::from_hex (otherkey, otherpk.bytes.data ());
      otherpk = lt::dht::ed25519_add_scalar (std::get<0> (ownkey), scalar);
      std::string passwd = lt::aux::to_hex (otherpk.bytes);
      std::vector<char> crv (putstring.begin (), putstring.end ());
      crv = af.cryptStrm (otherkey, passwd, crv);
      putstring = std::string (crv.begin (), crv.end ());
      putstring = af.string_to_hex (putstring);
      ses->dht_put_item (
	  std::get<0> (newpk).bytes,
	  std::bind (&AuxFunc::put_string, af, std::placeholders::_1,
		     std::placeholders::_2, std::placeholders::_3,
		     std::placeholders::_4, std::get<0> (newpk).bytes,
		     std::get<1> (newpk).bytes, putstring),
	  salt);
    }
  else
    {
      putstring = "0-0";
      ses->dht_put_item (
	  std::get<0> (newpk).bytes,
	  std::bind (&AuxFunc::put_string, af, std::placeholders::_1,
		     std::placeholders::_2, std::placeholders::_3,
		     std::placeholders::_4, std::get<0> (newpk).bytes,
		     std::get<1> (newpk).bytes, putstring),
	  salt);
    }
  ownipv6mtx.unlock ();
  std::string result = lt::aux::to_hex (std::get<0> (newpk).bytes);

  return result;
}

void
NetworkOperations::getvResult (std::string key, uint32_t ip, uint16_t port,
			       int seq)
{
  std::tuple<std::string, uint32_t, uint16_t, int> ttup;
  std::string keyloc = key;
  ttup = std::make_tuple (key, ip, port, seq);
  std::vector<char> temp;
  uint32_t iploc = ip;
  temp.resize (INET_ADDRSTRLEN);
  if (inet_ntop (AF_INET, &iploc, temp.data (), temp.size ()))
    {
      std::cout << "ip4 " << key << " " << temp.data () << ":" << ntohs (port)
	  << " seq=" << seq << std::endl;
      contmtx.lock ();
      auto itc = std::find_if (contacts.begin (), contacts.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<1>(el) == keyloc;
	});
      if (itc != contacts.end ())
	{
	  getfrresmtx.lock ();
	  auto it = std::find_if (getfrres.begin (), getfrres.end (), [&keyloc]
	  (auto &el)
	    {
	      return std::get<0>(el) == keyloc;
	    });
	  if (it == getfrres.end ())
	    {
	      if (ip != 0)
		{
		  getfrres.push_back (ttup);
		  maintblockmtx.lock ();
		  time_t bltm = time (NULL);
		  auto itmnt = std::find_if (
		      maintblock.begin (), maintblock.end (), [keyloc]
		      (auto &el)
			{
			  return std::get<0>(el) == keyloc;
			});
		  if (itmnt != maintblock.end ())
		    {
		      std::get<1> (*itmnt) = bltm;
		    }
		  else
		    {
		      maintblock.push_back (std::make_tuple (key, bltm));
		    }
		  maintblockmtx.unlock ();
		}
	    }
	  else
	    {
	      if (std::get<3> (*it) < seq && ip != 0)
		{
		  *it = ttup;
		  maintblockmtx.lock ();
		  time_t bltm = time (NULL);
		  auto itmnt = std::find_if (
		      maintblock.begin (), maintblock.end (), [keyloc]
		      (auto &el)
			{
			  return std::get<0>(el) == keyloc;
			});
		  if (itmnt != maintblock.end ())
		    {
		      std::get<1> (*itmnt) = bltm;
		    }
		  else
		    {
		      maintblock.push_back (std::make_tuple (key, bltm));
		    }
		  maintblockmtx.unlock ();
		}
	    }
	  getfrresmtx.unlock ();
	}
      contmtx.unlock ();
    }
  else
    {
#ifdef __linux
      std::cerr << "DHT error on getting ipv4: " << errno << std::endl;
#endif
#ifdef _WIN32
      int respol = WSAGetLastError ();
      std::cerr << "DHT error on getting ipv4: " << respol << std::endl;
#endif

    }
}

void
NetworkOperations::getvResult6 (std::string key, std::string ip, uint16_t port,
				int seq)
{
  std::tuple<std::string, std::string, uint16_t, int> ttup;
  std::string keyloc = key;
  ttup = std::make_tuple (key, ip, port, seq);
  std::cout << "ip6 " << key << " " << ip << " " << ntohs (port) << " seq="
      << seq << std::endl;
  contmtx.lock ();
  auto itc = std::find_if (contacts.begin (), contacts.end (), [&keyloc]
  (auto &el)
    {
      return std::get<1>(el) == keyloc;
    });
  if (itc != contacts.end ())
    {
      ipv6contmtx.lock ();
      auto it = std::find_if (ipv6cont.begin (), ipv6cont.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	});
      if (it != ipv6cont.end ())
	{
	  if (seq >= std::get<3> (*it))
	    {
	      *it = ttup;
	    }
	}
      else
	{
	  ipv6cont.push_back (ttup);
	}
      ipv6contmtx.unlock ();
    }
  contmtx.unlock ();

}

void
NetworkOperations::checkMsg (std::filesystem::path p, int *check)
{
  sendbufmtx.lock ();
  if (std::filesystem::exists (p))
    {
      *check = 1;
    }
  sendbufmtx.unlock ();
}

int
NetworkOperations::createMsg (std::string key, std::filesystem::path p,
			      int type)
{
  AuxFunc af;
  lt::dht::public_key othkey;
  std::string usname = key;
  std::string passwd;
  int result = -1;
  lt::aux::from_hex (usname, othkey.bytes.data ());
  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
  okp = lt::dht::ed25519_create_keypair (*seed);

  std::array<char, 32> scalar;
  scalar = lt::dht::ed25519_key_exchange (othkey, std::get<1> (okp));
  othkey = lt::dht::ed25519_add_scalar (othkey, scalar);
  passwd = lt::aux::to_hex (othkey.bytes);
  contmtx.lock ();
  auto itcont = std::find_if (contacts.begin (), contacts.end (), [&usname]
  (auto &el)
    {
      return std::get<1>(el) == usname;
    });
  if (itcont != contacts.end ())
    {
      int ind = std::get<0> (*itcont);
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << ind;
      std::string index = strm.str ();
      std::string filename;
      af.homePath (&filename);
      filename = filename + "/.Communist/" + index;
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      std::vector<std::filesystem::path> pv;
      if (std::filesystem::exists (filepath))
	{
	  for (auto &dit : std::filesystem::directory_iterator (filepath))
	    {
	      std::filesystem::path d = dit.path ();
	      if (d.filename ().u8string () != "Profile"
		  && d.filename ().u8string () != "Yes")
		{
		  pv.push_back (d);
		}
	    }
	  std::string filenm;
	  if (pv.size () == 0)
	    {
	      filenm = "0";
	      result = 0;
	    }
	  else
	    {
	      std::sort (pv.begin (), pv.end (), []
	      (auto &el1, auto &el2)
		{
		  int first;
		  int second;
		  std::stringstream strm;
		  std::locale loc ("C");
		  strm.imbue(loc);
		  strm << el1.filename().u8string ();
		  strm >> first;
		  strm.str("");
		  strm.clear();
		  strm.imbue(loc);
		  strm << el2.filename().u8string();
		  strm >> second;
		  return first < second;
		});
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << pv[pv.size () - 1].filename ().u8string ();
	      int msgind;
	      strm >> msgind;
	      msgind = msgind + 1;
	      result = msgind;
	      strm.str ("");
	      strm.clear ();
	      strm.imbue (loc);
	      strm << msgind;
	      filenm = strm.str ();
	    }
	  if (type == 1)
	    {
	      filenm = filenm + "f";
	    }
	  filename = filename + "/" + filenm;
	  filepath = std::filesystem::u8path (filename);
	  af.cryptFile (usname, passwd, p.u8string (), filepath.u8string ());
	  af.homePath (&filename);
	  filename = filename + "/.Communist/SendBufer/" + index + "/" + filenm;
	  filepath = std::filesystem::u8path (filename);
	  sendbufmtx.lock ();
	  if (!std::filesystem::exists (filepath.parent_path ()))
	    {
	      std::filesystem::create_directories (filepath.parent_path ());
	    }
	  af.cryptFile (usname, passwd, p.u8string (), filepath.u8string ());
	  sendbufmtx.unlock ();
	  std::filesystem::remove (p);
	}
    }
  contmtx.unlock ();

  return result;
}

void
NetworkOperations::renewProfile (std::string key)
{
  std::string lockey = key;
  contmtx.lock ();
  auto it = std::find_if (contacts.begin (), contacts.end (), [&lockey]
  (auto &el)
    {
      return std::get<1>(el) == lockey;
    });
  if (it != contacts.end ())
    {
      AuxFunc af;
      int ind = std::get<0> (*it);
      std::locale loc ("C");
      std::stringstream strm;
      strm.imbue (loc);
      strm << ind;
      std::string index = strm.str ();
      std::string filename;
      af.homePath (&filename);
      filename = filename + "/.Communist/Profile";
      std::filesystem::path source = std::filesystem::u8path (filename);
#ifdef __linux
      filename = std::filesystem::temp_directory_path ().u8string ();
#endif
#ifdef _WIN32
      filename = std::filesystem::temp_directory_path ().parent_path ().u8string ();
#endif
      filename = filename + "/ComRP/Profile.zip";
      std::filesystem::path outpath = std::filesystem::u8path (filename);
      if (std::filesystem::exists (outpath.parent_path ()))
	{
	  std::filesystem::remove_all (outpath.parent_path ());
	}
      std::filesystem::create_directories (outpath.parent_path ());
      af.decryptFile (Username, Password, source.u8string (),
		      outpath.u8string ());
      af.unpacking (outpath.u8string (), outpath.parent_path ().u8string ());
      std::filesystem::remove (outpath);
      filename = outpath.parent_path ().u8string ();
      filename = filename + "/Profile";
      source = std::filesystem::u8path (filename);
      std::vector<std::filesystem::path> pv;
      for (auto &dit : std::filesystem::directory_iterator (source))
	{
	  std::filesystem::path p = dit.path ();
	  if (p.filename ().u8string () != "Profile"
	      && p.filename ().u8string () != "Avatar.jpeg")
	    {
	      pv.push_back (p);
	    }
	}
      for (size_t i = 0; i < pv.size (); i++)
	{
	  std::filesystem::remove_all (pv[i]);
	}

      af.packing (source.u8string (), outpath.u8string ());
      source = outpath;
      af.homePath (&filename);
      filename = filename + "/.Communist/SendBufer/" + index + "/Profile";
      outpath = std::filesystem::u8path (filename);
      sendbufmtx.lock ();
      if (!std::filesystem::exists (outpath.parent_path ()))
	{
	  std::filesystem::create_directories (outpath.parent_path ());
	}
      std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
      okp = lt::dht::ed25519_create_keypair (*seed);
      lt::dht::public_key opk;
      std::string unm = key;
      lt::aux::from_hex (unm, opk.bytes.data ());
      std::array<char, 32> scalar;
      scalar = lt::dht::ed25519_key_exchange (opk, std::get<1> (okp));
      opk = lt::dht::ed25519_add_scalar (opk, scalar);
      std::string passwd = lt::aux::to_hex (opk.bytes);
      af.cryptFile (unm, passwd, source.u8string (), outpath.u8string ());
      sendbufmtx.unlock ();
      std::filesystem::remove_all (source.parent_path ());
    }
  contmtx.unlock ();
}

void
NetworkOperations::commOps ()
{
  if (Netmode == "1")
    {
      bootstrFunc ();
    }
  sockmtx.lock ();
  getfrmtx.lock ();
  for (size_t i = 0; i < sockets4.size (); i++)
    {
      getfr.push_back (std::get<0> (sockets4[i]));
    }
  getfrmtx.unlock ();
  size_t sizesock = sockets4.size () + 1;
  sockmtx.unlock ();
  std::vector<std::tuple<std::string, uint32_t, uint16_t, time_t>> ownips;
  std::mutex *ownipsmtx = new std::mutex;
  std::vector<std::tuple<std::string, time_t>> blockip;
  std::mutex *blockipmtx = new std::mutex;
  std::vector<std::tuple<std::string, time_t>> lastsent;
  std::mutex *lastsentmtx = new std::mutex;
  std::vector<std::string> sendingthr;
  std::mutex *sendingthrmtx = new std::mutex;
  std::vector<std::tuple<std::string, time_t>> blockchsock;
  for (;;)
    {
      sockmtx.lock ();
      if (cancel > 0 || sockets4.size () == 0)
	{
	  sockmtx.unlock ();
	  break;
	}
      if (threadvectmtx.try_lock ())
	{
	  for (;;)
	    {
	      auto itthrv = std::find_if (
		  threadvect.begin (), threadvect.end (), []
		  (auto &el)
		    {
		      std::mutex *thrmtx = std::get<0>(el);
		      if (thrmtx->try_lock())
			{
			  thrmtx->unlock();
			  return true;
			}
		      else
			{
			  return false;
			}
		    });
	      if (itthrv != threadvect.end ())
		{
		  std::mutex *thrmtx = std::get<0> (*itthrv);
		  threadvect.erase (itthrv);
		  delete thrmtx;
		}
	      else
		{
		  break;
		}
	    }

	  threadvectmtx.unlock ();
	}

      std::vector<std::string> fordel;

      //Clean blockchsock vector
      for (size_t i = 0; i < blockchsock.size (); i++)
	{
	  std::string key = std::get<0> (blockchsock[i]);
	  auto it = std::find_if (sockets4.begin (), sockets4.end (), [&key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it == sockets4.end ())
	    {
	      fordel.push_back (key);
	    }
	}
      for (size_t i = 0; i < fordel.size (); i++)
	{
	  std::string key = fordel[i];
	  blockchsock.erase (
	      std::remove_if (blockchsock.begin (), blockchsock.end (), [&key]
	      (auto &el)
		{ return std::get<0>(el) == key;}),
	      blockchsock.end ());
	}
      fordel.clear ();

      //Clean ownips vector
      ownipsmtx->lock ();
      for (size_t i = 0; i < ownips.size (); i++)
	{
	  std::string key = std::get<0> (ownips[i]);
	  auto it = std::find_if (sockets4.begin (), sockets4.end (), [&key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it == sockets4.end ())
	    {
	      fordel.push_back (key);
	    }
	}
      for (size_t i = 0; i < fordel.size (); i++)
	{
	  std::string key = fordel[i];
	  ownips.erase (std::remove_if (ownips.begin (), ownips.end (), [&key]
	  (auto &el)
	    { return std::get<0>(el) == key;}),
			ownips.end ());
	}
      ownipsmtx->unlock ();
      fordel.clear ();

      //Clean blockip vector
      blockipmtx->lock ();
      for (size_t i = 0; i < blockip.size (); i++)
	{
	  std::string key = std::get<0> (blockip[i]);
	  auto it = std::find_if (sockets4.begin (), sockets4.end (), [&key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it == sockets4.end ())
	    {
	      fordel.push_back (key);
	    }
	}
      for (size_t i = 0; i < fordel.size (); i++)
	{
	  std::string key = fordel[i];
	  blockip.erase (std::remove_if (blockip.begin (), blockip.end (), [key]
	  (auto &el)
	    { return std::get<0>(el) == key;}),
			 blockip.end ());
	}
      blockipmtx->unlock ();
      fordel.clear ();

      lastsentmtx->lock ();
      for (size_t i = 0; i < lastsent.size (); i++)
	{
	  std::string key = std::get<0> (lastsent[i]);
	  auto it = std::find_if (sockets4.begin (), sockets4.end (), [&key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it == sockets4.end ())
	    {
	      fordel.push_back (key);
	    }
	}
      for (size_t i = 0; i < fordel.size (); i++)
	{
	  std::string key = fordel[i];
	  lastsent.erase (
	      std::remove_if (lastsent.begin (), lastsent.end (), [key]
	      (auto &el)
		{ return std::get<0>(el) == key;}),
	      lastsent.end ());
	}
      fordel.clear ();
      lastsentmtx->unlock ();
      sockmtx.unlock ();

      contmtx.lock ();
      for (size_t i = 0; i < contacts.size (); i++)
	{
	  smthrcvdsig.emit (std::get<1> (contacts[i]), 0);
	}
      contmtx.unlock ();

      //Change socket
      sockmtx.lock ();
      for (size_t i = 0; i < sockets4.size (); i++)
	{
	  time_t curtime = time (NULL);
	  time_t lr = std::get<3> (sockets4[i]);
	  std::string key = std::get<0> (sockets4[i]);
	  int addtime = 1;
	  holepunchstopmtx.lock ();
	  auto hpsit = std::find_if (holepunchstop.begin (),
				     holepunchstop.end (), [&key]
				     (auto &el)
				       {
					 return std::get<0>(el) == key;
				       });
	  if (hpsit != holepunchstop.end ())
	    {
	      addtime = 2;
	    }

	  std::shared_ptr<std::mutex> smtx = std::get<2> (sockets4[i]);
	  int sock = std::get<1> (sockets4[i]);
	  auto it = std::find_if (blockchsock.begin (), blockchsock.end (),
				  [&key]
				  (auto &el)
				    {
				      return std::get<0>(el) == key;
				    });
	  if (it == blockchsock.end ())
	    {
	      blockchsock.push_back (std::make_tuple (key, curtime));
	    }
	  else
	    {
	      time_t lch = std::get<1> (*it);
	      if (curtime - lr > Tmttear
		  && curtime - lch > Tmttear * 5 * addtime)
		{
		  std::get<1> (*it) = curtime;
		  if (smtx)
		    {
		      smtx->lock ();
		      if (sock >= 0)
			{
#ifdef __linux
			  close (sock);
#endif
#ifdef _WIN32
                          closesocket (sock);
#endif
			}
#ifdef __linux
		      sock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK,
		      IPPROTO_UDP);
#endif
#ifdef _WIN32
		      sock = socket (AF_INET, SOCK_DGRAM,
		      IPPROTO_UDP);
		      u_long nonblocking_enabled = TRUE;
		      ioctlsocket (sock, FIONBIO, &nonblocking_enabled);
#endif
		      sockaddr_in addripv4 =
			{ };
		      addripv4.sin_family = AF_INET;
		      IPV4mtx.lock ();
		      inet_pton (AF_INET, IPV4.c_str (),
				 &addripv4.sin_addr.s_addr);
		      IPV4mtx.unlock ();
		      addripv4.sin_port = 0;
		      int addrlen1 = sizeof(addripv4);
		      bind (sock, (const sockaddr*) &addripv4, addrlen1);
		      std::get<1> (this->sockets4[i]) = sock;
		      std::get<3> (sockets4[i]) = curtime;
		      std::cerr << "Socket changed on " << key << std::endl;
		      smtx->unlock ();
		      if (hpsit != holepunchstop.end ())
			{
			  holepunchstop.erase (hpsit);
			}
		    }

		  ownipsmtx->lock ();
		  auto itoip = std::find_if (ownips.begin (), ownips.end (),
					     [&key]
					     (auto &el)
					       {
						 return std::get<0>(el) == key;
					       });
		  if (itoip != ownips.end ())
		    {
		      if (Netmode == "0")
			{
			  ownips.erase (itoip);
			}
		      if (Netmode == "1")
			{
			  uint32_t ip;
			  IPV4mtx.lock ();
			  inet_pton (AF_INET, IPV4.c_str (), &ip);
			  IPV4mtx.unlock ();
			  sockaddr_in addressp =
			    { };
			  uint16_t port;
#ifdef __linux
			  uint len = sizeof(addressp);
#endif
#ifdef _WIN32
			  int len = sizeof(addressp);
#endif
			  getsockname (std::get<1> (sockets4[i]),
				       (sockaddr*) &addressp, &len);
			  port = addressp.sin_port;
			  std::get<1> (*itoip) = ip;
			  std::get<2> (*itoip) = port;
			  std::get<3> (*itoip) = curtime;
			}
		    }
		  ownipsmtx->unlock ();
		}
	    }
	  holepunchstopmtx.unlock ();
	}
      sockmtx.unlock ();

      //Receive own ips
      if (Netmode == "0")
	{

	  std::mutex *thrmtx = nullptr;
	  if (threadvectmtx.try_lock ())
	    {
	      thrmtx = new std::mutex;
	      thrmtx->lock ();
	      threadvect.push_back (std::make_tuple (thrmtx, "Own ips"));
	      threadvectmtx.unlock ();
	    }
	  if (thrmtx != nullptr)
	    {
	      std::thread *throip = new std::thread (
		  [&ownips, ownipsmtx, this, thrmtx]
		  {
		    std::vector<std::pair<uint32_t, uint16_t>> ips;
		    time_t curtime = time (NULL);
		    this->sockmtx.lock ();
		    for (size_t i = 0; i < this->sockets4.size (); i++)
		      {
			std::string key = std::get<0> (this->sockets4[i]);
			int sock = std::get<1> (this->sockets4[i]);
			std::shared_ptr<std::mutex> smtx = std::get<4> (
			    this->sockets4[i]);
			ownipsmtx->lock ();
			auto it = std::find_if (
			    ownips.begin (), ownips.end (), [&key]
			    (auto &el)
			      {
				return std::get<0>(el) == key;
			      });
			if (it == ownips.end ())
			  {
			    std::vector<size_t> rplcstun;
			    this->stunipsmtx.lock ();
			    for (size_t j = 0; j < this->stunips.size (); j++)
			      {
				smtx->lock ();
				std::pair<uint32_t, uint16_t> p =
				    this->getOwnIps (sock, this->stunips[j]);
				if (p.first != 0)
				  {
				    ips.push_back (p);
				  }
				else
				  {
				    rplcstun.push_back (j);
				  }
				smtx->unlock ();
				if (ips.size () == 3)
				  {
				    break;
				  }
			      }
			    for (size_t j = 0; j < rplcstun.size (); j++)
			      {
				std::pair<struct in_addr, uint16_t> replpair;
				replpair = this->stunips[rplcstun[j]];
				this->stunips.erase (
				    this->stunips.begin () + rplcstun[j]);
				this->stunips.push_back (replpair);
			      }
			    this->stunipsmtx.unlock ();
			    for (size_t j = 0; j < ips.size (); j++)
			      {
				if (j > 0)
				  {
				    if (ips[0] != ips[j])
				      {
					ips[0].second = 0;
					break;
				      }
				  }
			      }

			    if (ips.size () > 0)
			      {
				std::tuple<std::string, uint32_t, uint16_t,
				    time_t> ttup;
				ttup = std::make_tuple (key, ips[0].first,
							ips[0].second, curtime);
				ownips.push_back (ttup);
				this->putOwnIps (key, ips[0].first,
						 ips[0].second);
			      }
			    ips.clear ();
			  }
			else
			  {
			    time_t curtime = time (NULL);
			    time_t lr = std::get<3> (this->sockets4[i]);
			    if (curtime - lr > Tmttear
				&& curtime - lr <= Shuttmt
				&& curtime - std::get<3> (*it) > Tmttear)
			      {
				std::vector<size_t> rplcstun;
				this->stunipsmtx.lock ();
				for (size_t j = 0; j < this->stunips.size ();
				    j++)
				  {
				    smtx->lock ();
				    std::pair<uint32_t, uint16_t> p =
					this->getOwnIps (sock,
							 this->stunips[j]);
				    if (p.first != 0)
				      {
					ips.push_back (p);
				      }
				    else
				      {
					rplcstun.push_back (j);
				      }
				    smtx->unlock ();
				    if (ips.size () == 3)
				      {
					break;
				      }
				  }
				for (size_t j = 0; j < rplcstun.size (); j++)
				  {
				    std::pair<struct in_addr, uint16_t> replpair;
	      replpair = this->stunips[rplcstun[j]];
	      this->stunips.erase (
		  this->stunips.begin () + rplcstun[j]);
	      this->stunips.push_back (replpair);
	    }
				this->stunipsmtx.unlock ();
				for (size_t j = 0; j < ips.size (); j++)
				  {
				    if (j > 0)
				      {
					if (ips[0] != ips[j])
					  {
					    ips[0].second = 0;
					    break;
					  }
				      }
				  }
				if (ips.size () > 0)
				  {
				    std::tuple<std::string, uint32_t, uint16_t,
					time_t> ttup;
				    ttup = std::make_tuple (key, ips[0].first,
							    ips[0].second,
							    curtime);
				    if (std::get<1> (ttup) != std::get<1> (*it)
					|| std::get<1> (ttup)
					    != std::get<1> (*it))
				      {
					this->putOwnIps (key, ips[0].first,
							 ips[0].second);
				      }
				    *it = ttup;
				  }
				ips.clear ();

			      }
			    if (curtime - lr > Shuttmt
				&& curtime - std::get<3> (*it) >= 300)
			      {
				std::vector<size_t> rplcstun;
				this->stunipsmtx.lock ();
				for (size_t j = 0; j < this->stunips.size ();
				    j++)
				  {
				    smtx->lock ();
				    std::pair<uint32_t, uint16_t> p =
					this->getOwnIps (sock,
							 this->stunips[j]);
				    if (p.first != 0)
				      {
					ips.push_back (p);
				      }
				    else
				      {
					rplcstun.push_back (j);
				      }
				    smtx->unlock ();
				    if (ips.size () == 3)
				      {
					break;
				      }
				  }
				for (size_t j = 0; j < rplcstun.size (); j++)
				  {
				    std::pair<struct in_addr, uint16_t> replpair;
				    replpair = this->stunips[rplcstun[j]];
				    this->stunips.erase (
					this->stunips.begin () + rplcstun[j]);
				    this->stunips.push_back (replpair);
				  }
				this->stunipsmtx.unlock ();
				for (size_t j = 0; j < ips.size (); j++)
				  {
				    if (j > 0)
				      {
					if (ips[0] != ips[j])
					  {
					    ips[0].second = 0;
					    break;
					  }
				      }
				  }
				if (ips.size () > 0)
				  {
				    std::tuple<std::string, uint32_t, uint16_t,
					time_t> ttup;
				    ttup = std::make_tuple (key, ips[0].first,
							    ips[0].second,
							    curtime);
				    if (std::get<1> (ttup) != std::get<1> (*it)
					|| std::get<1> (ttup)
					    != std::get<1> (*it))
				      {
					this->putOwnIps (key, ips[0].first,
							 ips[0].second);
				      }
				    *it = ttup;
				  }
				ips.clear ();
			      }
			  }
			ownipsmtx->unlock ();
		      }
		    this->sockmtx.unlock ();
		    thrmtx->unlock ();
		  });
		  throip->detach ();
		  delete throip;
		}
	    }
      if (Netmode == "1" && ownips.size () == 0)
	{

	  time_t curtime = time (NULL);
	  uint32_t ip;
	  IPV4mtx.lock ();
	  inet_pton (AF_INET, IPV4.c_str (), &ip);
	  IPV4mtx.unlock ();
	  sockmtx.lock ();
	  for (size_t i = 0; i < sockets4.size (); i++)
	    {
	      sockaddr_in addressp =
		{ };
	      uint16_t port;
#ifdef __linux
	      uint len = sizeof(addressp);
#endif
#ifdef _WIN32
	      int len = sizeof(addressp);
#endif
	      getsockname (std::get<1> (sockets4[i]), (sockaddr*) &addressp,
			   &len);
	      port = addressp.sin_port;
	      std::string key = std::get<0> (sockets4[i]);
	      ownipsmtx->lock ();
	      auto itoip = std::find_if (ownips.begin (), ownips.end (), [&key]
	      (auto &el)
		{
		  return std::get<0>(el) == key;
		});
	      if (itoip == ownips.end ())
		{
		  ownips.push_back (std::make_tuple (key, ip, port, curtime));
		}
	      ownipsmtx->unlock ();
	    }
	  sockmtx.unlock ();
	}

      //Get friends ips
      if (Netmode == "0")
	{

	  std::mutex *thrmtx = nullptr;
	  if (threadvectmtx.try_lock ())
	    {
	      thrmtx = new std::mutex;
	      thrmtx->lock ();
	      threadvect.push_back (std::make_tuple (thrmtx, "Friend ips"));
	      threadvectmtx.unlock ();
	    }
	  if (thrmtx != nullptr)
	    {
	      std::thread *throthip = new std::thread (
		  [this, &blockip, blockipmtx, thrmtx]
		  {
		    time_t curtime = time (NULL);

		    this->sockmtx.lock ();
		    for (size_t i = 0; i < this->sockets4.size (); i++)
		      {
			time_t lastrcvd = std::get<3> (this->sockets4[i]);
			std::string key = std::get<0> (this->sockets4[i]);
			time_t blocktm = 0;
			blockipmtx->lock ();
			auto itbl = std::find_if (
			    blockip.begin (), blockip.end (), [&key]
			    (auto &el)
			      {
				return std::get<0>(el) == key;
			      });
			if (itbl != blockip.end ())
			  {
			    blocktm = std::get<1> (*itbl);
			  }
			else
			  {
			    blockip.push_back (std::make_tuple (key, curtime));
			  }

			if (curtime - lastrcvd > Tmttear
			    && curtime - lastrcvd <= Shuttmt
			    && curtime > blocktm)
			  {
			    this->getfrmtx.lock ();
			    auto itgfr = std::find (this->getfr.begin (),
						    this->getfr.end (), key);
			    if (itgfr == this->getfr.end ())
			      {
				this->getfr.push_back (key);
			      }
			    itbl = std::find_if (
				blockip.begin (), blockip.end (), [&key]
				(auto &el)
				  {
				    return std::get<0>(el) == key;
				  });
			    if (itbl != blockip.end ())
			      {
				std::get<1> (*itbl) = curtime;
			      }
			    this->getfrmtx.unlock ();
			  }
			if (curtime - lastrcvd > Shuttmt)
			  {
			    if (curtime - blocktm >= Tmttear)
			      {
				this->getfrmtx.lock ();
				auto itgfr = std::find (this->getfr.begin (),
							this->getfr.end (),
							key);
				if (itgfr == this->getfr.end ())
				  {
				    this->getfr.push_back (key);
				  }
				if (itbl != blockip.end ())
				  {
				    std::get<1> (*itbl) = curtime;
				  }
				this->getfrmtx.unlock ();
			      }
			  }
			blockipmtx->unlock ();
		      }
		    this->sockmtx.unlock ();
		    thrmtx->unlock ();
		  });
	      throthip->detach ();
	      delete throthip;
	    }
	}

      //Connection maintenance
      sockmtx.lock ();
      for (size_t i = 0; i < sockets4.size (); i++)
	{
	  std::string key = std::get<0> (sockets4[i]);
	  time_t lr = std::get<3> (sockets4[i]);
	  time_t curtime = time (NULL);
	  time_t blocktime = 0;
	  time_t blockmaint = 0;
	  maintblockmtx.lock ();
	  auto itmnt = std::find_if (maintblock.begin (), maintblock.end (),
				     [key]
				     (auto &el)
				       {
					 return std::get<0>(el) == key;
				       });
	  if (itmnt != maintblock.end ())
	    {
	      blockmaint = std::get<1> (*itmnt);
	    }
	  else
	    {
	      maintblock.push_back (std::make_tuple (key, curtime));
	      blockmaint = curtime;
	    }
	  maintblockmtx.unlock ();
	  if (curtime - blockmaint <= Shuttmt)
	    {
	      lastsentmtx->lock ();
	      auto lsit = std::find_if (lastsent.begin (), lastsent.end (),
					[&key]
					(auto &el)
					  {
					    return std::get<0>(el) == key;
					  });
	      if (lsit != lastsent.end ())
		{
		  blocktime = std::get<1> (*lsit);
		}

	      if (curtime - blocktime > 1)
		{
		  if (lsit != lastsent.end ())
		    {
		      std::get<1> (*lsit) = curtime;
		    }
		  else
		    {
		      lastsent.push_back (std::make_tuple (key, curtime));
		    }
		  int s = 0;
		  ipv6contmtx.lock ();
		  auto it6 = std::find_if (ipv6cont.begin (), ipv6cont.end (),
					   [&key]
					   (auto &el)
					     {
					       return std::get<0>(el) == key;
					     });
		  if (it6 != ipv6cont.end () && ownipv6 != ""
		      && ownipv6port != 0)
		    {
		      std::string ip6 = std::get<1> (*it6);
		      uint16_t port = std::get<2> (*it6);
		      if (ip6 != "" && ip6 != "0" && port != 0)
			{
			  std::vector<char> msg;
			  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
			  ownkey = lt::dht::ed25519_create_keypair (
			      *(this->seed));
			  std::array<char, 32> keyarr;
			  keyarr = std::get<0> (ownkey).bytes;
			  std::copy (keyarr.begin (), keyarr.end (),
				     std::back_inserter (msg));
			  msg.push_back ('T');
			  msg.push_back ('T');
			  std::string unm = key;
			  lt::dht::public_key othpk;
			  AuxFunc af;
			  lt::aux::from_hex (unm, othpk.bytes.data ());
			  std::array<char, 32> scalar;
			  scalar = lt::dht::ed25519_key_exchange (
			      othpk, std::get<1> (ownkey));
			  othpk = lt::dht::ed25519_add_scalar (othpk, scalar);
			  std::string passwd = lt::aux::to_hex (othpk.bytes);
			  msg = af.cryptStrm (unm, passwd, msg);
			  sockipv6mtx.lock ();
			  s = sendMsg6 (sockipv6, ip6, port, msg);
			  std::cout << "Maintenance message to " << ip6 << " "
			      << ntohs (port) << std::endl;
			  sockipv6mtx.unlock ();
			}
		    }
		  ipv6contmtx.unlock ();
		  if (s <= 0)
		    {
		      getfrresmtx.lock ();
		      auto it4 = std::find_if (
			  getfrres.begin (), getfrres.end (), [&key]
			  (auto &el)
			    {
			      return std::get<0>(el) == key;
			    });
		      if (it4 != getfrres.end ())
			{
			  int sock = std::get<1> (sockets4[i]);
			  uint32_t ip = std::get<1> (*it4);
			  uint16_t port = std::get<2> (*it4);
			  if (ip != 0)
			    {
			      ownipsmtx->lock ();
			      auto itoip = std::find_if (
				  ownips.begin (), ownips.end (), [&key]
				  (auto &el)
				    {
				      return std::get<0>(el) == key;
				    });
			      if (itoip != ownips.end ())
				{
				  if (port != 0 && std::get<2> (*itoip) != 0)
				    {
				      std::vector<char> msg;
				      std::tuple<lt::dht::public_key,
					  lt::dht::secret_key> ownkey;
				      ownkey = lt::dht::ed25519_create_keypair (
					  *(this->seed));
				      std::array<char, 32> keyarr;
				      keyarr = std::get<0> (ownkey).bytes;
				      std::copy (keyarr.begin (), keyarr.end (),
						 std::back_inserter (msg));
				      msg.push_back ('T');
				      msg.push_back ('T');
				      std::string unm = key;
				      lt::dht::public_key othpk;
				      AuxFunc af;
				      lt::aux::from_hex (unm,
							 othpk.bytes.data ());
				      std::array<char, 32> scalar;
				      scalar = lt::dht::ed25519_key_exchange (
					  othpk, std::get<1> (ownkey));
				      othpk = lt::dht::ed25519_add_scalar (
					  othpk, scalar);
				      std::string passwd = lt::aux::to_hex (
					  othpk.bytes);
				      msg = af.cryptStrm (unm, passwd, msg);
				      std::shared_ptr<std::mutex> mtx =
					  std::get<2> (sockets4[i]);
				      mtx->lock ();
				      sendMsg (sock, ip, port, msg);
				      mtx->unlock ();
				      std::vector<char> tmpv;
				      tmpv.resize (INET_ADDRSTRLEN);
				      std::cout << "Maintenance message to "
					  << inet_ntop (AF_INET, &ip,
							tmpv.data (),
							tmpv.size ()) << ":"
					  << ntohs (port) << std::endl;
				    }
				  else
				    {
				      if (curtime - lr > Tmttear)
					{
					  std::mutex *thrmtx = nullptr;
					  if (threadvectmtx.try_lock ())
					    {
					      thrmtx = new std::mutex;
					      thrmtx->lock ();
					      threadvect.push_back (
						  std::make_tuple (
						      thrmtx,
						      "Connection maintenance"));
					      threadvectmtx.unlock ();
					    }
					  if (thrmtx != nullptr)
					    {
					      std::thread *hpthr =
						  new std::thread (
						      [this, i, curtime, sock,
						       ip, thrmtx]
						      {
							this->sockmtx.lock ();
							time_t lrcvd = std::get<
							    3> (sockets4[i]);
							this->sockmtx.unlock ();
							if (curtime - lrcvd
							    > Tmttear)
							  {
							    this->sockmtx.lock ();
							    std::shared_ptr<
								std::mutex> mtx =
								std::get<2> (
								    sockets4[i]);
							    std::string otherkey =
								std::get<0> (
								    sockets4[i]);
							    this->sockmtx.unlock ();
							    mtx->lock ();
							    holePunch (
								sock, ip,
								otherkey);
							    mtx->unlock ();
							  }
							thrmtx->unlock ();
						      });
					      hpthr->detach ();
					      delete hpthr;
					    }
					}
				      else
					{
					  if (port != 0)
					    {
					      std::vector<char> msg;
					      std::tuple<lt::dht::public_key,
						  lt::dht::secret_key> ownkey;
					      ownkey =
						  lt::dht::ed25519_create_keypair (
						      *(this->seed));
					      std::array<char, 32> keyarr;
					      keyarr =
						  std::get<0> (ownkey).bytes;
					      std::copy (
						  keyarr.begin (),
						  keyarr.end (),
						  std::back_inserter (msg));
					      msg.push_back ('T');
					      msg.push_back ('T');
					      std::string unm = key;
					      lt::dht::public_key othpk;
					      AuxFunc af;
					      lt::aux::from_hex (
						  unm, othpk.bytes.data ());
					      std::array<char, 32> scalar;
					      scalar =
						  lt::dht::ed25519_key_exchange (
						      othpk,
						      std::get<1> (ownkey));
					      othpk =
						  lt::dht::ed25519_add_scalar (
						      othpk, scalar);
					      std::string passwd =
						  lt::aux::to_hex (othpk.bytes);
					      msg = af.cryptStrm (unm, passwd,
								  msg);
					      std::shared_ptr<std::mutex> mtx =
						  std::get<2> (sockets4[i]);
					      mtx->lock ();
					      sendMsg (sock, ip, port, msg);
					      mtx->unlock ();
					      std::vector<char> tmpv;
					      tmpv.resize (INET_ADDRSTRLEN);
					      std::cout
						  << "Maintenance message to "
						  << inet_ntop (AF_INET, &ip,
								tmpv.data (),
								tmpv.size ())
						  << ":" << ntohs (port)
						  << std::endl;
					    }
					}
				    }
				}

			      ownipsmtx->unlock ();
			    }
			  else
			    {
			      if (Netmode == "0")
				{
				  std::string adr = "3.3.3.3";
				  uint32_t ip;
				  uint16_t port = htons (3000);
				  inet_pton (AF_INET, adr.c_str (), &ip);
				  int sock = std::get<1> (sockets4[i]);
				  std::shared_ptr<std::mutex> mtx =
				      std::get<2> (sockets4[i]);
				  std::vector<char> msg;
				  std::tuple<lt::dht::public_key,
				      lt::dht::secret_key> ownkey;
				  ownkey = lt::dht::ed25519_create_keypair (
				      *(this->seed));
				  std::array<char, 32> keyarr;
				  keyarr = std::get<0> (ownkey).bytes;
				  std::copy (keyarr.begin (), keyarr.end (),
					     std::back_inserter (msg));
				  msg.push_back ('T');
				  msg.push_back ('T');
				  mtx->lock ();
				  sendMsg (sock, ip, port, msg);
				  mtx->unlock ();
				  std::cout << "Maintenance message to " << adr
				      << ":" << ntohs (port) << std::endl;
				}
			    }
			}
		      else
			{
			  if (Netmode == "0")
			    {
			      std::string adr = "3.3.3.3";
			      uint32_t ip;
			      uint16_t port = htons (3000);
			      inet_pton (AF_INET, adr.c_str (), &ip);
			      int sock = std::get<1> (sockets4[i]);
			      std::shared_ptr<std::mutex> mtx = std::get<2> (
				  sockets4[i]);
			      std::vector<char> msg;
			      std::tuple<lt::dht::public_key,
				  lt::dht::secret_key> ownkey;
			      ownkey = lt::dht::ed25519_create_keypair (
				  *(this->seed));
			      std::array<char, 32> keyarr;
			      keyarr = std::get<0> (ownkey).bytes;
			      std::copy (keyarr.begin (), keyarr.end (),
					 std::back_inserter (msg));
			      msg.push_back ('T');
			      msg.push_back ('T');
			      mtx->lock ();
			      sendMsg (sock, ip, port, msg);
			      mtx->unlock ();
			      std::cout << "Maintenance message to " << adr
				  << ":" << ntohs (port) << std::endl;
			    }
			}
		      getfrresmtx.unlock ();
		    }
		}
	      lastsentmtx->unlock ();
	    }
	}
      sockmtx.unlock ();

      //Polling
      std::vector<int> sforpoll;
      sockmtx.lock ();
      for (size_t i = 0; i < sockets4.size (); i++)
	{
	  std::shared_ptr<std::mutex> mtxgip = std::get<4> (sockets4[i]);
	  if (mtxgip->try_lock ())
	    {
	      sforpoll.push_back (std::get<1> (sockets4[i]));
	      mtxgip->unlock ();
	    }
	}
      sockmtx.unlock ();
      sockipv6mtx.lock ();
      sforpoll.push_back (sockipv6);
      sizesock = sforpoll.size ();
      pollfd *fds = new pollfd[sforpoll.size ()];
      for (size_t i = 0; i < sforpoll.size (); i++)
	{
	  fds[i].fd = sforpoll[i];
	  fds[i].events = POLLRDNORM;
	}
      sockipv6mtx.unlock ();
      int respol = poll (fds, sizesock, 3000);
      if (respol < 0)
	{
#ifdef __linux
	  std::cerr << "Polling error: " << strerror (errno) << std::endl;
#endif
#ifdef _WIN32
	  respol = WSAGetLastError ();
	  std::cerr << "Polling error: " << respol << std::endl;
#endif
	}
      else
	{
	  if (respol > 0)
	    {
	      sockmtx.lock ();

	      for (size_t i = 0; i < sizesock; i++)
		{
		  if (fds[i].revents == POLLRDNORM)
		    {
		      int sock = fds[i].fd;
		      std::mutex *thrmtx = nullptr;
		      if (threadvectmtx.try_lock ())
			{
			  thrmtx = new std::mutex;
			  thrmtx->lock ();
			  threadvect.push_back (
			      std::make_tuple (thrmtx, "Commops poll"));
			  threadvectmtx.unlock ();
			}
		      if (thrmtx != nullptr)
			{
			  std::thread *rcvthr = new std::thread (
			      [this, sock, thrmtx]
			      {
				this->sockmtx.lock ();
				auto it = std::find_if (
				    this->sockets4.begin (),
				    this->sockets4.end (), [sock]
				    (auto &el)
				      {
					return std::get<1>(el) == sock;
				      });
				if (it != this->sockets4.end ())
				  {
				    std::shared_ptr<std::mutex> mtx =
					std::get<2> (*it);
				    std::string key = std::get<0> (*it);
				    mtx->lock ();
				    sockaddr_in from =
				      { };
				    int rr = this->receiveMsg (sock, &from);
				    mtx->unlock ();
				    if (rr > 0)
				      {
					this->getfrresmtx.lock ();
					auto itgfr =
					    std::find_if (
						this->getfrres.begin (),
						this->getfrres.end (),
						[&key]
						(auto &el)
						  { return std::get<0>(el) == key;}                                                                                                );
					if (itgfr != this->getfrres.end ())
					  {
					    std::get<1> (*itgfr) =
						from.sin_addr.s_addr;
					    std::get<2> (*itgfr) =
						from.sin_port;
					  }
					else
					  {
					    std::tuple<std::string, uint32_t,
						uint16_t, int> ttup;
					    std::get<0> (ttup) = key;
					    std::get<1> (ttup) =
						from.sin_addr.s_addr;
					    std::get<2> (ttup) = from.sin_port;
					    std::get<3> (ttup) = 1;
					    this->getfrres.push_back (ttup);
					  }
					this->getfrresmtx.unlock ();
				      }

				  }
				else
				  {
				    if (sock == this->sockipv6)
				      {
					sockaddr_in from =
					  { };
					this->receiveMsg (sock, &from);
				      }
				  }
				this->sockmtx.unlock ();
				thrmtx->unlock ();
			      });
			      rcvthr->detach ();
			      delete rcvthr;
			    }
			}
		    }
	      sockmtx.unlock ();
	    }
	}

      delete[] fds;

      //Sending messages
      sockmtx.lock ();
      for (size_t i = 0; i < sockets4.size (); i++)
	{
	  if (cancel > 0)
	    {
	      break;
	    }
	  std::string key = std::get<0> (sockets4[i]);
	  int sock = std::get<1> (sockets4[i]);
	  std::shared_ptr<std::mutex> mtx = std::get<2> (sockets4[i]);
	  time_t lrt4 = std::get<3> (sockets4[i]);
	  time_t curtime = time (NULL);
	  time_t lrt6 = 0;
	  ipv6lrmtx.lock ();
	  auto it6 = std::find_if (ipv6lr.begin (), ipv6lr.end (), [&key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it6 != ipv6lr.end ())
	    {
	      lrt6 = std::get<1> (*it6);
	    }
	  ipv6lrmtx.unlock ();
	  sendingthrmtx->lock ();
	  auto itth = std::find (sendingthr.begin (), sendingthr.end (), key);
	  if (itth == sendingthr.end ()
	      && (curtime - lrt4 <= Tmttear || curtime - lrt6 <= Tmttear))
	    {
	      sendingthr.push_back (key);
	      sendingthrmtx->unlock ();
	      std::mutex *thrmtxsm = nullptr;
	      if (threadvectmtx.try_lock ())
		{
		  thrmtxsm = new std::mutex;
		  thrmtxsm->lock ();
		  threadvect.push_back (std::make_tuple (thrmtxsm, "Send msg"));
		  threadvectmtx.unlock ();
		}
	      if (thrmtxsm != nullptr)
		{
		  std::thread *sendthr = new std::thread (
		      [sendingthrmtx, &sendingthr, this, key, mtx, sock,
		       thrmtxsm]
		      {
			uint32_t ip = 0;
			uint16_t port = 0;
			this->getfrresmtx.lock ();
			auto gfrit = std::find_if (
			    this->getfrres.begin (), this->getfrres.end (),
			    [key]
			    (auto &el)
			      {
				return std::get<0>(el) == key;
			      });
			if (gfrit != this->getfrres.end ())
			  {
			    ip = std::get<1> (*gfrit);
			    port = std::get<2> (*gfrit);
			  }
			this->getfrresmtx.unlock ();
			mtx->lock ();

			sendMsgGlob (sock, key, ip, port);
			mtx->unlock ();
			sendingthrmtx->lock ();
			sendingthr.erase (
			    std::remove (sendingthr.begin (), sendingthr.end (),
					 key),
			    sendingthr.end ());
			sendingthrmtx->unlock ();
			thrmtxsm->unlock ();
		      });
		  sendthr->detach ();
		  delete sendthr;
		}
	    }
	  else
	    {
	      sendingthrmtx->unlock ();
	    }

	}
      sockmtx.unlock ();
    }
  delete ownipsmtx;
  delete blockipmtx;
  delete lastsentmtx;
  delete sendingthrmtx;
}

void
NetworkOperations::fileReject (std::string key, time_t tm)
{
  AuxFunc af;
  std::array<char, 32> okarr;
  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
  okp = lt::dht::ed25519_create_keypair (*seed);
  okarr = std::get<0> (okp).bytes;
  std::vector<char> msg;
  std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
  std::string type = "FJ";
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
  time_t curtime = time (NULL);
  int sent = 0;
  ipv6lrmtx.lock ();
  auto it6 = std::find_if (ipv6lr.begin (), ipv6lr.end (), [key]
  (auto &el)
    {
      return std::get<0>(el) == key;
    });
  if (it6 != ipv6lr.end ())
    {
      if (curtime - std::get<1> (*it6) <= Tmttear)
	{
	  ipv6contmtx.lock ();
	  auto it6c = std::find_if (ipv6cont.begin (), ipv6cont.end (), [key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it6c != ipv6cont.end ())
	    {
	      std::string ip = std::get<1> (*it6c);
	      uint16_t port = std::get<2> (*it6c);
	      sockipv6mtx.lock ();
	      int ch = sendMsg6 (sockipv6, ip, port, msg);
	      sockipv6mtx.unlock ();
	      if (ch > 0)
		{
		  sent = 1;
		}
	    }
	  ipv6contmtx.unlock ();
	}
    }
  ipv6lrmtx.unlock ();
  if (sent <= 0)
    {
      sockmtx.lock ();
      auto it4 = std::find_if (sockets4.begin (), sockets4.end (), [key]
      (auto &el)
	{
	  return std::get<0>(el) == key;
	});
      if (it4 != sockets4.end ())
	{
	  if (curtime - std::get<3> (*it4) <= Tmttear)
	    {
	      getfrresmtx.lock ();
	      auto itgfr = std::find_if (getfrres.begin (), getfrres.end (),
					 [key]
					 (auto &el)
					   {
					     return std::get<0>(el) == key;
					   });
	      if (itgfr != getfrres.end ())
		{
		  uint32_t ip = std::get<1> (*itgfr);
		  uint16_t port = std::get<2> (*itgfr);
		  std::shared_ptr<std::mutex> mtx = std::get<2> (*it4);
		  mtx->lock ();
		  sendMsg (std::get<1> (*it4), ip, port, msg);
		  mtx->unlock ();
		}
	      getfrresmtx.unlock ();
	    }
	}
      sockmtx.unlock ();
    }

  fqrcvdmtx.lock ();
  fqrcvd.erase (std::remove_if (fqrcvd.begin (), fqrcvd.end (), [key, tm]
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
		fqrcvd.end ());
  fqrcvdmtx.unlock ();
}

void
NetworkOperations::fileAccept (std::string key, time_t tm,
			       std::filesystem::path sp)
{
  AuxFunc af;
  std::array<char, 32> okarr;
  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
  okp = lt::dht::ed25519_create_keypair (*seed);
  okarr = std::get<0> (okp).bytes;
  std::vector<char> msg;
  std::copy (okarr.begin (), okarr.end (), std::back_inserter (msg));
  std::string type = "FA";
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
  time_t curtime = time (NULL);
  int sent = 0;
  ipv6lrmtx.lock ();
  auto it6 = std::find_if (ipv6lr.begin (), ipv6lr.end (), [key]
  (auto &el)
    {
      return std::get<0>(el) == key;
    });
  if (it6 != ipv6lr.end ())
    {
      if (curtime - std::get<1> (*it6) <= Tmttear)
	{
	  ipv6contmtx.lock ();
	  auto it6c = std::find_if (ipv6cont.begin (), ipv6cont.end (), [key]
	  (auto &el)
	    {
	      return std::get<0>(el) == key;
	    });
	  if (it6c != ipv6cont.end ())
	    {
	      std::string ip = std::get<1> (*it6c);
	      uint16_t port = std::get<2> (*it6c);
	      sockipv6mtx.lock ();
	      int ch = sendMsg6 (sockipv6, ip, port, msg);
	      sockipv6mtx.unlock ();
	      if (ch > 0)
		{
		  sent = 1;
		}
	    }
	  ipv6contmtx.unlock ();
	}
    }
  ipv6lrmtx.unlock ();
  if (sent <= 0)
    {
      sockmtx.lock ();
      auto it4 = std::find_if (sockets4.begin (), sockets4.end (), [key]
      (auto &el)
	{
	  return std::get<0>(el) == key;
	});
      if (it4 != sockets4.end ())
	{
	  if (curtime - std::get<3> (*it4) <= Tmttear)
	    {
	      getfrresmtx.lock ();
	      auto itgfr = std::find_if (getfrres.begin (), getfrres.end (),
					 [key]
					 (auto &el)
					   {
					     return std::get<0>(el) == key;
					   });
	      if (itgfr != getfrres.end ())
		{
		  uint32_t ip = std::get<1> (*itgfr);
		  uint16_t port = std::get<2> (*itgfr);
		  std::shared_ptr<std::mutex> mtx = std::get<2> (*it4);
		  mtx->lock ();
		  sendMsg (std::get<1> (*it4), ip, port, msg);
		  mtx->unlock ();
		}
	      getfrresmtx.unlock ();
	    }
	}
      sockmtx.unlock ();
    }
  filehashvectmtx.lock ();
  auto itfhv = std::find_if (
      filehashvect.begin (), filehashvect.end (), [key, tm]
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
  if (itfhv == filehashvect.end ())
    {
      std::vector<char> hv;
      filehashvect.push_back (std::make_tuple (key, tm, hv, sp, -1));
    }
  filehashvectmtx.unlock ();
}

int
NetworkOperations::msgMe (std::string key, uint64_t tm, uint64_t partnum)
{
  int result = 0;
  contmtx.lock ();
  auto itc = std::find_if (contacts.begin (), contacts.end (), [key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itc != contacts.end ())
    {
      int index = std::get<0> (*itc);
      std::string indexstr;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << index;
      indexstr = strm.str ();
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist/Bufer/" + indexstr;
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      rcvmtx.lock ();
      if (!std::filesystem::exists (filepath))
	{
	  std::filesystem::create_directories (filepath);
	}
      msgrcvdpnummtx.lock ();
      auto itmrpnm = std::find_if (msgrcvdpnum.begin (), msgrcvdpnum.end (),
				   [key, tm]
				   (auto &el)
				     {
				       if (std::get<0>(el) == key
					   && std::get<1>(el) == tm)
					 {
					   return true;
					 }
				       else
					 {
					   return false;
					 }
				     });
      if (itmrpnm != msgrcvdpnum.end ())
	{
	  uint64_t lpnm = std::get<2> (*itmrpnm);
	  if (partnum - lpnm == 1)
	    {
	      msgparthashmtx.lock ();
	      auto itmph = std::find_if (msgparthash.begin (),
					 msgparthash.end (), [key, tm]
					 (auto &el)
					   {
					     if (std::get<0>(el) == key &&
						 std::get<1>(el) == tm)
					       {
						 return true;
					       }
					     else
					       {
						 return false;
					       }
					   });
	      if (itmph != msgparthash.end ())
		{
		  uint64_t num = 0;
		  std::vector<char> part;
		  msgpartrcvmtx.lock ();
		  for (;;)
		    {
		      auto itmpr = std::find_if (
			  msgpartrcv.begin (), msgpartrcv.end (),
			  [key, tm, &num]
			  (auto &el)
			    {
			      if (std::get<0>(el) == key &&
				  std::get<1>(el) == tm &&
				  std::get<2>(el) == num)
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
		      if (itmpr != msgpartrcv.end ())
			{
			  std::vector<char> pt = std::get<3> (*itmpr);
			  std::copy (pt.begin (), pt.end (),
				     std::back_inserter (part));
			  msgpartrcv.erase (itmpr);
			}
		      else
			{
			  break;
			}
		      num = num + 1;
		    }
		  msgpartrcvmtx.unlock ();
		  std::vector<char> hash = std::get<2> (*itmph);
		  std::vector<char> chhash = af.strhash (part, 2);
		  if (hash == chhash)
		    {
		      strm.clear ();
		      strm.str ("");
		      strm.imbue (loc);
		      strm << tm;
		      filename = filepath.u8string ();
		      filename = filename + "/" + strm.str ();
		      filepath = std::filesystem::u8path (filename);
		      std::fstream f;
		      if (std::filesystem::exists (filepath))
			{
			  f.open (
			      filepath,
			      std::ios_base::out | std::ios_base::app
				  | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      else
			{
			  f.open (filepath,
				  std::ios_base::out | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      std::get<2> (*itmrpnm) = partnum;
		      result = 1;
		    }
		  else
		    {
		      std::cerr << "Part not correct" << std::endl;
		    }
		}
	      else
		{
		  std::cerr << "Msg part hash not found" << std::endl;
		}
	      msgparthashmtx.unlock ();
	    }
	  else
	    {
	      if (partnum - lpnm == 0)
		{
		  result = 1;
		}
	      std::cerr << "Msg part number not correct" << std::endl;
	    }
	}
      else
	{
	  if (partnum == 0)
	    {
	      msgparthashmtx.lock ();
	      auto itmph = std::find_if (msgparthash.begin (),
					 msgparthash.end (), [key, tm]
					 (auto &el)
					   {
					     if (std::get<0>(el) == key &&
						 std::get<1>(el) == tm)
					       {
						 return true;
					       }
					     else
					       {
						 return false;
					       }
					   });
	      if (itmph != msgparthash.end ())
		{
		  uint64_t num = 0;
		  std::vector<char> part;
		  msgpartrcvmtx.lock ();
		  for (;;)
		    {
		      auto itmpr = std::find_if (
			  msgpartrcv.begin (), msgpartrcv.end (),
			  [key, tm, &num]
			  (auto &el)
			    {
			      if (std::get<0>(el) == key &&
				  std::get<1>(el) == tm &&
				  std::get<2>(el) == num)
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
		      if (itmpr != msgpartrcv.end ())
			{
			  std::vector<char> pt = std::get<3> (*itmpr);
			  std::copy (pt.begin (), pt.end (),
				     std::back_inserter (part));
			  msgpartrcv.erase (itmpr);
			}
		      else
			{
			  break;
			}
		      num = num + 1;
		    }
		  msgpartrcvmtx.unlock ();
		  std::vector<char> hash = std::get<2> (*itmph);
		  std::vector<char> chhash = af.strhash (part, 2);
		  if (hash == chhash)
		    {
		      strm.clear ();
		      strm.str ("");
		      strm.imbue (loc);
		      strm << tm;
		      filename = filepath.u8string ();
		      filename = filename + "/" + strm.str ();
		      filepath = std::filesystem::u8path (filename);
		      std::fstream f;
		      if (std::filesystem::exists (filepath))
			{
			  f.open (
			      filepath,
			      std::ios_base::out | std::ios_base::app
				  | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      else
			{
			  f.open (filepath,
				  std::ios_base::out | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      msgrcvdpnum.push_back (
			  std::make_tuple (key, tm, partnum));
		      result = 1;
		    }
		  else
		    {
		      std::cerr << "Part not correct" << std::endl;
		    }
		}
	      else
		{
		  std::cerr << "Msg part hash not found" << std::endl;
		}
	      msgparthashmtx.unlock ();
	    }
	}
      msgrcvdpnummtx.unlock ();

      rcvmtx.unlock ();
    }
  contmtx.unlock ();
  return result;
}

int
NetworkOperations::msgPe (std::string key, uint64_t tm, uint64_t partnum)
{
  int result = 0;
  contmtx.lock ();
  auto itc = std::find_if (contacts.begin (), contacts.end (), [key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itc != contacts.end ())
    {
      int index = std::get<0> (*itc);
      std::string indexstr;
      std::stringstream strm;
      std::locale loc ("C");
      strm.imbue (loc);
      strm << index;
      indexstr = strm.str ();
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist/Bufer/" + indexstr;
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      rcvmtx.lock ();
      if (!std::filesystem::exists (filepath))
	{
	  std::filesystem::create_directories (filepath);
	}
      msgrcvdpnummtx.lock ();
      auto itmrpnm = std::find_if (msgrcvdpnum.begin (), msgrcvdpnum.end (),
				   [key, tm]
				   (auto &el)
				     {
				       if (std::get<0>(el) == key
					   && std::get<1>(el) == tm)
					 {
					   return true;
					 }
				       else
					 {
					   return false;
					 }
				     });
      if (itmrpnm != msgrcvdpnum.end ())
	{
	  uint64_t lpnm = std::get<2> (*itmrpnm);
	  if (partnum - lpnm == 1)
	    {
	      msgparthashmtx.lock ();
	      auto itmph = std::find_if (msgparthash.begin (),
					 msgparthash.end (), [key, tm]
					 (auto &el)
					   {
					     if (std::get<0>(el) == key &&
						 std::get<1>(el) == tm)
					       {
						 return true;
					       }
					     else
					       {
						 return false;
					       }
					   });
	      if (itmph != msgparthash.end ())
		{
		  uint64_t num = 0;
		  std::vector<char> part;
		  msgpartrcvmtx.lock ();
		  for (;;)
		    {
		      auto itmpr = std::find_if (
			  msgpartrcv.begin (), msgpartrcv.end (),
			  [key, tm, &num]
			  (auto &el)
			    {
			      if (std::get<0>(el) == key &&
				  std::get<1>(el) == tm &&
				  std::get<2>(el) == num)
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
		      if (itmpr != msgpartrcv.end ())
			{
			  std::vector<char> pt = std::get<3> (*itmpr);
			  std::copy (pt.begin (), pt.end (),
				     std::back_inserter (part));
			  msgpartrcv.erase (itmpr);
			}
		      else
			{
			  break;
			}
		      num = num + 1;
		    }
		  msgpartrcvmtx.unlock ();
		  std::vector<char> hash = std::get<2> (*itmph);
		  std::vector<char> chhash = af.strhash (part, 2);
		  if (hash == chhash)
		    {
		      filename = filepath.u8string ();
		      filename = filename + "/Profile";
		      filepath = std::filesystem::u8path (filename);
		      std::fstream f;
		      if (std::filesystem::exists (filepath))
			{
			  f.open (
			      filepath,
			      std::ios_base::out | std::ios_base::app
				  | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      else
			{
			  f.open (filepath,
				  std::ios_base::out | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      std::get<2> (*itmrpnm) = partnum;
		      result = 1;
		    }
		  else
		    {
		      std::cerr << "Profile part not correct" << std::endl;
		    }
		}
	      else
		{
		  std::cerr << "Profile part hash not found" << std::endl;
		}
	      msgparthashmtx.unlock ();
	    }
	  else
	    {
	      if (partnum - lpnm == 0)
		{
		  result = 1;
		}
	      std::cerr << "Profile part number incorrect" << std::endl;
	    }
	}
      else
	{
	  if (partnum == 0)
	    {
	      msgparthashmtx.lock ();
	      auto itmph = std::find_if (msgparthash.begin (),
					 msgparthash.end (), [key, tm]
					 (auto &el)
					   {
					     if (std::get<0>(el) == key &&
						 std::get<1>(el) == tm)
					       {
						 return true;
					       }
					     else
					       {
						 return false;
					       }
					   });
	      if (itmph != msgparthash.end ())
		{
		  uint64_t num = 0;
		  std::vector<char> part;
		  msgpartrcvmtx.lock ();
		  for (;;)
		    {
		      auto itmpr = std::find_if (
			  msgpartrcv.begin (), msgpartrcv.end (),
			  [key, tm, &num]
			  (auto &el)
			    {
			      if (std::get<0>(el) == key &&
				  std::get<1>(el) == tm &&
				  std::get<2>(el) == num)
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
		      if (itmpr != msgpartrcv.end ())
			{
			  std::vector<char> pt = std::get<3> (*itmpr);
			  std::copy (pt.begin (), pt.end (),
				     std::back_inserter (part));
			  msgpartrcv.erase (itmpr);
			}
		      else
			{
			  break;
			}
		      num = num + 1;
		    }
		  msgpartrcvmtx.unlock ();
		  std::vector<char> hash = std::get<2> (*itmph);
		  std::vector<char> chhash = af.strhash (part, 2);
		  if (hash == chhash)
		    {
		      filename = filepath.u8string ();
		      filename = filename + "/Profile";
		      filepath = std::filesystem::u8path (filename);
		      std::fstream f;
		      if (std::filesystem::exists (filepath))
			{
			  f.open (
			      filepath,
			      std::ios_base::out | std::ios_base::app
				  | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      else
			{
			  f.open (filepath,
				  std::ios_base::out | std::ios_base::binary);
			  f.write (&part[0], part.size ());
			  f.close ();
			}
		      msgrcvdpnum.push_back (
			  std::make_tuple (key, tm, partnum));
		      result = 1;
		    }
		  else
		    {
		      std::cerr << "Profile part not correct" << std::endl;
		    }
		}
	      else
		{
		  std::cerr << "Profile part hash not found" << std::endl;
		}
	      msgparthashmtx.unlock ();
	    }
	}
      msgrcvdpnummtx.unlock ();

      rcvmtx.unlock ();
    }
  contmtx.unlock ();
  return result;
}

int
NetworkOperations::msgME (std::string key, uint64_t tm)
{
  int result = 0;
  contmtx.lock ();
  auto itc = std::find_if (contacts.begin (), contacts.end (), [key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itc != contacts.end ())
    {
      int index = std::get<0> (*itc);
      std::locale loc ("C");
      std::stringstream strm;
      strm.imbue (loc);
      strm << index;
      std::string indexstr = strm.str ();
      strm.clear ();
      strm.str ("");
      strm.imbue (loc);
      strm << tm;
      std::string tmstr = strm.str ();
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist/Bufer/" + indexstr + "/" + tmstr;
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      rcvmtx.lock ();
      if (std::filesystem::exists (filepath))
	{
	  msghashmtx.lock ();
	  auto itmh = std::find_if (msghash.begin (), msghash.end (), [key, tm]
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
	  if (itmh != msghash.end ())
	    {
	      std::vector<char> hash = std::get<3> (*itmh);
	      std::vector<char> chhash = af.filehash (filepath);
	      uint64_t msgsz = std::get<2> (*itmh);
	      if (hash == chhash
		  && msgsz == uint64_t (std::filesystem::file_size (filepath)))
		{
		  af.homePath (&filename);
		  filename = filename + "/.Communist/" + indexstr;
		  std::filesystem::path outpath = std::filesystem::u8path (
		      filename);
		  if (std::filesystem::exists (outpath))
		    {
		      std::vector<int> findv;
		      for (auto &ditp : std::filesystem::directory_iterator (
			  outpath))
			{
			  std::filesystem::path p = ditp.path ();
			  if (p.filename ().u8string () != "Profile"
			      && p.filename ().u8string () != "Yes")
			    {
			      filename = p.filename ().u8string ();
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
			      int tint;
			      strm >> tint;
			      findv.push_back (tint);
			    }
			}
		      int msgind = 0;
		      if (findv.size () > 0)
			{
			  std::sort (findv.begin (), findv.end ());
			  msgind = findv[findv.size () - 1] + 1;
			}
		      strm.clear ();
		      strm.str ("");
		      strm.imbue (loc);
		      strm << msgind;
		      filename = outpath.u8string ();
		      filename = filename + "/" + strm.str ();
		      outpath = std::filesystem::u8path (filename);
		      std::filesystem::copy (filepath, outpath);
		      messageReceived.emit (key, outpath);
		      result = 1;
		    }
		}
	    }
	  msghashmtx.unlock ();
	  std::filesystem::remove (filepath);
	}
      rcvmtx.unlock ();
    }
  contmtx.unlock ();
  return result;
}

int
NetworkOperations::msgPE (std::string key, uint64_t tm)
{
  int result = 0;
  contmtx.lock ();
  auto itc = std::find_if (contacts.begin (), contacts.end (), [key]
  (auto &el)
    {
      return std::get<1>(el) == key;
    });
  if (itc != contacts.end ())
    {
      int index = std::get<0> (*itc);
      std::locale loc ("C");
      std::stringstream strm;
      strm.imbue (loc);
      strm << index;
      std::string indexstr = strm.str ();
      strm.clear ();
      strm.str ("");
      strm.imbue (loc);
      strm << tm;
      std::string tmstr = strm.str ();
      std::string filename;
      AuxFunc af;
      af.homePath (&filename);
      filename = filename + "/.Communist/Bufer/" + indexstr + "/Profile";
      std::filesystem::path filepath = std::filesystem::u8path (filename);
      rcvmtx.lock ();
      if (std::filesystem::exists (filepath))
	{
	  msghashmtx.lock ();
	  auto itmh = std::find_if (msghash.begin (), msghash.end (), [key, tm]
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
	  if (itmh != msghash.end ())
	    {
	      std::vector<char> hash = std::get<3> (*itmh);
	      std::vector<char> chhash = af.filehash (filepath);
	      uint64_t msgsz = std::get<2> (*itmh);
	      if (hash == chhash
		  && msgsz == uint64_t (std::filesystem::file_size (filepath)))
		{
		  af.homePath (&filename);
		  filename = filename + "/.Communist/" + indexstr + "/Profile";
		  std::filesystem::path outpath = std::filesystem::u8path (
		      filename);
		  if (!std::filesystem::exists (outpath.parent_path ()))
		    {
		      std::filesystem::create_directories (
			  outpath.parent_path ());
		    }
		  if (std::filesystem::exists (outpath))
		    {
		      std::filesystem::remove (outpath);
		    }
		  std::filesystem::copy (filepath, outpath);
		  int indint;
		  strm.clear ();
		  strm.str ("");
		  strm.imbue (loc);
		  strm << indexstr;
		  strm >> indint;
		  profReceived.emit (key, indint);
		  result = 1;
		}
	      else
		{
		  if (hash != chhash)
		    {
		      std::cerr << "Prof hash incorrect" << std::endl;
		    }
		  if (msgsz != uint64_t (std::filesystem::file_size (filepath)))
		    {
		      std::cerr << "Prof size incorrect" << std::endl;
		    }
		}
	    }
	  else
	    {
	      std::cerr << "Profile hash not found" << std::endl;
	    }
	  msghashmtx.unlock ();
	  std::filesystem::remove (filepath);
	}
      else
	{
	  msghashmtx.lock ();
	  auto itmh = std::find_if (msghash.begin (), msghash.end (), [key, tm]
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
	  if (itmh != msghash.end ())
	    {
	      af.homePath (&filename);
	      filename = filename + "/.Communist/" + indexstr + "/Profile";
	      filepath = std::filesystem::u8path (filename);
	      if (std::filesystem::exists (filepath))
		{
		  std::vector<char> hash = std::get<3> (*itmh);
		  std::vector<char> chhash = af.filehash (filepath);
		  if (hash == chhash)
		    {
		      result = 1;
		    }
		}
	      else
		{
		  std::cerr << "Profile not found" << std::endl;
		}
	    }
	  else
	    {
	      std::cerr << "Profile hash not found" << std::endl;
	    }
	  msghashmtx.unlock ();
	}
      rcvmtx.unlock ();
    }
  contmtx.unlock ();
  return result;
}

void
NetworkOperations::blockFriend (std::string key)
{
  std::string keyloc = key;
  if (sockmtx.try_lock ())
    {
      auto itsock = std::find_if (sockets4.begin (), sockets4.end (), [&keyloc]
      (auto &el)
	{
	  if (std::get<0>(el) == keyloc)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
      if (itsock != sockets4.end ())
	{
	  int sock = std::get<1> (*itsock);
	  std::shared_ptr<std::mutex> mtx = std::get<2> (*itsock);
	  if (mtx->try_lock ())
	    {
	      if (sock >= 0)
		{
#ifdef __linux
		  close (sock);
#endif
#ifdef _WIN32
	      closesocket (sock);
#endif
		}
	      mtx->unlock ();
	      sockets4.erase (itsock);
	    }
	  else
	    {
	      std::thread *thr = new std::thread ( [keyloc, this]
	      {
		usleep (100000);
		this->blockFriend (keyloc);
	      });
	      thr->detach ();
	      delete thr;
	      sockmtx.unlock ();
	      return void ();
	    }
	}
      sockmtx.unlock ();
    }
  else
    {
      std::thread *thr = new std::thread ( [keyloc, this]
      {
	usleep (100000);
	this->blockFriend (keyloc);
      });
      thr->detach ();
      delete thr;
      return void ();
    }

  addfrmtx.lock ();
  Addfriends.erase (
      std::remove (Addfriends.begin (), Addfriends.end (), keyloc),
      Addfriends.end ());
  addfrmtx.unlock ();

  contmtx.lock ();
  auto contit = std::find_if (contacts.begin (), contacts.end (), [&keyloc]
  (auto &el)
    {
      return std::get<1>(el) == keyloc;
    });
  if (contit != contacts.end ())
    {
      contacts.erase (contit);
    }
  contmtx.unlock ();

  msgpartbufmtx.lock ();
  msgpartbuf.erase (
      std::remove_if (msgpartbuf.begin (), msgpartbuf.end (), [&keyloc]
      (auto &el)
	{ return std::get<0>(el) == keyloc;}),
      msgpartbuf.end ());
  msgpartbufmtx.unlock ();

  msghashmtx.lock ();
  msghash.erase (std::remove_if (msghash.begin (), msghash.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		 msghash.end ());
  msghashmtx.unlock ();

  msgparthashmtx.lock ();
  msgparthash.erase (
      std::remove_if (msgparthash.begin (), msgparthash.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgparthash.end ());
  msgparthashmtx.unlock ();

  msgpartrcvmtx.lock ();
  msgpartrcv.erase (
      std::remove_if (msgpartrcv.begin (), msgpartrcv.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgpartrcv.end ());
  msgpartrcvmtx.unlock ();

  msgpartbufmtx.lock ();
  msgpartbuf.erase (
      std::remove_if (msgpartbuf.begin (), msgpartbuf.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgpartbuf.end ());
  msgpartbufmtx.unlock ();

  msgrcvdpnummtx.lock ();
  msgrcvdpnum.erase (
      std::remove_if (msgrcvdpnum.begin (), msgrcvdpnum.end (), [keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      msgrcvdpnum.end ());
  msgrcvdpnummtx.unlock ();

  getfrmtx.lock ();
  getfr.erase (std::remove_if (getfr.begin (), getfr.end (), [&keyloc]
  (auto &el)
    {
      return el == keyloc;
    }),
	       getfr.end ());
  getfrmtx.unlock ();

  getfrresmtx.lock ();
  getfrres.erase (std::remove_if (getfrres.begin (), getfrres.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		  getfrres.end ());
  getfrresmtx.unlock ();

  putipmtx.lock ();
  putipv.erase (std::remove_if (putipv.begin (), putipv.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		putipv.end ());
  putipmtx.unlock ();

  ipv6contmtx.lock ();
  ipv6cont.erase (std::remove_if (ipv6cont.begin (), ipv6cont.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		  ipv6cont.end ());
  ipv6contmtx.unlock ();

  ipv6lrmtx.lock ();
  ipv6lr.erase (std::remove_if (ipv6lr.begin (), ipv6lr.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		ipv6lr.end ());
  ipv6lrmtx.unlock ();

  filesendreqmtx.lock ();
  filesendreq.erase (
      std::remove_if (filesendreq.begin (), filesendreq.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filesendreq.end ());
  filesendreqmtx.unlock ();

  fqrcvdmtx.lock ();
  fqrcvd.erase (std::remove_if (fqrcvd.begin (), fqrcvd.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		fqrcvd.end ());
  fqrcvdmtx.unlock ();

  filepartbufmtx.lock ();
  filepartbuf.erase (
      std::remove_if (filepartbuf.begin (), filepartbuf.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartbuf.end ());
  filepartbufmtx.unlock ();

  filehashvectmtx.lock ();
  filehashvect.erase (
      std::remove_if (filehashvect.begin (), filehashvect.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filehashvect.end ());
  filehashvectmtx.unlock ();

  fileparthashmtx.lock ();
  fileparthash.erase (
      std::remove_if (fileparthash.begin (), fileparthash.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      fileparthash.end ());
  fileparthashmtx.unlock ();

  filepartrcvmtx.lock ();
  filepartrcv.erase (
      std::remove_if (filepartrcv.begin (), filepartrcv.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartrcv.end ());
  filepartrcvmtx.unlock ();

  filepartrlogmtx.lock ();
  filepartrlog.erase (
      std::remove_if (filepartrlog.begin (), filepartrlog.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartrlog.end ());
  filepartrlogmtx.unlock ();

  currentpartmtx.lock ();
  currentpart.erase (
      std::remove_if (currentpart.begin (), currentpart.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      currentpart.end ());
  currentpartmtx.unlock ();

  fbrvectmtx.lock ();
  fbrvect.erase (std::remove_if (fbrvect.begin (), fbrvect.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		 fbrvect.end ());
  fbrvectmtx.unlock ();

  filepartendmtx.lock ();
  filepartend.erase (
      std::remove_if (filepartend.begin (), filepartend.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      filepartend.end ());
  filepartendmtx.unlock ();

  fileendmtx.lock ();
  fileend.erase (std::remove_if (fileend.begin (), fileend.end (), [&keyloc]
  (auto &el)
    {
      return std::get<0>(el) == keyloc;
    }),
		 fileend.end ());
  fileendmtx.unlock ();

  maintblockmtx.lock ();
  maintblock.erase (
      std::remove_if (maintblock.begin (), maintblock.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      maintblock.end ());
  maintblockmtx.unlock ();

  holepunchstopmtx.lock ();
  holepunchstop.erase (
      std::remove_if (holepunchstop.begin (), holepunchstop.end (), [&keyloc]
      (auto &el)
	{
	  return std::get<0>(el) == keyloc;
	}),
      holepunchstop.end ());
  holepunchstopmtx.unlock ();
}

void
NetworkOperations::startFriend (std::string key, int ind)
{
  contmtx.lock ();
  auto itcont = std::find_if (contacts.begin (), contacts.end (), [key]
  (auto &el)
    { return std::get<1>(el) == key;});
  if (itcont == contacts.end ())
    {
      std::pair<int, std::string> p;
      std::get<0> (p) = ind;
      std::get<1> (p) = key;
      contacts.push_back (p);
    }
  contmtx.unlock ();

  getfrmtx.lock ();
  auto gfrit = std::find (getfr.begin (), getfr.end (), key);
  if (gfrit == getfr.end ())
    {
      getfr.push_back (key);
    }
  getfrmtx.unlock ();
  sockmtx.lock ();
  int ss = 0;
  auto itsock = std::find_if (sockets4.begin (), sockets4.end (), [key]
  (auto &el)
    { return std::get<0>(el) == key;});
  if (itsock == sockets4.end ())
    {
#ifdef __linux
      int sock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
      int sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      u_long nonblocking_enabled = TRUE;
      ioctlsocket (sock, FIONBIO, &nonblocking_enabled);
#endif
      sockaddr_in addripv4 =
	{ };
      addripv4.sin_family = AF_INET;
      addripv4.sin_addr.s_addr = INADDR_ANY;
      addripv4.sin_port = 0;
      int addrlen1 = sizeof(addripv4);
      bind (sock, (const sockaddr*) &addripv4, addrlen1);
      std::shared_ptr<std::mutex> mtx (new std::mutex);
      std::shared_ptr<std::mutex> mtxgip (new std::mutex);
      time_t tm = time (NULL);
      sockets4.push_back (std::make_tuple (key, sock, mtx, tm, mtxgip));
    }
  if (sockets4.size () == 1)
    {
      ss = 1;
    }
  sockmtx.unlock ();
  if (ss == 1)
    {
      std::mutex *thrmtx = new std::mutex;
      thrmtx->lock ();
      threadvectmtx.lock ();
      threadvect.push_back (std::make_tuple (thrmtx, "Start friend"));
      threadvectmtx.unlock ();
      std::thread *thr = new std::thread ( [this, thrmtx]
      {
	if (this->copsrun.try_lock ())
	  {
	    this->commOps ();
	    this->copsrun.unlock ();
	  }
	thrmtx->unlock ();
      });
      thr->detach ();
      delete thr;
    }
}

void
NetworkOperations::setIPv6 (std::string ip)
{
  ownipv6mtx.lock ();
  ownipv6 = ip;
  ownipv6mtx.unlock ();
}

void
NetworkOperations::dnsFunc ()
{
  std::mutex *thrmtx = new std::mutex;
  thrmtx->lock ();
  threadvectmtx.lock ();
  threadvect.push_back (std::make_tuple (thrmtx, "Dns func"));
  threadvectmtx.unlock ();
  std::thread *dnsthr = new std::thread ( [this, thrmtx]
  {
    std::vector<std::pair<std::string, std::string>> readstun;
    AuxFunc af;
    std::string filename;
    std::filesystem::path filepath;
    filename = Sharepath + "/StunList";
    filepath = std::filesystem::u8path (filename);
    if (!std::filesystem::exists (filepath))
      {
	std::cout << "StunList not found!" << std::endl;
      }
    else
      {
	std::fstream f;
	f.open (filepath, std::ios_base::in);
	while (!f.eof ())
	  {
	    std::string line;
	    getline (f, line);
	    if (line != "")
	      {
		std::pair<std::string, std::string> p;
		p.first = line;
		p.first = p.first.substr (0, p.first.find (" "));
		p.second = line;
		p.second = p.second.erase (0, p.second.find (" ") + std::string ( " ").size ());
		readstun.push_back (p);
	      }
	  }
	f.close ();
      }
    for (size_t i = 0; i < readstun.size (); i++)
      {
	if (cancel == 1)
	  {
	    thrmtx->unlock ();
	    return void ();
	  }
	std::pair<struct in_addr, uint16_t> p;
	std::string line = readstun[i].second;
	uint16_t port;
	std::stringstream strm;
	std::locale loc ("C");
	strm.imbue (loc);
	strm << line;
	strm >> port;
	p.second = htons (port);
	line = readstun[i].first;
	int ch = inet_pton (AF_INET, line.c_str (), &p.first);
	if (ch < 1)
	  {
	    hostent *hn;
	    hn = gethostbyname (readstun.at (i).first.c_str ());
	    if (hn != nullptr)
	      {
		int count = 0;
		for (;;)
		  {
		    if (cancel == 1)
		      {
			thrmtx->unlock ();
			return void ();
		      }
		    if ((struct in_addr*) hn->h_addr_list[count] == nullptr)
		      {
			break;
		      }
		    p.first = *(struct in_addr*) hn->h_addr_list[count];
		    count++;
		    this->stunipsmtx.lock ();
		    auto iter = std::find_if (
			this->stunips.begin (), this->stunips.end (), [&p]
			(auto &el)
			  {
			    if (p.first.s_addr == el.first.s_addr &&
				p.second == el.second)
			      {
				return true;
			      }
			    else
			      {
				return false;
			      }
			  });
		    if (iter == this->stunips.end ())
		      {
			this->stunips.push_back (p);
		      }
		    this->stunipsmtx.unlock ();
		  }
	      }
	  }
	else
	  {
	    this->stunipsmtx.lock ();
	    auto iter = std::find_if (
		this->stunips.begin (), this->stunips.end (), [&p]
		(auto &el)
		  {
		    if (p.first.s_addr == el.first.s_addr &&
			p.second == el.second)
		      {
			return true;
		      }
		    else
		      {
			return false;
		      }
		  });
	    if (iter == this->stunips.end ())
	      {
		this->stunips.push_back (p);
	      }
	    this->stunipsmtx.unlock ();
	  }
      }

    readstun.clear ();
    this->dnsfinished.emit ();
    thrmtx->unlock ();
  });
  dnsthr->detach ();
  delete dnsthr;
}

void
NetworkOperations::setIPv4 (std::string ip)
{
  IPV4mtx.lock ();
  IPV4 = ip;
  IPV4mtx.unlock ();
}

void
NetworkOperations::bootstrFunc ()
{
  std::mutex *thrmtx = new std::mutex;
  thrmtx->lock ();
  threadvectmtx.lock ();
  threadvect.push_back (std::make_tuple (thrmtx, "Bootstr func"));
  threadvectmtx.unlock ();
  std::thread *bts = new std::thread ( [this, thrmtx]
  {
    int sintv = 60;
    int brdcstfl = 1;
    int ch;
    std::cout << "Local bootstrap started" << std::endl;
#ifdef __linux
    int sock6 = socket (AF_INET6, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
    int sock6 = socket (AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    u_long nonblocking_enabled = TRUE;
    ioctlsocket (sock6, FIONBIO, &nonblocking_enabled);
#endif
    sockaddr_in6 addripv6 =
      { };
    addripv6.sin6_family = AF_INET6;
#ifdef __linux
    addripv6.sin6_addr = in6addr_any;
#endif
#ifdef _WIN32
    inet_pton (AF_INET6, this->ownipv6.c_str(), &addripv6.sin6_addr);
#endif
    std::string lip6 = "";
    int grp6 = 0;
    auto itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Locip6port";
    });
    if (itprv != this->prefvect.end ())
      {
	if (std::get<1> (*itprv) != "")
	  {
	    lip6 = std::get<1> (*itprv);
	    lip6.erase (0, lip6.find ("[") + std::string ("[").size ());
	    lip6 = lip6.substr (0, lip6.find ("]"));
	    std::string lop = std::get<1> (*itprv);
	    lop.erase (0, lop.find ("]:") + std::string ("]:").size ());
	    std::stringstream strm;
	    std::locale loc ("C");
	    strm.imbue (loc);
	    strm << lop;
	    strm >> grp6;
	    addripv6.sin6_port = htons (grp6);
	    ;
	  }
	else
	  {
	    lip6 = "ff15::13";
	    grp6 = 48666;
	    addripv6.sin6_port = htons (grp6);
	  }
      }
    else
      {
	lip6 = "ff15::13";
	grp6 = 48666;
	addripv6.sin6_port = htons (grp6);
      }
    ch = bind (sock6, (const sockaddr*) &addripv6, sizeof(addripv6));
    if (ch < 0)
      {
#ifdef __linux
	std::cerr << "ipv6 bind error: " << strerror (errno) << std::endl;
#endif
#ifdef _WIN32
    ch = WSAGetLastError ();
    std::cerr << "ipv6 bind error: " << ch << std::endl;
#endif
      }
    struct ipv6_mreq group;
    group.ipv6mr_interface = 0;
    ch = inet_pton (AF_INET6, lip6.c_str (), &group.ipv6mr_multiaddr);
    if (ch <= 0)
      {
	if (ch < 0)
	  {
#ifdef __linux
	    std::cerr << "ipv6 group creation error: ";
	    std::cerr << strerror (errno) << std::endl;
#endif
#ifdef _WIN32
    ch = WSAGetLastError ();
    std::cerr << "ipv6 group creation error: " << ch << std::endl;
#endif
	  }
	else
	  {
	    std::cerr << "ipv6 group creation error: ";
	    std::cerr << "not valid group adress" << std::endl;
	  }
      }
#ifdef __linux
    setsockopt (sock6, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group,
		sizeof(group));
    int sock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
    setsockopt (sock6, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
			(char*) &group, sizeof(group));
    int sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    ioctlsocket (sock, FIONBIO, &nonblocking_enabled);
#endif
#ifdef __linux
    setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &brdcstfl, sizeof(brdcstfl));
#endif
#ifdef _WIN32
    setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char*) &brdcstfl,
			sizeof(brdcstfl));
#endif
    sockaddr_in addripv4 =
      { };
    addripv4.sin_family = AF_INET;
    uint32_t ip4;
    std::string lip;
#ifdef __linux
    addripv4.sin_addr.s_addr = INADDR_ANY;
#endif
#ifdef _WIN32
    inet_pton (AF_INET, IPV4.c_str (), &addripv4.sin_addr.s_addr);
#endif
    int grp4 = 0;
    itprv = std::find_if (this->prefvect.begin (), this->prefvect.end (), []
    (auto &el)
      {
	return std::get<0>(el) == "Locip4port";
      });
    if (itprv != this->prefvect.end ())
      {
	if (std::get<1> (*itprv) != "")
	  {
	    lip = std::get<1> (*itprv);
	    lip = lip.substr (0, lip.find (":"));
	    ch = inet_pton (AF_INET, lip.c_str (), &ip4);
	    std::string lop = std::get<1> (*itprv);
	    lop.erase (0, lop.find (":") + std::string (":").size ());
	    std::stringstream strm;
	    std::locale loc ("C");
	    strm.imbue (loc);
	    strm << lop;
	    strm >> grp4;
	    addripv4.sin_port = htons (grp4);
	  }
	else
	  {
	    lip = "239.192.150.8";
	    ch = inet_pton (AF_INET, lip.c_str (), &ip4);
	    grp4 = 48655;
	    addripv4.sin_port = htons (grp4);
	  }
      }
    else
      {
	lip = "239.192.150.8";
	ch = inet_pton (AF_INET, lip.c_str (), &ip4);
	grp4 = 48655;
	addripv4.sin_port = htons (grp4);
      }
    int addrlen1 = sizeof(addripv4);
    ch = bind (sock, (const sockaddr*) &addripv4, addrlen1);
    ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = ip4;
    mreq.imr_interface.s_addr = INADDR_ANY;
#ifdef __linux
    setsockopt (sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
#endif
#ifdef _WIN32
    setsockopt (sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq,
			sizeof(mreq));
#endif
    if (ch >= 0)
      {
	sockaddr_in6 sendipv6 =
	  { };
	sendipv6.sin6_family = AF_INET6;
	sendipv6.sin6_port = htons (grp6);
	inet_pton (AF_INET6, lip6.c_str (), &sendipv6.sin6_addr);
	std::vector<char> bindmsg6;
	std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	okp = lt::dht::ed25519_create_keypair (*seed);
	std::array<char, 32> okey = std::get<0> (okp).bytes;
	std::copy (okey.begin (), okey.end (), std::back_inserter (bindmsg6));
	std::string type = "BB";
	std::copy (type.begin (), type.end (), std::back_inserter (bindmsg6));
	std::string b6ip;
	this->ownipv6mtx.lock ();
	b6ip = this->ownipv6;
	uint16_t b6port = this->ownipv6port;
	this->ownipv6mtx.unlock ();
	std::stringstream strm;
	std::locale loc ("C");
	strm.imbue (loc);
	strm << b6port;
	b6ip = b6ip + " " + strm.str ();
	std::copy (b6ip.begin (), b6ip.end (), std::back_inserter (bindmsg6));

	inet_pton (AF_INET, lip.c_str (), &ip4);
	sockaddr_in sendipv4 =
	  { };
	sendipv4.sin_family = AF_INET;
	sendipv4.sin_port = htons (grp4);
	sendipv4.sin_addr.s_addr = ip4;
	std::vector<char> bindmsg;

	std::copy (okey.begin (), okey.end (), std::back_inserter (bindmsg));
	std::copy (type.begin (), type.end (), std::back_inserter (bindmsg));
	uint32_t ownlip;
	this->IPV4mtx.lock ();
	inet_pton (AF_INET, IPV4.c_str (), &ownlip);
	this->IPV4mtx.unlock ();
	bindmsg.resize (bindmsg.size () + sizeof(ownlip));
	std::memcpy (&bindmsg[34], &ownlip, sizeof(ownlip));
	time_t tmb = 0;
	pollfd *fd = new pollfd[2];
	fd[0].fd = sock;
	fd[0].events = POLLRDNORM;
	fd[1].fd = sock6;
	fd[1].events = POLLRDNORM;
	AuxFunc af;
	for (;;)
	  {
	    this->sockmtx.lock ();
	    if ((this->cancel) > 0 || this->sockets4.size () == 0)
	      {
		this->sockmtx.unlock ();
		break;
	      }
	    this->sockmtx.unlock ();
	    time_t curtime = time (NULL);

	    if (curtime - tmb > sintv)
	      {
		this->getfrresmtx.lock ();
		this->contmtx.lock ();
		if (this->getfrres.size () < this->contacts.size ())
		  {
		    tmb = curtime;
		    int ch = sendto (sock, bindmsg.data (), bindmsg.size (), 0,
				     (struct sockaddr*) &sendipv4,
				     sizeof(sendipv4));
		    if (ch < 0)
		      {
#ifdef __linux
			std::cerr << "Send BB ip4 error: " << strerror (errno)
			    << std::endl;
#endif
#ifdef _WIN32
                        ch = WSAGetLastError ();
                        std::cerr << "Send BB ip4 error: " << ch << std::endl;
#endif
		      }
		    else
		      {
			std::vector<char> tmpmsg;
			tmpmsg.resize (INET_ADDRSTRLEN);

			std::cout << "BB sent to "
			    << inet_ntop (AF_INET, &sendipv4.sin_addr.s_addr,
					  tmpmsg.data (), tmpmsg.size ())
			    << std::endl;
		      }
		  }

		this->getfrresmtx.unlock ();

		this->ipv6contmtx.lock ();
		if (this->ownipv6 != "")
		  {
		    if (this->ipv6cont.size () < this->contacts.size ())
		      {
			tmb = curtime;
			int ch = sendto (sock6, bindmsg6.data (),
					 bindmsg6.size (), 0,
					 (struct sockaddr*) &sendipv6,
					 sizeof(sendipv6));
			if (ch < 0)
			  {
#ifdef __linux
			    std::cerr << "Send BB ip6 error: "
				<< strerror (errno) << std::endl;
#endif
#ifdef _WIN32
                            ch = WSAGetLastError ();
                            std::cerr << "Send BB ip6 error: " << ch << std::endl;
#endif
			  }
			else
			  {
			    std::vector<char> tmpmsg;
			    tmpmsg.resize (INET6_ADDRSTRLEN);

			    std::cout << "BB ip6 sent to "
				<< inet_ntop (AF_INET6, &sendipv6.sin6_addr,
					      tmpmsg.data (), tmpmsg.size ())
				<< std::endl;
			  }
		      }
		  }
		this->ipv6contmtx.unlock ();
		this->contmtx.unlock ();
	      }

	    int respol = poll (fd, 2, 1000);
	    if (respol > 0)
	      {
		for (size_t i = 0; i < 2; i++)
		  {
		    if (fd[i].revents == POLLNVAL || fd[i].revents == POLLERR
			|| fd[i].revents == POLLHUP)
		      {
			std::cerr << "Polling error: "
			    << strerror (fd[i].revents) << std::endl;
		      }
		    else
		      {
			if (i == 0 && fd[i].revents == POLLRDNORM)
			  {
			    std::vector<char> buf;
			    buf.resize (200);
			    sockaddr_in from =
			      { };
#ifdef __linux
			    uint sizefrom = sizeof(from);
#endif
#ifdef _WIN32
                            int sizefrom = sizeof(from);
#endif
			    int n = recvfrom (fd[i].fd, buf.data (),
					      buf.size (),
					      MSG_PEEK,
					      (struct sockaddr*) &from,
					      &sizefrom);
			    if (n > 0 && n <= 576)
			      {
				buf.resize (n);
				recvfrom (fd[i].fd, buf.data (), buf.size (), 0,
					  (struct sockaddr*) &from, &sizefrom);
				std::array<char, 32> keyarr;
				std::copy_n (buf.begin (), 32, keyarr.begin ());
				std::string key = lt::aux::to_hex (keyarr);
				uint32_t rcvdip = 0;

				this->contmtx.lock ();
				auto itcont = std::find_if (
				    this->contacts.begin (),
				    this->contacts.end (), [&key]
				    (auto &el)
				      {
					return std::get<1>(el) == key;
				      });
				if (itcont != this->contacts.end ())
				  {
				    std::string msgtype = std::string (
					buf.begin () + 32, buf.begin () + 34);
				    if (msgtype == "BB")
				      {
					std::cout << msgtype << " rcvd from "
					    << key << std::endl;
					if (buf.size () == 38)
					  {
					    std::memcpy (&rcvdip, &buf[34],
							 sizeof(rcvdip));
					  }

					std::vector<char> msg;
					std::copy (okey.begin (), okey.end (),
						   std::back_inserter (msg));
					type = "BR";
					std::copy (type.begin (), type.end (),
						   std::back_inserter (msg));
					sockaddr_in addressp =
					  { };
#ifdef __linux
					unsigned int len = sizeof(addressp);
#endif
#ifdef _WIN32
                                        int len = sizeof(addressp);
#endif
					uint16_t oport;
					if (this->sockmtx.try_lock ())
					  {
					    auto itsock =
						std::find_if (
						    this->sockets4.begin (),
						    this->sockets4.end (),
						    [&key]
						    (auto &el)
						      {
							return std::get<0>(el) == key;
						      });
					    if (itsock != this->sockets4.end ())
					      {
						getsockname (
						    std::get<1> (*itsock),
						    (sockaddr*) &addressp,
						    &len);
						oport = addressp.sin_port;
					      }
					    else
					      {
						oport = 0;
					      }
					    this->sockmtx.unlock ();
					    msg.resize (
						msg.size () + sizeof(ownlip));
					    std::memcpy (&msg[34], &ownlip,
							 sizeof(ownlip));
					    msg.resize (
						msg.size () + sizeof(oport));
					    std::memcpy (&msg[38], &oport,
							 sizeof(oport));
					    sockaddr_in repadr =
					      { };
					    repadr.sin_family = AF_INET;
					    repadr.sin_addr.s_addr = rcvdip;
					    repadr.sin_port = htons (grp4);
					    int ch = sendto (
						fd[i].fd, msg.data (),
						msg.size (), 0,
						(struct sockaddr*) &repadr,
						sizeof(repadr));
					    if (ch < 0)
					      {
#ifdef __linux
						std::cerr
						    << "Send BR ip4 error: "
						    << strerror (errno)
						    << std::endl;
#endif
#ifdef _WIN32
                                                ch = WSAGetLastError ();
                                                std::cerr << "Send BR ip4 error: " << ch << std::endl;
#endif
					      }
					  }
				      }
				    if (msgtype == "BR" || msgtype == "BC")
				      {
					std::cout << msgtype
					    << " ip4 rcvd from " << key
					    << std::endl;
					uint32_t othip;
					uint16_t othport;
					std::memcpy (&othip, &buf[34],
						     sizeof(othip));
					std::memcpy (&othport, &buf[38],
						     sizeof(othport));
					this->getfrresmtx.lock ();
					auto it = std::find_if (
					    this->getfrres.begin (),
					    this->getfrres.end (), [&key]
					    (auto &el)
					      {
						return std::get<0>(el) == key;
					      });
					if (it == this->getfrres.end ()
					    && othport != 0)
					  {
					    std::tuple<std::string, uint32_t,
						uint16_t, int> ttup;
					    std::get<0> (ttup) = key;
					    std::get<1> (ttup) = othip;
					    std::get<2> (ttup) = othport;
					    std::get<3> (ttup) = 1;
					    this->getfrres.push_back (ttup);
					  }
					else
					  {
					    if (othport != 0)
					      {
						std::tuple<std::string,
						    uint32_t, uint16_t, int> ttup =
						    *it;
						std::get<0> (ttup) = key;
						std::get<1> (ttup) = othip;
						std::get<2> (ttup) = othport;
						std::get<3> (ttup) =
						    std::get<3> (ttup) + 1;
						*it = ttup;
					      }
					  }
					this->getfrresmtx.unlock ();
					if (msgtype == "BR")
					  {
					    std::vector<char> msg;
					    std::copy (
						okey.begin (), okey.end (),
						std::back_inserter (msg));
					    type = "BC";
					    std::copy (
						type.begin (), type.end (),
						std::back_inserter (msg));
					    sockaddr_in addressp =
					      { };
#ifdef __linux
					    unsigned int len = sizeof(addressp);
#endif
#ifdef _WIN32
                                	    int len = sizeof(addressp);
#endif
					    uint16_t oport;
					    if (this->sockmtx.try_lock ())
					      {
						auto itsock =
						    std::find_if (
							this->sockets4.begin (),
							this->sockets4.end (),
							[&key]
							(auto &el)
							  {
							    return std::get<0>(el) == key;
							  });
						if (itsock
						    != this->sockets4.end ())
						  {
						    getsockname (
							std::get<1> (*itsock),
							(sockaddr*) &addressp,
							&len);
						    oport = addressp.sin_port;
						  }
						else
						  {
						    oport = 0;
						  }
						this->sockmtx.unlock ();
						msg.resize (
						    msg.size ()
							+ sizeof(ownlip));
						std::memcpy (&msg[34], &ownlip,
							     sizeof(ownlip));
						msg.resize (
						    msg.size ()
							+ sizeof(oport));
						std::memcpy (&msg[38], &oport,
							     sizeof(oport));
						sockaddr_in repadr =
						  { };
						repadr.sin_family = AF_INET;
						repadr.sin_addr.s_addr = othip;
						repadr.sin_port = htons (grp4);
						int ch = sendto (
						    fd[i].fd, msg.data (),
						    msg.size (), 0,
						    (struct sockaddr*) &repadr,
						    sizeof(repadr));
						if (ch < 0)
						  {
#ifdef __linux
						    std::cerr
							<< "Send BC ip4 error: "
							<< strerror (errno)
							<< std::endl;
#endif
#ifdef _WIN32
						    ch = WSAGetLastError ();
						    std::cerr << "Send BC ip4 error: " << ch << std::endl;
#endif
						  }
					      }
					  }
				      }
				  }
				else
				  {
				    sockaddr_in delfrom =
				      { };
				    sizefrom = sizeof(delfrom);
				    std::vector<char> tmpmsg;
				    tmpmsg.resize (INET_ADDRSTRLEN);
				    recvfrom (fd[i].fd, buf.data (),
					      buf.size (), 0,
					      (struct sockaddr*) &delfrom,
					      &sizefrom);
				    std::cerr << "Wrong packet from "
					<< inet_ntop (AF_INET,
						      &delfrom.sin_addr.s_addr,
						      tmpmsg.data (),
						      tmpmsg.size ()) << ":"
					<< ntohs (delfrom.sin_port)
					<< std::endl;
				  }
				this->contmtx.unlock ();
			      }
			    else
			      {
				std::cout << "Bootstrap ip4 error!"
				    << std::endl;
			      }
			  }
			if (i == 1 && fd[i].revents == POLLRDNORM)
			  {
			    std::vector<char> buf;
			    buf.resize (200);
			    sockaddr_in6 from =
			      { };
#ifdef __linux
			    uint sizefrom = sizeof(from);
#endif
#ifdef _WIN32
	                    int sizefrom = sizeof(from);
#endif
			    int n = recvfrom (fd[i].fd, buf.data (),
					      buf.size (),
					      MSG_PEEK,
					      (struct sockaddr*) &from,
					      &sizefrom);
			    if (n > 0 && n <= 576)
			      {
				buf.resize (n);
				recvfrom (fd[i].fd, buf.data (), buf.size (), 0,
					  (struct sockaddr*) &from, &sizefrom);
				std::array<char, 32> keyarr;
				std::copy_n (buf.begin (), 32, keyarr.begin ());
				std::string key = lt::aux::to_hex (keyarr);
				std::string rcvdip = "";

				this->contmtx.lock ();
				auto itcont = std::find_if (
				    this->contacts.begin (),
				    this->contacts.end (), [&key]
				    (auto &el)
				      {
					return std::get<1>(el) == key;
				      });
				if (itcont != this->contacts.end ())
				  {
				    std::string msgtype = std::string (
					buf.begin () + 32, buf.begin () + 34);
				    if (msgtype == "BB")
				      {
					std::cout << msgtype
					    << " ip6 rcvd from " << key
					    << std::endl;

					rcvdip = std::string (buf.begin () + 34,
							      buf.end ());
					uint16_t rcvdport;
					std::stringstream strm2;
					strm2.imbue (loc);
					strm2
					    << rcvdip.substr (
						rcvdip.find (" ")
						    + std::string (" ").size (),
						std::string::npos);
					strm2 >> rcvdport;
					rcvdip = rcvdip.substr (
					    0, rcvdip.find (" "));
					std::vector<char> msg;
					std::copy (okey.begin (), okey.end (),
						   std::back_inserter (msg));
					type = "BR";
					std::copy (type.begin (), type.end (),
						   std::back_inserter (msg));

					uint16_t oport;
					this->ownipv6mtx.lock ();
					std::string sndstr = this->ownipv6;
					oport = this->ownipv6port;
					this->ownipv6mtx.unlock ();
					strm2.clear ();
					strm2.str ("");
					strm2.imbue (loc);
					strm2 << oport;
					sndstr = sndstr + " " + strm2.str ();
					std::copy (sndstr.begin (),
						   sndstr.end (),
						   std::back_inserter (msg));
					sockaddr_in6 repadr =
					  { };
					repadr.sin6_family = AF_INET6;
					inet_pton (AF_INET6, rcvdip.c_str (),
						   &repadr.sin6_addr);
					repadr.sin6_port = htons (grp6);
					int ch = sendto (
					    fd[i].fd, msg.data (), msg.size (),
					    0, (struct sockaddr*) &repadr,
					    sizeof(repadr));
					if (ch < 0)
					  {
#ifdef __linux
					    std::cerr << "Send BR ip6 error: "
						<< strerror (errno)
						<< std::endl;
#endif
#ifdef _WIN32
					    ch = WSAGetLastError ();
					    std::cerr << "Send BR ip6 error: " << ch << std::endl;
#endif
					  }

				      }
				    if (msgtype == "BR" || msgtype == "BC")
				      {
					std::cout << msgtype
					    << " ip6 rcvd from " << key
					    << std::endl;
					std::string othip (buf.begin () + 34,
							   buf.end ());
					std::stringstream strm2;
					strm2.imbue (loc);
					strm2
					    << othip.substr (
						othip.find (" ")
						    + std::string (" ").size (),
						std::string::npos);
					uint16_t othport;
					strm2 >> othport;
					othip = othip.substr (0,
							      othip.find (" "));
					this->ipv6contmtx.lock ();
					auto it = std::find_if (
					    this->ipv6cont.begin (),
					    this->ipv6cont.end (), [&key]
					    (auto &el)
					      {
						return std::get<0>(el) == key;
					      });
					if (it == this->ipv6cont.end ()
					    && othport != 0)
					  {
					    std::tuple<std::string, std::string,
						uint16_t, int> ttup;
					    std::get<0> (ttup) = key;
					    std::get<1> (ttup) = othip;
					    std::get<2> (ttup) = othport;
					    std::get<3> (ttup) = 1;
					    this->ipv6cont.push_back (ttup);
					  }
					else
					  {
					    if (othport != 0)
					      {
						std::tuple<std::string,
						    std::string, uint16_t, int> ttup =
						    *it;
						std::get<0> (ttup) = key;
						std::get<1> (ttup) = othip;
						std::get<2> (ttup) = othport;
						std::get<3> (ttup) =
						    std::get<3> (ttup) + 1;
						*it = ttup;
					      }
					  }
					this->ipv6contmtx.unlock ();
					if (msgtype == "BR")
					  {
					    std::vector<char> msg;
					    std::copy (
						okey.begin (), okey.end (),
						std::back_inserter (msg));
					    type = "BC";
					    std::copy (
						type.begin (), type.end (),
						std::back_inserter (msg));
					    uint16_t oport;
					    this->ownipv6mtx.lock ();
					    std::string sndstr = this->ownipv6;
					    oport = this->ownipv6port;
					    this->ownipv6mtx.unlock ();
					    strm2.clear ();
					    strm2.str ("");
					    strm2.imbue (loc);
					    strm2 << oport;
					    sndstr = sndstr + " "
						+ strm2.str ();
					    std::copy (
						sndstr.begin (), sndstr.end (),
						std::back_inserter (msg));

					    sockaddr_in6 repadr =
					      { };
					    repadr.sin6_family = AF_INET6;
					    inet_pton (AF_INET6, othip.c_str (),
						       &repadr.sin6_addr);
					    repadr.sin6_port = htons (grp6);
					    int ch = sendto (
						fd[i].fd, msg.data (),
						msg.size (), 0,
						(struct sockaddr*) &repadr,
						sizeof(repadr));
					    if (ch < 0)
					      {
#ifdef __linux
						std::cerr
						    << "Send BC ip6 error: "
						    << strerror (errno)
						    << std::endl;
#endif
#ifdef _WIN32
						ch = WSAGetLastError ();
						std::cerr << "Send BC ip6 error: " << ch << std::endl;
#endif
					      }
					  }
				      }
				  }
				else
				  {
				    sockaddr_in6 delfrom =
				      { };
				    sizefrom = sizeof(delfrom);
				    std::vector<char> tmpmsg;
				    tmpmsg.resize (INET6_ADDRSTRLEN);
				    recvfrom (fd[i].fd, buf.data (),
					      buf.size (), 0,
					      (struct sockaddr*) &delfrom,
					      &sizefrom);
				    std::cerr << "Wrong packet from "
					<< inet_ntop (AF_INET6,
						      &delfrom.sin6_addr,
						      tmpmsg.data (),
						      tmpmsg.size ()) << ":"
					<< ntohs (delfrom.sin6_port)
					<< std::endl;
				  }
				this->contmtx.unlock ();
			      }
			    else
			      {
				std::cout << "Bootstrap ip6 error!"
				    << std::endl;
			      }
			  }
		      }
		  }
	      }
	    else
	      {
		if (respol < 0)
		  {
		    std::cout << "Bootstrap error!" << std::endl;
		  }
	      }
	  }
	delete[] fd;
      }
    else
      {
#ifdef __linux
	std::cerr << "Bind error: " << strerror (errno) << std::endl;
#endif
#ifdef _WIN32
        int ch = WSAGetLastError ();
        std::cerr << "Bind error: " << ch << std::endl;
#endif
      }
    thrmtx->unlock ();
  });
  bts->detach ();
  delete bts;
}

std::filesystem::path
NetworkOperations::removeMsg (std::string key, std::filesystem::path msgpath)
{
  std::string filename = msgpath.parent_path ().filename ().u8string ();
  AuxFunc af;
  af.homePath (&filename);
  filename = filename + "/.Communist/SendBufer/"
      + msgpath.parent_path ().filename ().u8string () + "/"
      + msgpath.filename ().u8string ();
  std::filesystem::path npath = std::filesystem::u8path (filename);
  std::filesystem::path retpath;
  sendbufmtx.lock ();
  if (std::filesystem::exists (npath))
    {
      filename = npath.filename ().u8string ();
      std::string::size_type n;
      n = filename.find ("f");
      std::string nmrnm;
      if (n == std::string::npos)
	{
	  msgpartbufmtx.lock ();
	  msgpartbuf.erase (
	      std::remove_if (msgpartbuf.begin (), msgpartbuf.end (), [&npath]
	      (auto &el)
		{
		  return std::get<3>(el) == npath;
		}),
	      msgpartbuf.end ());
	  msgpartbufmtx.unlock ();
	  std::filesystem::remove (npath);
	  nmrnm = npath.filename ().u8string ();
	}
      else
	{
	  nmrnm = npath.filename ().u8string ();
	  nmrnm = nmrnm.substr (0, nmrnm.find ("f"));
	  std::string unm = key;
	  std::tuple<lt::dht::public_key, lt::dht::secret_key> okp;
	  okp = lt::dht::ed25519_create_keypair (*seed);
	  lt::dht::public_key pkpass;
	  lt::aux::from_hex (unm, pkpass.bytes.data ());
	  std::array<char, 32> scalar;
	  scalar = lt::dht::ed25519_key_exchange (pkpass, std::get<1> (okp));
	  pkpass = lt::dht::ed25519_add_scalar (pkpass, scalar);
	  std::string passwd = lt::aux::to_hex (pkpass.bytes);
	  filename = npath.parent_path ().u8string ();
	  filename = filename + "/TmpMsg";
	  std::filesystem::path outpath = std::filesystem::u8path (filename);
	  af.decryptFile (unm, passwd, npath.u8string (), outpath.u8string ());
	  std::fstream f;
	  int count = 0;
	  f.open (outpath, std::ios_base::in);
	  while (!f.eof ())
	    {
	      getline (f, filename);
	      if (count == 5)
		{
		  break;
		}
	      count++;
	    }
	  f.close ();
	  std::filesystem::path sp = std::filesystem::u8path (filename);
	  retpath = sp;
	  filepartbufmtx.lock ();
	  auto itfpb = std::find_if (filepartbuf.begin (), filepartbuf.end (),
				     [&sp]
				     (auto &el)
				       {
					 return std::get<2> (el) == sp;
				       });
	  if (itfpb != filepartbuf.end ())
	    {
	      std::get<3> (*itfpb) = std::filesystem::file_size (sp);
	      std::get<6> (*itfpb) = 1;
	      std::tuple<std::string, time_t> ttup;
	      std::get<0> (ttup) = key;
	      std::get<1> (ttup) = std::get<1> (*itfpb);
	      filecanceledmtx.lock ();
	      filecanceled.push_back (ttup);
	      filecanceledmtx.unlock ();
	    }
	  filepartbufmtx.unlock ();
	  std::filesystem::remove (outpath);
	  std::filesystem::remove (npath);
	}
      af.homePath (&filename);
      filename = filename + "/.Communist/SendBufer/"
	  + msgpath.parent_path ().filename ().u8string ();
      std::filesystem::path dirp = std::filesystem::u8path (filename);
      std::stringstream strm;
      std::locale loc ("C");
      int indint;
      strm.imbue (loc);
      strm << nmrnm;
      strm >> indint;
      for (auto &dirit : std::filesystem::directory_iterator (dirp))
	{
	  std::filesystem::path p = dirit.path ();
	  int tmpi;
	  std::string ind = p.filename ().u8string ();
	  std::string::size_type n;
	  n = ind.find ("f");
	  ind = ind.substr (0, n);
	  strm.clear ();
	  strm.str ("");
	  strm.imbue (loc);
	  strm << ind;
	  strm >> tmpi;
	  if (tmpi > indint)
	    {
	      tmpi = tmpi - 1;
	      strm.clear ();
	      strm.str ("");
	      strm.imbue (loc);
	      strm << tmpi;
	      ind = p.parent_path ().u8string ();
	      ind = ind + "/" + strm.str ();
	      if (n != std::string::npos)
		{
		  ind = ind + "f";
		}
	      std::filesystem::path np = std::filesystem::u8path (ind);
	      std::filesystem::rename (p, np);
	    }
	}

    }
  sendbufmtx.unlock ();
  return retpath;
}

void
NetworkOperations::cancelAll ()
{
  cancel = 1;
  std::thread *thr = new std::thread (
      [this]
      {
	for (;;)
	  {
	    if (this->threadvectmtx.try_lock ())
	      {
		for (;;)
		  {
		    auto itthrv = std::find_if (
			this->threadvect.begin (), this->threadvect.end (), []
			(auto &el)
			  {
			    std::mutex *gmtx = std::get<0>(el);

			    if (gmtx)
			      {
				if (gmtx->try_lock())
				  {
				    gmtx->unlock();
				    return true;
				  }
				else
				  {
				    return false;
				  }}
			    else
			      {
				return true;
			      }

			  });
		    if (itthrv != this->threadvect.end ())
		      {
			std::mutex *gmtx = std::get<0> (*itthrv);
			this->threadvect.erase (itthrv);
			delete gmtx;
		      }
		    if (this->threadvect.size () == 0)
		      {
			break;
		      }
		    usleep (100);
		  }
		this->threadvectmtx.unlock ();
		break;
	      }
	    usleep (100);
	  }
	for (size_t i = 0; i < this->sockets4.size (); i++)
	  {
	    if (std::get<1> (this->sockets4[i]) >= 0)
	      {
#ifdef __linux
	close (std::get<1> (this->sockets4[i]));
#endif
#ifdef _WIN32
	  	int ch = closesocket (std::get<1> (this->sockets4[i]));
	  	if (ch != 0)
	  	  {
	  	    ch = WSAGetLastError ();
	  	    std::cerr << "ipv4 close socket error: " << ch << std::endl;
	  	  }
	  #endif
      }
  }
this->sockets4.clear ();

#ifdef __linux
	close (this->sockipv6);
#endif
#ifdef _WIN32
	int ch = closesocket (this->sockipv6);
	if (ch != 0)
	  {
	    ch = WSAGetLastError ();
	    std::cerr << "ipv6 close socket error: " << ch << std::endl;
	  }
	ch = WSACleanup ();
	if (ch != 0)
	  {
	    ch = WSAGetLastError ();
	    std::cerr << "Windows cleanup error: " << ch << std::endl;
	  }
#endif
	this->canceled.emit ();
      });
  thr->detach ();
  delete thr;
}

void
NetworkOperations::cancelSendF (std::string key, std::filesystem::path filepath)
{
  filepartbufmtx.lock ();
  auto itfpb = std::find_if (filepartbuf.begin (), filepartbuf.end (),
			     [filepath]
			     (auto &el)
			       {
				 return std::get<2> (el) == filepath;
			       });
  if (itfpb != filepartbuf.end ())
    {
      std::get<3> (*itfpb) = std::filesystem::file_size (filepath);
      std::get<6> (*itfpb) = 1;
      std::tuple<std::string, time_t> ttup;
      std::get<0> (ttup) = key;
      std::get<1> (ttup) = std::get<1> (*itfpb);
      filecanceledmtx.lock ();
      filecanceled.push_back (ttup);
      filecanceledmtx.unlock ();
    }
  filepartbufmtx.unlock ();
}

void
NetworkOperations::cancelReceivF (std::string key,
				  std::filesystem::path filepath)
{
  time_t tm = 0;
  filehashvectmtx.lock ();
  auto itfhv = std::find_if (
      filehashvect.begin (), filehashvect.end (), [key, filepath]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<3>(el) == filepath)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
  if (itfhv != filehashvect.end ())
    {
      tm = std::get<1> (*itfhv);
      filehashvect.erase (itfhv);
    }
  filehashvectmtx.unlock ();

  filesendreqmtx.lock ();
  filesendreq.erase (
      std::remove_if (filesendreq.begin (), filesendreq.end (), [key, tm]
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
	}),
      filesendreq.end ());
  filesendreqmtx.unlock ();

  fqrcvdmtx.lock ();
  fqrcvd.erase (std::remove_if (fqrcvd.begin (), fqrcvd.end (), [key, tm]
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
		fqrcvd.end ());
  fqrcvdmtx.unlock ();

  fileparthashmtx.lock ();
  fileparthash.erase (
      std::remove_if (fileparthash.begin (), fileparthash.end (), [key, tm]
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
      fileparthash.end ());
  fileparthashmtx.unlock ();

  filepartrcvmtx.lock ();
  filepartrcv.erase (
      std::remove_if (filepartrcv.begin (), filepartrcv.end (), [key, tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == uint64_t(tm))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
      filepartrcv.end ());
  filepartrcvmtx.unlock ();

  filepartrlogmtx.lock ();
  filepartrlog.erase (
      std::remove_if (filepartrlog.begin (), filepartrlog.end (), [key, tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == uint64_t(tm))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
      filepartrlog.end ());
  filepartrlogmtx.unlock ();

  currentpartmtx.lock ();
  currentpart.erase (
      std::remove_if (currentpart.begin (), currentpart.end (), [key, tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == uint64_t(tm))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
      currentpart.end ());
  currentpartmtx.unlock ();

  fbrvectmtx.lock ();
  fbrvect.erase (std::remove_if (fbrvect.begin (), fbrvect.end (), [key, tm]
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
		 fbrvect.end ());
  fbrvectmtx.unlock ();

  filepartendmtx.lock ();
  filepartend.erase (
      std::remove_if (filepartend.begin (), filepartend.end (), [key, tm]
      (auto &el)
	{
	  if (std::get<0>(el) == key && std::get<1>(el) == uint64_t(tm))
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
      filepartend.end ());
  filepartendmtx.unlock ();

  fileendmtx.lock ();
  fileend.erase (std::remove_if (fileend.begin (), fileend.end (), [key, tm]
  (auto &el)
    {
      if (std::get<0>(el) == key && std::get<1>(el) == uint64_t(tm))
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }),
		 fileend.end ());
  fileendmtx.unlock ();
  std::filesystem::remove (filepath);
}

#ifdef _WIN32
int
NetworkOperations::poll (struct pollfd *pfd, int nfds, int timeout)
{
  return WSAPoll (pfd, nfds, timeout);
}
#endif

void
NetworkOperations::stunSrv ()
{
  int addrlen = 0;
  if (Enablestun == "active")
    {
#ifdef __linux
      int stnsrvsock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
      int stnsrvsock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      u_long nonblocking_enabled = TRUE;
      int ch = 0;
      ioctlsocket (stnsrvsock, FIONBIO, &nonblocking_enabled);
#endif
      sockaddr_in stunsrvaddr =
	{ };
      stunsrvaddr.sin_family = AF_INET;
      stunsrvaddr.sin_addr.s_addr = INADDR_ANY;
      stunsrvaddr.sin_port = htons (stunport);
      addrlen = sizeof(stunsrvaddr);
      if (bind (stnsrvsock, (const sockaddr*) &stunsrvaddr, addrlen) == 0)
	{
	  std::mutex *thrmtx = new std::mutex;
	  thrmtx->lock ();
	  threadvectmtx.lock ();
	  threadvect.push_back (std::make_tuple (thrmtx, "STUN server thread"));
	  threadvectmtx.unlock ();
	  std::thread *stthr = new std::thread ( [this, thrmtx, stnsrvsock]
	  {
	    pollfd fdsl[1];
	    fdsl[0].fd = stnsrvsock;
	    fdsl[0].events = POLLRDNORM;

	    for (;;)
	      {
		if (this->cancel > 0)
		  {
		    break;
		  }
		int respol = poll (fdsl, 1, 3000);
		if (respol > 0)
		  {
		    std::vector<char> msgv;
		    msgv.resize (20);
		    sockaddr_in from =
		      { };
		    socklen_t sizefrom = sizeof(from);
		    recvfrom (stnsrvsock, msgv.data (), msgv.size (), 0, (struct sockaddr*) &from, &sizefrom);
		    uint16_t tt = 1000;
		    std::memcpy (&tt, &msgv[0], sizeof(tt));
		    uint16_t tt2 = 1000;
		    std::memcpy (&tt2, &msgv[2], sizeof(tt2));
		    if (ntohs (tt) == 1 && ntohs (tt2) == 0)
		      {
			uint32_t ttt = 0;
			std::memcpy (&ttt, &msgv[4], sizeof(ttt));
			msgv.clear ();
			uint16_t type = htons (32);
			msgv.resize (msgv.size () + sizeof(type));
			std::memcpy (&msgv[0], &type, sizeof(type));
			uint16_t length = 64;
			msgv.resize (msgv.size () + sizeof(length));
			std::memcpy (&msgv[2], &length, sizeof(length));
			uint8_t zer = 0;
			msgv.resize (msgv.size () + sizeof(zer));
			std::memcpy (&msgv[4], &zer, sizeof(zer));
			uint8_t family = uint8_t (0x01);
			msgv.resize (msgv.size () + sizeof(family));
			std::memcpy (&msgv[5], &family, sizeof(family));
			uint16_t port = from.sin_port;
			port = ntohs (port);
			port ^= 8466;
			port = htons (port);
			msgv.resize (msgv.size () + sizeof(port));
			std::memcpy (&msgv[6], &port, sizeof(port));
			uint32_t ip = from.sin_addr.s_addr;
			ip = ntohl (ip);
			ip ^= 554869826;
			ip = htonl (ip);
			msgv.resize (msgv.size () + sizeof(ip));
			std::memcpy (&msgv[8], &ip, sizeof(ip));
			sockaddr_in stunrp =
			  { };
			stunrp.sin_family = AF_INET;
			stunrp.sin_port = from.sin_port;
			stunrp.sin_addr.s_addr = from.sin_addr.s_addr;
			sendto (stnsrvsock, msgv.data (), msgv.size (), 0,
				(struct sockaddr*) &stunrp, sizeof(stunrp));
		      }
		  }
	      }
#ifdef __linux
	    close (stnsrvsock);
#endif
#ifdef _WIN32
	    closesocket (stnsrvsock);
#endif
	    thrmtx->unlock ();
	  });
	  stthr->detach ();
	  delete stthr;
	}
      else
	{
#ifdef __linux
	  std::cerr << "STUN socket bind error: " << strerror (errno)
	      << std::endl;
#endif
#ifdef _WIN32
	  ch = WSAGetLastError ();
	  std::cerr << "STUN socket bind error: " << ch << std::endl;
#endif
	}
    }
#ifdef __linux
  int stnsock = socket (AF_INET, SOCK_DGRAM | O_NONBLOCK, IPPROTO_UDP);
#endif
#ifdef _WIN32
  int stnsock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  u_long nonblocking_enabled = TRUE;
  int ch = 0;
  ioctlsocket (stnsock, FIONBIO, &nonblocking_enabled);
#endif
  sockaddr_in stunaddr =
    { };
  stunaddr.sin_family = AF_INET;
  stunaddr.sin_addr.s_addr = INADDR_ANY;
  stunaddr.sin_port = 0;
  addrlen = sizeof(stunaddr);
  if (bind (stnsock, (const sockaddr*) &stunaddr, addrlen) == 0)
    {
      std::mutex *thrmtx = new std::mutex;
      thrmtx->lock ();
      threadvectmtx.lock ();
      threadvect.push_back (std::make_tuple (thrmtx, "STUN check thread"));
      threadvectmtx.unlock ();
      std::thread *stthr = new std::thread ( [this, thrmtx, stnsock]
      {
	std::vector<std::tuple<std::string, uint32_t, int, time_t>> chvect;
	for (;;)
	  {
	    if (this->cancel > 0)
	      {
		break;
	      }
	    this->getfrresmtx.lock ();
	    for (size_t i = 0; i < this->getfrres.size (); i++)
	      {
		std::string key = std::get<0> (this->getfrres[i]);
		auto itchv = std::find_if (chvect.begin (), chvect.end (), [&key]
      (auto &el)
	{
	  return key == std::get<0>(el);
	});
		if (itchv == chvect.end ())
		  {
		    time_t sttm = time (NULL);
		    chvect.push_back (
			std::make_tuple (key, std::get<1> (this->getfrres[i]),
					 0, sttm));
		  }
	      }
	    this->getfrresmtx.unlock ();

	    for (size_t i = 0; i < chvect.size (); i++)
	      {
		int chk = std::get<2> (chvect[i]);
		time_t ltm = std::get<3> (chvect[i]);
		time_t curtm = time (NULL);
		if (chk == 0)
		  {
		    chk = chk + 1;
		    std::get<2> (chvect[i]) = chk;
		    std::pair<struct in_addr, uint16_t> p;
		    p.first.s_addr = std::get<1> (chvect[i]);
		    p.second = htons (stunport);
		    std::pair<uint32_t, uint16_t> result;
		    result = this->getOwnIps (stnsock, p);
		    if (std::get<0> (result) != 0 && std::get<1> (result) != 0)
		      {
			this->stunipsmtx.lock ();
			this->stunips.insert (this->stunips.begin (), p);
			this->stunipsmtx.unlock ();
		      }
		    std::get<2> (chvect[i]) = curtm;
		  }
		else
		  {
		    if (chk <= 3 && curtm - ltm >= 3)
		      {
			chk = chk + 1;
			std::get<2> (chvect[i]) = chk;
			std::pair<struct in_addr, uint16_t> p;
			p.first.s_addr = std::get<1> (chvect[i]);
			p.second = htons (stunport);
			std::pair<uint32_t, uint16_t> result;
			result = this->getOwnIps (stnsock, p);
			if (std::get<0> (result) != 0
			    && std::get<1> (result) != 0)
			  {
			    this->stunipsmtx.lock ();
			    this->stunips.insert (this->stunips.begin (), p);
			    this->stunipsmtx.unlock ();
			  }
			std::get<2> (chvect[i]) = curtm;
		      }
		  }
	      }
	    sleep (3);
	  }
#ifdef __linux
	close (stnsock);
#endif
#ifdef _WIN32
	closesocket (stnsock);
#endif
	thrmtx->unlock ();
      });
      stthr->detach ();
      delete stthr;
    }
  else
    {
#ifdef __linux
      std::cerr << "STUN checksocket bind error: " << strerror (errno)
	  << std::endl;
#endif
#ifdef _WIN32
      ch = WSAGetLastError ();
      std::cerr << "STUN checksocket bind error: " << ch << std::endl;
#endif
    }
}
