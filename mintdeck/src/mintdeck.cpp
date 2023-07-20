/// @file mintdeck.cpp
#include "deck.hpp"

GtkWidget* MintDeck::window = nullptr;
GtkWidget* MintDeck::vbox = nullptr;
GtkWidget* MintDeck::hbox = nullptr;
decklibrary::deck MintDeck::device;
GtkTextBuffer* MintDeck::text_buffer;
int MintDeck::animationindex = 0;
GtkWidget* MintDeck::animationwidget = nullptr;;
std::vector<std::vector<unsigned char>> MintDeck::animation;
gboolean  MintDeck::stoptimer = true;

MintDeck::MintDeck() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    windowInit(window);
    const int numImages = 12;
    std::vector<std::vector<unsigned char>> images;
    std::string cassettePath = "data/ec72df53f5.bin";
    std::ifstream cassette(cassettePath, std::ios::binary);
    for (int i = 0; i < numImages; ++i) {
        std::streampos imageSize;
        cassette.read(reinterpret_cast<char*>(&imageSize), sizeof(imageSize));
        std::vector<unsigned char> image(imageSize);
        if (cassette.read(reinterpret_cast<char*>(image.data()), imageSize)) {
            images.push_back(image);
        }
    }
    cassette.close();
    animationindex = 0;
    animation = images;
}
void MintDeck::drawtiles() {
    int counter = 0;
    GtkWidget *button;
    GList *children = gtk_container_get_children(GTK_CONTAINER(vbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_container_remove(GTK_CONTAINER(vbox), GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    deckMenu();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    for (int i = 0; i < device.getbtncount(); i++) {
        button = gtk_button_new();
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        GtkWidget *image = device.getkeyimage(i);
        gtk_button_set_image(GTK_BUTTON(button), image);
        g_signal_connect(button, "clicked", G_CALLBACK(decklibrary::deck::mousebtnclick), GINT_TO_POINTER(i));
        counter++;
        if (counter % 5 == 0) {
            hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        }
        if (counter == 15) {
            break;
        }
    }
    gtk_widget_show_all(window);
}
GtkTextBuffer* MintDeck::gettextbuffer() {
    return text_buffer;
}
void MintDeck::messagedialog(std::string title, std::string message) {
    GtkWidget* dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(content_area), box);
    message = "\n   " + message + "   \n";
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);
}
void MintDeck::textdialog1(std::string dialogname, int width, std::string buttonname, void (*callbackfunction)(GtkWidget*, GtkWidget*)) {
    GtkWidget* dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), dialogname.c_str());
    gtk_window_set_default_size(GTK_WINDOW(dialog), width, 100);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), nullptr);
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    GtkWidget* margin_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(margin_box, 7);
    gtk_widget_set_margin_end(margin_box, 7);
    gtk_box_pack_start(GTK_BOX(vbox), margin_box, TRUE, TRUE, 0);
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(margin_box), hbox, FALSE, FALSE, 0);
    GtkWidget* dialogbutton = gtk_button_new_with_label(buttonname.c_str());
    gtk_widget_set_size_request(dialogbutton, -1, -1);
    gtk_box_pack_end(GTK_BOX(hbox), dialogbutton, FALSE, FALSE, 0);
    GtkWidget* spacing_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacing_box, TRUE);
    gtk_widget_set_vexpand(spacing_box, TRUE);
    gtk_box_pack_end(GTK_BOX(hbox), spacing_box, FALSE, FALSE, 0);
    GtkWidget* textinput = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(margin_box), textinput, FALSE, FALSE, 0);
    g_signal_connect(dialogbutton, "clicked", G_CALLBACK(callbackfunction), textinput);
    GtkAllocation allocation;
    gtk_widget_get_allocation(dialog, &allocation);
    int button_width = allocation.width / 3;
    gtk_widget_set_size_request(dialogbutton, button_width, -1);
    gtk_widget_set_halign(dialogbutton, GTK_ALIGN_END);
    gtk_widget_grab_focus(textinput);
    gtk_widget_show_all(dialog);
}
void MintDeck::textdialog2(std::string dialogname, int width, std::string buttonname1, std::string buttonname2, void (*callbackfunction)(GtkWidget*, GtkWidget*)) {
    GtkWidget* dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), dialogname.c_str());
    gtk_window_set_default_size(GTK_WINDOW(dialog), width, 100);
    g_signal_connect(dialog, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), nullptr);
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    GtkWidget* margin_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(margin_box, 7);
    gtk_widget_set_margin_end(margin_box, 7);
    gtk_box_pack_start(GTK_BOX(vbox), margin_box, TRUE, TRUE, 0);
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(margin_box), hbox, FALSE, FALSE, 0);
    GtkWidget* dialogbutton2 = gtk_button_new_with_label(buttonname2.c_str());
    gtk_widget_set_size_request(dialogbutton2, -1, -1);
    gtk_box_pack_end(GTK_BOX(hbox), dialogbutton2, FALSE, FALSE, 0);
    GtkWidget* dialogbutton1 = gtk_button_new_with_label(buttonname1.c_str());
    gtk_widget_set_size_request(dialogbutton1, -1, -1);
    gtk_box_pack_start(GTK_BOX(hbox), dialogbutton1, FALSE, FALSE, 0);
    GtkWidget* spacing_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacing_box, TRUE);
    gtk_widget_set_vexpand(spacing_box, TRUE);
    gtk_box_pack_end(GTK_BOX(hbox), spacing_box, FALSE, FALSE, 0);
    GtkWidget* textinput = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(margin_box), textinput, FALSE, FALSE, 0);
    g_signal_connect(dialogbutton1, "clicked", G_CALLBACK(callbackfunction), textinput);
    g_signal_connect(dialogbutton2, "clicked", G_CALLBACK(callbackfunction), textinput);
    GtkAllocation allocation;
    gtk_widget_get_allocation(dialog, &allocation);
    int button_width = allocation.width / 3;
    gtk_widget_set_size_request(dialogbutton1, button_width, -1);
    gtk_widget_set_size_request(dialogbutton2, button_width, -1);
    gtk_widget_set_halign(dialogbutton1, GTK_ALIGN_END);
    gtk_widget_set_halign(dialogbutton2, GTK_ALIGN_END);
    gtk_widget_grab_focus(textinput);
    gtk_widget_show_all(dialog);
}
void MintDeck::savebuttonsetas() {
    textdialog1("Name the Keyset", 300, "Save", decklibrary::keyfiles::savebuttonsetas);
}
void MintDeck::deletebuttonset() {
    textdialog1("Name to Delete", 300, "Delete", decklibrary::keyfiles::deletebuttonset);
}


