#ifndef PCH_H
#define PCH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL

void FatalError(const char* file, int line, const char* message);
void FatalError(const char* file, int line, const char* message, const char* context);

#define FATALERROR(m) FatalError( __FILE__, __LINE__, m )
#define ERRORMESSAGE(m,c) FatalError( __FILE__, __LINE__, m, c )

#endif //PCH_H
