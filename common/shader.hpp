#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <fstream>
#include <sstream>

unsigned int LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
