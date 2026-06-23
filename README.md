# Snake Online
A multiplayer snake game written in C using SDL2.

## Dependencies
- C Compiler
- git
- CMake 3.20 or newer
- SDL2 library
- SDL2_ttf library
- SDL2_image library

## How to build and run

### Linux
#### 1. Install Dependencies
On Debian/Ubuntu based distributions, use the following command:
```bash
sudo apt update
sudo apt install build-essential git cmake libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```

#### 2. Clone Repository
```bash
git clone https://github.com/Thermirino/snake-online.git
cd snake-online
```

#### 3. Configure and Build
```bash
cmake --preset unix-release
cmake --build --preset unix-release
```

#### 4. Run server
```bash
./build/release/bin/server 12345
```

#### 5. Run client (in another terminal)
```bash
./build/release/bin/client localhost 12345 Player1
```

### Windows
Not supported
