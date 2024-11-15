#pragma once

// Standard library.
#include <string>

// Third party.
#include "Aspect_DisplayConnection.hxx"
#include "Aspect_RenderingContext.hxx"
#include "Aspect_Window.hxx"

struct GLFWwindow;

class GlfwOcctWindow : public Aspect_Window
{
    DEFINE_STANDARD_RTTI_INLINE(GlfwOcctWindow, Aspect_Window)

public:
    GlfwOcctWindow(int width, int height, const std::string& title);

    virtual ~GlfwOcctWindow() 
    { 
        close();
    }
    
    void close();

    const Handle(Aspect_DisplayConnection)& get_display() const 
    { 
        return this->myDisplay; 
    }

    GLFWwindow* get_glfw_window() 
    { 
        return this->glfw_window; 
    }

    Aspect_RenderingContext opengl_context() const;

    Graphic3d_Vec2i cursor_position() const;

    virtual Aspect_Drawable NativeHandle() const override;

    virtual Aspect_Drawable NativeParentHandle() const override
    { 
        return 0; 
    }

    virtual Aspect_TypeOfResize DoResize() override;

    virtual Standard_Boolean IsMapped() const override;

    virtual Standard_Boolean DoMapping() const override 
    { 
        return Standard_True; 
    }

    virtual void Map() const override;

    virtual void Unmap() const override;

    virtual void Position(Standard_Integer& x1, Standard_Integer& y1,
                          Standard_Integer& x2, Standard_Integer& y2) const override
    {
        x1 = x_left;
        x2 = x_right;
        y1 = y_top;
        y2 = y_bottom;
    }

    virtual Standard_Real Ratio() const override
    {
        return Standard_Real(x_right - x_left) / Standard_Real(y_bottom - y_top);
    }

    virtual void Size(Standard_Integer& width, Standard_Integer& height) const override
    {
        width = x_right - x_left;
        height = y_bottom - y_top;
    }

    virtual Aspect_FBConfig NativeFBConfig() const override 
    { 
        return NULL; 
    }

protected:
    Handle(Aspect_DisplayConnection) display;
    GLFWwindow* glfw_window;
    Standard_Integer x_left;
    Standard_Integer y_top;
    Standard_Integer x_right;
    Standard_Integer y_bottom;
};
