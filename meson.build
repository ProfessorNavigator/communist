project ('Communist', 'cpp', default_options : ['cpp_std=c++17'])
src = ['AuxFunc.cpp', 'CommunistApp.cpp', 'Communist.cpp', 'DHTOperations.cpp', 'LocalNetworkOp.cpp', 'NetworkOperations.cpp', 'MainWindow.cpp']
gtkmm = dependency ('gtkmm-4.0', method : 'pkg-config')
icu = dependency ('icu-i18n', method : 'pkg-config')
zip = dependency ('libzip', method : 'pkg-config')
gcrypt = dependency ('libgcrypt', method : 'pkg-config')
libtorrent = dependency ('libtorrent-rasterbar', method : 'pkg-config')
hunspell = dependency ('hunspell', method : 'pkg-config')
thread_dep = dependency ('threads')
subdir ('po')
if host_machine.system() == 'windows'
	add_global_arguments ('-U_FILE_OFFSET_BITS', language : 'cpp')
endif 
executable ('communist', sources : src, dependencies : [gtkmm, icu, zip, gcrypt, libtorrent, hunspell, thread_dep], install : true, gui_app : true)
install_subdir ('Communist', install_dir : 'share')
if host_machine.system() == 'linux'
	install_data ('ru.mail.bobilev_yury.Communist.desktop', install_dir : 'share/applications')
	install_data ('communist.png', install_dir : 'share/icons/hicolor/128x128/apps')
endif
if host_machine.system() == 'windows'
	install_data ('communist.ico', install_dir : 'share/Communist')
endif
