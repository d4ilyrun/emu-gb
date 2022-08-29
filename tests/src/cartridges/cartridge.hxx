extern "C" {
#include <cartridge/cartridge.h>
}

template <uint rom_size, uint ram_size = 1> class CartridgeGenerator
{
  public:
    CartridgeGenerator(cartridge_type type)
    {
        constexpr struct game_info info = {.game_title = "EMUGB_TEST",
                                           .manufacturer_code = "LEO"};

        header_ = (struct cartridge_header){
            .start_vector = {0x00, 0x00}, // NOP NOP
            .game_info = info,
            .new_license_code = 0x014B,
            .sgb_flag = 0,
            .type = (u8)type,
            .rom_size = header_rom_size(),
            .ram_size = header_ram_size(),
            .dst_code = 0x01,
            .old_license_code = 0x33,
        };

        cart_ = {.filename = "/home/user/nope",
                 .multicart = false,
                 .rom_size = rom_size,
                 .rom = rom_,
                 .ram = ram_};
    }

    auto GetCart() const
    {
        return cart_;
    }

  protected:
    struct cartridge_header header_;
    struct cartridge cart_;

    u8 rom_[rom_size]{0};
    u8 ram_[ram_size]{0};

  private:
    constexpr auto header_rom_size()
    {
        u8 res = 0;
        auto size = (rom_size >> 5);

        while (size >>= 1)
            res += 1;

        return res;
    }

    constexpr u8 header_ram_size()
    {
        switch (ram_size) {
        case 1 << 13:
            return 2;
        case 1 << 15:
            return 3;
        case 1 << 17:
            return 4;
        case 1 << 16:
            return 5;
        default:
            return 0;
        }
    }
};
