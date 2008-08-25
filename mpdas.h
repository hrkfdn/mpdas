#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include <curl/curl.h>

// this is one ugly fix ..
#define new arg_new
#include <libmpd/libmpd.h>
#undef new
#include <curl/curl.h>
#include "md5.h"

#include "config.h"
#include "utils.h"
#include "mpd.h"
#include "cache.h"
#include "audioscrobbler.h"
