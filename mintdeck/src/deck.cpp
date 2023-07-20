/// @file deck.cpp
#include "deck.hpp"

using namespace std::string_literals;

namespace decklibrary
{

    int deck::btnpressed;
    buttoninfo* deck::buttons;
    int keyfiles::defaultindex;
    std::string keyfiles::loadedbuttonset;
    std::vector<std::string> keyfiles::keysetnames;
    buttoninfo* keyfiles::keybuttons;
    const char* keyfiles::keysetpath;
    std::stack<buttoninfo> deck::buttonstack;
    keytech deck::keymode;
    int deck::evemudevnumber;
    keyfiles::keyfiles() {
      keysetpath = "data/";
      findbuttonsets();
    }

  buttoninfo* keyfiles::loadkeys(int decksize, int setindex) {
    int imagelength, keystringlength;
    loadedbuttonset = keysetnames[setindex];
    std::string filename = keysetpath + loadedbuttonset + ".bin";
    keybuttons = new buttoninfo[decksize];
    std::ifstream fin(filename, std::ios::binary);
    for (int i = 0; i < decksize; i++) {
      fin.read(reinterpret_cast<char*>(&imagelength), sizeof(imagelength));
      std::vector<unsigned char> imagedata(imagelength);
      fin.read(reinterpret_cast<char*>(imagedata.data()), imagelength);
      keybuttons[i].image = imagedata;
      loadpixmap((unsigned char*)imagedata.data(), imagedata.size(), &keybuttons[i]);
      fin.read(reinterpret_cast<char*>(&keystringlength), sizeof(keystringlength));
      char* buffer = new char[keystringlength+1];
      fin.read(buffer, keystringlength);
      buffer[keystringlength] = '\0';
      keybuttons[i].kstrokes = buffer;
      delete[] buffer;
      loadkeygroups(&keybuttons[i]);
      deck::writebuttonimage(i, imagedata);
    }
    fin.close();
    return keybuttons;
  }  
  
