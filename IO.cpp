#pragma once

#include "IO.h"

void FileDialogCallback(void* userdata, const char* const* filelist, int filterIndex)
{
	if (!filelist || !*filelist) {
		SDL_Log("File dialog cancelled or error: %s", SDL_GetError());
		return;
	}

	SDL_Window* window = static_cast<SDL_Window*>(userdata);

	SDL_Log("Selected file: %s", *filelist);

	JSON2CSM(*window, *filelist);
}

void jsonOpenFile(SDL_Window* window)
{
	SDL_DialogFileFilter filters[] = {
		{ "JSON files", "json" },
		{ "All files", "*" }
	};

	SDL_ShowOpenFileDialog(
		FileDialogCallback,
		window,   // pass SDL_Window* as userdata
		window,
		filters,
		2,
		nullptr,
		false
	);
}

void FileSaveDialogCallback(void* userdata, const char* const* filelist, int filterIndex)
{
	if (!filelist || !*filelist) {
		SDL_Log("Save dialog cancelled or error: %s", SDL_GetError());
		return;
	}

	SDL_Log("Save file path: %s", *filelist);

	auto* data = static_cast<std::pair<SDL_Window*, std::stringstream*>*>(userdata);

	std::ofstream ofile(*filelist);
	ofile << data->second->str();
	ofile.close();

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success", "Model successfully converted!", data->first);

	delete data->second;
	delete data;
}

void csmSaveFile(SDL_Window* window, std::stringstream* fileData)
{
	SDL_DialogFileFilter filters[] = {
		{ "CSM files", "csm" },
		{ "All files", "*" }
	};

	auto* data = new std::pair<SDL_Window*, std::stringstream*>(window, fileData);

	SDL_ShowSaveFileDialog(
		FileSaveDialogCallback,
		data,     // user data!
		window,
		filters,
		2,
		nullptr
	);
}