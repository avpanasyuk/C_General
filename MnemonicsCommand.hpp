#ifndef MNEMONICSCOMMAND_HPP_INCLUDED
#define MNEMONICSCOMMAND_HPP_INCLUDED
namespace avp {
  class Command {
  public:
    static constexpr uint8_t NameLength = sizeof(uint32_t);
    static constexpr uint8_t MaxParamBytes = 100;
  protected:

    typedef void (* tFunc)(const uint8_t Params[]);  //<! callback function type
    static const Command *FindByID(uint32_t ID);

    const tFunc pFunc;
    union {
      const uint32_t ID; //!< command ID is just its ASCII name converted to uint32_t
      char Name[NameLength];
    };

    const uint8_t NumParamBytes;
    Command * pNext;
  public:
    Command(const char *Name_, tFunc pFunc_, uint8_t NumParamBytes_);
    bool IsIt(uint32_t ID_) const { return ID_ == ID; }

    static void Parse(uint8_t NewByte);

    // all functions below do thew same thing -  return object rT as data,
    // either though byte buffer, char buffer, or either with bytes being preferable
    /**
    * @brief Data pointed by pT should not be corrupted until pReleaseFunc is called.
    * @param size in Ts
    */
    template<typename T> static void ReturnPointed(const T *pT, uint16_t size = 1, uart::tReleaseFunc pReleaseFunc = nullptr) {
      // if we send uint16_t the lower byte gets sent first, so out error code detection by first byte sign does not work. Let's send it
      // one by one
      uint16_t ByteSize = sizeof(T)*size;
      uart::write(uint8_t(0)); // status
      uart::write(ByteSize);
      uart::write_pointed(pT, size, pReleaseFunc);
      uart::write(avp::sum<uint8_t>((const uint8_t *)pT,sizeof(T)*size));
    } // ReturnPointed

    //! following returns are buffering data
    template<typename T> static void Return(const T *pT, uint16_t size) {
      // if we send uint16_t the lower byte gets sent first, so out error code detection by first byte sign does not work. Let's send it
      // one by one
      uint16_t ByteSize = sizeof(T)*size;
      uint8_t CS = avp::sum<uint8_t>((const uint8_t *)pT,ByteSize);

      uart::write(uint8_t(0)); // status
      uart::write(ByteSize);
      while(size--) uart::write(*(pT++));
      uart::write(CS);
    } // ReturnPointed

    template<typename T> static void Return(T x) { Return<T>(&x,1); }

    static void ReturnOK() { uart::write(uint32_t(0)); } // 1 byte status, 2 - size and 1 - checksum
  }; // class Command

  class CommandLink {

  }; // class CommandLink
} // namespace avp

#endif /* MNEMONICSCOMMAND_HPP_INCLUDED */
