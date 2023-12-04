#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <deque>

static const int unsigned SCRATCHCARD_NUM_MAX = 99;
static const int unsigned SCRATCHCARD_NUM_MIN = 1;

class Card {
protected:
  // Use inheritance as a lightweight way of separating construction
  // from use. Alternative approaches are a factory, or a constructor
  // accepting a collection of numbers.
  //
  // The idea is that a card's numbers shouldn't be able to be
  // modified after it is issued!
  std::bitset<SCRATCHCARD_NUM_MAX+1> bs_winning_numbers;
  std::bitset<SCRATCHCARD_NUM_MAX+1> bs_have_numbers;

  friend Card parse_line(std::istream& is);

public:
  // for part 1:
  int unsigned score() {
    int unsigned num_winning_numbers = num_matches();
    return (num_winning_numbers > 0 ? 1 : 0) << (num_winning_numbers-1) ;
  }

  // for part 2:
  int unsigned num_matches() {
    std::bitset<SCRATCHCARD_NUM_MAX+1> have_winning_numbers;
    int unsigned num_winning_numbers;

    have_winning_numbers = bs_winning_numbers & bs_have_numbers;
    num_winning_numbers = have_winning_numbers.count();

    return num_winning_numbers;
  }
};

// This is done for encapsulation:
class CardUnderConstruction : public Card {
public:
  void register_winning_num(int unsigned n) {
    bs_winning_numbers.set(n);
  }

  void register_have_num(int unsigned n) {
    bs_have_numbers.set(n);
  }
};

// Could have used a library here. (e.g. Regex or other grammar.)
Card parse_line(std::istream& is) {
  CardUnderConstruction card;
  char sgamebuf[16];

  is.get(sgamebuf, 16, ':');
  is.ignore(1, ':'); // Game N:

  int unsigned num;

  // read winning numbers until can no longer parse as a number.
  for (is >> num; is.good(); is >> num) {
    card.register_winning_num(num);
  }
    
  if (is.eof() || is.bad())
    throw std::runtime_error("Problem parsing line.");
  // clear failbit - couldn't parse number
  is.clear();

  // accept separator (not bothering with the case that it's missing)
  is.ignore(4, '|');

  // read in remaining numbers (have)
  for (is >> num; is.good(); is >> num) {
    card.register_have_num(num);
  }

  if (is.bad())
    throw std::runtime_error("Problem parsing line.");

  is.setstate(~std::ifstream::failbit); // clear fail, since end of
					// numbers have been reached

  return card;
}

int main(int argc, char **argv) {
  std::ifstream infile;

  if (argc < 1) {
    return 1;
  }

  infile.open(argv[1]);

  std::istream& is = infile;

  int unsigned sum = 0;

  std::deque<int unsigned> copies;
  int unsigned num_cards = 0;

  while (is.good()) {
    std::string s;
    std::getline(is, s); // TODO: deduplicate line parse
    std::stringstream ss;
    ss << s << '\n';
    if (s.size() < 2)
      break;

    Card c = parse_line(ss);

    // Part 1
    sum += c.score();

    // Part 2
    int unsigned copies_this_card = 1;
    if (copies.size()) {
      copies_this_card += copies.front();
      copies.pop_front();
    }

    int unsigned num_to_copy = c.num_matches();
    for (int unsigned ii=0; ii < num_to_copy; ++ii) {
      if (ii < copies.size()) {
	copies[ii] += copies_this_card;
      } else {
	copies.push_back(copies_this_card);
      }
    }

    num_cards += copies_this_card;
  }

  std::cout << "score = " << sum << std::endl;
  std::cout << "num cards = " << num_cards << std::endl;

  return 0;
}
