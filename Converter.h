#pragma once

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <SDL3/SDL.h>
#include "IO.h"

bool JSON2CSM(SDL_Window& window, const std::string& inpath);