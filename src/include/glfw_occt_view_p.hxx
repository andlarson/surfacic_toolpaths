#pragma once

// Third party.
#include "AIS_InteractiveContext.hxx"
#include "AIS_ViewController.hxx"
#include "V3d_View.hxx"

// Library private. 
#include "glfw_occt_window_p.hxx"

class GlfwOcctView : protected AIS_ViewController
{
public:
    GlfwOcctView(); 
    ~GlfwOcctView(); 

    void main_loop();

    Handle(V3d_View) view;
    Handle(AIS_InteractiveContext) context;

private:
    void init_window(int width, int height, const char* title);
    void init_viewer();
    void init_demo_scene();
    void cleanup();

    /***************************************************************************
                                GLFW Callback Helpers 
    ****************************************************************************/

    void on_resize(int width, int height);
    void on_mouse_scroll(double offset_x, double offset_y);
    void on_mouse_button(int button, int action, int mods);
    void on_mouse_move(int pos_x, int pos_y);
    
    /***************************************************************************
                                  GLFW Callbacks 
    ****************************************************************************/

    static void error_callback(int error, const char* description);

    static GlfwOcctView* to_view(GLFWwindow* win);

    static void resize_callback(GLFWwindow* win, int width, int height)
    { 
        to_view(win)->on_resize(width, height); 
    }

    static void fb_resize_callback(GLFWwindow* win, int width, int height)
    { 
        to_view(win)->on_resize(width, height); 
    }

    static void mouse_scroll_callback(GLFWwindow* win, double offset_x, double offset_y)
    { 
        to_view(win)->on_mouse_scroll(offset_x, offset_y); 
    }

    static void mouse_button_callback(GLFWwindow* win, int button, int action, int mods)
    { 
        to_view(win)->on_mouse_button(button, action, mods);
    }

    static void mouse_move_callback(GLFWwindow* win, double pos_x, double pos_y)
    { 
        to_view(win)->on_mouse_move((int) pos_x, (int) pos_y); 
    }

    Handle(GlfwOcctWindow) occt_window;
};

void show_shapes(const std::vector<TopoDS_Shape>& shapes);
