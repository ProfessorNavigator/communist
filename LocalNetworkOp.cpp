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

#include "LocalNetworkOp.h"

LocalNetworkOp::LocalNetworkOp (NetworkOperations *No)
{
  no = No;
  bootstrFunc ();
}

LocalNetworkOp::~LocalNetworkOp ()
{
  // TODO Auto-generated destructor stub
}

void
LocalNetworkOp::bootstrFunc ()
{
  std::mutex *thrmtx = new std::mutex;
  thrmtx->lock ();
  no->threadvectmtx.lock ();
  no->threadvect.push_back (std::make_tuple (thrmtx, "Bootstr func"));
  no->threadvectmtx.unlock ();
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
    inet_pton (AF_INET6, this->no->ownipv6.c_str(), &addripv6.sin6_addr);
#endif
    std::string lip6 = "";
    int grp6 = 0;
    auto itprv = std::find_if (this->no->prefvect.begin (), this->no->prefvect.end (), []
  (auto &el)
    {
      return std::get<0>(el) == "Locip6port";
    });
    if (itprv != this->no->prefvect.end ())
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
    inet_pton (AF_INET, this->no->IPV4.c_str (), &addripv4.sin_addr.s_addr);
#endif
    int grp4 = 0;
    itprv = std::find_if (this->no->prefvect.begin (), this->no->prefvect.end (), []
    (auto &el)
      {
	return std::get<0>(el) == "Locip4port";
      });
    if (itprv != this->no->prefvect.end ())
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
	okp = lt::dht::ed25519_create_keypair (*(this->no->seed));
	std::array<char, 32> okey = std::get<0> (okp).bytes;
	std::copy (okey.begin (), okey.end (), std::back_inserter (bindmsg6));
	std::string type = "BB";
	std::copy (type.begin (), type.end (), std::back_inserter (bindmsg6));
	std::string b6ip;
	this->no->ownipv6mtx.lock ();
	b6ip = this->no->ownipv6;
	uint16_t b6port = this->no->ownipv6port;
	this->no->ownipv6mtx.unlock ();
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
	this->no->IPV4mtx.lock ();
	inet_pton (AF_INET, this->no->IPV4.c_str (), &ownlip);
	this->no->IPV4mtx.unlock ();
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
	    this->no->sockmtx.lock ();
	    if ((this->no->cancel) > 0 || this->no->sockets4.size () == 0)
	      {
		this->no->sockmtx.unlock ();
		break;
	      }
	    this->no->sockmtx.unlock ();
	    time_t curtime = time (NULL);

	    if (curtime - tmb > sintv)
	      {
		this->no->getfrresmtx.lock ();
		this->no->contmtx.lock ();
		if (this->no->getfrres.size () < this->no->contacts.size ())
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
		this->no->getfrresmtx.unlock ();

		this->no->ipv6contmtx.lock ();
		if (this->no->ownipv6 != "")
		  {
		    if (this->no->ipv6cont.size () < this->no->contacts.size ())
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
		this->no->ipv6contmtx.unlock ();
		this->no->contmtx.unlock ();
	      }

#ifdef __linux
	    int respol = poll (fd, 2, 1000);
#endif
#ifdef _WIN32
	    int respol = this->no->poll (fd, 2, 1000);
#endif
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

				this->no->contmtx.lock ();
				auto itcont = std::find_if (
				    this->no->contacts.begin (),
				    this->no->contacts.end (), [&key]
				    (auto &el)
				      {
					return std::get<1>(el) == key;
				      });
				if (itcont != this->no->contacts.end ())
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
					if (this->no->sockmtx.try_lock ())
					  {
					    auto itsock =
						std::find_if (
						    this->no->sockets4.begin (),
						    this->no->sockets4.end (),
						    [&key]
						    (auto &el)
						      {
							return std::get<0>(el) == key;
						      });
					    if (itsock != this->no->sockets4.end ())
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
					    this->no->sockmtx.unlock ();
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
					this->no->getfrresmtx.lock ();
					auto it = std::find_if (
					    this->no->getfrres.begin (),
					    this->no->getfrres.end (), [&key]
					    (auto &el)
					      {
						return std::get<0>(el) == key;
					      });
					if (it == this->no->getfrres.end ()
					    && othport != 0)
					  {
					    std::tuple<std::string, uint32_t,
						uint16_t, int> ttup;
					    std::get<0> (ttup) = key;
					    std::get<1> (ttup) = othip;
					    std::get<2> (ttup) = othport;
					    std::get<3> (ttup) = 1;
					    this->no->getfrres.push_back (ttup);
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
					this->no->getfrresmtx.unlock ();
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
					    if (this->no->sockmtx.try_lock ())
					      {
						auto itsock =
						    std::find_if (
							this->no->sockets4.begin (),
							this->no->sockets4.end (),
							[&key]
							(auto &el)
							  {
							    return std::get<0>(el) == key;
							  });
						if (itsock
						    != this->no->sockets4.end ())
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
						this->no->sockmtx.unlock ();
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
				this->no->contmtx.unlock ();
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

				this->no->contmtx.lock ();
				auto itcont = std::find_if (
				    this->no->contacts.begin (),
				    this->no->contacts.end (), [&key]
				    (auto &el)
				      {
					return std::get<1>(el) == key;
				      });
				if (itcont != this->no->contacts.end ())
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
					this->no->ownipv6mtx.lock ();
					std::string sndstr = this->no->ownipv6;
					oport = this->no->ownipv6port;
					this->no->ownipv6mtx.unlock ();
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
					this->no->ipv6contmtx.lock ();
					auto it = std::find_if (
					    this->no->ipv6cont.begin (),
					    this->no->ipv6cont.end (), [&key]
					    (auto &el)
					      {
						return std::get<0>(el) == key;
					      });
					if (it == this->no->ipv6cont.end ()
					    && othport != 0)
					  {
					    std::tuple<std::string, std::string,
						uint16_t, int> ttup;
					    std::get<0> (ttup) = key;
					    std::get<1> (ttup) = othip;
					    std::get<2> (ttup) = othport;
					    std::get<3> (ttup) = 1;
					    this->no->ipv6cont.push_back (ttup);
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
					this->no->ipv6contmtx.unlock ();
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
					    this->no->ownipv6mtx.lock ();
					    std::string sndstr = this->no->ownipv6;
					    oport = this->no->ownipv6port;
					    this->no->ownipv6mtx.unlock ();
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
				this->no->contmtx.unlock ();
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

