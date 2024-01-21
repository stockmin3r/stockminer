#!/bin/bash
sudo apt-get -y install libtool gcc make cmake npm python3-pip zlib1g-dev python3-pbr libssl-dev libcurl4-openssl-dev libcurl4 libcurl3-gnutls libcurl4-openssl-dev libcap2-bin texinfo python3-venv wkhtmltopdf libsodium libsodium-dev
sudo npm  install -g html-minifier terser
sudo pip3 install pandas pandas-datareader parsedatetime pbr yahoo-earnings-calendar yfinance urllib3 certbot xlsxwriter xlwt scipy holidays datedelta ta
sudo pip3 install --upgrade ta

#sysctl net.ipv4.ip_unprivileged_port_start=80
# RHEL/Fedora/CentOS
#sudo yum install libcap
#setcap 'cap_net_bind_service=+ep' stockminer

# windows compile
#sudo apt-get install gcc-multilib
#sudo ln -s /usr/bin/x86_64-w64-mingw32-windres /usr/bin/windres

# electrum
#sudo apt install python3-pip python3-setuptools python3-pyqt5 libsecp256k1-dev

# utf8icons fix
#sudo apt install fonts-noto

# chrome driver, selenium
#pip3 install selenium
#pip3 install webdriver-manager
#apt-get install chromium-chromedriver

# yarn pakage builde
# apt install cmdtest git-extras

# brew
# /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
# echo '# Set PATH, MANPATH, etc., for Homebrew.' >> /home/$USER/.profile
# echo 'eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"' >> /home/$USER/.profile
# eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"

# pip3 install h2o-wave

# https://apt.llvm.org/
# emscripten
# wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
# apt install emscripten wabt llvm lld
# git clone https://github.com/emscripten-core/emsdk.git
# cd emsdk && ./emsdk install latest
# cd emsdk && ./emsdk activate latest
# echo 'source "/usr/src/wasm/emsdk/emsdk_env.sh"' >> $HOME/.bash_profile


# npm install ts-node

# apt install jupyter-core
# apt-get install gnuplot
# apt-get install libeigen3-dev
# apt-get install liblzma-dev libbz2-dev
# apt-get install qemu-kvm virt-manager libvirt-daemon-system virtinst libvirt-clients bridge-utils


# fedora-dev pkgs
# fedora yum install npm git python3-pip texinfo automake help2man
# dnf install python3-virtualenv
# dnf -y install dnf-plugins-core
# dnf config-manager --add-repo https://download.docker.com/linux/fedora/docker-ce.repo
# dnf install docker-ce docker-ce-cli containerd.io docker-compose-plugin
# npm install browserify JSONStream


# osx

brew install git automake autoconf libtool libtoolize glibtool glibtoolize
ln -s /usr/local/bin/glibtoolize /usr/local/bin/libtoolize


# freebsd
pkg install automake libtool node16 npm py39-pip editors/vim converters/base64 science/py-scipy

# discord bot
npm install request cheerio dotenv discord.js moment-timezone
