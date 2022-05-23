# Communist

Communist is a simple p2p messenger.

## Description

Communist is a simple GTK 4 based p2p messenger. It uses torrent's DHT combined with UDP hole punch technique to establish direct connection between users. Program can work in two modes: local network and internet, both IPv4 and IPv6 protocols are supported. At the moment Communist has two possibilities: you can send simple text messages and files. All your messages and files are stored encrypted and being sent encrypted (AES encrypting algorithm combined with ed25519, see source code for more details) .

## Installation

### Linux

`git clone https://github.com/ProfessorNavigator/communist.git`\
`cd communist`\
`meson -Dbuildtype=release build`\
`cd build`\
`ninja install`

You may need superuser privileges to execute last command.

### Windows

You can build Communist from source by MSYS2 project [https://www.msys2.org/](https://www.msys2.org/). Follow installation instructions from their site. Install dependencies from [Dependencies](#dependencies) section and git (all must be mingw64 packages). Than create folder where you want to download source code (path must not include spaces or non Latin letters). Open mingw64 console and execute following commands (in example we download code to C:\Communist)

`cd /c/Communist`\
`git clone https://github.com/ProfessorNavigator/communist.git`\
`cd communist`\
`meson -Dbuildtype=release build`\
`cd build`\
`ninja install`

If everything was correct, you can find communist.exe file in `your_msys2_folder/mingw64/bin`.

## Dependencies

Communist uses meson building system, so to build it from source you need meson and ninja (and git of course).\
Also you need [gtkmm-4.0](http://www.gtkmm.org/), [libzip](https://libzip.org/), [icu](https://icu.unicode.org/) (version >= 69), [libgcrypt](https://www.gnupg.org/software/libgcrypt/index.html), [libtorrent-rasterbar](https://www.libtorrent.org/), [hunspell](https://github.com/hunspell/hunspell). All libraries must have headers (for building), so if you use for example Debian Linux, you need ...-devel versions of packages.

You may also need to install Good plugins for gstreamer. They are used to play default sound signals.

## Usage

- On startup program will offer you to create  user.\
		![](rdmimg/create_usr.png)\
		User name and password can be either you want, but author advises you to make them as reliable as possible - program uses user name and password to encrypt your local data.
- Next step - creation of your profile.\
		![](rdmimg/profile.png)\
If you want to add avatar click `Open image` button (supported formats depend on your version of GDK Pixbuf and it's settings)
- Now everything is almost ready. Last thing - add friends. Press `Add contact` button and input friends' key. Of course you need to send each other your public keys first (by email for example).\
		![](rdmimg/mainwin.png)
		
Your public key can be found in `Instruments->Own key`.

If program cannot automatically determine your net interface on startup, you see window with proposition to choose proper interface. Such thing can happen if your machine is connected to two or more networks.

![](rdmimg/chipwin.png)

In such case choose proper net interface and press `Choose` button.

## Settings

### Setting's window

Program have several settings, which can be found in `Instruments->Settings`.

![](rdmimg/settings.png)

You do not need change anything if everything works fine 'out of the box'. In other case...

 Spell checking is carried out by libhuspell. You can disable/enable it completely or change language. Only two languages are available now: English and Russian. It is 'hard coded', so if you need more variants - contact author (by email indicated below or by any platform you downloaded source code from, speaking languages are the same: English and Russian). Global language of the program can be changed by `LANG` variable. Available languages are the same, default language is English, Russian can be enabled by setting `LANG=ru_RU` (it will happen automatically if your system uses proper locale). If you want assist to translate the program, file `Communist.pot` is available in repository (program uses GNU gettext). Contact author by email to receive further instructions.
 
 `Network mode` has two variants: Internet and Local. In `Internet` mode program will use DHT ([libtorrent-rasterbar](https://www.libtorrent.org/) implementation now) to find friends' ip and establish direct connection. In `Local` mode DHT is disabled, program will send multicast UDP datagrams trying to find contacts. Next two lines indicate group addresses and ports to be used (be careful, they must be the same on all machines you want establish connection with). 
 
 Message's auto remove is introduced  to save memory - all messages you sent and received are stored on your hard drive. So you will need to clean them time to time (also you can remove messages manually one by one - there is proper item in each message context menu).
 
 `DHT listen interfaces` - libtorrent setting, indicates ip addresses and ports DHT sockets to be bound to. Zero values mean "default". If your machine has "white" ip address, consider changing this setting and publishing it anywhere to help other users on bootstrap to DHT network.
 
 `DHT bootstrap nodes` setting indicates which nodes Communist will try bootstrap to on very first connection to network. Later, if first bootstrap operation was successful, it uses information from cached file (~/.cache/Communist/dhtstate on Linux and C:\Users\\'user'\\.cache\Communist\dhtstate on Windows).
 
 Incoming message's size limit is included to prevent possible attacks. You can change it to any value you suppose is safe.
 
 `Sending part size...` Your message will be sent part to part with hash checking of each part on receiving. This parameter indicates size of the part. It is not recommended to make it too small - technical traffic will increase. At the same time it is not recommended to make this parameter too big - in case of any errors program will try to resend part, so it will increase traffic consumption too.
 
 `Contact connection...` parameter shows after how many seconds contact state will be considered as 'offline'. In this case attempts to connect will be stopped until program receives new ip address from DHT.
 
 `Connection broken...` parameter shows after how many seconds contact will be considered as disconnected (after that label 'online' on contacts' button will disappear).
 
 `Send message by`. Now only two variants are available: Ctrl + Enter and Enter.
 
 `Enable message receive sound`. Enable sound notification on message receiving.
 
 `Message sound path`. If this line is empty, program tries to use default signal. To use your own signal, input full path.
 
 `Enable STUN server` enables local STUN server (not compliant with RFCs, only for Communist network). Please, check it, if you have direct access to the Internet.
 
 `STUN port` - port to be used with Communist STUN servers and requests. Must be the same on all machines in the same overlay network.
 
 `Direct connection to Internet` - check it, if you have it. This option disables requests to STUN servers.
 
 Warning! After you press 'Apply' button program will be closed. You need to restart it manually.
 
### STUN list
 
 Program uses STUN servers to determine own 'external' IPv4 address. Some server's addresses are already placed to ../share/Communist/StunList file, but you may need to change them. Program will work even with one STUN server available, but optimal quantity is three (3). Each server's address must be placed to new line and must include ip  address and port or 'string' address and port (see default StunList file as example). If you place addresses in 'string' form, be aware that program needs to determine their ips  by DNS request first,  this operation may take some time on startup.

### View
 
 There is  ../share/Communist/mainWindow.css file. You can customize program's view by changing css properties inside it (see source code and [https://docs.gtk.org/gtk4/css-properties.html]() for more details).

## Plans
 
 Author wants to add more message's types, such as voice messages, video messages, direct p2p audio and video calling. But unfortunately he is not qualified on audio and video recording, so it may take some time. Also he wants to add group and channel functionality, but unfortunately due to IPv4 features (NAT) this can be realised only for IPv6 networks (if you have any ideas, please contact author by e-mail or on GitFlic and Github).
 

## License

GPLv3+ (see `COPYING`).

## Donation

If you want to help on developing this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573) or by code development (contact author by email to obtain access to repositories)

## Contacts

You can contact author by email \
bobilev_yury@mail.ru