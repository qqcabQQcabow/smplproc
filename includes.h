#include <cctype>
#include <cstddef>
#include <endian.h>
#include <exception>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <signal.h>
#include <functional>
#include <istream>
#include <climits>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <cmath>
#include <ctime>
#include <cstring>
#include <chrono>


//graphics
#include <SFML/Main.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

#define ATR_COUNT 52
#define DEFAUL_DELAY 1
#define PINS 10
#define SEND_SIGNAL 11
#define SHOW_INFO 12
#define KILL 13
#define UTIME 16
#define STIME 17
#define CUTIME 18
#define CSTIME 19
#define STARTTIME 24
#define UID 0
#define PID 1
#define MEM 4
#define CPU 5
