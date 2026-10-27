
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
        cout << i + 1 << ". "
             << refreshRates[i] << " Hz\n";
    }

    cout << "\nChoose refresh rate (1-7): ";
    cin >> choice;

    if (!cin || choice < 1 || choice > 7) {
        cout << "Invalid choice!\n";
        return 1;
    }

    int targetHz = refreshRates[choice - 1];

    DEVMODE selected = {};
    bool supported = false;

    // Find a supported mode with the current resolution.
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

    // Test the mode before applying it.
    LONG result = ChangeDisplaySettings(&selected, CDS_TEST);

    if (result != DISP_CHANGE_SUCCESSFUL) {
        cout << "Display mode test failed!\n";
        return 1;
    }

    // Apply the selected refresh rate.
    result = ChangeDisplaySettings(&selected, 0);

    if (result == DISP_CHANGE_SUCCESSFUL) {
        cout << "\nSuccess! Refresh rate changed to "
             << targetHz << " Hz.\n";
    } else {
        cout << "\nFailed to change refresh rate!\n";
    }

    return 0;
}
