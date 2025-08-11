#include <socapp_icons.h>
#include <gui/gui.h>
#include <input/input.h>
#include <furi.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#define TAG "socapp"

typedef struct {
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;
    NotificationApp* notification;
} socapp;

void draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Hello Flipper!");
}

void input_callback(InputEvent* event, void* context) {
    socapp* app = context;
    furi_message_queue_put(app->input_queue, event, 0);
}

int32_t socapp_main(void* p) {
    UNUSED(p);

    socapp app;
    app.view_port = view_port_alloc();
    app.input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app.notification = furi_record_open("notification"); // ⚠ IMPORTANT sinon crash

    view_port_draw_callback_set(app.view_port, draw_callback, &app);
    view_port_input_callback_set(app.view_port, input_callback, &app);

    app.gui = furi_record_open("gui");
    gui_add_view_port(app.gui, app.view_port, GuiLayerFullscreen);

    InputEvent input;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(app.input_queue, &input, FuriWaitForever) == FuriStatusOk) {
            switch(input.key) {
                case InputKeyLeft:
                    notification_message(app.notification, &sequence_set_only_green_255);
                    break;
                case InputKeyRight:
                    notification_message(app.notification, &sequence_set_only_red_255);
                    break;
                case InputKeyOk:
                    notification_message(app.notification, &sequence_set_only_blue_255);
                    break;
                case InputKeyUp:
                case InputKeyDown:
                case InputKeyBack:
                    notification_message(app.notification, &sequence_empty); // éteint LED
                    running = false; // quitte l'app
                    break;
                default:
                    break;
            }
            view_port_update(app.view_port);
        }
    }

    // Nettoyage
    view_port_enabled_set(app.view_port, false);
    gui_remove_view_port(app.gui, app.view_port);
    furi_record_close("gui");
    furi_record_close("notification"); // fermeture du handle
    view_port_free(app.view_port);
    furi_message_queue_free(app.input_queue);

    return 0;
}
