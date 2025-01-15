/*
    The content of this file was heavily inspired by: 
    https://github.com/caadxyz/glfwOcctViewer 
*/

// Standard library.
#include <string>
#include <stdexcept>

// Third party.

// GLFW
#if defined (__APPLE__)
    // Work around OCCT's unfortunately named custom smart pointer type.
    #undef Handle 
    #define GLFW_EXPOSE_NATIVE_COCOA
    #define GLFW_EXPOSE_NATIVE_NSGL
#else
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Library private.
#include "glfw_occt_window_p.hxx"

/*
    Creates a GLFW window and initializes the data members in this class to
        mirror the characteristics of the underlying window.
*/
GlfwOcctWindow::GlfwOcctWindow(int width, int height, const std::string& title)
{
    GLFWwindow *res {glfwCreateWindow(width, height, title.c_str(), NULL, NULL)};
    if (res == nullptr)
        throw std::runtime_error("Window creation failed!");
    this->glfw_window = res;

    int cur_width = 0, cur_height = 0;
    glfwGetWindowPos(this->glfw_window, &this->x_left, &this->y_top);
    glfwGetWindowSize(this->glfw_window, &cur_width, &cur_height);
    this->x_right = x_left + cur_width;
    this->y_bottom = y_top + cur_height;
    
#if !defined(__APPLE__)
    // For reasons that I don't understand, the "display" is only required
    //     on Linux. The "display" is used to extract an OpenGL graphic driver,
    //     and apparently on non-Linux systems, an OpenGL graphic driver can
    //     be extracted without a "display".
    Display* disp {glfwGetX11Display()};
    if (disp == nullptr)
        throw std::runtime_error("Error occurred when getting the display!");
    this->myDisplay = new Aspect_DisplayConnection(disp);
#endif
}

/*
    Closes the underlying GLFW window.
*/
void GlfwOcctWindow::close()
{
    // This seems like odd behavior. If this nullptr check and nullptr set are
    //     omitted, then GLFW_NOT_INITIALIZED errors happen upon the call to
    //     glfwDestroyWindow(). The reason for this is that, when the destructor
    //     for the view data member runs, for some reason, it ends up calling
    //     close() on the this->glfw_window. This means that close() can be
    //     called twice: once explicitly and once implicitly via the destructor.
    //     The nullptr check and set prevents the same glfw window from being
    //     destroyed twice, which would be an error!
    if (this->glfw_window != nullptr)
    {
        glfwDestroyWindow(this->glfw_window);
        glfw_window = nullptr;
    }
}

/*
    Gets the native handle of the underlying GLFW window. GLFW, under the hood,
        uses OS-native libraries to create a window assciated with an OpenGL
        context.
*/
Aspect_Drawable GlfwOcctWindow::NativeHandle() const
{
#if defined (__APPLE__)
    const id window_id {glfwGetCocoaWindow(get_glfw_window())};
    if (window_id == nil)
        throw std::runtime_error("Failed to get a Cocoa Window!");
    return (Aspect_Drawable) window_id; 
#else
    const Window window_id {glfwGetX11Window(get_glfw_window())}; 
    if (window_id == None)
        throw std::runtime_error("Failed to get an X11 Window!");
    return (Aspect_Drawable) window_id; 
#endif
}

/*
    Gets the OpenGL context used by the the underlying GLFW window.
*/
Aspect_RenderingContext GlfwOcctWindow::opengl_context() const
{
#if defined (__APPLE__)
    const id opengl_context {glfwGetNSGLContext(get_glfw_window())};
    if (opengl_context == nil)
        throw std::runtime_error("Failed to get NSGL context!");
    return (Aspect_RenderingContext) opengl_context; 
#else
    const GLXContext opengl_context {glfwGetGLXContext(get_glfw_window())};
    if (opengl_context == None)
        throw std::runtime_error("Failed to get NSGL context!");
    return (Aspect_RenderingContext) opengl_context;
#endif
}

/*
    Checks if the underlying GLFW window is visible.
*/
Standard_Boolean GlfwOcctWindow::IsMapped() const
{
    return glfwGetWindowAttrib(this->glfw_window, GLFW_VISIBLE) != 0;
}

/*
    Makes the underlying GLFW window visible.
*/
void GlfwOcctWindow::Map() const
{
    glfwShowWindow(this->glfw_window);
}

/*
    Makes the GLFW window not visible.
*/
void GlfwOcctWindow::Unmap() const
{
    glfwHideWindow(this->glfw_window);
}

/*
    Updates the recorded window size. Should be called when the GLFW window is
        actually resized.
*/
Aspect_TypeOfResize GlfwOcctWindow::DoResize()
{
    if (glfwGetWindowAttrib(this->glfw_window, GLFW_VISIBLE) == 1)
    {
        int anXPos = 0, anYPos = 0, aWidth = 0, aHeight = 0;
        glfwGetWindowPos(this->glfw_window, &anXPos, &anYPos);
        glfwGetWindowSize(this->glfw_window, &aWidth, &aHeight);
        x_left = anXPos;
        x_right = anXPos + aWidth;
        y_top = anYPos;
        y_bottom = anYPos + aHeight;
    }
    return Aspect_TOR_UNKNOWN;
}

/*
    Returns the position of the cursor in the GLFW window. Unclear what happens
        if the cursor isn't in the GLFW window. 
*/
Graphic3d_Vec2i GlfwOcctWindow::cursor_position() const
{
    Graphic3d_Vec2d cur_pos;
    glfwGetCursorPos(this->glfw_window, &cur_pos.x(), &cur_pos.y());
    return Graphic3d_Vec2i((int) cur_pos.x(), (int) cur_pos.y());
}
