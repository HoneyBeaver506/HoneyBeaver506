#include <cctype>     // For std::isspace
#include <climits>    // For INT_MAX
#include <filesystem> // For file system operations (C++17)
#include <fstream>    // For file operations
#include <iomanip>    // For std::setw
#include <iostream>
#include <limits>  // For std::numeric_limits
#include <sstream> // For string stream operations
#include <string>
#include <vector>

class CharacterGrid {
private:
  std::vector<char> items_;
  int num_columns_;
  char padding_char_;

public:
  // Constructor with string input
  CharacterGrid(const std::string &input_string, int num_columns)
      : num_columns_(num_columns), padding_char_('-') {

    if (num_columns_ <= 0) {
      std::cerr << "Error: Number of columns must be positive. Defaulting to 1."
                << std::endl;
      num_columns_ = 1;
    }

    initializeFromString(input_string);
  }

  // Constructor with file input
  CharacterGrid(const std::string &filename, int num_columns, bool from_file)
      : num_columns_(num_columns), padding_char_('-') {

    if (num_columns_ <= 0) {
      std::cerr << "Error: Number of columns must be positive. Defaulting to 1."
                << std::endl;
      num_columns_ = 1;
    }

    if (from_file) {
      if (!initializeFromFile(filename)) {
        std::cerr << "Error: Failed to read from file. Grid will be empty."
                  << std::endl;
      }
    } else {
      initializeFromString(filename); // Treat as string if not from file
    }
  }

private:
  void initializeFromString(const std::string &input_string) {
    // Reserve space to minimize reallocations
    items_.reserve(input_string.length());

    // Parse input string, removing whitespace
    for (char ch : input_string) {
      if (!std::isspace(static_cast<unsigned char>(ch))) {
        items_.push_back(ch);
      }
    }

    // Shrink to fit actual size
    items_.shrink_to_fit();

    if (items_.empty()) {
      std::cout << "Warning: Input contained no non-whitespace characters. "
                   "Grid will be empty."
                << std::endl;
    }
  }

  bool initializeFromFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
      return false;
    }

    // Get file size for efficient memory allocation
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size <= 0) {
      std::cout << "Warning: File is empty or cannot determine size."
                << std::endl;
      return true; // Not an error, just empty
    }

    // Reserve space based on file size (upper bound)
    items_.reserve(static_cast<size_t>(file_size));

    // Read file in chunks for better performance
    const size_t BUFFER_SIZE = 8192;
    std::vector<char> buffer(BUFFER_SIZE);

    while (file.read(buffer.data(), BUFFER_SIZE) || file.gcount() > 0) {
      std::streamsize bytes_read = file.gcount();

      for (std::streamsize i = 0; i < bytes_read; ++i) {
        char ch = buffer[static_cast<size_t>(i)];
        if (!std::isspace(static_cast<unsigned char>(ch))) {
          items_.push_back(ch);
        }
      }
    }

    file.close();

    // Shrink to fit actual size
    items_.shrink_to_fit();

    if (items_.empty()) {
      std::cout << "Warning: File contained no non-whitespace characters. Grid "
                   "will be empty."
                << std::endl;
    }

    return true;
  }

