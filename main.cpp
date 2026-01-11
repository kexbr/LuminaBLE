/*pactl load-module module-remap-source \
    master=alsa_output.pci-0000_00_1f.3.analog-stereo.monitor \
    source_name=virt_mic \
    source_properties=device.description=Virtual_System_Output*/

#include <simpleble/Peripheral.h>
#include "commands.h"
#include "simpleble/SimpleBLE.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <thread>
#include "commands.h"
#include <rtaudio/RtAudio.h>
#include <fftw3.h>
#include <atomic>

SimpleBLE::Peripheral GetController() {
    if (!SimpleBLE::Adapter::bluetooth_enabled()) {
        std::cout << "Bluetooth is disabled" << std::endl;
        exit(1);
    }
    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty()) {
        std::cout << "No bluetooth adapters found" << std::endl;
        exit(2);
    }
    auto adapter = adapters[0];
    std::vector<SimpleBLE::Peripheral> peripherals;
    std::cout << "Scanning from connected devices..." << std::endl;
    auto connected_pers = adapter.get_paired_peripherals();
    for (auto& peripheral : connected_pers) {
        if (peripheral.identifier().find("BLEDOM") != std::string::npos ||
            peripheral.identifier() == "ELK-BLEDOM") {
            if (peripheral.is_connected()) {
                peripherals.push_back(peripheral);
                std::cout << "Found connected peripheral" << std::endl;
                goto cancel_scanning;
            }
        }
    }
    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        std::cout << "Device: " << peripheral.identifier() << "[" << peripheral.address() << "]"
                  << std::endl;
        if (peripheral.identifier() == "BLEDOM" || peripheral.identifier() == "ELK-BLEDOM" ||
            peripheral.identifier() == "ELK-BLEDOM       ") {
            peripherals.push_back(peripheral);
        }
    });
rescan:;
    adapter.scan_for(2000);
    if (peripherals.empty()) {
        std::cout << "NO Bledom peripherals found. Rescanning..." << std::endl;
        goto rescan;
    }
cancel_scanning:;
    peripherals[0].connect();
    return peripherals[0];
}

const int N = 1024;
float* fft_in = nullptr;
fftwf_complex* fft_out = nullptr;
fftwf_plan plan = nullptr;
std::vector<float> window(N);

constexpr const size_t BUFFER_SIZE = 10;
constexpr const size_t MAGNITUDE_COUNT = 10;
std::atomic<float> magnitude_sum[BUFFER_SIZE][MAGNITUDE_COUNT];
const float freq_boundaries[MAGNITUDE_COUNT] = {
    48,
    100,    // Sub-bass
    250,    // Bass
    500,    // Low mids
    1000,   // Mids
    2000,   // High mids
    4000,   // Presence
    8000,   // Brilliance
    12000,  // Air
    16000   // Ultra high
};
std::atomic<size_t> head_pos = 0;
int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status, void* userData) {
    if (status)
        std::cerr << "Stream overflow/underflow detected!" << std::endl;
    float* samples = (float*)inputBuffer;
    for (int i = 0; i < N; i++) {
        fft_in[i] = 0.0f;
    }
    unsigned int framesToProcess = std::min((unsigned int)N, nBufferFrames);
    for (unsigned int i = 0; i < framesToProcess; i++) {
        fft_in[i] = samples[i] * window[i];
    }
    fftwf_execute(plan);
    float sum[MAGNITUDE_COUNT];
    for (size_t i = 0; i < MAGNITUDE_COUNT; i++) {
        sum[i] = 0.0f;
    }
    for (int i = 1; i < N / 2 + 1; i++) {
        float real = fft_out[i][0];
        float imag = fft_out[i][1];
        float magnitude = std::sqrt(real * real + imag * imag);
        float freq = i * (44100.0f / N);
        for (size_t j = 0; j < MAGNITUDE_COUNT; j++) {
            if (freq <= freq_boundaries[j]) {
                sum[j] += magnitude;
                break;
            }
        }
    }
    for (size_t i = 0; i < MAGNITUDE_COUNT; i++) {
        magnitude_sum[head_pos][i].store(sum[i]);
    }
    head_pos.store((head_pos.load() + 1) % BUFFER_SIZE);
    return 0;
}

const float bass_coef = 25.0f;
const float mid_coef = 12.0f;
const float high_coef = 10.0f;

uint8_t prev_red = 0;
uint8_t prev_green = 0;
uint8_t prev_blue = 0;

const float power_val = 1.2f;
const float smooth_alpha = 0.18f;
const float silence_threshold = 5.0f;

SimpleBLE::Peripheral controller;

float f_red = 0, f_green = 0, f_blue = 0;

