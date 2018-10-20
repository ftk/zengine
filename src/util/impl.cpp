#define STB_IMAGE_IMPLEMENTATION
//#define STBI_ONLY_PNG // compile out most formats
#include "stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#define STB_VORBIS_NO_PUSHDATA_API
//#define STB_VORBIS_NO_PULLDATA_API
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_MAX_CHANNELS 2
#include "stb/stb_vorbis.c"