public:
  void displayGrid() const {
    if (items_.empty()) {
      std::cout << "Grid is empty." << std::endl;
      return;
    }

    // Display column headers
    std::cout << "   "; // Space for row numbers
    for (int c = 0; c < num_columns_; ++c) {
      std::cout << std::setw(3) << c;
    }
    std::cout << std::endl;

    // Display grid rows
    int rows = totalRows();
    for (int r = 0; r < rows; ++r) {
      std::cout << std::setw(3) << r << " ";
      for (int c = 0; c < num_columns_; ++c) {
        size_t current_index =
            static_cast<size_t>(r) * static_cast<size_t>(num_columns_) +
            static_cast<size_t>(c);
        if (current_index < items_.size()) {
          std::cout << std::setw(3) << items_[current_index];
        } else {
          std::cout << std::setw(3) << padding_char_;
        }
      }
      std::cout << std::endl;
    }
    std::cout << std::endl; // Add blank line after grid
  }

  bool getElementAt(int r, int c, char &out_char) const {
    // Check if coordinates are within grid bounds
    if (r < 0 || c < 0 || r >= totalRows() || c >= num_columns_) {
      return false;
    }

    // Calculate index safely
    size_t index = static_cast<size_t>(r) * static_cast<size_t>(num_columns_) +
                   static_cast<size_t>(c);

    if (index < items_.size()) {
      out_char = items_[index];
    } else {
      out_char = padding_char_; // Return padding character for valid
                                // coordinates in empty cells
    }
    return true;
  }

  bool getElementAtIndex(int index, char &out_char) const {
    if (index < 0 || static_cast<size_t>(index) >= items_.size()) {
      return false;
    }
    out_char = items_[static_cast<size_t>(index)];
    return true;
  }

  bool indexToCoordinates(int index, int &out_r, int &out_c) const {
    if (index < 0 || static_cast<size_t>(index) >= items_.size()) {
      return false;
    }
    out_r = index / num_columns_;
    out_c = index % num_columns_;
    return true;
  }

  bool coordinatesToIndex(int r, int c, int &out_index) const {
    // Check bounds
    if (r < 0 || c < 0 || r >= totalRows() || c >= num_columns_) {
      return false;
    }

    // Safe calculation using size_t to avoid overflow
    size_t safe_index =
        static_cast<size_t>(r) * static_cast<size_t>(num_columns_) +
        static_cast<size_t>(c);

    // Check if the result fits in an int
    if (safe_index > static_cast<size_t>(INT_MAX)) {
      return false;
    }

    out_index = static_cast<int>(safe_index);
    return true;
  }

  size_t totalItems() const { return items_.size(); }

  int totalRows() const {
    if (num_columns_ <= 0 || items_.empty()) {
      return 0;
    }
    // Calculate rows needed (ceiling division)
    return static_cast<int>(
        (items_.size() + static_cast<size_t>(num_columns_) - 1) /
        static_cast<size_t>(num_columns_));
  }

  // Additional utility methods
  void printGridInfo() const {
    std::cout << "Grid Information:" << std::endl;
    std::cout << "  Total items: " << totalItems() << std::endl;
    std::cout << "  Columns: " << num_columns_ << std::endl;
    std::cout << "  Rows: " << totalRows() << std::endl;
    std::cout << "  Padding character: '" << padding_char_ << "'" << std::endl;
    std::cout << std::endl;
  }

  bool isEmpty() const { return items_.empty(); }

  // Method to save grid to file
  bool saveToFile(const std::string &filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: Cannot create file '" << filename << "'"
                << std::endl;
      return false;
    }

    for (size_t i = 0; i < items_.size(); ++i) {
      file << items_[i];
      // Add space every num_columns_ characters for readability
      if ((i + 1) % static_cast<size_t>(num_columns_) == 0) {
        file << '\n';
      }
    }

    file.close();
    return true;
  }
};

// Helper function to check if file exists
bool fileExists(const std::string &filename) {
  std::ifstream file(filename);
  return file.good();
}

// Helper function to read an integer with validation
bool readInteger(const std::string &prompt, int &value) {
  std::cout << prompt;
  if (!(std::cin >> value)) {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid input. Please enter a valid integer." << std::endl;
    return false;
  }
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  return true;
}

// Helper function to read a positive integer
bool readPositiveInteger(const std::string &prompt, int &value) {
  if (!readInteger(prompt, value)) {
    return false;
  }
  if (value <= 0) {
    std::cout << "Invalid input. Number of columns must be a positive integer."
              << std::endl;
    return false;
  }
  return true;
}

// Helper function to get input source choice
int getInputChoice() {
  int choice;
  std::cout << "\nChoose input source:" << std::endl;
  std::cout << "1. Enter string manually" << std::endl;
  std::cout << "2. Read from file" << std::endl;

  while (!readInteger("Enter your choice (1 or 2): ", choice) ||
         (choice != 1 && choice != 2)) {
    std::cout << "Please enter 1 or 2." << std::endl;
  }

  return choice;
}

