#include <socapp_icons.h>                       // (Optionnel ici) Sert si tu utilises des icônes générées par le SDK
#include <gui/gui.h>                             // Pour l'affichage à l'écran
#include <input/input.h>                         // Pour gérer les entrées des boutons
#include <furi.h>                                // Fonctions de base du firmware Flipper
#include <notification/notification.h>           // Gestion des notifications (LED, vibration, etc.)
#include <gui/modules/submenu.h>                  // Pour les sous-menus (optionnel ici)
#include <notification/notification_messages.h>  // Séquences de notifications pré-définies (couleurs, etc.)

#define TAG "socapp" // Tag utilisé pour le debug/log (pas obligatoire)

// Structure qui contient toutes les données de notre application
typedef struct {
    FuriMessageQueue* input_queue;   // File de messages pour stocker les événements de touches
    ViewPort* view_port;             // Zone d'affichage
    Gui* gui;                        // Interface graphique
    NotificationApp* notification;   // Gestion des LEDs / vibrations
    Submenu* submenu;               // (Optionnel) Pour les sous-menus
} socapp;

// ---------- FONCTION D'AFFICHAGE ----------
// Dessine "Hello Flipper!" au centre de l'écran
void draw_callback(Canvas* canvas, void* context) {
    UNUSED(context); // On ne se sert pas du "context" ici

    canvas_clear(canvas); // Efface l'écran

}

// ---------- FONCTION D'ENTRÉE ----------
// Appelée automatiquement lorsqu'un bouton est pressé
void input_callback(InputEvent* event, void* context) {
    socapp* app = context; // On récupère notre structure principale
    // On met l'événement "event" dans la file d'attente input_queue
    furi_message_queue_put(app->input_queue, event, 0);
}


// ---------- FONCTION PRINCIPALE ----------
// C'est le "point d'entrée" de l'application sur le Flipper Zero
int32_t socapp_main(void* p) {
    UNUSED(p); // On ignore ce paramètre

    // Création de la structure de l'app
    socapp app;

    app.submenu = submenu_alloc(); // (Optionnel) Alloue de la mémoire pour un sous-menu

    submenu_add_item(app.submenu, "Led Verte", 0)

    // 1. Crée une "view port" (zone d'affichage)
    app.view_port = view_port_alloc();

    // 2. Crée une file d'attente pour 8 événements (de type InputEvent)
    app.input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // 3. Ouvre le module "notification" pour contrôler LED/vibration
    app.notification = furi_record_open("notification");

    // 4. Définit la fonction de dessin pour notre view port
    view_port_draw_callback_set(app.view_port, draw_callback, &app);

    // 5. Définit la fonction appelée lors d'un appui de bouton
    view_port_input_callback_set(app.view_port, input_callback, &app);

    // 6. Ouvre le module graphique (GUI)
    app.gui = furi_record_open("gui");

    // 7. Ajoute notre view port à l'écran, en plein écran (GuiLayerFullscreen)
    gui_add_view_port(app.gui, app.view_port, GuiLayerFullscreen);

    // Variable pour stocker les événements de touches
    InputEvent input;

    // Variable de boucle principale
    bool running = true;

    // ---------- BOUCLE PRINCIPALE ----------
   /* while(running) {
        // Attente d'un événement de touche (infini : FuriWaitForever)
        if(furi_message_queue_get(app.input_queue, &input, FuriWaitForever) == FuriStatusOk) {
            // Analyse de la touche pressée
            switch(input.key) {
                case InputKeyLeft:
                    // Allume LED en vert
                    notification_message(app.notification, &sequence_set_only_green_255);
                    break;
                case InputKeyRight:
                    // Allume LED en rouge
                    notification_message(app.notification, &sequence_set_only_red_255);
                    break;
                case InputKeyOk:
                    // Allume LED en bleu
                    notification_message(app.notification, &sequence_set_only_blue_255);
                    break;
                case InputKeyUp:
                case InputKeyDown:
                case InputKeyBack:
                    // Éteint LED
                    notification_message(app.notification, &sequence_empty);
                    // Arrête la boucle (quitte l'app)
                    running = false;
                    break;
                default:
                    break;
            }
            view_port_update(app.view_port);
        }
    }
    */
    // ---------- NETTOYAGE AVANT SORTIE ----------

    notification_message(app.notification, &sequence_reset_red); // Réinitialise les LEDs
    notification_message(app.notification, &sequence_reset_blue); // Réinitialise les LEDs
    notification_message(app.notification, &sequence_reset_green); // Réinitialise les LEDs
    // Désactive notre view port
    view_port_enabled_set(app.view_port, false);

    // Retire la view port de l'écran
    gui_remove_view_port(app.gui, app.view_port);

    // Ferme le module graphique
    furi_record_close("gui");

    // Ferme le module de notification
    furi_record_close("notification");

    // Libère la mémoire de la view port
    view_port_free(app.view_port);

    submenu_free(app.submenu); // (Optionnel) Libère la mémoire du sous-menu

    // Libère la mémoire de la file d'attente
    furi_message_queue_free(app.input_queue);

    // Retourne 0 = succès
    return 0;
}
