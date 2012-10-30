#include "glesplash.h"
#include "shaders.h"

void print_shader_info_log(GLuint shader)
{
   GLint length;

   glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

   if(length) {
      char* buffer = new char[length];
      glGetShaderInfoLog(shader, length, NULL, buffer);
      cout << "shader info: " <<  buffer << flush;
      delete[] buffer;

      GLint success;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (success != GL_TRUE)   exit(1);
   }
}

GLuint load_shader(const char *shader_source, GLenum type)
{
   GLuint shader = glCreateShader(type);

   glShaderSource(shader, 1, &shader_source, NULL);
   glCompileShader(shader);

   print_shader_info_log(shader);

   return shader;
}
