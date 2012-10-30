#ifndef _SHADERS_H
#define _SHADERS_H

const char vertex_src[] =
"                                               \
   attribute vec4        aPosition;             \
   attribute vec2        aTexCoord;             \
   varying mediump vec2  tCoord;                \
                                                \
   void main()                                  \
   {                                            \
      gl_Position = aPosition;                  \
      tCoord = aTexCoord;                       \
   }                                            \
";

const char fragment_src[] =
"                                               \
  varying mediump vec2 tCoord;                  \
  uniform sampler2D tex;                        \
                                                \
  void  main()                                  \
  {                                             \
      gl_FragColor = texture2D(tex, tCoord);    \
  }                                             \
";

void print_shader_info_log(GLuint shader);
GLuint load_shader(const char *shader_source, GLenum type);
#endif
