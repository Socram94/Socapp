/**
 * @file example_view_dispatcher.c
 */

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/loading.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>
#include <gui/modules/number_input.h>
#include <gui/modules/submenu.h>

// Enumeration of the view indexes.
typedef enum {
    ViewIndexWidget,
    ViewIndexSubmenu,
    ViewIndexTextInput,
    ViewIndexNumberInput,
    ViewIndexLoading,
    ViewIndexPopup,
    ViewIndexCount,
} ViewIndex;

// Custom events (au-dessus de ViewIndexCount pour éviter la confusion)
typedef enum {
    EventShowLoading = 100,
} CustomEvent;

// Enumeration of submenu items.
typedef enum {
    SubmenuIndexNothing,
    SubmenuIndexNumberInput,
    SubmenuIndexSwitchView,
    SubmenuIndexTextInput,
} SubmenuIndex;

// Main application structure.
typedef struct {
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    TextInput* text_input;
    Submenu* submenu;
    NumberInput* number_input;
    Popup* popup;
    Loading* loading;
    int32_t current_number; 
    char input_buffer[64];
} SocappViewDispatcherApp;

// Back button callback
static bool socapp_navigation_callback(void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

// Custom event callback
static bool socapp_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    if(event < ViewIndexCount) {
        view_dispatcher_switch_to_view(app->view_dispatcher, event);
    } else {
        switch(event) {
            case EventShowLoading:

                view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexLoading);

                // Et ajouter un petit timer pour revenir au Submenu après 2s
                furi_delay_ms(2000);
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexSubmenu);
                break;
        }
    }
    return true;
}

// Button callback → affiche juste le popup
static void socapp_button_callback(
    GuiButtonType button_type,
    InputType input_type,
    void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    if(button_type == GuiButtonTypeCenter && input_type == InputTypeShort) {
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexPopup);
    }
}

static void scocapp_numberinput_callback(void *context, int32_t number) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    widget_reset(app->widget);
    itoa(number, app->input_buffer, 10);
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, app->input_buffer);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexWidget);
}

// Popup timeout callback → envoie un event custom
static void socapp_popup_callback(void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, EventShowLoading);
}

// Text input callback
static void socapp_textinput_callback(void* context) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    widget_reset(app->widget);
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, app->input_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexWidget);
}

// Submenu callback
static void socapp_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    SocappViewDispatcherApp* app = context;

    if(index == SubmenuIndexSwitchView) {
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexWidget);
    }

    if(index == SubmenuIndexTextInput) {
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexTextInput);
    }

    if(index == SubmenuIndexNumberInput) {
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexNumberInput);
    }
    
}

// Application constructor
static SocappViewDispatcherApp* socapp_alloc() {
    SocappViewDispatcherApp* app = malloc(sizeof(SocappViewDispatcherApp));
    memset(app, 0, sizeof(SocappViewDispatcherApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    // Widget
    app->widget = widget_alloc();
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, "Press the Button below");
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,
        "Switch View",
        socapp_button_callback,
        app);

    // Loading
    app->loading = loading_alloc();

    // Popup
    app->popup = popup_alloc();
    popup_set_context(app->popup, app);
    popup_set_header(app->popup, "Popup Socapp", 64, 10, AlignCenter, AlignTop);
    popup_set_text(app->popup,
        "Socapp popup\nTest",
        64, 30, AlignCenter, AlignCenter);
    popup_set_timeout(app->popup, 2000);   // 2s
    popup_enable_timeout(app->popup);
    popup_set_callback(app->popup, socapp_popup_callback);

    // Text input
    app->text_input = text_input_alloc();
    text_input_set_header_text(app->text_input, "Entrez votre texte");   
    text_input_set_result_callback(
        app->text_input,
        socapp_textinput_callback,
        app,
        app->input_buffer,
        sizeof(app->input_buffer),
        true);

    // Number input
    app->number_input = number_input_alloc();
    number_input_set_header_text (app->number_input, "Choissiez un nombre");
    number_input_set_result_callback (
        app->number_input, 
        scocapp_numberinput_callback, 
        app, 
        app->current_number, 
        0, 
        100);

    // Submenu
    app->submenu = submenu_alloc();
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
    submenu_add_item(
        app->submenu,
        "Number Input",
        SubmenuIndexNumberInput,
        socapp_submenu_callback,
        app);



    // Register views
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexWidget, widget_get_view(app->widget));
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexTextInput, text_input_get_view(app->text_input));
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexPopup, popup_get_view(app->popup));
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexLoading, loading_get_view(app->loading));
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexNumberInput, number_input_get_view(app->number_input));

    // Callbacks
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, socapp_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, socapp_navigation_callback);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    return app;
}

// Destructor
static void socapp_free(SocappViewDispatcherApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexWidget);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexLoading);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexPopup);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexNumberInput);

    view_dispatcher_free(app->view_dispatcher);

    text_input_free(app->text_input);
    popup_free(app->popup);
    loading_free(app->loading);
    number_input_free(app->number_input);
    widget_free(app->widget);
    submenu_free(app->submenu);

    furi_record_close(RECORD_GUI);
    free(app);
}

// Run
static void socapp_run(SocappViewDispatcherApp* app) {
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexSubmenu);
    view_dispatcher_run(app->view_dispatcher);
}

// Entry point
int32_t socapp(void* arg) {
    UNUSED(arg);

    SocappViewDispatcherApp* app = socapp_alloc();
    socapp_run(app);
    socapp_free(app);

    return 0;
}
