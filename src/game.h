#pragma once

#ifdef PLATFORM_ANDROID
    #define ASSET_PATH(path) path
#else
    #define ASSET_PATH(path) "assets/" path
#endif


void init_game();