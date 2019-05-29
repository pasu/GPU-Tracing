#include"glmacros.h"


using namespace std;

#define make_str( x ) #x

GLuint qp::createShader(const char* source, GLenum type, const char* errinfo) {
    int len = std::strlen(source);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &len);
    glCompileShader(shader);
    GLint testVal = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &testVal);
    if (testVal == GL_FALSE) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        glDeleteShader(shader);
        printf("shader error (%s) : %s\n", errinfo, infoLog);
        return 0;
    }
    return shader;
}



void qp::linkProgram(GLuint program, const char* errinfo) {
    GLint testVal;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &testVal);
    if (testVal == GL_FALSE) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        glDeleteProgram(program);

        printf("link error (%s) : %s\n", errinfo, infoLog);
    }
}

GLuint qp::createProgram_C(std::string source) {
    GLuint program = glCreateProgram();
    GLuint shader = createShader(source.c_str(),GL_COMPUTE_SHADER, "compute");

    glAttachShader(program, shader);
    linkProgram(program);

    return program;
}

GLuint qp::createProgram_VF(std::string v_source, std::string f_source) {
    GLuint program = glCreateProgram();
    GLuint v_shader = createShader(v_source.c_str(), GL_VERTEX_SHADER, "vertex shader");
    GLuint f_shader = createShader(f_source.c_str(), GL_FRAGMENT_SHADER, "fragment shader");

    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);

    linkProgram(program);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
    return program;
}

GLuint qp::createProgram_TESS(std::string v_source, std::string tc_source, std::string te_source, std::string f_source) {
    GLuint program = glCreateProgram();
    GLuint v_shader = createShader(v_source.c_str(), GL_VERTEX_SHADER, "vertex shader");
    GLuint tc_shader = createShader(tc_source.c_str(), GL_TESS_CONTROL_SHADER, "tcs");
    GLuint te_shader = createShader(te_source.c_str(), GL_TESS_EVALUATION_SHADER, "tes");
    GLuint f_shader = createShader(f_source.c_str(), GL_FRAGMENT_SHADER, "fragment shader");

    glAttachShader(program, v_shader);
    glAttachShader(program, tc_shader);
    glAttachShader(program, te_shader);
    glAttachShader(program, f_shader);

    linkProgram(program);

    glDeleteShader(v_shader);
    glDeleteShader(tc_shader);
    glDeleteShader(te_shader);
    glDeleteShader(f_shader);
    return program;
}

void qp::renderTriangles(const std::vector<glm::vec3>& vertex_data, const glm::mat4 & mViewProjection, const glm::vec3 & vLight, const glm::mat4 & mModel) {
    static GLuint program, vao, vbo;
    static const char* vertex_source = make_str(#version 430\n
        layout(location = 0)in vec3 vVertex; \n
        layout(location = 1)in vec3 vNormal; \n
        layout(location = 2)in vec3 vColor; \n
        uniform mat4 mMVP; \n
        uniform mat4 mModelRot; \n
        uniform vec3 vLight;\n
        out vec3 color; \n
        void main(void) {
        \n
            gl_Position = mMVP * vec4(vVertex, 1.0); \n
            vec4 n = mModelRot * vec4(vNormal, 1);\n
            color = vColor * (-dot(n.xyz, vLight));\n
        }
    );
    static const char* fragment_source = make_str(#version 430\n
        in vec3 color;\n
        void main(void) {
        \n
            gl_FragColor = vec4(color,1);
        }
    );
    bool onFirstCall = [&]() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * 4 * 3 * 1024 * 3, NULL, GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * 4 * 3, 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * 4 * 3, (void*)12);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * 4 * 3, (void*)24);

        program = qp::createProgram_VF(vertex_source, fragment_source);
        return true;
    }();


    glUseProgram(program);

    glm::mat4 mMVP = mViewProjection * mModel;
    glm::mat4 mModelRot = glm::extractMatrixRotation(mModel);

    glUniformMatrix4fv(glGetUniformLocation(program, "mMVP"),1,GL_FALSE, &mMVP[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(program, "mModelRot"), 1, GL_FALSE, &mModelRot[0][0]);

    glUniform3fv(glGetUniformLocation(program, "vLight"), 1, &vLight.x);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_data.size() * sizeof(glm::vec3), &vertex_data[0].x);

    glBindVertexArray(vao);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, vertex_data.size() / 3);
}

