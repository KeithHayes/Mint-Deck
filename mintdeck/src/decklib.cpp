/// @file decklib.cpp
#include "decklib.hpp"

using namespace std::string_literals;

namespace decklibrary
{
  context deck::ctx;
  int deck::deckindex;
  std::string deck::serialnumber;
  std::string deck::firmware;
  std::string deck::mode = "Edit";
  std::bitset<STATECOUNT> deck::keystate;
  std::bitset<STATECOUNT> deck::priorkeystate;
  std::string deck::keystring = "";
  keyfiles deck::keyset;
  namespace
  {
    /// @brief First generation Stream Deck.
    class gen1devtype : public devtype
    {
    public:
      using base_type = devtype;
      const unsigned hidreportlength;
      static constexpr unsigned header_length = 16;
      const unsigned payload_length;
      gen1devtype(const char *path, unsigned width, unsigned height, unsigned cols, unsigned rows, unsigned reportlength, bool hflip, bool vflip)
          : devtype(path, width, height, cols, rows, imagetype::bmp, reportlength, hflip, vflip), hidreportlength(reportlength), payload_length(reportlength - header_length) {}
      payloadtype::iterator dataheader(payloadtype &buffer, unsigned button, unsigned remaining, unsigned page) override final;
      std::vector<bool> read() override final;
      std::string getserialnumber() override final;
      std::string getfirmwareversion() override final;
      void setbrightness(std::byte percent) override final;
      void reset() override final;
    private:
      std::string getstring(std::byte c);
    };
    /// @brief Writes a data header to the buffer.
    /// @param buffer Memory to be filled with data.
    /// @param button The target button on the Stream Deck.
    /// @param remaining Number of data bytes.
    /// @param page Data page.
    /// @return A pointer into the buffer following the header data.
    gen1devtype::payloadtype::iterator gen1devtype::dataheader(payloadtype &buffer, unsigned button, unsigned remaining, unsigned page) {
      auto it = buffer.begin();
      *it++ = std::byte(0x02);
      *it++ = std::byte(0x01);
      *it++ = std::byte(page + 1);
      *it++ = std::byte(0x00);
      *it++ = std::byte(remaining > payload_length ? 0 : 1);
      *it++ = std::byte(button + 1);
      std::fill_n(it, 10, std::byte(0x00));
      return it;
    }
    /// @brief Read button states of a connected device.
    /// @return Button states bit packed in a vector. 
    std::vector<bool> gen1devtype::read() {
      std::vector<std::byte> state(1 + btncount);
      auto n = base_type::read(state);
      std::vector<bool> res(btncount);
      std::transform(state.begin() + 1, state.begin() + n, res.begin(), [](auto v) {
        return v != std::byte(0);
      });
      return res;
    }
    /// @brief A memory buffer is filled with data from the Stream Deck.
    /// @param cmd A byte that identifies what the Stream Deck will return.
    /// @return A count of bytes written.
    std::string gen1devtype::getstring(std::byte cmd) {
      std::array<std::byte, 17> buf{cmd};
      auto len = getreport(buf);
      return len > 5 ? std::string(reinterpret_cast<const char *>(buf.data()) + 5) : "";
    }
    /// @brief Reads the Stream Deck Serial Number by sending (0x03) to the device in a buffer.
    /// @return The Serial Number
    std::string gen1devtype::getserialnumber() {
      return getstring(std::byte(0x03));
    }
    /// @brief Reads the Stream Deck Firmware version by sending (0x04) to the device in a buffer.
    /// @return The Firmware Vewrsion.
    std::string gen1devtype::getfirmwareversion() {
      return getstring(std::byte(0x04));
    }
    /// @brief Sets screen brightness by sending (0x05, 0x55, 0xaa, 0xd1, 0x01, percent) to the device in a buffer.
    /// @param percent The percentage brightness to set.
    void gen1devtype::setbrightness(std::byte percent) {
      const std::array<std::byte, 17> req{std::byte(0x05), std::byte(0x55), std::byte(0xaa), std::byte(0xd1), std::byte(0x01), percent};
      sendreport(req);
    }
    /// @brief Resets the Stream Deck by sending (0x0b, 0x63) to the device in a buffer.
    void gen1devtype::reset() {
      const std::array<std::byte, 17> req{std::byte(0x0b), std::byte(0x63)};
      sendreport(req);
    }
    ///
    /// @brief Second generation Stream Deck
    class gen2devtype : public devtype 
    {
    public:
      using base_type = devtype;
      static constexpr unsigned hidreportlength = 1024;
      static constexpr unsigned header_length = 8;
      static constexpr unsigned payload_length = hidreportlength - header_length;
      gen2devtype(const char *path, unsigned width, unsigned height, unsigned cols, unsigned rows)
          : devtype(path, width, height, cols, rows, imagetype::jpeg, hidreportlength, true, true) {}
      payloadtype::iterator dataheader(payloadtype &buffer, unsigned button, unsigned remaining, unsigned page) override final;
      std::vector<bool> read() override final;
      std::string getserialnumber() override final;
      std::string getfirmwareversion() override final;
      void setbrightness(std::byte percent) override final;
      void reset() override final;
    private:
      std::string getstring(std::byte c, size_t offset);
    };
    /// @brief Writes a data header to the buffer.
    /// @param buffer Memory to be filled with data.
    /// @param button The target button on the Stream Deck.
    /// @param remaining Number of data bytes.
    /// @param page Data page.
    /// @return A pointer into the buffer following the header data.
    gen2devtype::payloadtype::iterator gen2devtype::dataheader(payloadtype &buffer, unsigned button, unsigned remaining, unsigned page) {
      auto it = buffer.begin();
      *it++ = std::byte(0x02);
      *it++ = std::byte(0x07);
      *it++ = std::byte(button);
      if (remaining > payload_length)
      {
        *it++ = std::byte(0x00);
        *it++ = std::byte(payload_length & 0xff);
        *it++ = std::byte(payload_length >> 8);
      }
      else
      {
        *it++ = std::byte(0x01);
        *it++ = std::byte(remaining & 0xff);
        *it++ = std::byte(remaining >> 8);
      }
      *it++ = std::byte(page & 0xff);
      *it++ = std::byte(page >> 8);

      return it;
    }
    /// @brief Read button states of a connected device.
    /// @return Button states bit packed in a vector. 
    std::vector<bool> gen2devtype::read() {
      std::vector<bool> res(btncount);
      std::vector<std::byte> state(4 + btncount);
      int n;
      while ((n = base_type::read(state)) < 4)
        continue;
      std::transform(state.begin() + 4, state.begin() + n, res.begin(), [](auto v) {
        return v != std::byte(0);
      });
      return res;
    }
    /// @brief A memory buffer is filled with data from the Stream Deck.
    /// @param cmd A byte that identifies what the Stream Deck will return.
    /// @param offset Where returned data begins.
    /// @return The number of bytes written.
    std::string gen2devtype::getstring(std::byte cmd, size_t offset) {
      std::array<std::byte, 32> buf{cmd};
      auto len = getreport(buf);
      return len >= 0 && size_t(len) > offset ? std::string(reinterpret_cast<const char *>(buf.data()) + offset) : "";
    }
    /// @brief The serial number is read from a Stream Deck.
    /// @return The serial number as a string.
    std::string gen2devtype::getserialnumber() {
      return getstring(std::byte(0x06), 2);
    }
    /// @brief The firmware version is read from a Stream Deck.
    /// @return The firmware version as a string.
    std::string gen2devtype::getfirmwareversion() {
      return getstring(std::byte(0x05), 6);
    }
    /// @brief Sets the Stream Deck brighness by sending (0x03, 0x08, brightness) to the device.
    /// @param brightness 
    void gen2devtype::setbrightness(std::byte brightness) {
      const std::array<std::byte, 32> req{std::byte(0x03), std::byte(0x08), brightness};
      sendreport(req);
    }
    /// @brief Resets the Stream Deck by sending (0x03, 0x02) to the device in a buffer.
    void gen2devtype::reset() {
      const std::array<std::byte, 32> req{std::byte(0x03), std::byte(0x02)};
      sendreport(req);
    }
    /// @brief Demo Stream Deck.
    class demodevtype : public devtype {
    public:
      using base_type = devtype;
      const unsigned hidreportlength;
      static constexpr unsigned header_length = 16;
      const unsigned payload_length;
      demodevtype(const char *path, unsigned width, unsigned height, unsigned cols, unsigned rows, unsigned reportlength, bool hflip, bool vflip)
          : devtype(path, width, height, cols, rows, imagetype::bmp, reportlength, hflip, vflip), hidreportlength(reportlength), payload_length(reportlength - header_length) {}
      payloadtype::iterator dataheader(payloadtype &buffer, unsigned button, unsigned remaining, unsigned page) override final;
      std::vector<bool> read() override final;
      std::string getserialnumber() override final;
      std::string getfirmwareversion() override final;
      void setbrightness(std::byte percent) override final;
      void reset() override final;
      int writebuttonimage(int button, std::vector<unsigned char> buttonimage);
      void run(int index, context& ctx, std::function<void(std::vector<bool>&)> btnstate);
    };
    /// @brief Needed to simulate Stream Deck communication in demo mode.
    /// @param buffer A memory buffer.
    /// @param button Unused.
    /// @param remaining Unused.
    /// @param page Unused.
    /// @return A pointer to the start of the buffer.
    demodevtype::payloadtype::iterator demodevtype::dataheader(payloadtype &buffer, [[maybe_unused]] unsigned button, [[maybe_unused]] unsigned remaining, [[maybe_unused]] unsigned page) {
      auto it = buffer.begin();
      return it;
    }
    /// @brief Simulates a read of Stream Deck buttons.
    /// @return A vector simulating no buttons pressed.
    std::vector<bool> demodevtype::read() {
      std::vector<bool> res(btncount, false);
      return res;
    }
    /// @brief Simulate the read of a serial number from a Stream Deck Device.
    /// @return A simulated serial number.
    std::string demodevtype::getserialnumber() { return("A0000000DEMO"); }
    /// @brief Simulate the read of the firmware version from a Stream Deck Device.
    /// @return A simulated firmware release.
    std::string demodevtype::getfirmwareversion() { return("0.00.0000"); }
    /// @brief Needed so demo code does not throw an error.
    /// @param percent What brightness would be set to if there were brightness to set.
    void demodevtype::setbrightness([[maybe_unused]] std::byte percent) { ; }
    /// @brief Needed so demo code does not throw an error.
    void demodevtype::reset() { ; }
    /// @brief Needed so demo code does not throw an error.
    /// @param button A button number.
    /// @param buttonimage A jpeg image.
    /// @return Zero to indicate success.
    int demodevtype::writebuttonimage([[maybe_unused]] int button, [[maybe_unused]] std::vector<unsigned char> buttonimage) { 
      return 0; 
    }
    /// @brief Needed so demo code does not throw an error.
    /// @param index An index into the ctx array, not used.
    /// @param ctx The array of connected devices, not used.
    /// @param btnstate The callback function, not used.
    void demodevtype::run([[maybe_unused]] int index, [[maybe_unused]] context& ctx, [[maybe_unused]] std::function<void(std::vector<bool>&)> btnstate) { ; }

