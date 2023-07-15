/// @file decklib.hpp
#ifndef DEVTYPE
#define DEVTYPE

#include <experimental/array>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <hidapi.h>
#include <jpeglib.h>
#include <opencv2/opencv.hpp>
#include <jerror.h>
#include <thread>
#include <bitset>
#include <filesystem>

#include "keymap.hpp"
#include "deck.hpp"
#include "test.hpp"

#define INHERIT_DOCS
#define RECEIVE_PIPE "/tmp/receivemintdeck"

static_assert(__cpp_static_assert >= 200410L, "extended static_assert missing");
static_assert(__cpp_concepts >= 201907L);
static_assert(__cpp_if_constexpr >= 201606L);
static_assert(__cpp_lib_ranges >= 201911L);
static_assert(__cpp_lib_byte >= 201603L);
static_assert(__cpp_lib_clamp >= 201603L);
static_assert(__cpp_lib_make_unique >= 201304L);
static_assert(__cpp_lib_optional >= 201606L);

/// @brief Namespace @namespace decklibrary controls a Stream Deck device.
namespace decklibrary
{
  static constexpr uint16_t vendorelgato = 0x0fd9;          ///< The Stream Deck USB vendor ID.
  static constexpr uint16_t streamdeckoriginal = 0x0060;    ///< USB  SD original product ID.
  static constexpr uint16_t streamdeckoriginalv2 = 0x006d;  ///< USB SD original v2 product ID.
  static constexpr uint16_t streamdeckmini = 0x0063;        ///< USB SD mini product ID.
  static constexpr uint16_t streamdeckxl = 0x006c;          ///< USB SD XL product ID.
  static constexpr uint16_t streamdeckdemo = 0xffff;        ///< USB SD demo product ID.
  struct context;

  class devtype /// Core functionality for USB communication.
  {
  public:
    enum struct imagetype
    {
      bmp,      ///< Bitmap image format.
      jpeg      ///< JPEG image format.
    };
    /// @brief Construct a new device type object
    /// @param path The path to the device.
    /// @param width Width in pixels.
    /// @param height Height in pixels.
    /// @param cols Grid columns.
    /// @param rows Grid rows.
    /// @param imgfmt Image format to use.
    /// @param reportlength Image replacement string length.
    /// @param hflip Horizontal flip.
    /// @param vflip Vertical flip.
    devtype(const char *path, unsigned width, unsigned height, unsigned cols, unsigned rows, imagetype imgfmt, unsigned reportlength, bool hflip, bool vflip);

    ~devtype();

