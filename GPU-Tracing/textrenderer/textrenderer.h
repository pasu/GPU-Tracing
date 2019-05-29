#pragma once
#include<string>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include"glmacros.h"


namespace qp {
    class TextRenderer {
    private:
        GLuint _texture_;

        static constexpr int max_char_number = 256;

#pragma pack(push)
#pragma pack(4)
        using FontFaceVertex = struct {
            glm::vec2 vVertex;
            glm::vec2 vTex;
        };
#pragma pack(pop)
        static std::vector<FontFaceVertex> vertices;
        static GLuint program, vao, vbo;
        static const char* vertex_shader_source, *fragment_shader_source;

        static bool onFirstCall();
        
        int _text_height_;// max height of all characters in pixel, reference height in pixel
        using Character = struct {
            glm::vec2 texBound1;// left bottom corner
            glm::vec2 texBound2;// right top corner
            glm::ivec2 vertexBound1;
            glm::ivec2 vertexBound2;
            int advance;
        };
        std::vector<Character> characters;
    public:
        TextRenderer(const std::string fontname);
        ~TextRenderer();

        void render(const std::string s, int x, int y, int height, const glm::vec3 color = glm::vec3(1, 1, 1));

    };

    void renderText(const std::string s, int x, int y, int height, const glm::vec3 color = glm::vec3(1, 1, 1));
    void renderText(float f, int x, int y, int height, const glm::vec3 color = glm::vec3(1, 1, 1));
    void renderText(int i, int x, int y, int height, const glm::vec3 color = glm::vec3(1, 1, 1));
}