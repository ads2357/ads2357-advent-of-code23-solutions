#include <iostream>
#include <vector>
#include <memory>
#include <string>

class SchematicElement {
protected:
  int unsigned line;
  int unsigned column;

  virtual int unsigned lineMin() const {
    return line;
  }

  virtual int unsigned lineMax() const {
    return line;
  }

  virtual int unsigned colMin() const {
    return column;
  }

  virtual int unsigned colMax() const {
    return column;
  }

  friend bool isAdjacent(SchematicElement &a, SchematicElement &b);

  SchematicElement(int unsigned line, int unsigned column) : line(line), column(column) {}
};

class SchematicNumber : public SchematicElement {
  int unsigned length;
  int unsigned number;

  int unsigned colMax() const {
    return column + length - 1;
  }

  friend std::ostream& operator<<(std::ostream &os, const SchematicNumber &sn);

public:
  SchematicNumber(int unsigned line, int unsigned column,
                  int unsigned length, int unsigned number)
    : SchematicElement(line, column), length(length), number(number)
  {}

  int unsigned getNumber() {
    return number;
  }
};

std::ostream& operator<<(std::ostream &os, const SchematicNumber &sn) {
  return os << sn.number << " @" << sn.colMin() << "," << sn.colMax() << ";" << sn.lineMin();
}

class SchematicSymbol : public SchematicElement {
  char symbol;

  friend std::ostream& operator<<(std::ostream &os, const SchematicSymbol &ss);
public:
  SchematicSymbol(int unsigned column, int unsigned line, char symbol) :
    SchematicElement(column, line), symbol(symbol)
  {}

  char getSymbol() {
    return symbol;
  }
};

std::ostream& operator<<(std::ostream &os, const SchematicSymbol &ss) {
  return os << ss.symbol << "@" <<  ss.colMin() << "," << ss.colMax() << ";" << ss.lineMin();;
}

bool isAdjacent(SchematicElement &a, SchematicElement &b) {
  bool isColAdjacent = a.colMin() <= b.colMax()+1 && a.colMax()+1 >= b.colMin();
  bool isLineAdjacent = a.lineMin() <= b.lineMax()+1 && a.lineMax()+1 >= b.lineMin();
  return isColAdjacent && isLineAdjacent;
}


// TODO: this is bad, remove it, it reflects bad OOP.
std::ostream& operator<<(std::ostream& os, const SchematicElement &se) {
  try {
    const SchematicNumber& sn = dynamic_cast<const SchematicNumber&>(se);
    return os << sn;
  } catch (const std::bad_cast& e) {
    const SchematicSymbol& ss = dynamic_cast<const SchematicSymbol&>(se);
    return os << ss;
  }
}

void parse_elements(std::istream &is,
                    std::vector<SchematicNumber>& numbers_vec,
                    std::vector<SchematicSymbol>& symbols_vec)
{
  char c;
  int unsigned num = 0;
  enum { NONE, NUM, SYM } elem_state_tag = NONE, n_elem_state_tag = NONE;
  int unsigned line = 0;
  int unsigned column = 0;
  int unsigned length = 0;
  int unsigned symbol = '0';

  // Process labouriously character by character using a hand-crafted
  // state machine.  An alternative would be e.g. a regex library.
  while (is.get(c)) {
    // Assume input ends with a blank line.
    if (c >= '0' && c <= '9') {
      n_elem_state_tag = NUM;
      num *= 10;
      num += c - '0';
      ++ length;
    } else if (c == '.' || c == '\n') {
      n_elem_state_tag = NONE;
    } else { // symbol
      n_elem_state_tag = SYM;
      symbol = c;
    }

    if ((n_elem_state_tag != elem_state_tag && elem_state_tag != NONE)
        ) {
      switch (elem_state_tag) {
      case NUM:
        numbers_vec.emplace_back(
          SchematicNumber(line, column-1-(length-1), length, num));
        length = 0;
        num = 0;

        break;
      case SYM:
        symbols_vec.emplace_back(
          SchematicSymbol(line, column-1, symbol));

        break;
      default:
        ; // Do nothing
      }
    }

    ++ column;

    if (c == '\n') {
      column = 0;
      ++ line ;
    }

    elem_state_tag = n_elem_state_tag;
  }
}

int main(int argc, char **argv)
{
  std::vector<SchematicNumber> numbers_vec;
  std::vector<SchematicSymbol> symbols_vec;

  parse_elements(std::cin, numbers_vec, symbols_vec);

  int unsigned sum = 0;
  bool has_symbol;

  for (auto &num : numbers_vec) {
    has_symbol = false;
    for (auto &sym : symbols_vec) {
      if (isAdjacent(num, sym)) {
        sum += num.getNumber();
        has_symbol = true;
        break;
      }
    }

    if (!has_symbol) {
      std::cout << num << std::endl;
    }
  }

  std::cout << sum << std::endl;

  // part 2:

  int ratio_sum = 0;
  for (auto &sym : symbols_vec) {
    if (sym.getSymbol() != '*')
      continue;

    int number_cnt = 0;
    int ratio = 1;
    for (auto &num : numbers_vec) {
      if (isAdjacent(num, sym)) {
        ++ number_cnt;
        ratio *= num.getNumber();
      }
    }

    if (number_cnt == 2) {
      std::cout << "Gear : " << sym << std::endl;
      ratio_sum += ratio;
    } else {
      std::cout << "Not a gear : " << sym << std::endl;
    }

  }

  std::cout << "Ratio sum: " << ratio_sum << std::endl;

  return 0;
}
