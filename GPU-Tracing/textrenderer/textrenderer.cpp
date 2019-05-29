#include"textrenderer.h"
#include"glmacros.h"
#include<iostream>
#include<algorithm>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H 

GLuint qp::TextRenderer::program=0;
GLuint qp::TextRenderer::vao=0;
GLuint qp::TextRenderer::vbo=0;

const char* qp::TextRenderer::vertex_shader_source = "#version 430 core\n\
layout(location = 0) in vec4 vertex;\n\
out vec2 TexCoords; \n\
void main()\
{\
    gl_Position = vec4(vertex.xy, 0.0, 1.0);\
    TexCoords = vertex.zw;\
}";
const char* qp::TextRenderer::fragment_shader_source = "#version 430 core\n\
in vec2 TexCoords;\
out vec4 color;\
uniform sampler2D text;\
uniform vec3 textColor;\
void main()\
{\
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\
    color = vec4(textColor, 1.0) * sampled;\
}";

bool qp::TextRenderer::onFirstCall() {
	
    program = qp::createProgram_VF(vertex_shader_source, fragment_shader_source);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
	
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
    glBufferData(GL_ARRAY_BUFFER, sizeof(FontFaceVertex)*6*max_char_number, NULL, GL_STREAM_DRAW);
	
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FontFaceVertex), NULL);
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	
    glBindVertexArray(0);
    
    return true;
}

qp::TextRenderer::TextRenderer(const std::string fontname) {
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, "c.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    FT_Set_Pixel_Sizes(face, 0, 48);

    unsigned int maxHeight = 0;
    unsigned int maxWidth = 0;
    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        maxWidth = std::max(face->glyph->bitmap.width, maxWidth);
        maxHeight = std::max(face->glyph->bitmap.rows, maxHeight);
    }

    _text_height_ = maxHeight;

    maxWidth += 4;
    maxHeight += 4;

    glm::ivec2 texSize = glm::ivec2(maxWidth * 16, maxHeight * 8);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	glActiveTexture( GL_TEXTURE5 );
    glGenTextures(1, &_texture_);
    glBindTexture(GL_TEXTURE_2D, _texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texSize.x, texSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (GLubyte c = 0; c < 128; c++)
    {
   
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        GLuint row = c / 16;
        GLuint collum = c % 16;
        glm::ivec2 upperLeft = glm::ivec2(collum*maxWidth + 2, row*maxHeight + 2);
        glm::ivec2 characterSize = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);

        glTexSubImage2D(GL_TEXTURE_2D, 0, upperLeft.x, upperLeft.y, characterSize.x, characterSize.y, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glm::vec2 texLowerLeft = glm::vec2(upperLeft + glm::ivec2(0, characterSize.y))/glm::vec2(texSize);
        glm::vec2 texUpperRight = glm::vec2(upperLeft + glm::ivec2(characterSize.x, 0)) / glm::vec2(texSize);

        glm::ivec2 vertexLowerLeft = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top- face->glyph->bitmap.rows);
        glm::ivec2 vertexUpperRight = vertexLowerLeft + characterSize;
        // Now store character for later use
        Character character = {
            texLowerLeft,
            texUpperRight,
            vertexLowerLeft,
            vertexUpperRight,
            face->glyph->advance.x
        };
        characters.push_back(character);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

	glActiveTexture( GL_TEXTURE0 );
}

qp::TextRenderer::~TextRenderer() {
}

std::vector<qp::TextRenderer::FontFaceVertex> qp::TextRenderer::vertices(100);

void qp::TextRenderer::render(const std::string text, int x, int y, int height, const glm::vec3 color) {
	
    static bool firstCall = onFirstCall();
	
    static GLuint textColorLocation = glGetUniformLocation(program, "textColor");
    static GLuint textTextureLocation = glGetUniformLocation(program, "text");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(program);
    glUniform3fv(textColorLocation, 1, &color[0]);
    glUniform1i(textTextureLocation, 5);
    glBindVertexArray(vao);

    GLint windowSize[4];
    glGetIntegerv(GL_VIEWPORT, windowSize);

    glm::vec2 pixel2pos = glm::vec2(2.0f / static_cast<float>(windowSize[2]), 2.0 / static_cast<float>(windowSize[3]));

    // pos of orig of font
    glm::vec2 pos = glm::vec2(x, -y)*pixel2pos + glm::vec2(-1, 1);

    float scale = static_cast<float>(height) / static_cast<float>(_text_height_);

    vertices.clear();

    for (unsigned char c : text)
    {
        const Character& ch = characters[c];

        glm::vec2 p1 = glm::vec2(ch.vertexBound1)*pixel2pos*scale + pos;
        glm::vec2 p2 = glm::vec2(ch.vertexBound2)*pixel2pos*scale + pos;
        glm::vec2 p3 = glm::vec2(p1.x, p2.y);
        glm::vec2 p4 = glm::vec2(p2.x, p1.y);

        glm::vec2 t1 = ch.texBound1;
        glm::vec2 t2 = ch.texBound2;
        glm::vec2 t3 = glm::vec2(t1.x, t2.y);
        glm::vec2 t4 = glm::vec2(t2.x, t1.y);

        vertices.push_back({ p1,   t1 });
        vertices.push_back({ p2,   t2 });
        vertices.push_back({ p3,   t3 });
        vertices.push_back({ p1,   t1 });
        vertices.push_back({ p4,   t4 });
        vertices.push_back({ p2,   t2 });


        pos.x += (ch.advance >> 6) * pixel2pos.x * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, _texture_);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FontFaceVertex)*vertices.size(), &vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_BLEND);
	glEnable( GL_DEPTH_TEST );
	glActiveTexture( GL_TEXTURE0 );
    
    // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
}

void qp::renderText(const std::string s, int x, int y, int height, const glm::vec3 color) {
    static TextRenderer renderer("assets\\fonts\\c.ttf");

    renderer.render(s, x, y, height, color);
}

void qp::renderText(float f, int x, int y, int height, const glm::vec3 color) {
    renderText(std::to_string(f), x, y, height, color);
}

void qp::renderText(int i, int x, int y, int height, const glm::vec3 color) {
    renderText(std::to_string(i), x, y, height, color);
}
