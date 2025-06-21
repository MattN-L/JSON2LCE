#pragma once

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <nlohmann/json.hpp>

void JSON2CSM(HWND& hWnd, const std::wstring& inpath, const std::wstring& outpath);

void JSON2PSM(const std::wstring& inpath, const std::wstring& outpath);