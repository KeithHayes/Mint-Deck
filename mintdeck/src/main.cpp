/// @file main.cpp
#include "mintdeck.hpp"
///
/// @brief The main function.
///
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-theme-name", "Mint-Deck", NULL);
    MintDeck mintDeck;
    mintDeck.run();
    return 0;
}
