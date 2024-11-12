// Standard Library.
#include <iostream>

// Third party.

// OCCT
#include "AIS_Shape.hxx"
#include "Aspect_DisplayConnection.hxx"
#include "BRepPrimAPI_MakeBox.hxx"
#include "BRepPrimAPI_MakeCone.hxx"
#include "Message.hxx"
#include "Message_Messenger.hxx"
#include "OpenGl_GraphicDriver.hxx"

// GLFW
#include "GLFW/glfw3.h"

// Library private.
#include "glfw_occt_view_p.hxx"

/* 
   ============================================================================
                            File Local Declarations
   ============================================================================ 
*/ 

static Aspect_VKeyMouse mouse_button_from_glfw(int button);
static Aspect_VKeyFlags key_flags_from_glfw(int flags);

/* 
   ============================================================================
                            File Local Definitions 
   ============================================================================ 
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
   ============================================================================
                                    GlfwOcctView 
   ============================================================================ 
*/ 

/*
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
        (6) Make the GLFW window visible.

    Notably, the main loop is not started. The user is responsible for kicking
        off the main loop. Typically, the caller would kick off the main loop
        after adding some shapes to the exposed AIS_InteractiveContext object 
        and making some changes to the exposed V3d_View object.
*/
GlfwOcctView::GlfwOcctView()
{
    init_window(1200, 1200, "OCCT Visualization with GLFW and OpenGL.");
    init_viewer();
    view->MustBeResized();
    occt_window->Map();
}

GlfwOcctView::~GlfwOcctView()
{
    cleanup(); 
}

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
    Message::DefaultMessenger()->Send(TCollection_AsciiString("Error") + error + ": " + description, Message_Fail);
}

/*
    Creates a GLFW window and associates a wrapping GlfwOcctWindow object
        with it, by using the glfwSetWindowUserPointer() function.
*/
void GlfwOcctView::init_window(int width, int height, const char* title)
{
    glfwSetErrorCallback(GlfwOcctView::error_callback);
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // Only necessary on MacOS due to old system version of OpenGL? 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create a OCCT-native data structure, GlfwOcctWindow, that has characteristics
    //     which mirror the GLFW window, and associate it with the GLFW window.
    occt_window = new GlfwOcctWindow(width, height, title);
    glfwSetWindowUserPointer(occt_window->get_glfw_window(), this);
    
    // Set callbacks for various actions.
    glfwSetWindowSizeCallback(occt_window->get_glfw_window(), GlfwOcctView::resize_callback);
    glfwSetFramebufferSizeCallback(occt_window->get_glfw_window(), GlfwOcctView::fb_resize_callback);
    glfwSetScrollCallback(occt_window->get_glfw_window(), GlfwOcctView::mouse_scroll_callback);
    glfwSetMouseButtonCallback(occt_window->get_glfw_window(), GlfwOcctView::mouse_button_callback);
    glfwSetCursorPosCallback(occt_window->get_glfw_window(), GlfwOcctView::mouse_move_callback);
}

/*
    Creates an OCCT viewer with an OCCT AIS context. 
    
    In order to create the OCCT viewer, a GLFW window must exist and it must be
        integrated with OCCT (in this case, GlfwOcctWindow inherits from OCCT's
        Aspect_Window class).
*/
void GlfwOcctView::init_viewer()
{
    // Is it actually necessary to pass the theDisp argument?
    Handle(OpenGl_GraphicDriver) graphic_driver {new OpenGl_GraphicDriver(occt_window->get_display(), false)};

    Handle(V3d_Viewer) viewer {new V3d_Viewer(graphic_driver)};
    viewer->SetDefaultLights();
    viewer->SetLightOn();
    viewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
    viewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

    view = viewer->CreateView();
    view->SetImmediateUpdate(false);

    // This is the important step I think. It associates the context underlying
    //     the GLFW window with the OCCT viewer. Now OCCT knows where it should
    //     put data so that it shows up.
    view->SetWindow(occt_window, occt_window->opengl_context());
    view->ChangeRenderingParams().ToShowStats = true;

    context = new AIS_InteractiveContext(viewer);
}

