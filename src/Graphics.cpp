#include <optional>
#include <iostream>
#include <array>
#include <variant>
#include <fmt/core.h>

#include "GLAD/glad.h"
#include <GLFW/glfw3.h>

using window_deleter = decltype(&glfwDestroyWindow);
using GLError = std::string;

using GLResult = std::variant<GLuint, GLError>;

auto create_shader_program(std::string_view vertex_src, std::string_view fragment_src) -> GLResult;

auto create_shader(std::string_view src, GLenum type) -> GLuint;

// Note to self: Backup is in ~/backup_repos
// TODO: Refactor GLAD as library

auto main() -> int {
    auto success = int { };

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int width = 800, height = 600;

    auto ctx = std::unique_ptr<GLFWwindow, window_deleter> {
        glfwCreateWindow(width, height, "Graphics", nullptr, nullptr),
        glfwDestroyWindow };
    if (!ctx) {
        fmt::print("Error creating window context.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(ctx.get());

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        fmt::print("Failed to initialize OpenGL context.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // temporary

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    std::string_view vertex_shader_src = " \
        #version 460 core\n \
        layout (location = 0) in vec3 aPos; \
        void main() { \
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f); \
        }";

    std::string_view fragment_shader_src = " \
        #version 460 core\n \
        out vec4 FragColor; \
        void main() { \
            FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); \
        }";

    auto shader_program_res = create_shader_program(vertex_shader_src, fragment_shader_src);
    if (std::holds_alternative<GLError>(shader_program_res)) {
        fmt::print(stderr, "{}", std::get<GLError>(shader_program_res));
        glfwTerminate();
        return EXIT_FAILURE;
    }
    auto shader_program = std::get<GLuint>(shader_program_res);

    glUseProgram(shader_program);

    // end of temporary

    while (!glfwWindowShouldClose(ctx.get())) {
        glClearColor(0.4f, 0.3f, 0.45f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(ctx.get());
        glfwPollEvents();
    }
    
    glfwTerminate();
}

// I'm moving everything below this in a future patch

GLuint create_shader(std::string_view src, GLenum type) {
    auto shader = glCreateShader(type);

    auto data = src.data();
    glShaderSource(shader, 1, &data, nullptr);
    glCompileShader(shader);

    return shader;
}

GLResult create_shader_(std::string_view src, GLenum type) {
    auto shader = glCreateShader(type);
    if (!shader) {
        return "Could not create shader.";
    }

    auto data = src.data();
    glShaderSource(shader, 1, &data, nullptr);
    glCompileShader(shader);

    auto success = GLint { };
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        auto err_log = std::string { };
        err_log.reserve(512);
        glGetShaderInfoLog(shader, static_cast<GLsizei>(err_log.capacity()), nullptr, err_log.data());
        return err_log;
    }

    return shader;
}
GLResult create_shader_program(std::string_view vertex_src, std::string_view fragment_src) {
    static constexpr std::string_view err_fmt = "Error creating shader program: {}";

    auto vertex_shader_res = create_shader_(vertex_src, GL_VERTEX_SHADER);
    if (std::holds_alternative<GLError>(vertex_shader_res)) {
        glDeleteShader(std::get<GLuint>(vertex_shader_res));
        return GLError { fmt::format(err_fmt.data(), std::get<GLError>(vertex_shader_res)).c_str()};
    }

    auto fragment_shader_res = create_shader_(fragment_src, GL_FRAGMENT_SHADER);
    if (std::holds_alternative<GLError>(vertex_shader_res)) {
        glDeleteShader(std::get<GLuint>(fragment_shader_res));
        return GLError { fmt::format(err_fmt.data(), std::get<GLError>(fragment_shader_res)).c_str()};
    }

    auto vertex_shader = std::get<GLuint>(vertex_shader_res);
    auto fragment_shader = std::get<GLuint>(fragment_shader_res);

    auto shader_program = glCreateProgram();
    if (!shader_program) {
        return "Could not create shader program.";
    }

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    auto success = GLint { };
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        auto err_log = std::string { };
        err_log.reserve(512);
        glGetProgramInfoLog(shader_program, err_log.capacity(), nullptr, err_log.data());
        return fmt::format(err_fmt.data(), err_log);
    }

    return shader_program;
}