gboolean MintDeck::ondrawcallbacktimer(gpointer data) {
    animationindex = (animationindex + 1) % 12;
    if (animationwidget && GTK_IS_WIDGET(animationwidget)) {
        gtk_widget_queue_draw(animationwidget);
    }
    if(!stoptimer) return G_SOURCE_CONTINUE; // Keep the timer running
    else return G_SOURCE_REMOVE; // Stop the timer
}
gboolean MintDeck::ondraw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    std::vector<unsigned char>& image = animation[animationindex];
    GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, reinterpret_cast<const guint8*>(image.data()), image.size(), nullptr);
    GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
    gdk_pixbuf_loader_close(loader, nullptr);
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    cairo_paint(cr);
    g_object_unref(pixbuf);
    return FALSE;
}
void MintDeck::recordkeystring(int btn) {
    if (animationwidget && GTK_IS_WIDGET(animationwidget)) {
        gtk_widget_destroy(animationwidget);
        animationwidget = nullptr;
    }
    animationindex = 0;
    GtkDialog *savedialog = GTK_DIALOG(gtk_dialog_new());
    animationwidget = GTK_WIDGET(savedialog);
    gtk_window_set_title(GTK_WINDOW(savedialog), "Close to End");
    gtk_window_set_default_size(GTK_WINDOW(savedialog), 300, 186);
    gtk_container_set_border_width(GTK_CONTAINER(savedialog), 10);
    GtkWidget *drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawingArea, 300, 186);
    GtkWidget *contentArea = gtk_dialog_get_content_area(savedialog);
    gtk_container_add(GTK_CONTAINER(contentArea), drawingArea);
    g_signal_connect(drawingArea, "draw", G_CALLBACK(ondraw), nullptr);
    gtk_widget_add_events(GTK_WIDGET(savedialog), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
    g_signal_connect(savedialog, "key-press-event", G_CALLBACK(decklibrary::deck::onKeyPress), GINT_TO_POINTER(btn));
    g_signal_connect(savedialog, "key-release-event", G_CALLBACK(decklibrary::deck::onKeyRelease), GINT_TO_POINTER(btn));
    g_signal_connect(savedialog, "response", G_CALLBACK(decklibrary::deck::savekeystring), NULL);
    stoptimer = false;
    g_timeout_add(41, ondrawcallbacktimer, nullptr);
    gtk_widget_show_all(GTK_WIDGET(savedialog));
    gtk_dialog_run(savedialog);
}


