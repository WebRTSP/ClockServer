[![webrtsp-clock](https://snapcraft.io/webrtsp-clock/badge.svg)](https://snapcraft.io/webrtsp-clock)

### How to build it from sources and run (Ubuntu 20.10)

1. Install required packages:
```
sudo apt install build-essential git cmake \
    libspdlog-dev libconfig-dev libssl-dev \
    libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-nice
```

2. Build and install libwebsockets (at least v4.1 is required):
```
git clone https://github.com/warmcat/libwebsockets.git --branch v4.1-stable --depth 1
mkdir -p libwebsockets-build
cd libwebsockets-build
cmake -DLWS_WITH_GLIB=ON -DCMAKE_INSTALL_PREFIX:PATH=/usr ../libwebsockets
make -j4
sudo make install
cd -
```

3. Build application:
```
git clone https://github.com/WebRTSP/ClockServer.git --recursive
mkdir -p ClockServer-build
cd ClockServer-build
cmake ../ClockServer
make -j4
cd -
```

4. Run application: `cd ClockServer && ../ClockServer-build/ClockServer`
5. Open in browser `http://localhost:5080/`

### How to build it from sources and run on Raspberry Pi OS 12 (Bookworm)

1. Install required packages:
```
sudo apt install build-essential git cmake \
    libspdlog-dev libconfig-dev libssl-dev \
    libwebsockets-dev libmicrohttpd-dev \
    libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev libnice-dev \
    gstreamer1.0-plugins-{base,good,bad,ugly} gstreamer1.0-nice
```

2. Build application:
```
git clone https://github.com/WebRTSP/ClockServer.git --recursive
mkdir -p ClockServer-build
cd ClockServer-build
cmake ../ClockServer
make -j4
cd -
```

3. Run application: `cd ClockServer && ../ClockServer-build/ClockServer`
4. Open in browser `http://localhost:5080/`