    /// @brief Declaration for a specific device type.
    /// @tparam D The device type identifier.
    template <unsigned short D> struct streamdecktype;
    /// @brief Stream Deck Original
    template <> struct streamdecktype<streamdeckoriginal> final : public gen1devtype {
      using base_type = gen1devtype;
      static constexpr unsigned hidreportlength = 8191;
      streamdecktype(const char *path) : base_type(path, 72, 72, 5, 3, hidreportlength, true, true) {}
    };
    /// @brief Stream Deck Original V2
    template <> struct streamdecktype<streamdeckoriginalv2> final : public gen2devtype {
      using base_type = gen2devtype;
      streamdecktype(const char *path) : base_type(path, 72, 72, 5, 3) {}
    };
    /// @brief Stream Deck Mini
    template <> struct streamdecktype<streamdeckmini> final : public gen1devtype {
      using base_type = gen1devtype;
      static constexpr unsigned hidreportlength = 1024;
      streamdecktype(const char *path) : base_type(path, 80, 80, 3, 2, hidreportlength, false, true) {}
    };
    /// @brief Stream Deck XL
    template <> struct streamdecktype<streamdeckxl> final : public gen2devtype {
      using base_type = gen2devtype;
      streamdecktype(const char *path) : base_type(path, 96, 96, 8, 4) {}
    };
    /// @brief Stream Deck Demo
    template <> struct streamdecktype<streamdeckdemo> final : public demodevtype {
      using base_type = demodevtype;
      static constexpr unsigned hidreportlength = 8191;
      streamdecktype(const char *path) : base_type(path, 72, 72, 5, 3, hidreportlength, true, true) {}
    };
    /// @brief An array of Stream Deck types.
    constexpr auto products = std::experimental::make_array(streamdeckoriginal, streamdeckoriginalv2, streamdeckmini, streamdeckxl, streamdeckdemo);
    /// @brief Recursive function to identify a Stream Deck device.
    /// @tparam N Indexes the products array.
    /// @param product_id The type of Stream deck device.
    /// @param path
    /// @return A pointer to a Stream Deck device of the correct type.
    template <size_t N = 0>
    std::unique_ptr<devtype> getdevice(unsigned short product_id, const char *path) {
      if constexpr (N == products.size())
        //return std::make_unique<streamdecktype<products[N - 1]>>("");
        return nullptr;
      else
      {
        if (product_id == products[N]) return std::make_unique<streamdecktype<products[N]>>(path);
        return getdevice<N + 1>(product_id, path);
      }
    }
    /// @brief imagewrapper wraps an image.
    struct imagewrapper
    {
      std::vector<unsigned char> data;
      imagewrapper() : front(data.data()), back(data.data() + data.size()) {}
      imagewrapper(const std::vector<unsigned char> &imageData) : data(imageData), front(data.data()), back(data.data() + data.size()) {}
      auto begin() const { return front; } ///< Returns beginning of data.
      auto end() const { return back; }    ///< Returns end of data.
      const unsigned char *front;          ///< The beginning of data.
      const unsigned char *back;           ///< The end of data.
    };
  }
}
namespace decklibrary {
  /// @brief Construct the devtype object.
  /// @param path The Stream Deck path.
  /// @param width The image pixel width.
  /// @param height The image pixel height.
  /// @param cols The number of button columns.
  /// @param rows The number of button rows.
  /// @param imgfmt The image format.
  /// @param reportlength The length of the image HID report.
  /// @param hflip Horizontal flip flag.
  /// @param vflip Vertical flip flag.
  devtype::devtype(const char *path, unsigned width, unsigned height, unsigned cols, unsigned rows, imagetype imgfmt, unsigned reportlength, bool hflip, bool vflip) : pixelwidth(width), pixelheight(height), btncols(cols), btnrows(rows), btncount(rows * cols), imageformat(imgfmt), fliphorizontal(hflip), flipvertical(vflip), hidreportlength(reportlength), devpath(path), devhndl(hid_open_path(devpath)) {
    showwindow = true;
  }
  /// @brief Destruct the devtype object.
  devtype::~devtype() {
    close();
  }
  /// @brief Close the Stream Deck connection.
  /// @see connected()
  void devtype::close() {
    if (connected()) hid_close(devhndl);
  }
  /// @brief A button image is formatted to match the target button format.
  /// This function uses OpenCV to perform image reformatting operations, including flipping, resizing, and encoding.
  /// @param sourceimage The source image.
  /// @return The formatted image.
  std::vector<unsigned char> devtype::reformat(std::vector<unsigned char> sourceimage) {
    cv::Mat image = cv::imdecode(sourceimage, cv::IMREAD_COLOR);
    if (fliphorizontal)
      cv::flip(image, image, 1); // Flip horizontally
    if (flipvertical)
      cv::flip(image, image, 0); // Flip vertically
    cv::Size size = image.size();
    if (size.width != static_cast<int>(pixelwidth) || size.height != static_cast<int>(pixelheight))
    {
      double factor = std::min(static_cast<double>(pixelwidth) / size.width, static_cast<double>(pixelheight) / size.height);
      cv::Size new_size(static_cast<int>(size.width * factor), static_cast<int>(size.height * factor));
      cv::resize(image, image, new_size);
      if (new_size.width != static_cast<int>(pixelwidth) || new_size.height != static_cast<int>(pixelheight))
      {
        cv::Mat new_image(cv::Size(static_cast<int>(pixelwidth), static_cast<int>(pixelheight)), image.type(), cv::Scalar(0, 0, 0));
        cv::Rect roi((static_cast<int>(pixelwidth) - new_size.width) / 2, (static_cast<int>(pixelheight) - new_size.height) / 2, new_size.width, new_size.height);
        image.copyTo(new_image(roi));
        cv::resize(new_image, image, cv::Size(static_cast<int>(pixelwidth), static_cast<int>(pixelheight)));
      }
    }
    std::vector<unsigned char> result;
    cv::imencode(".jpg", image, result);
    return result;
  }
  /// @brief A thread is launched to monitor a Stream Deck Device.
  /// @param index Selects a Stream deck device.
  /// @param ctx The array of connected devices.
  /// @param callback The callback function which receives data.
  void devtype::run(int index, context &ctx, std::function<void(std::vector<bool> &)> callback) {
    std::thread t([index, this, &ctx, callback] {
      while (true) {
        std::vector<bool> data = ctx[index]->read();
        callback(data);
      }
    });
    t.detach();
  }
  /// @brief A thread is launched to monitor a named pipe for commands.
  /// @param widget The Mint Deck window.
  void devtype::cmdpipe(GtkWidget *widget) { 
    std::thread t([widget, this]
    {
      std::string fifostring;
      mkfifo(RECEIVE_PIPE, 0666);
      while (true) {
        std::ifstream pipe(RECEIVE_PIPE);
        getline(pipe, fifostring);
        deckcommand(fifostring, widget);
        pipe.close();
      }
    });
    t.detach();
  }
  /// @brief A command received from a named pipe is executed.
  /// @param cmd The command text.
  /// @param widget The Mint Deck window.
  void devtype::deckcommand(std::string cmd, GtkWidget *widget) {
    enum options {
      UNDEFINED, TOGGLE, HIDE, SHOW
    };
    std::map<std::string, options> mapcommand = {
      {"toggledeck", TOGGLE},
      {"hidedeck", HIDE},
      {"showdeck", SHOW}
    };
    switch (mapcommand[cmd])
    {
    case UNDEFINED:
      break;
    case TOGGLE:
      gtk_widget_set_visible(widget, !showwindow);
      showwindow = !showwindow;
      break;
    case HIDE:
      if (showwindow) {
        gtk_widget_set_visible(widget, false);
        showwindow = (gboolean) false;
      }
      break;
    case SHOW:
      if (!showwindow) {
        gtk_widget_set_visible(widget, true);
        showwindow = (gboolean) true;
      }
      break;
    }
  }
  /// @brief Formats the button image for the Stream Deck target and sends the image to the HID write fuunction.
  /// @param button 
  /// @param buttonimage 
  /// @return 
  int devtype::writebuttonimage(int button, std::vector<unsigned char> buttonimage) {
    std::vector<unsigned char> formattedimage = reformat(buttonimage);
    return sendbuttonimage(button, formattedimage);
  }
  /// @brief Sends data packets to the Stream Deck.
  /// @param btn 
  /// @param formattedimage 
  /// @return Zero on success.
  int devtype::sendbuttonimage(unsigned btn, std::vector<unsigned char> formattedimage) {
    if (btn > btncount)
      return -1;
    imagewrapper wrappedimage(formattedimage);
    payloadtype buffer(hidreportlength);
    unsigned page = 0;
    for (auto srcit = wrappedimage.begin(); srcit != wrappedimage.end(); ++page) {
      auto destit = dataheader(buffer, btn, wrappedimage.end() - srcit, page);
      while (srcit != wrappedimage.end() && destit != buffer.end())
        *destit++ = std::byte(*srcit++);
      std::fill(destit, buffer.end(), std::byte(0));
      if (auto r = write(buffer); r < 0)
        return r;
    }
    return 0;
  }
  /// @brief Creates an array describing connected Stream Deck devices.
  context::context() {
    if (auto r = hid_init(); r < 0) throw std::runtime_error("hid_init failed with "s + std::to_string(r));
    devs = hid_enumerate(vendorelgato, 0);
    if (devs != nullptr) {
      for (auto p = devs; p != nullptr; p = p->next)
        if (auto ap = getdevice(p->product_id, p->path); ap)
          devinfo.emplace_back(std::move(ap));
    } else {
      devinfo.emplace_back(std::make_unique<streamdecktype<products[products.size()-1]>>(""));
    }
  }
  context::~context() {
    devinfo.clear();
    if (devs) hid_free_enumeration(devs);
    hid_exit();
  }
}
