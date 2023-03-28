#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ShaderProgram {
public:
    unsigned int ID;

    ShaderProgram(const std::string& vspath, const std::string& fspath) {
        ID = createShaderProgram(vspath, fspath);
    }

    ~ShaderProgram() {
        glDeleteProgram(ID);
    }

    void setUniform1i(const std::string &name, int val) {
        glProgramUniform1i(ID, getLocation(name), val);
    }

    void setUniform1f(const std::string &name, float val) {
        glProgramUniform1f(ID, getLocation(name), val);
    }

    void setUniform3fv(const std::string &name, glm::vec3 val) {
        glProgramUniform3fv(ID, getLocation(name), 1, glm::value_ptr(val));
    }

    void setUniformMatrix4fv(const std::string &name, glm::mat4 val) {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(val));
    }

private:
    std::map<std::string, GLint> locations;  // For fast caching of locations

    ShaderProgram();

    GLint getLocation(const std::string &name) {
        std::map<std::string, GLint>::iterator it = locations.find(name);
        if (it == locations.end())  // This location doesn't exist in the map yet
            locations[name] = glGetUniformLocation(ID, name.c_str());

        return locations[name];
    }

    unsigned int createShaderProgram(const std::string& vspath, const std::string& fspath)
    {
        std::string vscode = "", fscode = "";
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit || std::ifstream::badbit); //Set up io exceptions
        try
        {
            shaderFile.open(vspath);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf(); //Read buffer into the string stream
            shaderFile.close();
            vscode = shaderStream.str();

            shaderFile.open(fspath);
            shaderStream.str(""); //remove old data
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            fscode = shaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "Error reading shader file " << vspath << std::endl;
            return -1;
        }
        const char* c_vscode = vscode.c_str();
        const char* c_fscode = fscode.c_str();

        //2. attach code to shader
        unsigned int vshader = glCreateShader(GL_VERTEX_SHADER), fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vshader, 1, &c_vscode, NULL);
        glShaderSource(fshader, 1, &c_fscode, NULL);

        //3. compile and report errors.
        GLchar log[1024];
        GLint success;
        glCompileShader(vshader);
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vshader, 1024, NULL, log);
            std::cout << vspath << ": Error compiling Vertex shader: " << log << std::endl;
            return -1;
        }
        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fshader, 1024, NULL, log);
            std::cout << fspath << ": Error compiling Fragment shader: " << log << std::endl;
            return -1;
        }

        //4. Attach to a program
        GLuint program = glCreateProgram();
        glAttachShader(program, vshader);
        glAttachShader(program, fshader);
        glValidateProgram(program);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 1024, NULL, log);
            std::cout << "Error linking program: " << log << std::endl;
            return -1;
        }
        // We've linked successfully, no need for shaders to occupy memory anymore
        glDeleteShader(vshader);
        glDeleteShader(fshader);
        return program;
    }
};

#endif