void MintDeck::editkeystring(int btn) {
    if (animationwidget && GTK_IS_WIDGET(animationwidget)) {
        gtk_widget_destroy(animationwidget);
        animationwidget = nullptr;
    }
    std::string keystring = decklibrary::deck::getbuttonkeys(btn);
    GtkDialog* editdialog = GTK_DIALOG(gtk_dialog_new());
    gtk_window_set_title(GTK_WINDOW(editdialog), ("Button " + std::to_string(btn+1)).c_str());
    gtk_window_set_default_size(GTK_WINDOW(editdialog), 350, 165);
    g_signal_connect(editdialog, "delete-event", G_CALLBACK(decklibrary::deck::setkeystring), GINT_TO_POINTER(btn));
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(editdialog)), vbox);
    GtkWidget* scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    GtkWidget* text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(text_view), 5);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(text_buffer, keystring.c_str(), -1);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_show_all(GTK_WIDGET(editdialog));
    gtk_dialog_run(editdialog);
}
void MintDeck::loadbuttonimage(int btn) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        setnewbuttonimage(filename, btn);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}
void MintDeck::setnewbuttonimage(gchar* filename, int btn) {
    GError* error = nullptr;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (error != nullptr) {
        messagedialog("Error Loading Image",error->message);
        g_error_free(error);
    }
    if (pixbuf != nullptr) {
        int width, height;
        width = gdk_pixbuf_get_width(pixbuf);
        height = gdk_pixbuf_get_height(pixbuf);
        if(width!=height) {
            messagedialog("Image Aspect Ratio","Buttons are square.");
        } else {
            pixbuf = gdk_pixbuf_scale_simple(pixbuf, 96, 96, GDK_INTERP_BILINEAR);
            decklibrary::deck::setnewimage(pixbuf,btn);
            drawtiles();
        }
        g_object_unref(pixbuf);
    }
}
void MintDeck::setnewbackgroundimage(gchar* filename) {
    GError* error = nullptr;
    GdkPixbuf* filebuffer = gdk_pixbuf_new_from_file(filename, &error);
    GdkPixbuf* buttonbuf;
    GdkPixbuf* trimmedbuf;
    GdkPixbuf* resizedbuf;
    if (error != nullptr) {
        messagedialog("Error Loading Image", error->message);
        g_error_free(error);
    }
    if (filebuffer != nullptr) {
        int meshrows = 3;
        int meshcols = 5;
        int width = gdk_pixbuf_get_width(filebuffer);
        int height = gdk_pixbuf_get_height(filebuffer);
        int borderwidth = width / 50;
        int newwidth = 2 * borderwidth + width;
        int newheight = 2 * borderwidth + height;
        int buttonimagewidth = width / meshcols;
        int buttonimageheight = height / meshrows;
        GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, newwidth, newheight);
        gdk_pixbuf_fill(pixbuf, 0xFF000000);
        gdk_pixbuf_copy_area(filebuffer, 0, 0, width, height, pixbuf, borderwidth, borderwidth);
        std::vector<GdkPixbuf*> imagetiles;
        for (int row = 0; row < meshrows; row++) {
            for (int col = 0; col < meshcols; col++) {
                int x = col * buttonimagewidth;
                int y = row * buttonimageheight;
                buttonbuf = gdk_pixbuf_new_subpixbuf(pixbuf, x + borderwidth, y + borderwidth, buttonimagewidth, buttonimageheight);
                int width = gdk_pixbuf_get_width(buttonbuf);
                int trimSize = width / 10;
                trimmedbuf = gdk_pixbuf_new_subpixbuf(buttonbuf, trimSize, trimSize, width - 2 * trimSize, width - 2 * trimSize);
                resizedbuf = gdk_pixbuf_scale_simple(trimmedbuf, 96, 96, GDK_INTERP_BILINEAR);
                int height = gdk_pixbuf_get_height(resizedbuf);
                int rowstride = gdk_pixbuf_get_rowstride(resizedbuf);
                const unsigned char* pixels = gdk_pixbuf_read_pixels(resizedbuf);
                std::vector<unsigned char> pixmap(pixels, pixels + height * rowstride);
                imagetiles.push_back(resizedbuf);
            }
        }
        decklibrary::deck::loadbbackgroundimage(imagetiles);
        drawtiles();
    }
    g_object_unref(resizedbuf);
    g_object_unref(trimmedbuf);
    g_object_unref(buttonbuf);
    g_object_unref(filebuffer);
}
void MintDeck::run() {
    device.init(window);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    drawtiles();
    gtk_main();
}
void MintDeck::windowInit(GtkWidget* window) {
    gtk_window_set_title(GTK_WINDOW(window), "Mint Deck++");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 500, 308);
}
void MintDeck::deckMenu() {
    GtkWidget *menu_bar;
    GtkWidget *keyset_menu;
    GtkWidget *keyset_item;
    GtkWidget *action_menu;
    GtkWidget *action_item;
    GtkWidget *mode_menu;
    GtkWidget *mode_item;
    // The menu bar.
    menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    // The keyset menu.  Add to the menu bar.
    keyset_menu = gtk_menu_new();
    keyset_item = gtk_menu_item_new_with_label("keyset");
    gtk_menu_item_set_label(GTK_MENU_ITEM(keyset_item), "Keysets");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), keyset_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(keyset_item), keyset_menu);
    // The mode menu.  Add to the menu bar.
    mode_menu = gtk_menu_new();
    mode_item = gtk_menu_item_new_with_label("mode");
    gtk_menu_item_set_label(GTK_MENU_ITEM(mode_item), "Mode");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), mode_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mode_item), mode_menu);    
    // The action menu.  Add to the menu bar.
    action_menu = gtk_menu_new();
    action_item = gtk_menu_item_new_with_label("action");
    gtk_menu_item_set_label(GTK_MENU_ITEM(action_item), "Action");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), action_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(action_item), action_menu);
    // keyset menu  items.
    std::vector<std::string> setnames = decklibrary::keyfiles::getbuttonsets();
    for (const std::string& setname : setnames) {
        GtkWidget* menuentry = gtk_menu_item_new_with_label(setname.c_str());
        gtk_menu_shell_append(GTK_MENU_SHELL(keyset_menu), menuentry);
        std::string* userdata = new std::string(setname);
        g_signal_connect_data(menuentry, "activate", G_CALLBACK(decklibrary::keyfiles::loadbuttonset), userdata, NULL, G_CONNECT_SWAPPED);
    }
    // The "mode" menu items.
    GtkWidget *record = gtk_menu_item_new_with_label("Record");
    GtkWidget *edit = gtk_menu_item_new_with_label("Edit");
    GtkWidget *loadimage = gtk_menu_item_new_with_label("Load Image");
    GtkWidget *pushbutton = gtk_menu_item_new_with_label("Push Button");
    GtkWidget *popbutton = gtk_menu_item_new_with_label("Pop Button");
    GtkWidget *demo = gtk_menu_item_new_with_label("Demo");
    gtk_menu_shell_append(GTK_MENU_SHELL(mode_menu), record);
    gtk_menu_shell_append(GTK_MENU_SHELL(mode_menu), edit);
    gtk_menu_shell_append(GTK_MENU_SHELL(mode_menu), loadimage);
    gtk_menu_shell_append(GTK_MENU_SHELL(mode_menu), pushbutton);
    gtk_menu_shell_append(GTK_MENU_SHELL(mode_menu), popbutton);
    gtk_menu_shell_append(GTK_MENU_SHELL(mode_menu), demo);
    const char* recordmode = "Record";
    gpointer userdata = static_cast<gpointer>(const_cast<char*>(recordmode));
    g_signal_connect_data(record, "activate", G_CALLBACK(decklibrary::deck::setmode), userdata, NULL, G_CONNECT_SWAPPED);
    const char* editmode = "Edit";
    userdata = static_cast<gpointer>(const_cast<char*>(editmode));
    g_signal_connect_data(edit, "activate", G_CALLBACK(decklibrary::deck::setmode), userdata, NULL, G_CONNECT_SWAPPED);
    const char* loadimagemode = "LoadImage";
    userdata = static_cast<gpointer>(const_cast<char*>(loadimagemode));
    g_signal_connect_data(loadimage, "activate", G_CALLBACK(decklibrary::deck::setmode), userdata, NULL, G_CONNECT_SWAPPED);
    const char* pushbuttonmode = "PushButton";
    userdata = static_cast<gpointer>(const_cast<char*>(pushbuttonmode));
    g_signal_connect_data(pushbutton, "activate", G_CALLBACK(decklibrary::deck::setmode), userdata, NULL, G_CONNECT_SWAPPED);
    const char* popbuttonmode = "PopButton";
    userdata = static_cast<gpointer>(const_cast<char*>(popbuttonmode));
    g_signal_connect_data(popbutton, "activate", G_CALLBACK(decklibrary::deck::setmode), userdata, NULL, G_CONNECT_SWAPPED);
    const char* demonmode = "Demo";
    userdata = static_cast<gpointer>(const_cast<char*>(demonmode));
    g_signal_connect_data(demo, "activate", G_CALLBACK(decklibrary::deck::setmode), userdata, NULL, G_CONNECT_SWAPPED);
    // The "action" menu items.
    GtkWidget *action_save = gtk_menu_item_new_with_label("Save");
    GtkWidget *action_saveas = gtk_menu_item_new_with_label("Save As");
    GtkWidget *action_delete = gtk_menu_item_new_with_label("Delete");
    GtkWidget *action_deckinfo = gtk_menu_item_new_with_label("Deck Info");
    GtkWidget *action_brightness = gtk_menu_item_new_with_label("Deck Brightness");
    GtkWidget *action_reset = gtk_menu_item_new_with_label("Deck Reset");
    GtkWidget* action_keygeneration = gtk_menu_item_new_with_label("Key Generation");
    GtkWidget *action_dicephoto = gtk_menu_item_new_with_label("Dice Photo");
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_save);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_saveas);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_delete);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_deckinfo);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_brightness);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_reset);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_keygeneration);
    gtk_menu_shell_append(GTK_MENU_SHELL(action_menu), action_dicephoto);
    GtkWidget *submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(action_deckinfo), submenu);
    std::pair<std::string, std::string> info = decklibrary::deck::deckInfo();
    GtkWidget *serialnumber = gtk_menu_item_new_with_label(("Serial Number : " + std::string(info.first)).c_str());
    GtkWidget *firmware = gtk_menu_item_new_with_label(("  Firmware Version : " + std::string(info.second)).c_str());
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), serialnumber);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), firmware);
    gtk_widget_show(serialnumber);
    gtk_widget_show(firmware);
    GtkWidget* key_generation_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(action_keygeneration), key_generation_submenu);
    GtkWidget* x11 = gtk_menu_item_new_with_label("X11");
    gtk_menu_shell_append(GTK_MENU_SHELL(key_generation_submenu), x11);
    GtkWidget* evemu = gtk_menu_item_new_with_label("EVEMU");
    gtk_menu_shell_append(GTK_MENU_SHELL(key_generation_submenu), evemu);
    g_signal_connect(action_save, "activate", G_CALLBACK(decklibrary::keyfiles::savebuttonset), nullptr);
    g_signal_connect(action_saveas, "activate", G_CALLBACK(MintDeck::savebuttonsetas), nullptr);
    g_signal_connect(action_delete, "activate", G_CALLBACK(MintDeck::deletebuttonset), nullptr);
    g_signal_connect(action_brightness, "activate", G_CALLBACK(MintDeck::onBrightnessActivated), nullptr);
    g_signal_connect(action_reset, "activate", G_CALLBACK(decklibrary::deck::deckreset), nullptr);
    g_signal_connect(x11, "activate", G_CALLBACK(decklibrary::deck::setx11mode), nullptr);
    g_signal_connect(evemu, "activate", G_CALLBACK(MintDeck::setevemumode), nullptr);
    g_signal_connect(action_dicephoto, "activate", G_CALLBACK(MintDeck::loadbackgroundimage), nullptr);
}

