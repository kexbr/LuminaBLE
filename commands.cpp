#include "commands.h"
#include <simpleble/Peripheral.h>
#include <algorithm>
#include <iostream>
#include <vector>
void Command::OnOff(bool off_on, SimpleBLE::Peripheral& per) {
    std::vector<uint8_t> cmd = {
        0x7e,
        0x04,
        0x04,
        0x00,
        0x00,
        0x00,
        0xff,
        0x00,
        0xef
    };
    if (off_on) {
        cmd[3] = 0x01;
        cmd[5] = 0x01;
    }
    if (!per.initialized() || !per.is_connectable() || !per.is_connected()) {
        std::cerr << "unable to send OnOff command" << std::endl;
        return;
    }
    per.write_command(SERVICE_UUID, CHAR_UUID, cmd);
}

void Command::ChangeColor(uint8_t red, uint8_t green, uint8_t blue, SimpleBLE::Peripheral& per) {
    std::vector<uint8_t> cmd = {
        0x7e,
        0x07,
        0x05,
        0x03,
        red,
        green,
        blue,
        0x10,
        0xef
    };
    if (!per.initialized()) {
        std::cerr << "not initialized" << std::endl;
    }
    if (!per.is_connectable()) {
        std::cerr << "not connectable" << std::endl;
    }
    if (!per.is_connected()) {
        std::cerr << "not connected" << std::endl;
        per.connect();
    }
    if (!per.initialized() || !per.is_connectable() || !per.is_connected()) {
        std::cerr << "unable to send ChangeColor command" << std::endl;
        return;
    }
    per.write_command(SERVICE_UUID, CHAR_UUID, cmd);
}

void Command::ChangeBrightness(uint8_t brightness, SimpleBLE::Peripheral& per) {
    if (brightness > 100) {
        brightness = 100;
    }
    std::vector<uint8_t> cmd = {
        0x7e,
        0x04,
        0x01,
        brightness,
        0xff,
        0xff,
        0xff,
        0x00,
        0xef
    };
    if (!per.initialized() || !per.is_connectable() || !per.is_connected()) {
        std::cerr << "unable to send ChangeBrightness command" << std::endl;
        return;
    }
    per.write_command(SERVICE_UUID, CHAR_UUID, cmd);
}

void Command::SetPattern(Pattern pat, SimpleBLE::Peripheral& per) {
    std::vector<uint8_t> cmd = {
        0x7e,
        0x05,
        0x03,
        static_cast<uint8_t>(static_cast<uint8_t>(pat) + 128),
        0x03,
        0xff,
        0xff,
        0x00,
        0xef
    };
    if (!per.initialized()) {
        std::cerr << "not initialized" << std::endl;
    }
    if (!per.is_connectable()) {
        std::cerr << "not connectable" << std::endl;
    }
    if (!per.is_connected()) {
        std::cerr << "not connected" << std::endl;
        per.connect();
    }
    if (!per.initialized() || !per.is_connectable() || !per.is_connected()) {
        std::cerr << "unable to send SetPattern command" << std::endl;
        return;
    }
    per.write_command(SERVICE_UUID, CHAR_UUID, cmd);
}

void Command::SetPatternSpeed(uint8_t speed, SimpleBLE::Peripheral& per) {
    speed = std::clamp(speed, static_cast<uint8_t>(0), static_cast<uint8_t>(100));
    std::vector<uint8_t> cmd = {
        0x7e,
        0x04,
        0x02,
        speed,
        0xff,
        0xff,
        0xff,
        0x00,
        0xef
    };
    if (!per.initialized() || !per.is_connectable() || !per.is_connected()) {
        std::cerr << "unable to send SetPatternSpeed command" << std::endl;
        return;
    }
    per.write_command(SERVICE_UUID, CHAR_UUID, cmd);
}

void Command::SetClock(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day_of_week, SimpleBLE::Peripheral& per) {
    std::vector<uint8_t> cmd = {
        0x7e,
        0x07,
        0x83,
        hour,
        minute,
        second,
        day_of_week,
        0xff,
        0xef
    };
    if (hour > 23) return;
    if (minute > 59) return;
    if (second > 59) return;
    if (day_of_week > 6) return;
    if (!per.initialized() || !per.is_connectable() || !per.is_connected()) {
        std::cerr << "unable to send SetClock command" << std::endl;
        return;
    }
    per.write_command(SERVICE_UUID, CHAR_UUID, cmd);

}
