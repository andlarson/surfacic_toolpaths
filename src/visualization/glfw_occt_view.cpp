/*
    The content of this file was heavily inspired by: 
    https://github.com/caadxyz/glfwOcctViewer 
*/

// Standard library.
#include <stdexcept>

// Third party.

// OCCT
#include "AIS_Shape.hxx"
#include "OpenGl_GraphicDriver.hxx"

// GLFW
#include "GLFW/glfw3.h"

// Library private.
#include "glfw_occt_view_p.hxx"

/* 
   ****************************************************************************
                            File Local Declarations
   **************************************************************************** 
*/ 

static Aspect_VKeyMouse mouse_button_from_glfw(int button);
static Aspect_VKeyFlags key_flags_from_glfw(int flags);

/* 
   ****************************************************************************
                            File Local Definitions 
   **************************************************************************** 
*/ 

static Aspect_VKeyMouse mouse_button_from_glfw(int button)
{
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:   
            return Aspect_VKeyMouse_LeftButton;
        case GLFW_MOUSE_BUTTON_RIGHT:  
            return Aspect_VKeyMouse_RightButton;
        case GLFW_MOUSE_BUTTON_MIDDLE: 
            return Aspect_VKeyMouse_MiddleButton;
    }
    return Aspect_VKeyMouse_NONE;
}

static Aspect_VKeyFlags key_flags_from_glfw(int flags)
{
    Aspect_VKeyFlags a_flags = Aspect_VKeyFlags_NONE;
    if ((flags & GLFW_MOD_SHIFT) != 0)
        a_flags |= Aspect_VKeyFlags_SHIFT;
    if ((flags & GLFW_MOD_CONTROL) != 0)
        a_flags |= Aspect_VKeyFlags_CTRL;
    if ((flags & GLFW_MOD_ALT) != 0)
        a_flags |= Aspect_VKeyFlags_ALT;
    if ((flags & GLFW_MOD_SUPER) != 0)
        a_flags |= Aspect_VKeyFlags_META;
    return a_flags;
}

/* 
   ****************************************************************************
                                    GlfwOcctView 
   **************************************************************************** 
*/ 

/*
    Retrieves the GlfwOcctView class instance that wraps the underlying GLFW 
        window. 
*/
GlfwOcctView* GlfwOcctView::to_view(GLFWwindow* win)
{
    return static_cast<GlfwOcctView*>(glfwGetWindowUserPointer(win));
}

void GlfwOcctView::error_callback(int error, const char* description)
{
    std::cout << "Error number " << error << " occurred in GLFW, and the error description is: " << description << std::endl;
}