void MintDeck::onBrightnessActivated(gpointer data) {
    MintDeck* mintDeck = static_cast<MintDeck*>(data);
    mintDeck->setDeckBrightness();
}
void MintDeck::setDeckBrightness() {
    GtkWidget *dialog;
    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Brightness");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 250, 50);
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    GtkWidget* scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 100, 1);
    gtk_range_set_value(GTK_RANGE(scale), 30);
    gtk_container_add(GTK_CONTAINER(content_area), scale);
    g_signal_connect(scale, "value-changed", G_CALLBACK(decklibrary::deck::deckbrightness), NULL);
    gtk_widget_show_all(dialog);
}
void MintDeck::setevemumode() {
	decklibrary::deck::setevemumode();
    textdialog2("Save The Evemu Keyboard Event Number", 450, "Show Evemu Devices", "Save Keyboard Event Number", verifydevnumber);
}

void MintDeck::verifydevnumber(GtkWidget* widget, GtkWidget* devnumber) {
    const char* buttonLabel = gtk_button_get_label(GTK_BUTTON(widget));
    std::string buttonName(buttonLabel);
    if (buttonName == "Show Evemu Devices") {
        const char* command = "gnome-terminal -e 'evemu-describe'";
        FILE* pipe = popen(command, "r");
        pclose(pipe);
        gtk_entry_set_text(GTK_ENTRY(devnumber), "");
        gtk_widget_grab_focus(devnumber);
    } else if (buttonName == "Save Keyboard Event Number") {
        std::string input = std::string(gtk_entry_get_text(GTK_ENTRY(devnumber)));
        bool isInteger = !input.empty() && std::all_of(input.begin(), input.end(), ::isdigit);
        if (isInteger) {
            int int_input = std::stoi(input);
            decklibrary::deck::setevemudevice(int_input);
            GtkWidget* dialog = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_DIALOG);
            gtk_widget_hide(dialog);
        } else {
            gtk_entry_set_text(GTK_ENTRY(devnumber), "");
            gtk_widget_grab_focus(devnumber);
            GtkWidget* error_dialog = gtk_message_dialog_new(
                nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Invalid input. Enter an integer."
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
    }
}

void MintDeck::loadbackgroundimage() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        setnewbackgroundimage(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}
