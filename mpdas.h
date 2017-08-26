#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>

#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>

#include <curl/curl.h>

#include <mpd/client.h>
#include <curl/curl.h>
#include "md5.h"

#include "ini.h"

#include "config.h"
#include "utils.h"
#include "mpd.h"
#include "cache.h"
#include "audioscrobbler.h"
