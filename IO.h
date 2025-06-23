#pragma once
#include <SDL3/SDL.h>
#include <string>

// The actual converter code
#include "Converter.h"

void FileDialogCallback(void* userdata, const char* const* filelist, int filterIndex);

void jsonOpenFile(SDL_Window* window);

void FileSaveDialogCallback(void* userdata, const char* const* filelist, int filterIndex);

void csmSaveFile(SDL_Window* window, std::stringstream* fileData);