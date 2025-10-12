#ifndef __glad_h_
#define __glad_h_

#include <stddef.h>

#ifdef __gl_h_
#error OpenGL header already included, remove this include, glad already provides it
#endif

#define __gl_h_

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#define APIENTRY __stdcall
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GLAPI
#define GLAPI extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FLOAT 0x1406
#define GL_TRIANGLES 0x0004
#define GL_UNSIGNED_INT 0x1405
#define GL_VERSION 0x1F02
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303

GLAPI const GLubyte* glGetString(GLenum name);
GLAPI void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLAPI void glClear(GLbitfield mask);
GLAPI void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
GLAPI void glDrawArrays(GLenum mode, GLint first, GLsizei count);
GLAPI void glEnable(GLenum cap);
GLAPI void glBlendFunc(GLenum sfactor, GLenum dfactor);

typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

extern PFNGLCREATESHADERPROC glad_glCreateShader;
#define glCreateShader glad_glCreateShader
extern PFNGLSHADERSOURCEPROC glad_glShaderSource;
#define glShaderSource glad_glShaderSource
extern PFNGLCOMPILESHADERPROC glad_glCompileShader;
#define glCompileShader glad_glCompileShader
extern PFNGLGETSHADERIVPROC glad_glGetShaderiv;
#define glGetShaderiv glad_glGetShaderiv
extern PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
#define glGetShaderInfoLog glad_glGetShaderInfoLog
extern PFNGLDELETESHADERPROC glad_glDeleteShader;
#define glDeleteShader glad_glDeleteShader
extern PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
#define glCreateProgram glad_glCreateProgram
extern PFNGLATTACHSHADERPROC glad_glAttachShader;
#define glAttachShader glad_glAttachShader
extern PFNGLLINKPROGRAMPROC glad_glLinkProgram;
#define glLinkProgram glad_glLinkProgram
extern PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
#define glGetProgramiv glad_glGetProgramiv
extern PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
#define glGetProgramInfoLog glad_glGetProgramInfoLog
extern PFNGLUSEPROGRAMPROC glad_glUseProgram;
#define glUseProgram glad_glUseProgram
extern PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;
#define glDeleteProgram glad_glDeleteProgram
extern PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
#define glGenVertexArrays glad_glGenVertexArrays
extern PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
#define glBindVertexArray glad_glBindVertexArray
extern PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
#define glDeleteVertexArrays glad_glDeleteVertexArrays
extern PFNGLGENBUFFERSPROC glad_glGenBuffers;
#define glGenBuffers glad_glGenBuffers
extern PFNGLBINDBUFFERPROC glad_glBindBuffer;
#define glBindBuffer glad_glBindBuffer
extern PFNGLBUFFERDATAPROC glad_glBufferData;
#define glBufferData glad_glBufferData
extern PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
#define glBufferSubData glad_glBufferSubData
extern PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
#define glDeleteBuffers glad_glDeleteBuffers
extern PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
#define glVertexAttribPointer glad_glVertexAttribPointer
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
extern PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
#define glGetUniformLocation glad_glGetUniformLocation
extern PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;
#define glUniformMatrix4fv glad_glUniformMatrix4fv

typedef void* (*GLADloadproc)(const char *name);
int gladLoadGLLoader(GLADloadproc);

#ifdef __cplusplus
}
#endif

#endif