    /// @brief Checks whether the device is connected.
    /// True is returned if the device is connected.
    /// or not. The internal pointer to the device (devhndl) is checked.
    /// Not null, shows connection.
    /// @return Indicates current connection.
    bool connected() const { return devhndl != nullptr; }
    /// @brief The device path name.
    /// @return Points to a file path string.
    auto path() const { return devpath; }
    /// @brief  Closes the HID device.
    /// Checks connection and then closes the connection.
    void close();
    const unsigned pixelwidth;      ///< Button pixel width.
    const unsigned pixelheight;     ///< Button pixel height.
    const unsigned btncols;         ///< Button column count.
    const unsigned btnrows;         ///< Button row count.
    const unsigned btncount;        ///< Button count.
    const imagetype imageformat;    ///< Button image format.
    const bool fliphorizontal;      ///< Image flip horizontal.
    const bool flipvertical;        ///< Image flip vertical.
    const unsigned hidreportlength; ///< HID image report length.
    /// Used to send data to the HID.
    using payloadtype = std::vector<std::byte>;
    /// @brief The image is reformatted and written to the HID.
    /// @param button 
    /// @param buttonimage 
    /// @return int 0 on success.
    virtual int writebuttonimage(int button, std::vector<unsigned char> buttonimage);
    /// @brief Set the key image
    /// The image is paged and sent to the device.
    /// @tparam C A container with a begin() and end() method such as std::vector.
    /// @param  The key number.
    /// @param data
    /// @return int 0 on success.
    int sendbuttonimage(unsigned key, std::vector<unsigned char> formattedimage);
    /// @brief Launches detatched threads.
    /// A detached thread Sends data from a connected device to @see keypress.
    /// @param index Index to an connected device.
    /// @param ctx An array of connected devices.
    /// @param btnstate Function that processes keys pressed.
    virtual void run(int index, context& ctx, std::function<void(std::vector<bool>&)> btnstate);
    /// @brief  A detatched thread receives commands from a named pipe.
    /// Sends commands to @see deckcommand.
    /// @param widget The MintDeck window.
    void cmdpipe(GtkWidget* widget);
    /// @brief Adds a header to the beginning of buffer
    /// A virtual function
    /// Adds a header to data sent to the HID.
    /// @param buffer Buffer receiving the header.
    /// @param key The key number.
    /// @param remaining Count of remaining data.
    /// @param page The header page number.
    /// @return An iterator pointing to the byte after the header.
    virtual payloadtype::iterator dataheader(payloadtype &buffer, unsigned key, unsigned remaining, unsigned page) = 0;
    /// @brief Reads key states from the HID.
    /// A virtual function
    /// @note Byte values from each key are compared to zero.
    /// @return A vector of boolean.
    virtual std::vector<bool> read() = 0;
    /// @brief Resets the device using a USB HID protocol.
    /// Send a reset to the HID.
    /// @see sendreport()
    virtual void reset() = 0;
    /// @brief Returns the Stream Deck serial number.
    /// @return std::string S/N text.
    /// @see getstring()
    virtual std::string getserialnumber() = 0;
    /// @brief Gets the SD firmware version.
    /// @return std::string
    /// @see getstring()
    virtual std::string getfirmwareversion() = 0;
    /// @brief Sets screen brightness from byte value.
    /// @param p A value in range [0,100].
    /// @see sendreport()
    virtual void setbrightness(std::byte p) = 0;
    /// @brief Sends a report in HID format.
    /// Data is an unsigned character buffer.  Length `len` is specified.
    /// @param data A character buffer report data.
    /// @param len Buffer length in bytes.
    /// @return The result of `hid_send_feature_report`.
    /// @note hid_send_feature_report binding used
    auto sendreport(const unsigned char *data, size_t len) { return hid_send_feature_report(devhndl, data, len); }
    /// @brief Sends a report in HID format.
    /// Data is a contiguous range.
    /// @tparam C
    /// @param data
    /// @return The result of the `hid_send_feature_report`.
    /// @note A `requires` clause enforces the type of `data` be a contiguous_range.
    /// @note hid_send_feature_report binding used
    template <typename C>requires std::ranges::contiguous_range<C> auto sendreport(const C &data) {
      return hid_send_feature_report(devhndl, (const unsigned char *)data.data(), data.size());
    }
    /// @brief Get a feature report from the HID device.
    /// @param data A buffer to put the report into.
    /// @param len  Number of bytes to read + 1 for the report ID.
    /// @return Number of bytes read.  -1 on error.
    /// @note hid_get_feature_report binding used
    auto getreport(unsigned char *data, size_t len) { return hid_get_feature_report(devhndl, data, len); }
    /// @brief Retrieves a feature report from the HID device.
    /// @tparam C The type of param data.
    /// @param data A range with contiguous memory storage of type C.
    /// @return Number of bytes read.  -1 on error.
    /// @note hid_get_feature_report binding used
    template <typename C>requires std::ranges::contiguous_range<C> auto getreport(C &data) {
      return hid_get_feature_report(devhndl, (unsigned char *)data.data(), data.size());
    }
    /// @brief Write to the HID device.
    /// @param data An array of unsigned character containing data.
    /// @param len Length in bytes of the data.
    /// @return Number of bytes written, or -1 on error.
    /// @note hid_write binding used
    auto write(const unsigned char *data, size_t len) { return hid_write(devhndl, data, len); }
    /// @brief Writes to the HID device.
    /// @tparam C The type of contiguous range.
    /// @param data A contiguous range of data type.
    /// @return Number of bytes written, or -1 on error.
    /// @note hid_write binding used
    template <typename C>requires std::ranges::contiguous_range<C>auto write(const C &data) {
      assert(data.size() == hidreportlength);
      return hid_write(devhndl, (const unsigned char *)data.data(), hidreportlength);
    }
    /// @brief Reads an input report from the HID.
    /// @param data A byte array.
    /// @param len The length of the array.
    /// @return Number of bytes read, or -1 on error.
    /// @note hid_read binding used
    auto read(unsigned char *data, size_t len) { return hid_read(devhndl, data, len); }
    /// @brief Reads an input report from the HID.
    /// @tparam C The type of contiguous range.
    /// @param data A contiguous range of data type.
    /// @return Number of bytes read, or -1 on error.
    /// @note hid_read binding used
    template <typename C>requires std::ranges::contiguous_range<C>auto read(C &data) {
      return hid_read(devhndl, (unsigned char *)data.data(), data.size());
    }
    /// @brief A command line is received over a named pipe and executed.
    /// Mintdeck can be shown or hidden using the widget parameter.
    /// @param cmd A command line.
    /// @param widget The MintDeck window.
    /// @see hashstring A helper function.
    void deckcommand(std::string cmd, GtkWidget* widget);
  private:
    const char *const devpath;
    hid_device *const devhndl;
    bool showwindow;
    /// @brief  Formats an image for the HID.
    /// @param buttonimage 
    /// @return The formatted image.
    std::vector<unsigned char> reformat(std::vector<unsigned char> buttonimage);
  };
  /// @brief Connected HID devices.
  /// A vector of connected devices accessed by member functions.
  class context
  {
  public:
    context();                                        ///< Creates a vector of device_type for each HID.
    ~context();                                       ///< Deallocates the device_type vector.
    bool empty() const { return devinfo.empty(); }    ///< @return Checks for connected HID devices.
    auto size() const { return devinfo.size(); }      ///< @return The number of HID devices.
    auto begin() { return devinfo.begin(); }          ///< @return Iterator at the beginning of the device_type vector.
    auto end() { return devinfo.end(); }              ///< @return Iterator at the end of the device_type vector.
    auto &operator[](size_t n) { return devinfo[n]; } ///< @return A connected device_type by index.
  private:
    hid_device_info *devs = nullptr;                  ///< points to a HID list of connected devices.
    std::vector<std::unique_ptr<devtype>> devinfo;    ///< Vector of `device_type` for connected devices.
  };
}

#endif
