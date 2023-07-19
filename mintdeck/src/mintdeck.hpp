/// @file mintdeck.hpp
#ifndef MINTDECK_H
#define MINTDECK_H

#include "deck.hpp"
/// @mainpage MintDeck Documentation
///
/// MintDeck is a software tool to manage Stream Deck devices on a Linux system.
///
/// An overview of the features of MintDeck and installation instructions follow.
///
/// @section intro_sec Introduction
///
/// A Stream Deck device manager.
/// 
/// Mintdeck is a G.U.I. for Linux systems to operate Stream Deck USB devices.  Mintdeck is written in C++, and is loaded to the desktop, hidden, and shutdown using a Python script accessed by clicking a system tray icon.
///
/// Stream Deck key definitions are loaded and programed using the G.U.I.
/// 
/// ...
namespace decklibrary {
    class deck;
}

class MintDeck {
public:
    MintDeck();
    static gboolean stoptimer;
    static void drawtiles();
    static void savebuttonsetas();
    static void deletebuttonset();
    static void setnewbuttonimage(gchar* filename, int btn);
    void run();
    static void recordkeystring(int btn);
    static void editkeystring(int btn);
    static void loadbuttonimage(int btn);
    static GtkTextBuffer* gettextbuffer();
    static void messagedialog(std::string dialogname, std::string message);
private:
    static GtkWidget* window;
    static GtkWidget* vbox;
    static GtkWidget* hbox;
    static GtkTextBuffer* text_buffer;
    static int animationindex;
    static GtkWidget* animationwidget;
    static std::vector<std::vector<unsigned char>> animation;
    static decklibrary::deck device;
    static void windowInit(GtkWidget* window);
    static void deckMenu();
    static void onBrightnessActivated(gpointer data);
    void setDeckBrightness();
    static gboolean ondrawcallbacktimer(gpointer data);
    static gboolean ondraw(GtkWidget *widget, cairo_t *cr, gpointer data);
    static void textdialog1(std::string dialogname, int width, std::string buttonname, void (*callbackfunction)(GtkWidget*, GtkWidget*));
    static void textdialog2(std::string dialogname, int width, std::string buttonname1, std::string buttonname2, void (*callbackfunction)(GtkWidget*, GtkWidget*));
    static void setevemumode();
    static void verifydevnumber(GtkWidget *widget, GtkWidget *devnumber);
    static void loadbackgroundimage();
    static void setnewbackgroundimage(gchar* filename);
};

#endif