void Visualizer() {
    float visual_coeffs[MAGNITUDE_COUNT] = {2.0f, 2.0f, 1.0f, 1.0f, 1.0f,
                                            1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float diff_trigger[MAGNITUDE_COUNT] = {10.0f, 10.0f, 12.0f, 15.0f, 20.0f, 20.0f, 22.0f, 15.0f, 15.0f, 15.0f};
    float min_mag[MAGNITUDE_COUNT];
    float max_mag[MAGNITUDE_COUNT];
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        size_t pos = head_pos.load();
        size_t start_pos = (pos + 1) % BUFFER_SIZE;
        for (size_t i = start_pos; i < start_pos + BUFFER_SIZE; i++) {
            for (size_t j = 0; j < MAGNITUDE_COUNT; j++) {
                min_mag[j] = (i == start_pos ? 9999999.0f : std::min(min_mag[j], magnitude_sum[i % BUFFER_SIZE][j].load()));
                max_mag[j] = (i == start_pos ? 0.0f : std::max(max_mag[j], magnitude_sum[i % BUFFER_SIZE][j].load()));
            }
        }
        for (size_t i = 0; i < MAGNITUDE_COUNT; i++) {
            if (std::clamp((max_mag[i] - min_mag[i]) * visual_coeffs[i] / 10.0f, 0.0f, 40.0f) >= diff_trigger[i]) {
                switch (i) {
                    case 0: 
                    Command::SetPattern(Pattern::kRedGradualChange, controller);
                    break;
                    case 1:
                    Command::SetPattern(Pattern::kRedGradualChange, controller);
                    break;
                    case 2:
                    Command::SetPattern(Pattern::kYellowGradualChange, controller);
                    break;
                    case 3:
                    Command::SetPattern(Pattern::kYellowGradualChange, controller);
                    break;
                    case 4:
                    Command::SetPattern(Pattern::kGreenGradualChange, controller);
                    break;
                    case 5:
                    Command::SetPattern(Pattern::kGreenGradualChange, controller);
                    break;
                    case 6:
                    Command::SetPattern(Pattern::kCyanGradualChange, controller);
                    break;
                    case 7:
                    Command::SetPattern(Pattern::kCyanGradualChange, controller);
                    break;
                    case 8: 
                    Command::SetPattern(Pattern::kBlueGradualChange, controller);
                    break;
                    case 9:
                    Command::SetPattern(Pattern::kBlueGradualChange, controller);
                    break;
                    default:
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
            }
        }
    }
}

int main() {
    fft_in = fftwf_alloc_real(N);
    fft_out = fftwf_alloc_complex(N / 2 + 1);
    plan = fftwf_plan_dft_r2c_1d(N, fft_in, fft_out, FFTW_ESTIMATE);
    window.assign(N, 0.0f);
    for (int i = 0; i < N; i++) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (N - 1)));
    }
    controller = GetController();
    Command::SetPatternSpeed(100, controller);
    RtAudio adc(RtAudio::LINUX_PULSE);
    RtAudio::StreamParameters iParams;
    if (adc.getDeviceCount() < 1) {
        std::cout << "No sound devices found!" << std::endl;
        return 3;
    }
    unsigned int deviceId = 0;
    bool found = false;

    for (unsigned int i = 0; i < adc.getDeviceCount(); i++) {
        RtAudio::DeviceInfo info = adc.getDeviceInfo(i);
        std::cout << "ID: " << i << " | Name: " << info.name << " | In: " << info.inputChannels
                  << " | Out: " << info.outputChannels << std::endl;
        if (info.name.find("Output") != std::string::npos && info.inputChannels > 0) {
            deviceId = i;
            found = true;
        }
    }
    if (!found) {
        std::cout << "Device not found. Using standart input" << std::endl;
        iParams.deviceId = adc.getDefaultInputDevice();
    } else {
        iParams.deviceId = deviceId;
    }
    iParams.nChannels = 1;
    iParams.firstChannel = 0;
    unsigned int bufferFrames = 512;
    unsigned int sampleRate = 44100;
    for (int i = 0; i < N; i++) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (N - 1)));
    }
    std::cout << "starting catching" << std::endl;
    try {
        adc.openStream(nullptr, &iParams, RTAUDIO_FLOAT32, sampleRate, &bufferFrames,
                       &audioCallback);
        adc.startStream();
    } catch (RtAudioError& e) {
        e.printMessage();
        return 4;
    }
    std::thread th(Visualizer);
    th.detach();
    std::cout << "Processing catching... Press enter to abort." << std::endl;
    std::cin.get();
    try {
        adc.stopStream();
        adc.closeStream();
    } catch (RtAudioError& e) {
        e.printMessage();
        return 5;
    }
    adc.stopStream();
    fftwf_destroy_plan(plan);
    fftwf_free(fft_in);
    fftwf_free(fft_out);
    return 0;
}