void printMenu() {
  std::cout << "\nOptions:" << std::endl;
  std::cout << "1. Get element by (row, column)" << std::endl;
  std::cout << "2. Get element by linear index" << std::endl;
  std::cout << "3. Convert index to (row, column)" << std::endl;
  std::cout << "4. Convert (row, column) to index" << std::endl;
  std::cout << "5. Display grid again" << std::endl;
  std::cout << "6. Show grid information" << std::endl;
  std::cout << "7. Save grid to file" << std::endl;
  std::cout << "8. Exit" << std::endl;
}

int main() {
  std::cout << "Character Grid Application" << std::endl;
  std::cout << "=========================" << std::endl;

  int input_choice = getInputChoice();
  std::unique_ptr<CharacterGrid> grid;

  int num_cols;
  while (!readPositiveInteger("Enter the number of columns for the grid: ",
                              num_cols)) {
    // Keep asking until valid input
  }

  if (input_choice == 1) {
    // Manual string input
    std::string input_str;
    std::cout << "Enter the input string: ";
    std::getline(std::cin, input_str);
    grid = std::make_unique<CharacterGrid>(input_str, num_cols);
  } else {
    // File input
    std::string filename;
    std::cout << "Enter the filename: ";
    std::getline(std::cin, filename);

    if (!fileExists(filename)) {
      std::cerr << "Error: File '" << filename << "' does not exist."
                << std::endl;
      return 1;
    }

    grid = std::make_unique<CharacterGrid>(filename, num_cols, true);
  }

  if (!grid->isEmpty()) {
    grid->printGridInfo();
    grid->displayGrid();
  }

  int choice;
  char out_char;
  int r, c, index, out_r, out_c, out_index;

  while (true) {
    printMenu();

    if (!readInteger("Enter your choice: ", choice)) {
      continue;
    }

    switch (choice) {
    case 1: // Get by coordinates
      if (!readInteger("Enter row: ", r) || !readInteger("Enter column: ", c)) {
        continue;
      }
      if (grid->getElementAt(r, c, out_char)) {
        std::cout << "Element at (" << r << ", " << c << "): '" << out_char
                  << "'" << std::endl;
      } else {
        std::cout << "Invalid coordinates." << std::endl;
      }
      break;

    case 2: // Get by index
      if (!readInteger("Enter index: ", index)) {
        continue;
      }
      if (grid->getElementAtIndex(index, out_char)) {
        std::cout << "Element at index " << index << ": '" << out_char << "'"
                  << std::endl;
      } else {
        std::cout << "Invalid index." << std::endl;
      }
      break;

    case 3: // Index to coordinates
      if (!readInteger("Enter index: ", index)) {
        continue;
      }
      if (grid->indexToCoordinates(index, out_r, out_c)) {
        std::cout << "Index " << index << " corresponds to (row: " << out_r
                  << ", col: " << out_c << ")" << std::endl;
      } else {
        std::cout << "Invalid index." << std::endl;
      }
      break;

    case 4: // Coordinates to index
      if (!readInteger("Enter row: ", r) || !readInteger("Enter column: ", c)) {
        continue;
      }
      if (grid->coordinatesToIndex(r, c, out_index)) {
        std::cout << "Coordinates (" << r << ", " << c
                  << ") correspond to index " << out_index << std::endl;
      } else {
        std::cout << "Invalid coordinates." << std::endl;
      }
      break;

    case 5: // Display grid
      grid->displayGrid();
      break;

    case 6: // Show grid information
      grid->printGridInfo();
      break;

    case 7: // Save grid to file
    {
      std::string save_filename;
      std::cout << "Enter filename to save: ";
      std::getline(std::cin, save_filename);
      if (grid->saveToFile(save_filename)) {
        std::cout << "Grid saved successfully to '" << save_filename << "'"
                  << std::endl;
      }
    } break;

    case 8: // Exit
      std::cout << "Exiting program. Goodbye!" << std::endl;
      return 0;

    default:
      std::cout << "Invalid choice. Please select 1-8." << std::endl;
      break;
    }
  }

  return 0;
}
