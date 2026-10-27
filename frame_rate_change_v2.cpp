#include <iostream>
#include <windows.h>
using namespace std;

int main() {
    int choice;
    int refreshRates[] = {30, 48, 60, 75, 100, 120, 165};

    DEVMODE current = {};
    current.dmSize = sizeof(DEVMODE);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &current)) {
        cout << "Cannot get display settings!\n";
        return 1;
    }

    cout << "=============================\n";
    cout << "   DISPLAY REFRESH RATE\n";
    cout << "=============================\n";

    cout << "Current Refresh Rate: "
         << current.dmDisplayFrequency << " Hz\n\n";

    for (int i = 0; i < 7; i++) {
        cout << refreshRates[i] << ". "
             << refreshRates[i] << " Hz\n";
    }

    cout << "\nChoose refresh rate: ";
    cin >> choice;

    if (!cin) {
        cout << "Invalid input!\n";
        return 1;
    }

    // Check if the entered refresh rate is in the list
    bool valid = false;

    for (int i = 0; i < 7; i++) {
        if (choice == refreshRates[i]) {
            valid = true;
            break;
        }
    }

    if (!valid) {
        cout << "Invalid refresh rate!\n";
        return 1;
    }

    // Use the entered refresh rate directly
    int targetHz = choice;

    DEVMODE selected = {};
    bool supported = false;

    // Find a supported mode with the current resolution
    for (DWORD i = 0; ; i++) {
        DEVMODE mode = {};
        mode.dmSize = sizeof(DEVMODE);

        if (!EnumDisplaySettings(NULL, i, &mode))
            break;

        if (mode.dmPelsWidth == current.dmPelsWidth &&
            mode.dmPelsHeight == current.dmPelsHeight &&
            mode.dmBitsPerPel == current.dmBitsPerPel &&
            mode.dmDisplayOrientation ==
                current.dmDisplayOrientation &&
            mode.dmDisplayFrequency == targetHz) {

            selected = mode;
            supported = true;
            break;
        }
    }

    if (!supported) {
        cout << "\n" << targetHz
             << " Hz is not supported at this resolution.\n";
        return 1;
    }

    // Test the mode before applying it
    LONG result = ChangeDisplaySettings(&selected, CDS_TEST);

    if (result != DISP_CHANGE_SUCCESSFUL) {
        cout << "Display mode test failed!\n";
        return 1;
    }

    // Apply the selected refresh rate
    result = ChangeDisplaySettings(&selected, 0);

    if (result == DISP_CHANGE_SUCCESSFUL) {
        cout << "\nSuccess! Refresh rate changed to "
             << targetHz << " Hz.\n";
    } else {
        cout << "\nFailed to change refresh rate!\n";
    }

    return 0;
}