/*
    Creates a demo scene by using an AIS_InteractiveContext that is assumed to
        exist (and be associated with a V3D_Viewer).
*/
void GlfwOcctView::init_demo_scene()
{
    view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_WIREFRAME);

    gp_Ax2 axis;
    axis.SetLocation(gp_Pnt(0.0, 0.0, 0.0));
    Handle(AIS_Shape) box = new AIS_Shape(BRepPrimAPI_MakeBox(axis, 50, 50, 50).Shape());
    context->Display(box, AIS_Shaded, 0, false);
    axis.SetLocation(gp_Pnt(25.0, 125.0, 0.0));
    Handle(AIS_Shape) cone = new AIS_Shape(BRepPrimAPI_MakeCone(axis, 25, 0, 50).Shape());
    context->Display(cone, AIS_Shaded, 0, false);

    TCollection_AsciiString gl_info;
    TColStd_IndexedDataMapOfStringString rend_info;
    view->DiagnosticInformation(rend_info, Graphic3d_DiagnosticInfo_Basic);
    for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(rend_info); aValueIter.More(); aValueIter.Next())
    {
        if (!gl_info.IsEmpty()) { gl_info += "\n"; }
        gl_info += TCollection_AsciiString("  ") + aValueIter.Key() + ": " + aValueIter.Value();
    }

    Message::DefaultMessenger()->Send(TCollection_AsciiString("OpenGL info:\n") + gl_info, Message_Info);
}

void GlfwOcctView::main_loop()
{
    while (!glfwWindowShouldClose(occt_window->get_glfw_window()))
    {
        // Different rendering options.
        // glfwPollEvents();
        glfwWaitEvents();

        FlushViewEvents(context, view, true);
    }
}

void GlfwOcctView::cleanup()
{
    if (!view.IsNull())
    {
        view->Remove();
    }
    if (!occt_window.IsNull())
    {
        occt_window->close();
    }
    glfwTerminate();
}

void GlfwOcctView::on_resize(int width, int height)
{
    if (width != 0 && height != 0)
    {
        view->Window()->DoResize();
        view->MustBeResized();
        view->Invalidate();
        view->Redraw();
    }
}

void GlfwOcctView::on_mouse_scroll(double offset_x, double offset_y)
{
    UpdateZoom(Aspect_ScrollDelta(occt_window->cursor_position(), int(offset_y * 8.0)));
}

void GlfwOcctView::on_mouse_button(int button, int action, int mods)
{
    const Graphic3d_Vec2i aPos {occt_window->cursor_position()};
    if (action == GLFW_PRESS)
        PressMouseButton(aPos, mouse_button_from_glfw(button), key_flags_from_glfw(mods), false);
    else
        ReleaseMouseButton(aPos, mouse_button_from_glfw(button), key_flags_from_glfw(mods), false);
}

void GlfwOcctView::on_mouse_move(int pos_x, int pos_y)
{
    const Graphic3d_Vec2i aNewPos(pos_x, pos_y);
    UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), false);
}

/* 
   ============================================================================
                          Commonly Used With GlfwOcctView                                 
   ============================================================================ 
*/ 

/*
    Convenience function for suing GlfwOcctView. Creates a GlfwOcctView with
        a simple configuration, puts all the shapes into the view, and then
        kicks off the main loop. This function only returns when the user closes
        the window.

    Arguments:
        shapes: Shapes to be displayed.
*/
void show_shapes(const std::vector<TopoDS_Shape>& shapes)
{
    GlfwOcctView glfw_occt_view; 

    glfw_occt_view.view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_WIREFRAME);
    
    for (auto& shape : shapes)
    {
        Handle(AIS_Shape) ais_shape {new AIS_Shape(shape)};
        glfw_occt_view.context->Display(ais_shape, AIS_Shaded, 0, false);
    }
    
    glfw_occt_view.main_loop();
}
