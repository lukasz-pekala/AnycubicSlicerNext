#pragma once

#include <stdint.h>
#include <string>
#ifdef _WIN32

std::string getPEFileVersion(const char *filename);

#endif