  void keyfiles::loadpixmap(unsigned char* jpegData, unsigned long int jpegSize, buttoninfo* button) {
    struct jpeg_decompress_struct info;
    struct jpeg_error_mgr err;
    unsigned char* pixmap;
    unsigned char* pixmaprowbuffer[1];
    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);
    jpeg_mem_src(&info, jpegData, jpegSize);
    jpeg_read_header(&info, TRUE);
    jpeg_start_decompress(&info);
    unsigned long int pixmapsize = 3 * info.output_width * info.output_height;
    pixmap = (unsigned char*)malloc(sizeof(unsigned char)*pixmapsize);
    while(info.output_scanline < info.output_height) {
      pixmaprowbuffer[0] = (unsigned char *)(&pixmap[3*info.output_width*info.output_scanline]);
      jpeg_read_scanlines(&info, pixmaprowbuffer, 1);
    }
    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);
    std::vector<unsigned char> pixmapvector(pixmap, pixmap + pixmapsize);
    button->width =  info.output_width;
    button->height = info.output_height;
    button->pixmap = pixmapvector;
  }

  void keyfiles::loadkeygroups(buttoninfo* btn) {
    std::vector<keymap> elements = deck::parsestring(btn->kstrokes);
    btn->kaction = deck::groupCollector(elements);
  }

  void keyfiles::findbuttonsets() {
    defaultindex = 0;
    keysetnames.clear();
    std::string filename;
    for (const auto& entry : std::filesystem::directory_iterator(keysetpath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".bin") {
          filename = entry.path().filename().string();
          keysetnames.push_back(filename.substr(0, filename.length() - 4));
        }
    }
    std::sort(keysetnames.begin(), keysetnames.end());
    auto i = std::find(keysetnames.begin(), keysetnames.end(), "default");
    if (i != keysetnames.end()) {
      defaultindex = std::distance(keysetnames.begin(), i);
    }
    else {
      std::string defaultname = "default.bin";
      std::ifstream source(*keysetpath +  keysetnames[0] + ".bin", std::ios::binary);
      std::ofstream destination(*keysetpath + defaultname, std::ios::binary);
      destination << source.rdbuf();
      keysetnames.push_back("default");
      std::sort(keysetnames.begin(), keysetnames.end());
      i = std::find(keysetnames.begin(), keysetnames.end(), "default");
      defaultindex = std::distance(keysetnames.begin(), i);
    }
  }

  std::vector<std::string> keyfiles::getbuttonsets(){
    std::vector<std::string> list;
    for (const auto& str : keysetnames) {
        if ((str != "ec72df53f5")&&(str != "default")) { list.push_back(str); }
    }
    return list;
  }

  void keyfiles::loadbuttonset(gpointer menudata) {
    std::string* name = static_cast<std::string*>(menudata);
    std::string file = *name;
    int size = (int)keysetnames.size();
    for (int i=0;i<size;i++) {
      if (keysetnames[i] == file) {
        deck::setbuttons(loadkeys(deck::getbtncount(),i));
        break; 
      }
    }
    MintDeck::drawtiles();
    delete name;
  }

  int keyfiles::getfileindex() {
    return defaultindex;
  }

  void keyfiles::savebuttonset() {
    std::string filename = keysetpath + loadedbuttonset + ".bin";
    std::ofstream keyfile(filename, std::ios::binary);
    buttoninfo* deckbuttons = deck::getbuttons();
    int count = deck::getbtncount();
    for(int i=0;i<count;i++) {
      const buttoninfo& button = deckbuttons[i];
      int imagelength = button.image.size();
      keyfile.write(reinterpret_cast<const char*>(&imagelength), sizeof(imagelength));
      keyfile.write(reinterpret_cast<const char*>(button.image.data()), imagelength);
      int keystringlength = strlen(button.kstrokes.c_str());
      keyfile.write(reinterpret_cast<const char*>(&keystringlength), sizeof(keystringlength));
      keyfile.write(button.kstrokes.c_str(), keystringlength);
    }
    keyfile.close();
  }

  void keyfiles::savebuttonsetas(GtkWidget*widget, GtkWidget* keysetname) {
    loadedbuttonset = std::string(gtk_entry_get_text(GTK_ENTRY(keysetname)));
    if(!loadedbuttonset.empty()) {
      savebuttonset();
      findbuttonsets();
      MintDeck::drawtiles();
      deletedialog(widget);
    } else {
      gtk_widget_grab_focus(keysetname);
    }
  }

  void keyfiles::deletebuttonset(GtkWidget*widget, GtkWidget* keysetname) {
    std::string buttonset = std::string(gtk_entry_get_text(GTK_ENTRY(keysetname)));
    if(!buttonset.empty()) {
      std::vector<std::string>::iterator inlist = std::find(keysetnames.begin(), keysetnames.end(), buttonset);
      if(inlist != keysetnames.end() && buttonset != "default") {
        std::string filename = keysetpath + buttonset + ".bin";
        std::remove(filename.c_str());
        findbuttonsets();
        MintDeck::drawtiles();
        deletedialog(widget);
      } else {
        gtk_widget_grab_focus(keysetname);
      }
    }
  }

  void keyfiles::deletedialog(GtkWidget* widget) {
    GtkWidget* parent = gtk_widget_get_parent(widget);
    parent = gtk_widget_get_parent(parent);
    parent = gtk_widget_get_parent(parent);
    parent = gtk_widget_get_parent(parent);
    GtkWidget* dialog  = gtk_widget_get_parent(parent);
    gtk_widget_destroy(dialog);
  }

  deck::deck() {
    deckindex = 0;
    keystate = 0b0000;
    priorkeystate = keystate;
    decklibrary::keyfiles keysets;
    keymode = X11;
    evemudevnumber = 27;
  }

  void deck::init(GtkWidget* widget) {
    loadkeymap();
    ctx[deck::deckindex]->run(0, ctx, [this](std::vector<bool>& keys) { this->buttonpress(keys); });
    ctx[deck::deckindex]->cmdpipe(widget);
    serialnumber = ctx[deck::deckindex]->getserialnumber();
    firmware = ctx[deck::deckindex]->getfirmwareversion();
  }

  void deck::buttonpress(std::vector<bool>& btns) {
    std::vector<int> pressedkeys;
    for (std::vector<bool>::size_type i = 0; i < btns.size(); i++) {
      if (btns[i]) { pressedkeys.push_back(i); }
    }
    if (pressedkeys.size()>=1) {
      for (std::vector<int>::size_type i = 0; i < pressedkeys.size(); i++) {
        deckbutton(pressedkeys);
      }
    }
  }

  void deck::deckbutton(int btn) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::vector<int> singleButton{ btn };
    deckbutton(singleButton);
  }

  void deck::deckbutton(std::vector<int>& btns) {
    for (unsigned int key : btns) {
      if (key < ctx[deck::deckindex]->btncount) {
        std::vector<std::vector<keymap>> groups = buttons[key].kaction;
        for (std::vector<keymap> keygroup : groups) {
          runScript(keygroup);
        }
      }
    }
  }

  void deck::loadkeymap() {
    int decksize = ctx[deck::deckindex]->btncount;
    buttons = keyset.loadkeys(decksize, keyfiles::getfileindex());
  }

  int deck::getbtncount() {
    return ctx[deck::deckindex]->btncount;
  }

  buttoninfo* deck::getbuttons() {
    return buttons;
  }

  std::string deck::getbuttonkeys(int btn) {
    return buttons[btn].kstrokes;
  }

  void deck::setbuttons(buttoninfo* buttondata) {
    buttons = buttondata;
  }

  void deck::writebuttonimage(int button, std::vector<unsigned char> buttonimage) {
    ctx[deckindex]->writebuttonimage(button, buttonimage);
  }

  GtkWidget * deck::getkeyimage(int key) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(buttons[key].pixmap.data(), GDK_COLORSPACE_RGB, FALSE, 8, buttons[key].width, buttons[key].height, buttons[key].width * 3, NULL, NULL);
    GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
    return image;
  }

  std::pair<std::string, std::string> deck::deckInfo() {
    std::pair<std::string, std::string> info;
    info.first = serialnumber;
    info.second = firmware;
    return info;
  }

  void deck::deckreset() {
    ctx[deck::deckindex]->reset();
  }

  void deck::deckbrightness(GtkRange* range) {
    int value = static_cast<int>(gtk_range_get_value(range));
    std::byte percent = std::byte(std::clamp(value, 0, 100));
    ctx[deck::deckindex]->setbrightness(percent);
  }

  void deck::mousebtnclick(GtkWidget* widget, gpointer udat) {
    int btn = GPOINTER_TO_INT(udat);
    if(widget==NULL)  test::printstring("Widget is NULL");  // avoids a warning
    switch (modestringmap[deck::mode]) {
      case RECORD:
        keystring ="";
        MintDeck::recordkeystring(btn);
        break;
      case EDIT:
        MintDeck::editkeystring(btn);
        break;
      case LOADIMAGE:
        MintDeck::loadbuttonimage(btn);
        break;
      case PUSH:
        pushbutton(btn);
        break;
      case POP:
        popbutton(btn);
        break;
      case DEMO:
        deckbutton(btn);
        break;
    }
  }

  void deck::pushbutton(int btn) {
    buttonstack.push(buttons[btn]);
  }

  void deck::popbutton(int btn) {
    if(!buttonstack.empty()) {
      buttons[btn] = buttonstack.top();
      buttonstack.pop();
      MintDeck::drawtiles();
      writebuttonimage(btn, buttons[btn].image);
    }
  }

  gboolean deck::onKeyPress(GtkWidget *widget, GdkEventKey *event, int btn) {
    btnpressed = btn;  // the pressed button
    if(widget==NULL) std::cout << "Widget is NULL" << std::endl;  // avoids a warning
    unsigned int keydown = event->keyval;
    keymap key;
    switch(keydown) {
      case keymap::SHIFT_L:
      case keymap::SHIFT_R:
        keystate[KSHIFT] = true;
        break;
      case keymap::CTRL_L:
      case keymap::CTRL_R:
        keystate[KCTRL] = true;
        break;
      case keymap::ALT_L:
      case keymap::ALT_R:
        keystate[KALT] = true;
        break;
      case keymap::META_L:
      case keymap::META_R:
        keystate[KMETA] = true;
        break;
      default :
        key = static_cast<keymap>(keydown);
        if(isShifted(key)) {
          keystate[KSHIFT] = true;
          key = lowkeys[key];
        }
        break;
    }
    return TRUE;
  }

  gboolean deck::onKeyRelease(GtkWidget *widget, GdkEventKey *event) {
    std::bitset<STATECOUNT> unset(0b0000);
    if(widget==NULL) std::cout << "Widget is NULL" << std::endl;  // avoids a warning
    keymap key = static_cast<keymap>(event->keyval);
    keystate[KSHIFT] = isSHIFTkey(key) ? false : keystate[KSHIFT];
    keystate[KCTRL] = isCTRLkey(key) ? false : keystate[KCTRL];
    keystate[KALT] = isALTkey(key) ? false : keystate[KALT];
    keystate[KMETA] = isMETAkey(key) ? false : keystate[KMETA];
    if(isShifted(key)) key = lowkeys[key];
    if(priorkeystate!=keystate) {
    if (keystate[KSHIFT]) keystring += "<SHIFT>";
      if (keystate[KCTRL]) keystring += "<CTRL>";
      if (keystate[KALT]) keystring += "<ALT>";
      if (keystate[KMETA]) keystring += "<META>";
    } else {
      if(!((keystate!=priorkeystate)||(keystate==unset)))  keystring += "+";
    }
    if(!isModifyKey(key)) {
      if(key==SPACE) keystring += " ";
      else if(isSimpleKey(key)) {
        keystring += keystrings[key];
      } else {
        keystring += "<"+keystrings[key]+">"; // named keys get < >
      }
    }
    priorkeystate = keystate;
    return TRUE;
  }

  bool deck::isSimpleKey(keymap key) {
    bool simplekey = false;
      for (const keymap& item : simplestrings) {
          if (key == item) { simplekey = true; }
      }
      return simplekey;
  }

  bool deck::isShifted(keymap key) {
    bool shiftkey = false;
    for (const keymap& item : shiftkeys) {
      if (key == item) { shiftkey = true; }
    }
    return shiftkey;
  }

  bool deck::isModifyKey(keymap key) {
    bool modifykey = false;
    for (const keymap& item : modkeys) {
      if (key == item) {
        modifykey = true;
        break;
      }
    }
    return modifykey;
  }

  bool deck::isMETAkey(keymap key) {
    if((key==META_L)||(key==META_R)) return true;
    else return false;
  }

  bool deck::isALTkey(keymap key) {
    if((key==ALT_L)||(key==ALT_R)) return true;
    else return false;
  }

  bool deck::isCTRLkey(keymap key) {
    if((key==CTRL_L)||(key==CTRL_R)) return true;
    else return false;
  }

  bool deck::isSHIFTkey(keymap key) {
    if((key==SHIFT_L)||(key==SHIFT_R)) return true;
    else return false;
  }

  bool deck::isCaseKey(keymap key) {
    bool casedkey = true;
    for (const keymap& item : nocasekeys) {
      if (key == item) { casedkey = false; }
    }
    return casedkey;
  }

  bool deck::isPlusKey(keymap key) {
    bool isplus = false;
    if (key == PLUS) isplus = true;
    return isplus;
  }

  bool deck::isBackslashKey(keymap key) {
    bool isbackslash = false;
      if (key == BACKSLASH) isbackslash = true;
      return isbackslash;
  }

  std::vector<keymap> deck::parsestring(const std::string& keystring) {
    std::vector<keymap> elements;
    std::string namedelement;
    bool namedkey = false;
    for (char c : keystring) {
      if (c == '<') {
        namedkey = true;
      } else if (c == '>') {
        namedkey = false;
        elements.push_back(keycodes[namedelement]);
        namedelement = "";
      } else if (c == ' ') {
        elements.push_back(SPACE);
      } else if (namedkey) {
        namedelement.push_back(c);
      } else {
        std::string unnamedelement(1, c);
        elements.push_back(keycodes[unnamedelement]);
      }
    }
    if (!namedelement.empty()) {
      elements.push_back(keycodes[namedelement]);
    }
    return elements;
  }

  void deck::savekeygroup(std::vector<std::vector<keymap>>& groups, std::bitset<STATECOUNT>state, std::vector<keymap>& groupkeys) {
    if (!groupkeys.empty()) {
      std::vector<keymap> keys;
      keys.clear();
      if (state[KSHIFT]) keys.push_back(SHIFT_L);
      if (state[KCTRL]) keys.push_back(CTRL_R);
      if (state[KALT]) keys.push_back(ALT_L);
      if (state[KMETA]) keys.push_back(META_R);
      keys.insert(keys.end(), groupkeys.begin(), groupkeys.end());
      groups.push_back(keys);
      groupkeys.clear();
    }
  }

  std::vector<std::vector<keymap>> deck::groupCollector(const std::vector<keymap>& elements) {
    std::vector<std::vector<keymap>> groups;
    std::bitset<STATECOUNT> kstate;
    std::bitset<STATECOUNT> nkstate;
    nkstate[KSHIFT] = nkstate[KCTRL] = nkstate[KALT] = false;
    bool keepstate = false;
    std::vector<keymap> groupkeys;
    groupkeys.clear();
    for (auto keypointer = elements.begin(); keypointer != elements.end(); ++keypointer) {
      kstate[KSHIFT] = nkstate[KSHIFT];
      kstate[KCTRL] = nkstate[KCTRL];
      kstate[KALT] = nkstate[KALT];
      kstate[KMETA] = nkstate[KMETA];
      keymap key = *keypointer;
      if(isPlusKey(key)) {
        keepstate = true;
        continue;
      }
      if(!keepstate) nkstate.reset();
      if (isModifyKey(key)) {
        keepstate = true;
        nkstate[KSHIFT] = isSHIFTkey(key) ? true : nkstate[KSHIFT];
        nkstate[KCTRL] = isCTRLkey(key) ? true : nkstate[KCTRL];
        nkstate[KALT] = isALTkey(key) ? true : nkstate[KALT];
        nkstate[KMETA] = isMETAkey(key) ? true : nkstate[KMETA];
        if(kstate!=nkstate){
          savekeygroup(groups,kstate,groupkeys);
        }
        continue;
      } else if(isCaseKey(key)) {
        if(isBackslashKey(key)) {
          keypointer = std::next(keypointer);
          if (keypointer != elements.end()) {
            key = *keypointer;
          }
        } if(isShifted(key)&&nkstate[KSHIFT]) {
          key = lowkeys[key];
        } else if(isShifted(key)&&!nkstate[KSHIFT]) {
          nkstate[KSHIFT] = true;
          key = lowkeys[key];
        } else if(!isShifted(key)&&nkstate[KSHIFT]&&!keepstate) {
          savekeygroup(groups,kstate,groupkeys);
          nkstate[KSHIFT] = false;
        } else if(!isShifted(key)&&!nkstate[KSHIFT]) {
          ; // do nothing
        }
      }
      if(kstate!=nkstate) {
        savekeygroup(groups,kstate,groupkeys);
        keepstate = true;
      }
      keepstate = false;
      groupkeys.push_back(key);
    }
    if (!groupkeys.empty()) {
      savekeygroup(groups,nkstate,groupkeys);
    }
    return groups;
  }

  void decklibrary::deck::runScript(std::vector<keymap> groupkeys) {
    int modlength = 0;
    int length = groupkeys.size();
    std::string command;
    std::string keyup;
    std::string keydown;
    for (int i = 0; i < length; i++) {
      if (isModifyKey(groupkeys[i])) {
        modlength++;
      }
    }
    switch (keymode) {
      case EVEMU:
        command = "evemu-event /dev/input/event";
        command += std::to_string(evemudevnumber);
        command += " --type EV_KEY --code ";
        keyup = " --value 0 --sync";
        keydown = " --value 1 --sync";
      break;
      case X11:
        keyup = "xdotool keyup ";
        keydown = "xdotool keydown ";
      break;
    }
    auto executeCommand = [&](const std::string& cmd) {
      system(cmd.c_str());
    };
    for (int i = 0; i < modlength; i++) {
      switch (keymode) {
        case EVEMU:
          executeCommand(command + evemucode[groupkeys[i]] + keydown);
        break;
        case X11:
          executeCommand(keydown + xdotoolcode[groupkeys[i]]);
        break;
      }
    }
    for (int i = modlength; i < length; i++) {
      switch (keymode) {
        case EVEMU:
          executeCommand(command + evemucode[groupkeys[i]] + keydown);
          executeCommand(command + evemucode[groupkeys[i]] + keyup);
        break;
        case X11:
          executeCommand(keydown + xdotoolcode[groupkeys[i]]);
          executeCommand(keyup + xdotoolcode[groupkeys[i]]);
        break;
      }
    }
    for (int i = modlength - 1; i >= 0; i--) {
      switch (keymode) {
        case EVEMU:
          executeCommand(command + evemucode[groupkeys[i]] + keyup);
        break;
        case X11:
          executeCommand(keyup + xdotoolcode[groupkeys[i]]);
        break;
      }
    }
  }

  void deck::savekeystring(GtkWidget* widget) {
    MintDeck::stoptimer = true;
    buttons[btnpressed].kstrokes = keystring;
    std::vector<keymap> elements = parsestring(keystring);
    buttons[btnpressed].kaction = groupCollector(elements);
    keystring = "";
    keystate.reset();
    priorkeystate.reset();
    gtk_widget_destroy(widget);
  }

  void deck::setkeystring(GtkWidget* widget, GdkEvent*, gpointer userdata) {
    int btn = GPOINTER_TO_INT(userdata);
    GtkTextBuffer* text_buffer = MintDeck::gettextbuffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(text_buffer, &start);
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    gchar* text = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);
    buttons[btn].kstrokes = std::string(reinterpret_cast<const char*>(text));
    keyfiles::loadkeygroups(&buttons[btn]);
    gtk_widget_destroy(widget);
  }

  void deck::catchjpegerror(j_common_ptr cinfo) {
    MintDeck::messagedialog("JPEG Compression Error", cinfo->err->jpeg_message_table[cinfo->err->last_jpeg_message]);
    throw std::runtime_error("JPEG compression failed");
  }

  void deck::setnewimage(GdkPixbuf* pixbuf, int btn)
  {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = catchjpegerror;
    try {
      jpeg_create_compress(&cinfo);
      unsigned char* buffer = nullptr;
      unsigned long buffer_size = 0;
      jpeg_mem_dest(&cinfo, &buffer, &buffer_size);
      cinfo.image_width = gdk_pixbuf_get_width(pixbuf);
      cinfo.image_height = gdk_pixbuf_get_height(pixbuf);
      cinfo.input_components = gdk_pixbuf_get_n_channels(pixbuf);
      cinfo.in_color_space = JCS_RGB;
      jpeg_set_defaults(&cinfo);
      jpeg_start_compress(&cinfo, TRUE);
      JSAMPROW row_pointer[1];
      int row_stride;
      row_stride = gdk_pixbuf_get_width(pixbuf) * gdk_pixbuf_get_n_channels(pixbuf);
      while (cinfo.next_scanline < cinfo.image_height)
      {
        row_pointer[0] = gdk_pixbuf_get_pixels(pixbuf) + cinfo.next_scanline * row_stride;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }
      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);
      int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
      std::vector<unsigned char> jpegimage(buffer, buffer + buffer_size);
      const unsigned char* pixels = gdk_pixbuf_read_pixels(pixbuf);
      std::vector<unsigned char> pixmap(pixels, pixels + (int)cinfo.image_height * rowstride);
      buttons[btn].image = jpegimage;
      writebuttonimage(btn, jpegimage);
      buttons[btn].pixmap = pixmap;
    } catch (const std::exception& e) {
      return;
    }
  }

  void deck::loadbbackgroundimage(std::vector<GdkPixbuf*> dicedimages) {
    for (int btn=0;btn<(int)dicedimages.size();btn++) {
      setnewimage(dicedimages[btn], btn);
    }
  }

  void deck::setx11mode() { keymode = X11; }

  void deck::setevemumode() { keymode = EVEMU; }

  void deck::setmode(gpointer data) {
    const char* modedata = static_cast<const char*>(data);
    mode = std::string(modedata);
  }

  void deck::setevemudevice(int devnumber) { evemudevnumber = devnumber; }

}
