/*
    The content of this file was heavily inspired by: 
    https://github.com/caadxyz/glfwOcctViewer 
*/

// Standard library.
#include <string>
#include <cassert>

// Third party.

// GLFW
#include "GLFW/glfw3.h"

#undef Handle 
// #define GLFW_EXPOSE_NATIVE_COCOA
// #define GLFW_EXPOSE_NATIVE_NSGL
#define GLFW_EXPOSE_NATIVE_GLX
#define GLFW_EXPOSE_NATIVE_X11
#include "GLFW/glfw3native.h"

// Library private.
#include "glfw_occt_window_p.hxx"

/*
    Creates a GLFW window and initializes the data members in this class to
        mirror the characteristics of the underlying window.
*/
GlfwOcctWindow::GlfwOcctWindow(int width, int height, const std::string& title)
{
    GLFWwindow *res {glfwCreateWindow(width, height, title.c_str(), NULL, NULL)};
    assert(res != nullptr);
    this->glfw_window = res;

    int cur_width = 0, cur_height = 0;
    glfwGetWindowPos(this->glfw_window, &this->x_left, &this->y_top);
    glfwGetWindowSize(this->glfw_window, &cur_width, &cur_height);
    this->x_right = x_left + cur_width;
    this->y_bottom = y_top + cur_height;
    
    // Warning: This is Linux-specific for some reason!!
    myDisplay = new Aspect_DisplayConnection(glfwGetX11Display());
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
    return glfwGetX11Window(this->glfw_window);
}

/*
    Gets the OpenGL context used by the the underlying GLFW window.
*/
Aspect_RenderingContext GlfwOcctWindow::opengl_context() const
{
    GLXContext res {glfwGetGLXContext(this->glfw_window)};
    return res;
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
