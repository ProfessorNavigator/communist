project ('Communist', 'cpp', default_options : ['cpp_std=c++17'])
subdir ('src')
src = ['main.cpp', srcfile]
gtkmm = dependency ('gtkmm-4.0', method : 'pkg-config')
icu = dependency ('icu-i18n', method : 'pkg-config')
libcommunist = dependency ('libcommunist', method : 'pkg-config')
hunspell = dependency ('hunspell', method : 'pkg-config')
thread_dep = dependency ('threads')
subdir ('po')
incdir = include_directories ('include')
executable ('communist', sources : src, include_directories : incdir, dependencies : [gtkmm, icu, hunspell, libcommunist, thread_dep], install : true, gui_app : true)
install_subdir ('Communist', install_dir : 'share')
if host_machine.system() == 'linux'
	install_data ('ru.mail.bobilev_yury.Communist.desktop', install_dir : 'share/applications')
	install_data ('communist.png', install_dir : 'share/icons/hicolor/128x128/apps')
endif
if host_machine.system() == 'windows'
	install_data ('communist.ico', install_dir : 'share/Communist')
endif
