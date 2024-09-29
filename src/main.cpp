#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [<host>:]<port>"
              << "<bulk_size>" << std::endl;
    return 1;
  }

  return 0;
}
