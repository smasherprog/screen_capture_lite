#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include "ScreenCapture.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <atomic>
using namespace std::chrono_literals;

void CheckStatus(GLuint obj, bool isShader)
{
    GLint status = GL_FALSE, log[1 << 11] = {0};
    (isShader ? glGetShaderiv : glGetProgramiv)(obj, isShader ? GL_COMPILE_STATUS : GL_LINK_STATUS, &status);
    if (status == GL_TRUE)
        return;
    (isShader ? glGetShaderInfoLog : glGetProgramInfoLog)(obj, sizeof(log), NULL, (GLchar *)log);
    std::cerr << (GLchar *)log << "\n";
    std::exit(EXIT_FAILURE);
}

void AttachShader(GLuint program, GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    CheckStatus(shader, true);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

const char *vert = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;
out vec3 ourColor;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)GLSL";

const char *frag = R"GLSL(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;
uniform sampler2D ourTexture;
void main() {
    FragColor = texture(ourTexture, TexCoord);
}
)GLSL";

void ExtractAndConvertToRGBA(const SL::Screen_Capture::Image &img, unsigned char *dst, size_t dst_size)
{
    assert(dst_size >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) * sizeof(SL::Screen_Capture::ImageBGRA)));
    auto imgsrc = StartSrc(img);
    auto imgdist = dst;
    if (img.isContiguous) {
        memcpy(imgdist, imgsrc, dst_size);
    }
    else {
        for (auto h = 0; h < Height(img); h++) {
            auto startimgsrc = imgsrc;
            for (auto w = 0; w < Width(img); w++) {
                *imgdist++ = imgsrc->R;
                *imgdist++ = imgsrc->G;
                *imgdist++ = imgsrc->B;
                *imgdist++ = 0; // alpha should be zero
                imgsrc++;
            }
            imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
        }
    }
}

int main(int, char **)
{
    glfwSetErrorCallback([](int, const char *desc) {
        std::cerr << desc << "\n";
        std::exit(EXIT_FAILURE);
    });
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "GLFW", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    GLuint prog = glCreateProgram();
    AttachShader(prog, GL_VERTEX_SHADER, vert);
    AttachShader(prog, GL_FRAGMENT_SHADER, frag);
    glLinkProgram(prog);
    CheckStatus(prog, false);

    float vertices[] = {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture wrap parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering paremeters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image, create texture
    auto onNewFramestart = std::chrono::high_resolution_clock::now();
    std::atomic<int> onNewFramecounter = 0;
    auto mons = SL::Screen_Capture::GetMonitors();
    mons.resize(1);
    auto monitor = mons[0];
    std::atomic<int> imgbuffersize = monitor.Width * monitor.Height * sizeof(SL::Screen_Capture::ImageBGRA);
    std::unique_ptr<unsigned char[]> imgbuffer(std::make_unique<unsigned char[]>(imgbuffersize));
    memset(imgbuffer.get(), 0, imgbuffersize); // create a black image to start with
    std::atomic<bool> imgbufferchanged = false;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, monitor.Width, monitor.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgbuffer.get());
    glBindTexture(GL_TEXTURE_2D, 0);

    auto framgrabber =
        SL::Screen_Capture::CreateCaptureConfiguration([&]() { return mons; })
            ->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) {
                imgbufferchanged = true;
                ExtractAndConvertToRGBA(img, imgbuffer.get(), imgbuffersize); 
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >=
                    1000) {
                    std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
                    onNewFramecounter = 0;
                    onNewFramestart = std::chrono::high_resolution_clock::now();
                }
                onNewFramecounter += 1;
            })
            ->start_capturing();
    framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (imgbufferchanged) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, monitor.Width, monitor.Height, GL_RGBA, GL_UNSIGNED_BYTE, imgbuffer.get());
            imgbufferchanged = false;
        }
        // Render stuff
        glUseProgram(prog);
        unsigned int texture_unit = 0;
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glUniform1i(glGetUniformLocation(prog, "ourTexture"), texture_unit);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}