#pragma once
#include <string>

// application name
extern std::string argv0;

// commands
int help (int argc, const char *argv[]);
int version (int argc, const char *argv[]);
int binding (int argc, const char *argv[]);
