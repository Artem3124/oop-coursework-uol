#include "./menu.h"
#include "../../core/temperaturePoint.h"
#include "states/menuState.h"
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>

#define ESCAPE '\x1b'
#define ARROW '['
#define UP 'A'
#define DOWN 'B'

using namespace std;

void MenuOptions::setOptions(bool *showControls, bool *showFilters) {
  if (showControls != nullptr) {
    this->showControls = *showControls;
  }

  if (showFilters != nullptr) {
    this->showFilters = *showFilters;
  }

  return;
}

const bool &MenuOptions::getShowControls() { return this->showControls; }
const bool &MenuOptions::getShowFilters() { return this->showFilters; }

Menu::Menu(const TemperatureMenuDataTransfer &_parser,
           const MenuOptions &_options)
    : currentChoice(0), state(new MainMenu()),
      parser(shared_ptr<TemperatureMenuDataTransfer>(
          new TemperatureMenuDataTransfer(_parser))),
      coreEvents(new ExternalCoreEvents(false, false)),
      options(new MenuOptions(_options)) {};

// Menu::~Menu() { delete Menu::instance; }
Menu::~Menu() = default;

void Menu::changeState(MenuState *_state) {
  this->state = unique_ptr<MenuState>(_state);
}

void Menu::setOptions(const MenuOptions &_parser) {
  this->options = unique_ptr<MenuOptions>(new MenuOptions(_parser));
}

const MenuOptions &Menu::getOptions() { return *this->options; }

void Menu::setChoice(const unsigned int &value) { this->currentChoice = value; }

int Menu::getChoice() { return this->currentChoice; }

void Menu::setParser(TemperatureMenuDataTransfer &_parser) {
  *this->parser = _parser;
}

const TemperatureMenuDataTransfer &Menu::getParser() { return *this->parser; }

void Menu::requestChoice() {
  char input[3];

  read(STDIN_FILENO, input, 3);

  const unsigned int optionsLength = this->state->getOptions().size();

  if (input[0] == ESCAPE) {
    if (input[1] == ARROW) {
      switch (input[2]) {
      case UP:
        this->setChoice((this->currentChoice - 1) % optionsLength);
        break;
      case DOWN:
        this->setChoice((this->currentChoice + 1) % optionsLength);
        break;
      }
    }
  }

  if (input[0] != ESCAPE) {
    switch (input[0]) {
    case 'I':
      this->options->setOptions(new bool(!this->options->getShowControls()),
                                nullptr);
      break;
    case 'G':
      this->coreEvents->graph = !this->coreEvents->graph;
      break;
    case 'F':
      this->options->setOptions(nullptr,
                                new bool(!this->options->getShowFilters()));
      break;
    case 'j':
      this->setChoice((this->currentChoice + 1) % optionsLength);
      break;
    case 'k':
      this->setChoice((this->currentChoice - 1) % optionsLength);
      break;
    case 'q':
      cout << "Quitting..." << endl;
      exit(0);
      break;
    case ' ':
      this->state->handleChoice(*this, this->currentChoice);
      break;
    }
  }
}

const unordered_map<FilterType, string> filtersMap = {
    {FilterType::location, "Location"}, {FilterType::timeRange, "Time range"}};

void Menu::run() {
  this->state->render(*this);
  this->requestChoice();

  return;
}

void Menu::handleChoice() {
  this->state->handleChoice(*this, this->currentChoice);
}

void Menu::setCoreEvents(bool showGraph) {
  this->coreEvents->graph = showGraph;
}

const ExternalCoreEvents &Menu::getCoreEvents() { return *this->coreEvents; }

Menu *Menu::instance = nullptr;
mutex Menu::mutex_;

Menu *Menu::getInstance(const TemperatureMenuDataTransfer &_parser,
                        const MenuOptions &_options) {
  lock_guard<mutex> lock(mutex_);

  if (!Menu::instance) {
    Menu::instance = new Menu(_parser, _options);
  }

  return Menu::instance;
}

TemperatureMenuDataTransfer::TemperatureMenuDataTransfer(
    GraphParametersDTO *_graphParameters, vector<FilterDTO<string>> *_filters) {
  if (_graphParameters) {
    this->graphParameters = shared_ptr<GraphParametersDTO>(_graphParameters);
  } else {
    this->graphParameters =
        shared_ptr<GraphParametersDTO>(new GraphParametersDTO(10, 10));
  }

  if (_filters) {
    this->filters = shared_ptr<vector<FilterDTO<string>>>(_filters);
  } else {
    this->filters =
        shared_ptr<vector<FilterDTO<string>>>(new vector<FilterDTO<string>>({
            FilterDTO<string>("1980-01-01T00:00:00Z|2019-12-31T23:00:00Z",
                              FilterType::timeRange),
            FilterDTO<string>(
                LocationEnumProcessor::locationToString(EULocation::de),
                FilterType::location),
        }));
  }
}

void TemperatureMenuDataTransfer::setFilters(
    const vector<FilterDTO<string>> &_filters) {
  *this->filters = _filters;
}

const vector<FilterDTO<string>> &
TemperatureMenuDataTransfer::getFilters() const {
  return *this->filters;
}

void TemperatureMenuDataTransfer::setGraphParameters(
    const GraphParametersDTO &_graphParameters) {
  *this->graphParameters = _graphParameters;
}

const GraphParametersDTO &
TemperatureMenuDataTransfer::getGraphParameters() const {
  return *this->graphParameters;
}
