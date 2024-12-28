#include "menu_state.h"
#include "../../../core/temperature_point.h"
#include "../../../utils/terminalTextStyles.h"
#include "../menu.h"

#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <utility>

void MenuModeManager::controlMode() {
  if (mode == MenuMode::control) {
    return;
  }

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  mode = MenuMode::control;

  return;
}

void MenuModeManager::inputMode() {
  if (mode == MenuMode::input) {
    return;
  }

  tcsetattr(STDIN_FILENO, TCSADRAIN, &oldt);

  mode = MenuMode::input;

  return;
}

void MenuState::printControlsHelp() {
  using namespace terminalTextStyles;

  cout << "Controls: " << endl;
  cout << "  - Type " << BOLD << "\"Shift + i\"" << RESET
       << "to hide this help. " << endl;
  cout << "  - Type \"Shift + g\" to display graph. " << endl;
  cout << "  - Type \"Shift + f\" to display filters. " << endl;
  cout << "  - Use the arrow keys to navigate the menu." << endl;
  cout << "  - Or use j, k, h, l to navigate the menu." << endl;
  cout << "  - Press 'Enter' to select an option." << endl;
  cout << "  - Press 'q' to quit the application." << endl;
  return;
}

void MenuState::render(Menu &menu) {
  cout << "==== " << this->title << " ====" << endl;

  if (MenuModeManager::mode == MenuMode::control) {
    this->printControlsHelp();
  }

  for (unsigned int i = 0; i < options.size(); i++) {
    if (i == menu.getChoice()) {
      cout << " > " << options[i] << " < " << endl;
    } else {
      cout << "   " << options[i] << "   " << endl;
    }
  }

  return;
}

MainMenu::MainMenu() {
  title = "Main Menu";
  options = {"1. Help", "2. Weather Graph", "3. Weather Prediction",
             "4. Select country", "5. Quit"};
  MenuModeManager::controlMode();
}

void MainMenu::handleChoice(Menu &menu, const unsigned int &optionIndex) {
  if (optionIndex + 1 == 1) {
    this->printHelp();
  } else if (optionIndex + 1 == 2) {
    menu.changeState(GraphMenu());
  } else if (optionIndex + 1 == 3) {
    menu.changeState(GraphMenu());
  } else if (optionIndex + 1 == 4) {
    menu.changeState(CountrySelectionMenu());
  } else if (optionIndex + 1 == 5) {
    exit(0);
  } else {
    cout << "Invalid choice! Please select a number between 1 and "
         << this->options.size() << "." << endl;
  }

  return;
}

GraphMenu::GraphMenu() {
  title = "Graph Menu";
  options = {
      "1. Graph Settings",
      "2. Filters",
      "3. Back",
  };
  MenuModeManager::controlMode();
}

void GraphMenu::handleChoice(Menu &menu, const unsigned int &choice) {
  if (choice == 1) {
    menu.setCoreEvents(true);
  } else if (choice == 2) {
    menu.changeState(MainMenu());
  } else if (choice == 3) {
    menu.changeState(MainMenu());
  } else {
    cout << "Invalid choice! Please select a number between 1 and "
         << this->options.size() << "." << endl;
  }

  return;
}

CountrySelectionMenu::CountrySelectionMenu() {
  title = "Select Country";
  // convert enum values to the vector of string
  options = countries();
  MenuModeManager::controlMode();
}

void CountrySelectionMenu::handleChoice(Menu &menu,
                                        const unsigned int &choice) {
  if (choice >= options.size()) {
    cout << "Invalid choice! Please select a number between 1 and "
         << this->options.size() << "." << endl;
    return;
  }
}

vector<string> CountrySelectionMenu::countries() {
  vector<string> countries;

  for (pair<string, EULocation> locationPair : locationsMap) {
    if (locationPair.second == EULocation::uknown) {
      continue;
    }

    countries.emplace_back(locationPair.first);
  }

  return countries;
}
