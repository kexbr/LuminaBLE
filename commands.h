#ifndef COMMANDS_H
#define COMMANDS_H

#include <simpleble/Peripheral.h>
#include <string>

enum class Pattern {
    kStaticRed,
    kStaticBlue,
    kStaticGreen,
    kStaticCyan,
    kStaticYellow,
    kStaticPurple,
    kStaticWhite,
    kThreeColorJumpingChange,
    kSevenColorJumpingChange,
    kThreeColorCrossFade,
    kSevenColorCrossFade,
    kRedGradualChange,
    kGreenGradualChange,
    kBlueGradualChange,
    kYellowGradualChange,
    kCyanGradualChange,
    kPurpleGradualChange,
    kWhiteGradualChange,
    kRedGreenCrossFade,
    kRedBlueCrossFade,
    kGreenBlueCrossFade,
    kSevenColorStrobeFlash,
    kRedStrobeFlash,
    kGreenStrobeFlash,
    kBlueStrobeFlash,
    kYellowStrobeFlash,
    kCyanStrobeFlash,
    kPurpleStrobeFlash,
    kWhiteStrobeFlash
};

inline constexpr const char* SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";
inline constexpr const char* CHAR_UUID    = "0000fff3-0000-1000-8000-00805f9b34fb";

class Command {
    public:
    // Turn off/on controller
    static void OnOff(bool off_on, SimpleBLE::Peripheral&);
    // Change the current state of stripe to static color
    static void ChangeColor(uint8_t red, uint8_t green, uint8_t blue, SimpleBLE::Peripheral&);
    // Change the current brightness of stripe (brightness is between 0 and 100!)
    static void ChangeBrightness(uint8_t brightness, SimpleBLE::Peripheral&);
    // Sets one of the presetted patterns.
    static void SetPattern(Pattern, SimpleBLE::Peripheral&);
    // Sets the speed of the pattern
    static void SetPatternSpeed(uint8_t, SimpleBLE::Peripheral&);
    // Set the internal controller clock.
    // CALLING CONVENCTIONS:
    // hour: between 0 and 23
    // minute: between 0 and 59
    // second: between 0 and 59
    // day of week: between 0 and 6. Sunday - 0, Saturday - 6
    static void SetClock(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day_of_week, SimpleBLE::Peripheral&);
};


#endif