/*
    Creates a GLFW window and associates a wrapping GlfwOcctWindow object
        with it, by using the glfwSetWindowUserPointer() function.
*/
void GlfwOcctView::init_window(int width, int height, const char* title)
{
    const int res1 {glfwInit()};
    if (res1 != GLFW_TRUE)
        throw std::runtime_error("GLFW initialization failed!");
    
    // Return value not checked here because error callback persists across
    //     initialization and termination of GLFW library.
    glfwSetErrorCallback(GlfwOcctView::error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // Only necessary on MacOS due to old system version of OpenGL? 
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Only necessary on MacOS because system OpenGL only offers core profile?
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create a GlfwOcctWindow, that has characteristics which mirror the 
    //     GLFW window, and associate it with the GLFW window.
    this->occt_window = new GlfwOcctWindow(width, height, title);
    glfwSetWindowUserPointer(this->occt_window->get_glfw_window(), this);
    
    // Set callbacks for various actions.
    // These should be the first callbacks set, and hence shouldn't return anything.
    const auto res3 {glfwSetWindowSizeCallback(this->occt_window->get_glfw_window(), GlfwOcctView::resize_callback)};
    if (res3 != nullptr)
        throw std::runtime_error("A window size callback had already been set!");

    const auto res4 {glfwSetFramebufferSizeCallback(this->occt_window->get_glfw_window(), GlfwOcctView::fb_resize_callback)};
    if (res4 != nullptr)
        throw std::runtime_error("A frame buffer size callback had already been set!");

    const auto res5 {glfwSetScrollCallback(this->occt_window->get_glfw_window(), GlfwOcctView::mouse_scroll_callback)};
    if (res5 != nullptr)
        throw std::runtime_error("A set scroll callback had already been set!");

    const auto res6 {glfwSetMouseButtonCallback(this->occt_window->get_glfw_window(), GlfwOcctView::mouse_button_callback)};
    if (res6 != nullptr)
        throw std::runtime_error("A set mouse button callback had already been set!");

    const auto res7 {glfwSetCursorPosCallback(this->occt_window->get_glfw_window(), GlfwOcctView::mouse_move_callback)};
    if (res7 != nullptr)
        throw std::runtime_error("A set cursor position callback had already been set!");
}

/*
    Creates an OCCT viewer with an OCCT AIS context. 
    
    In order to create the OCCT viewer, a GLFW window must exist and it must be
        integrated with OCCT (in this case, GlfwOcctWindow inherits from OCCT's
        Aspect_Window class).
*/
void GlfwOcctView::init_viewer()
{
    // The display argument only needs to be non-null on Linux systems. I
    //     totally don't understand why this is the case.
    Handle(OpenGl_GraphicDriver) graphic_driver {new OpenGl_GraphicDriver(this->occt_window->get_display(), false)};

    Handle(V3d_Viewer) viewer {new V3d_Viewer(graphic_driver)};
    viewer->SetDefaultLights();
    viewer->SetLightOn();
    viewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
    viewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
    
    this->view = viewer->CreateView();
    this->view->SetImmediateUpdate(false);

    // This is the important step I think. It associates the context underlying
    //     the GLFW window with the OCCT viewer. Now OCCT knows where it should
    //     put data so that it shows up.
    Aspect_RenderingContext opengl_context {this->occt_window->opengl_context()};

    this->view->SetWindow(this->occt_window, opengl_context);
    
    this->view->ChangeRenderingParams().ToShowStats = true;

    context = new AIS_InteractiveContext(viewer);
}

void GlfwOcctView::main_loop()
{
    while (!glfwWindowShouldClose(this->occt_window->get_glfw_window()))
    {
        // Different rendering options.
        // glfwPollEvents();
        glfwWaitEvents();

        FlushViewEvents(this->context, this->view, true);
    }
}

void GlfwOcctView::cleanup()
{
    this->view->Remove();
    this->occt_window->close();
    glfwTerminate();
}

void GlfwOcctView::on_resize(int width, int height)
{
    if (width != 0 && height != 0)
    {
        this->view->Window()->DoResize();
        this->view->MustBeResized();
        this->view->Invalidate();
        this->view->Redraw();
    }
}

void GlfwOcctView::on_mouse_scroll(double offset_x, double offset_y)
{
    UpdateZoom(Aspect_ScrollDelta(occt_window->cursor_position(), int(offset_y * 8.0)));
}

void GlfwOcctView::on_mouse_button(int button, int action, int mods)
{
    const Graphic3d_Vec2i pos {this->occt_window->cursor_position()};
    if (action == GLFW_PRESS)
        PressMouseButton(pos, mouse_button_from_glfw(button), key_flags_from_glfw(mods), false);
    else
        ReleaseMouseButton(pos, mouse_button_from_glfw(button), key_flags_from_glfw(mods), false);
}

void GlfwOcctView::on_mouse_move(int pos_x, int pos_y)
{
    const Graphic3d_Vec2i aNewPos(pos_x, pos_y);
    UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), false);
}

/*
    Makes it easy to show shapes in a GUI. This function is blocking - it only
        returns when the user closes the GUI.
    
    Notes:
        Top level entry point to:
            (1) Create a GLFW window.
            (2) Create a GlfwOcctWindow object to wrap the GLFW window. 
            (3) Register callbacks with GLFW to ensure that the wrapping GlfwOcctWindow 
                    object stays in-sync with the underlying GLFW window 
            (4) Tell OCCT about the OpenGL context underlying the GLFW window (which 
                    makes it possible for the OCCT functions to write to the window). 
            (5) Create various OCCT convenience objects (AIS_InteractiveContext, 
                    V3D_Viewer, etc.) that (seemingly?!?) organize/coordinate OCCT's ability
                    to show things in the window.
            (6) Add the OCCT shapes to the window, going through OCCT's AIS_Interactive
                    Context.
            (7) Make the GLFW window visible.

    Arguments:
        shapes: Shapes to be displayed.

    Returns:
        None.
*/
void GlfwOcctView::show_shapes(const std::vector<TopoDS_Shape>& shapes)
{
    init_window(1200, 1200, "OCCT Visualization with GLFW and OpenGL.");
    init_viewer();

    this->view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.1, V3d_WIREFRAME);
    for (const auto& shape : shapes)
    {
        Handle(AIS_Shape) ais_shape {new AIS_Shape(shape)};
        this->context->Display(ais_shape, AIS_Shaded, 0, false);
    }
    
    this->view->MustBeResized();
    this->occt_window->Map();
    main_loop();
    cleanup(); 
}
