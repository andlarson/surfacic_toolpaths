/*
    Contains the implementation of the GlfwOcctWindow class. This class is a wrapper
        for a GLFW window. Most function members of this class are registered as
        callbacks for the GLFW window. For example, when the GLFW window is resized,
        the GlfwOcctWindow::DoResize() function is called-back and the data members
        of the GlfwOcctWindow class are updated to mirror the new size of the GLFW
        window.
*/

// Standard library.
#include <string>

// Third party.

// GLFW
#include "GLFW/glfw3.h"

#undef Handle 
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#include "GLFW/glfw3native.h"

// Library private.
#include "src/include/glfw_occt_window.hxx"

/*
    Creates an GLFW window and initializes the data members in this class to
        mirror the characteristics of the underlying window.
*/
GlfwOcctWindow::GlfwOcctWindow(int width, int height, const std::string& title)
  : glfw_window(glfwCreateWindow(width, height, title.c_str(), NULL, NULL)),
    x_left(0),
    y_top(0),
    x_right(0),
    y_bottom(0)
{
    int cur_width = 0, cur_height = 0;
    glfwGetWindowPos(glfw_window, &x_left, &y_top);
    glfwGetWindowSize(glfw_window, &cur_width, &cur_height);
    x_right = x_left + cur_width;
    y_bottom = y_top + cur_height;
}

/*
    Closes the underlying GLFW window.
*/
void GlfwOcctWindow::close()
{
    glfwDestroyWindow(glfw_window);
    glfw_window = nullptr;
}

/*
    Gets the native handle of the underlying GLFW window. GLFW, under the hood,
        uses OS-native libraries to create a window assciated with an OpenGL
        context.
*/
Aspect_Drawable GlfwOcctWindow::NativeHandle() const
{
    return (Aspect_Drawable) glfwGetCocoaWindow(glfw_window);
}

/*
    Gets the OpenGL context used by the the underlying GLFW window.
*/
Aspect_RenderingContext GlfwOcctWindow::opengl_context() const
{
    return (NSOpenGLContext*) glfwGetNSGLContext(glfw_window);
}

/*
    Checks if the underlying GLFW window is visible.
*/
Standard_Boolean GlfwOcctWindow::IsMapped() const
{
    return glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE) != 0;
}

/*
    Makes the underlying GLFW window visible.
*/
void GlfwOcctWindow::Map() const
{
    glfwShowWindow(glfw_window);
}

/*
    Makes the GLFW window not visible.
*/
void GlfwOcctWindow::Unmap() const
{
    glfwHideWindow(glfw_window);
}

/*
    Updates the recorded window size. Should be called when the GLFW window is
        actually resized.
*/
Aspect_TypeOfResize GlfwOcctWindow::DoResize()
{
    if (glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE) == 1)
    {
        int anXPos = 0, anYPos = 0, aWidth = 0, aHeight = 0;
        glfwGetWindowPos(glfw_window, &anXPos, &anYPos);
        glfwGetWindowSize(glfw_window, &aWidth, &aHeight);
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
    glfwGetCursorPos(glfw_window, &cur_pos.x(), &cur_pos.y());
    return Graphic3d_Vec2i((int) cur_pos.x(), (int) cur_pos.y());
}
