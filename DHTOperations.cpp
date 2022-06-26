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

#include "DHTOperations.h"

DHTOperations::DHTOperations (NetworkOperations *No)
{
  no = No;
}

DHTOperations::~DHTOperations ()
{
  // TODO Auto-generated destructor stub
}

void
DHTOperations::processDHT ()
{
  std::mutex *thrmtx = new std::mutex;
  thrmtx->lock ();
  no->threadvectmtx.lock ();
  no->threadvect.push_back (std::make_tuple (thrmtx, "Process DHT"));
  no->threadvectmtx.unlock ();
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
    auto itprv = std::find_if (no->prefvect.begin (), no->prefvect.end (), []
    (auto &el)
      {
	return std::get<0>(el) == "Listenifcs";
      });
    if (itprv != no->prefvect.end ())
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

    itprv = std::find_if (no->prefvect.begin (), no->prefvect.end (), []
    (auto &el)
      {
	return std::get<0>(el) == "Bootstrapdht";
      });
    if (itprv != no->prefvect.end ())
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
	if (ses.is_dht_running () || (no->cancel) != 0)
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
    if (count <= 2000 && (no->cancel) == 0)
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
	    if ((no->cancel) > 0)
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
					*(no->seed));
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
					    *(no->seed));
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
DHTOperations::getFrVect ()
{
  std::vector<std::string> result;
  no->getfrmtx.lock ();
  result = no->getfr;
  no->getfr.clear ();
  no->getfrmtx.unlock ();
  return result;
}

std::vector<std::tuple<std::string, uint32_t, uint16_t>>
DHTOperations::putVect ()
{
  std::vector<std::tuple<std::string, uint32_t, uint16_t>> result;
  no->putipmtx.lock ();
  result = no->putipv;
  no->putipv.clear ();
  no->putipmtx.unlock ();
  return result;
}

std::string
DHTOperations::getSes (std::string key, lt::session *ses)
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
  ownkey = lt::dht::ed25519_create_keypair (*(no->seed));
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
DHTOperations::getSes6 (std::string key, lt::session *ses)
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
  ownkey = lt::dht::ed25519_create_keypair (*(no->seed));
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
DHTOperations::putSes (std::string otherkey, uint32_t ip, uint16_t port,
		       lt::session *ses)
{
  AuxFunc af;
  lt::dht::public_key otherpk;
  lt::aux::from_hex (otherkey, otherpk.bytes.data ());
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*(no->seed));
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
DHTOperations::putSes6 (std::string otherkey, lt::session *ses)
{
  AuxFunc af;
  lt::dht::public_key otherpk;
  lt::aux::from_hex (otherkey, otherpk.bytes.data ());
  std::tuple<lt::dht::public_key, lt::dht::secret_key> ownkey;
  ownkey = lt::dht::ed25519_create_keypair (*(no->seed));
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
  no->ownipv6mtx.lock ();
  if (no->ownipv6 != "" && no->ownipv6port != 0)
    {
      putstring = no->ownipv6;
      strm.str ("");
      strm.clear ();
      strm.imbue (loc);
      strm << ntohs (no->ownipv6port);
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
  no->ownipv6mtx.unlock ();
  std::string result = lt::aux::to_hex (std::get<0> (newpk).bytes);

  return result;
}
void
DHTOperations::getvResult (std::string key, uint32_t ip, uint16_t port, int seq)
{
  if (seq > 0)
    {
      std::tuple<std::string, uint32_t, uint16_t, int> ttup;
      std::string keyloc = key;
      ttup = std::make_tuple (key, ip, port, seq);
      std::vector<char> temp;
      uint32_t iploc = ip;
      temp.resize (INET_ADDRSTRLEN);
      if (inet_ntop (AF_INET, &iploc, temp.data (), temp.size ()))
	{
	  std::cout << "ip4 " << key << " " << temp.data () << ":"
	      << ntohs (port) << " seq=" << seq << std::endl;
	  no->contmtx.lock ();
	  auto itc = std::find_if (no->contacts.begin (), no->contacts.end (),
				   [&keyloc]
				   (auto &el)
				     {
				       return std::get<1>(el) == keyloc;
				     });
	  if (itc != no->contacts.end ())
	    {
	      no->getfrresmtx.lock ();
	      auto it = std::find_if (no->getfrres.begin (),
				      no->getfrres.end (), [&keyloc]
				      (auto &el)
					{
					  return std::get<0>(el) == keyloc;
					});
	      if (it == no->getfrres.end ())
		{
		  if (ip != 0)
		    {
		      no->getfrres.push_back (ttup);
		      no->maintblockmtx.lock ();
		      time_t bltm = time (NULL);
		      auto itmnt = std::find_if (
			  no->maintblock.begin (), no->maintblock.end (),
			  [keyloc]
			  (auto &el)
			    {
			      return std::get<0>(el) == keyloc;
			    });
		      if (itmnt != no->maintblock.end ())
			{
			  std::get<1> (*itmnt) = bltm;
			}
		      else
			{
			  no->maintblock.push_back (
			      std::make_tuple (key, bltm));
			}
		      no->maintblockmtx.unlock ();
		    }
		}
	      else
		{
		  if (std::get<3> (*it) < seq && ip != 0)
		    {
		      *it = ttup;
		      no->maintblockmtx.lock ();
		      time_t bltm = time (NULL);
		      auto itmnt = std::find_if (
			  no->maintblock.begin (), no->maintblock.end (),
			  [keyloc]
			  (auto &el)
			    {
			      return std::get<0>(el) == keyloc;
			    });
		      if (itmnt != no->maintblock.end ())
			{
			  std::get<1> (*itmnt) = bltm;
			}
		      else
			{
			  no->maintblock.push_back (
			      std::make_tuple (key, bltm));
			}
		      no->maintblockmtx.unlock ();
		    }
		}
	      no->getfrresmtx.unlock ();
	    }
	  no->contmtx.unlock ();
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
}

void
DHTOperations::getvResult6 (std::string key, std::string ip, uint16_t port,
			    int seq)
{
  if (seq > 0)
    {
      std::tuple<std::string, std::string, uint16_t, int> ttup;
      std::string keyloc = key;
      ttup = std::make_tuple (key, ip, port, seq);
      std::cout << "ip6 " << key << " " << ip << " " << ntohs (port) << " seq="
	  << seq << std::endl;
      no->contmtx.lock ();
      auto itc = std::find_if (no->contacts.begin (), no->contacts.end (),
			       [&keyloc]
			       (auto &el)
				 {
				   return std::get<1>(el) == keyloc;
				 });
      if (itc != no->contacts.end ())
	{
	  no->ipv6contmtx.lock ();
	  auto it = std::find_if (no->ipv6cont.begin (), no->ipv6cont.end (),
				  [&keyloc]
				  (auto &el)
				    {
				      return std::get<0>(el) == keyloc;
				    });
	  if (it != no->ipv6cont.end ())
	    {
	      if (seq >= std::get<3> (*it))
		{
		  *it = ttup;
		}
	    }
	  else
	    {
	      no->ipv6cont.push_back (ttup);
	    }
	  no->ipv6contmtx.unlock ();
	}
      no->contmtx.unlock ();
    }
}
