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
#ifndef NETWORKOPERATIONS_H_
#define NETWORKOPERATIONS_H_

#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <cstring>
#include <thread>
#include <mutex>
#include <tuple>
#include <iostream>
#include <libtorrent/hex.hpp>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include "AuxFunc.h"
#include "DHTOperations.h"
#include "LocalNetworkOp.h"

#ifdef __linux
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif

#ifdef _WIN32
#include <Winsock2.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#endif

class DHTOperations;
class LocalNetworkOp;

class NetworkOperations
{
  friend class DHTOperations;
  friend class LocalNetworkOp;
public:
  NetworkOperations (
      std::string username, std::string password,
      std::vector<std::tuple<int, std::string>> *Contacts,
      std::array<char, 32> *Seed, std::vector<std::string> *addfriends,
      std::vector<std::tuple<std::string, std::string>> *Prefvect,
      std::string sharepath);
  virtual
  ~NetworkOperations ();
  sigc::signal<void
  ()> canceled;
  sigc::signal<void
  (std::string, std::filesystem::path)> messageReceived;
  sigc::signal<void
  (std::string, int)> profReceived;
  sigc::signal<void
  (std::string, int)> msgSent;
  sigc::signal<void
  (std::string, uint64_t, uint64_t, std::string)> filerequest;
  sigc::signal<void
  (std::string)> fileRejected;
  sigc::signal<void
  (std::string, std::string)> filehasherr;
  sigc::signal<void
  (std::string, std::string)> filercvd;
  sigc::signal<void
  (std::string, std::string)> filesentsig;
  sigc::signal<void
  (std::string, std::string)> filesenterror;
  sigc::signal<void
  (std::string)> ipv6signal;
  sigc::signal<void
  (std::string)> ipv4signal;
  sigc::signal<void
  ()> ipv6signalfinished;
  sigc::signal<void
  ()> ipv4signalfinished;
  sigc::signal<void
  (std::string, std::filesystem::path, uint64_t)> filepartrcvdsig; //key, path to file, file current size
  sigc::signal<void
  (std::string, std::filesystem::path, uint64_t)> filepartsendsig; //key, path to file, sent byte quantity
  sigc::signal<void
  (std::string, uint64_t)> smthrcvdsig;
  sigc::signal<void
  (std::string)> friendDeleted;
  void
  mainFunc ();
  void
  getNewFriends (std::string key);
  void
  removeFriend (std::string key);
  void
  checkMsg (std::filesystem::path p, int *check);
  int
  createMsg (std::string key, std::filesystem::path p, int type);
  void
  renewProfile (std::string key);
  void
  fileReject (std::string key, time_t tm);
  void
  fileAccept (std::string key, time_t tm, std::filesystem::path sp);
  void
  startFriend (std::string key, int ind);
  void
  blockFriend (std::string key);
  void
  setIPv6 (std::string ip);
  void
  setIPv4 (std::string ip);
  std::filesystem::path
  removeMsg (std::string key, std::filesystem::path msgpath);
  void
  cancelAll ();
  void
  cancelSendF (std::string key, std::filesystem::path filepath);
  void
  cancelReceivF (std::string key, std::filesystem::path filepath);
private:
  sigc::signal<void
  ()> dnsfinished;
  void
  dnsFunc ();
  void
  putOwnIps (std::string otherkey, uint32_t ip, uint16_t port);
  std::pair<uint32_t, uint16_t>
  getOwnIps (int udpsock, std::pair<struct in_addr, int> stunsv);
  void
  holePunch (int sock, uint32_t ip, std::string otherkey);
  int
  receiveMsg (int sockipv4, sockaddr_in *from);
  void
  receivePoll ();
  int
  sendMsg (int sockipv4, uint32_t ip, uint16_t port, std::vector<char> &msg);
  int
  sendMsg6 (int sock, std::string ip6, uint16_t port, std::vector<char> &msg);
  int
  sendMsgGlob (int sock, std::string keytos, uint32_t ip, uint16_t port);
  void
  commOps ();
  int
  msgMe (std::string key, uint64_t tm, uint64_t partnum);
  int
  msgME (std::string key, uint64_t tm);
  int
  msgPe (std::string key, uint64_t tm, uint64_t partnum);
  int
  msgPE (std::string key, uint64_t tm);
  void
  stunSrv ();
#ifdef _WIN32
  int
  poll (struct pollfd *pfd, int nfds, int timeout);
#endif
  std::vector<std::tuple<int, std::string>> contacts;
  std::mutex contmtx;
  std::vector<std::tuple<int, std::string>> contactsfull;
  std::mutex contfullmtx;
  int contsizech = 0;
  std::vector<std::string> Addfriends;
  std::mutex addfrmtx;
  std::vector<std::tuple<std::string, time_t>> maintblock;
  std::mutex maintblockmtx;
  std::vector<std::tuple<std::string, int, std::mutex*, time_t, std::mutex*>> sockets4;
  std::mutex sockmtx;
  std::vector<std::string> getfr;
  std::mutex getfrmtx;
  std::vector<std::tuple<std::string, uint32_t, uint16_t, int>> getfrres;
  std::mutex getfrresmtx;
  std::vector<std::tuple<std::string, uint32_t, uint16_t>> putipv;
  std::mutex putipmtx;
  std::vector<std::tuple<std::string, std::string, uint16_t, int>> ipv6cont; //key, ip, port, sequens
  std::mutex ipv6contmtx;
  std::vector<std::tuple<std::string, time_t>> ipv6lr;
  std::mutex ipv6lrmtx;
  std::vector<std::tuple<std::string, uint64_t, uint64_t, std::vector<char>>> msghash; //0-key, 1-time, 2-msg size, 3-hash
  std::mutex msghashmtx;
  std::vector<std::tuple<std::string, uint64_t, std::vector<char>>> msgparthash; //0-key, 1-time, 2-hash
  std::mutex msgparthashmtx;
  std::vector<std::tuple<std::string, uint64_t, uint64_t, std::vector<char>>> msgpartrcv; //0-key, 1-time, 2-part numb, 3-part
  std::mutex msgpartrcvmtx;
  std::vector<std::tuple<std::string, uint64_t, uint64_t>> msgrcvdpnum; //0-key, 1-time, 2-partnum
  std::mutex msgrcvdpnummtx;
  std::vector<
      std::tuple<std::string, uint64_t, int, std::filesystem::path, int,
	  uint64_t, std::vector<char>>> msgpartbuf; //0-key, 1-time, 2-receive status, 3-path to msg, 4-byte sent, 5-partnum, 6-part
  std::mutex msgpartbufmtx;
  std::vector<
      std::tuple<std::string, std::filesystem::path, time_t, int,
	  std::filesystem::path>> filesendreq; //0-key, 1-file to send path, 2-time of first datagram, 3-accept status, 4-msg path
  std::mutex filesendreqmtx;
  std::vector<std::tuple<std::string, time_t, std::string>> fqrcvd;
  std::mutex fqrcvdmtx;
  std::vector<std::pair<struct in_addr, uint16_t>> stunips;
  std::mutex stunipsmtx;
  std::vector<
      std::tuple<std::string, time_t, std::filesystem::path, int, uint64_t,
	  std::vector<char>, int>> filepartbuf; //0-key, 1-time from FQ message, 2-path to source file, 3-byte quantity sent, 4-part number, 5 - filepart, 6 - status
  std::mutex filepartbufmtx;
  std::vector<
      std::tuple<std::string, time_t, std::vector<char>, std::filesystem::path,
	  int>> filehashvect; //0-key, 1-time FB, 2-file hash, 3-path to file, 4-last rcvd part number
  std::mutex filehashvectmtx;
  std::vector<std::tuple<std::string, time_t, uint64_t, std::vector<char>>> fileparthash; //0-key, 1-time FB, 2-file part number, 3-part hash
  std::mutex fileparthashmtx;
  std::vector<std::tuple<std::string, uint64_t, uint64_t, std::vector<char>>> filepartrcv; //0-key, 1-time, 2-part number, 3-part
  std::mutex filepartrcvmtx;
  std::vector<std::tuple<std::string, uint64_t, int>> filepartrlog; //0-key, 1-time, 2-last part recieved
  std::mutex filepartrlogmtx;
  std::vector<std::tuple<std::string, uint64_t, std::vector<char>>> currentpart; //0-key, 1-time, 2-part
  std::mutex currentpartmtx;
  std::vector<std::tuple<std::string, time_t>> fbrvect; //"FB" received vector
  std::mutex fbrvectmtx;
  std::vector<std::tuple<std::string, uint64_t>> filepartend; //0-key, 1-time
  std::mutex filepartendmtx;
  std::vector<std::tuple<std::string, uint64_t>> fileend; //0-key, 1-time
  std::mutex fileendmtx;
  std::vector<std::tuple<std::string, time_t>> filecanceled; //0-key, 1-time from FQ msg
  std::mutex filecanceledmtx;
  std::vector<std::tuple<std::string, int>> holepunchstop; //0-key, 1-port to start with
  std::mutex holepunchstopmtx;
  std::string Username;
  std::string Password;
  int cancel = 0;
  std::mutex sendbufmtx;
  std::mutex rcvmtx;
  std::array<char, 32> *seed = nullptr;
  std::vector<std::tuple<std::string, std::string>> prefvect;

  DHTOperations *DOp = nullptr;
  LocalNetworkOp *LNOp = nullptr;
  int sockipv6;
  std::mutex sockipv6mtx;
  int ownnattype = 0;
  time_t contreqtmr = 0;
  std::string ownipv6 = "";
  uint16_t ownipv6port = 0;
  std::mutex ownipv6mtx;
  std::mutex copsrun;
  std::mutex copsrunmtx;
  std::string Netmode = "0";
  int Maxmsgsz = 1024 * 1024;
  int Partsize = 457 * 3;
  std::string IPV4 = "";
  std::mutex IPV4mtx;
  std::string Sharepath;
  time_t Shuttmt = 600;
  time_t Tmttear = 20;
  uint16_t stunport = 3478;
  std::string Enablestun = "notactive";
  std::string Directinet = "notdirect";
  std::vector<std::tuple<std::mutex*, std::string>> threadvect;
  std::mutex threadvectmtx;
};

#endif /* NETWORKOPERATIONS_H_ */
