#include "shader_manager.h"

shader_manager &shader_manager::instance = shader_manager::create();

shader_manager::shader_manager() : current_index(0)
{
}

void shader_manager::load(const char *vert_file, const char *frag_file)
{
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vShaderFile{};
    std::ifstream fShaderFile{};

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        vShaderFile.open(vert_file);
        fShaderFile.open(frag_file);

        std::stringstream vShaderStream, fShaderStream;

        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // close file handlers
        vShaderFile.close();
        fShaderFile.close();

        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << e.what() << std::endl;
    }

    const char *vertex_src = vertexCode.c_str();
    const char *fragment_src = fragmentCode.c_str();

    GLuint vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, nullptr);
    glCompileShader(vertex_shader);

    if (!_is_successful(vertex_shader, GL_COMPILE_STATUS)) return;

    GLuint fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, nullptr);
    glCompileShader(fragment_shader);

    if (!_is_successful(fragment_shader, GL_COMPILE_STATUS)) return;

    program_ids[current_index] = glCreateProgram();
    glAttachShader(program_ids[current_index], vertex_shader);
    glAttachShader(program_ids[current_index], fragment_shader);
    glLinkProgram(program_ids[current_index]);

    if (!_is_successful(program_ids[current_index], GL_LINK_STATUS)) return;

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    current_index++;
}

void shader_manager::use(unsigned short program_id_index)
{
    glUseProgram(program_ids[program_id_index]);
}

void shader_manager::set_int(const char* name, int value)
{
    glUniform1i(glGetUniformLocation(program_ids[0], name), value);
}

void shader_manager::set_mat4(const char* name, float* value)
{
    glUniformMatrix4fv(glGetUniformLocation(program_ids[0], name), 1, GL_FALSE, value);
}

bool shader_manager::_is_successful(GLuint id, GLuint action)
{
    int success;
    char infoLog[512];
    if (action == GL_COMPILE_STATUS)
    {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            std::cerr << "Shader Compilation Error: " << infoLog;
        }
    }
    else if (action == GL_LINK_STATUS)
    {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 512, NULL, infoLog);
            std::cerr << "Shader Linking Error: " << infoLog;
        }
    }

    return success;
}
