/**
 * @file example_view_dispatcher.c
 * @brief Example application demonstrating the usage of the ViewDispatcher library.
 *
 * This application can display one of two views: either a Widget or a Submenu.
 * Each view has its own way of switching to another one:
 *
 * - A center button in the Widget view.
 * - A submenu item in the Submenu view
 *
 * Press either to switch to a different view. Press Back to exit the application.
 *
 */

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
#include <gui/modules/submenu.h>

// Enumeration of the view indexes.
typedef enum {
    ViewIndexWidget,
    ViewIndexSubmenu,
    ViewIndexTextInput,
    ViewIndexPopup, // This is an example of a view that could be used for popups.
    ViewIndexCount,
} ViewIndex;

// Enumeration of the submenu items.
typedef enum {
    LedColorBlue,
    LedColorRed,
    LedColorGreen,
    LedColorDisable,
} LedColor;

// Enumeration of submenu items.
typedef enum {
    SubmenuIndexNothing,
    SubmenuIndexSwitchView,
    SubmenuIndexTextInput,
    SubmenuIndexPopup,
} SubmenuIndex;

// Main application structure.
typedef struct {
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    TextInput* text_input;
    Submenu* submenu;
    Popup* popup; // This is an example of a view that could be used for popups.
    char input_buffer[64]; //
} SocappViewDispatcherApp;

// This function is called when the user has pressed the Back key.
static bool socapp_navigation_callback(void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;
    // Back means exit the application, which can be done by stopping the ViewDispatcher.
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

// This function is called when there are custom events to process.
static bool socapp_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;
    // The event numerical value can mean different things (the application is responsible to uphold its chosen convention)
    // In this example, the only possible meaning is the view index to switch to.
    furi_assert(event < ViewIndexCount);
    // Switch to the requested view.
    view_dispatcher_switch_to_view(app->view_dispatcher, event);
    return true;
}

// This function is called when the user presses the "Switch View" button on the Widget view.
static void socapp_button_callback(
    GuiButtonType button_type,
    InputType input_type,
    void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;
    // Only request the view switch if the user short-presses the Center button.
    if(button_type == GuiButtonTypeCenter && input_type == InputTypeShort) {
        // Request switch to the Submenu view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexSubmenu);
    }
}

static void socapp_popup_callback(void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    // Reset the widget view.
    widget_reset(app->widget);
  
      widget_add_string_multiline_element(
        app->widget,
        64, 32,
        AlignCenter, AlignCenter,
        FontSecondary,
        "Retour depuis popup"
    );
    // Show the Popup view.
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexWidget);
}

static void socapp_textinput_callback(void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

     widget_reset(app->widget);

    // Utilise le texte du buffer
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, app->input_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexWidget);
}

// This function is called when the user activates the "Switch View" submenu item.
static void socapp_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;
    // Only request the view switch if the user activates the "Switch View" item.
    if(index == SubmenuIndexSwitchView) {
        // Request switch to the Widget view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexWidget);
    }

    if(index == SubmenuIndexTextInput) {
        // Request switch to the TextInput view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexTextInput);
    }

    if (index == SubmenuIndexPopup) {
        // Request switch to the Popup view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexPopup);
        // Create and show the Popup view.
    }
}

// Application constructor function.
static SocappViewDispatcherApp* socapp_alloc() {
    SocappViewDispatcherApp* app = malloc(sizeof(SocappViewDispatcherApp));
    memset(app, 0, sizeof(SocappViewDispatcherApp)); // Initialise toute la structure à zéro
    // Access the GUI API instance.
    Gui* gui = furi_record_open(RECORD_GUI);
    // Create the ViewDispatcher instance.
    app->view_dispatcher = view_dispatcher_alloc();
    // Let the GUI know about this ViewDispatcher instance.
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    
    // Create and initialize the Widget view.
    app->widget = widget_alloc();
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, "Press the Button below");
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,
        "Switch View",
        socapp_button_callback,
        app);
    app->text_input = text_input_alloc();
    text_input_set_header_text(app->text_input, "Entrez votre texte");   
    text_input_set_result_callback(app->text_input, socapp_textinput_callback,
        app,
        app->input_buffer,
        sizeof(app->input_buffer),
        true
    );

    app->popup = popup_alloc();
    popup_set_timeout(app->popup, 3000); // 2 secondes
    popup_enable_timeout(app->popup);    // active le timeout   
    popup_set_header(app->popup, "Popup Example", 10, 10, AlignCenter, AlignCenter);
    popup_set_text(app->popup, "This is an example of a popup view.",10, 10, AlignCenter, AlignCenter);
    popup_set_callback(app->popup, socapp_popup_callback);
    popup_set_context(app->popup, app);
    

        //view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexPopup);
    // Create and initialize the Submenu view.
    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "Popup", SubmenuIndexPopup, socapp_submenu_callback, app);
    submenu_add_item(app->submenu, "Do Nothing", SubmenuIndexNothing, NULL, NULL);
    submenu_add_item(
        app->submenu,
        "Switch View",
        SubmenuIndexSwitchView,
        socapp_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Text Input",
        SubmenuIndexTextInput,
        socapp_submenu_callback,
        app);

        
    // Register the views within the ViewDispatcher instance. This alone will not show any of them on the screen.
    // Each view must have its own index to refer to it later (it is best done via an enumeration as shown here).
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexWidget, widget_get_view(app->widget));
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexPopup, popup_get_view(app->popup));
    // Register the Submenu view.
    view_dispatcher_add_view(
        app->view_dispatcher, ViewIndexSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(
        app->view_dispatcher, ViewIndexTextInput, text_input_get_view(app->text_input));
    // Set the custom event callback. It will be called each time a custom event is scheduled
    // using the view_dispatcher_send_custom_callback() function.
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, socapp_custom_event_callback);
    // Set the navigation, or back button callback. It will be called if the user pressed the Back button
    // and the event was not handled in the currently displayed view.
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, socapp_navigation_callback);
    // The context will be passed to the callbacks as a parameter, so we have access to our application object.
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    return app;
}

// Application destructor function.
static void socapp_free(SocappViewDispatcherApp* app) {
    // All views must be un-registered (removed) from a ViewDispatcher instance
    // before deleting it. Failure to do so will result in a crash.
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexWidget);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexPopup);
    // Now it is safe to delete the ViewDispatcher instance.
    view_dispatcher_free(app->view_dispatcher);
    // Free the Popup view.
    popup_free(app->popup);
    // Delete the views
    text_input_free(app->text_input);
    widget_free(app->widget);
    submenu_free(app->submenu);
    // End access to hte the GUI API.
    furi_record_close(RECORD_GUI);
    // Free the remaining memory.
    free(app);
}

static void socapp_run(SocappViewDispatcherApp* app) {
    // Display the Widget view on the screen.
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexSubmenu);
    // This function will block until view_dispatcher_stop() is called.
    // Internally, it uses a FuriEventLoop (see FuriEventLoop examples for more info on this).
    view_dispatcher_run(app->view_dispatcher);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t socapp(void* arg) {
    UNUSED(arg);

    SocappViewDispatcherApp* app = socapp_alloc();
    socapp_run(app);
    socapp_free(app);

    return 0;
}
