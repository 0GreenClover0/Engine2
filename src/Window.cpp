#include "Window.h"

#include <iostream>

#include "Renderer.h"

#include <stdexcept>

#include "Engine.h"
#include <stb_image.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

Window::Window(Renderer::RendererApi const renderer_api, i32 const screen_width, i32 const screen_height, i32 const subsamples,
               bool const enable_vsync, bool const enable_mouse_capture)
{
    switch (renderer_api)
    {
    case Renderer::RendererApi::OpenGL:
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
        glfwWindowHint(GLFW_SAMPLES, subsamples);
        break;
    case Renderer::RendererApi::DirectX11:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_SAMPLES, GLFW_DONT_CARE);
        break;
    default:
        std::unreachable();
    }

    m_window = glfwCreateWindow(screen_width, screen_height, Engine::window_title.c_str(), nullptr, nullptr);

    if (m_window == nullptr)
        throw std::runtime_error("Could not create a window.");

    GLFWimage images[1];
    images[0].pixels = stbi_load("./res/icons/engineIcon.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(m_window, 1, images);
    stbi_image_free(images[0].pixels);

    switch (renderer_api)
    {
    case Renderer::RendererApi::OpenGL:
        glfwMakeContextCurrent(m_window);

        // Enable vsync
        glfwSwapInterval(enable_vsync);

        // Capture mouse
        if (enable_mouse_capture)
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        break;
    case Renderer::RendererApi::DirectX11:
        if (enable_mouse_capture)
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        break;

    default:
        std::unreachable();
    }
}

GLFWwindow* Window::get_glfw_window() const
{
    return m_window;
}

HWND Window::get_win32_window() const
{
    return glfwGetWin32Window(m_window);
}

void Window::maximize_glfw_window() const
{
    glfwMaximizeWindow(m_window);
}
