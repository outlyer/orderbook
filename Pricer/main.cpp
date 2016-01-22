//
//  main.cpp
//  Pricer
//
//  Created by Aubin Paul on 1/17/16.
//  Copyright © 2016 Aubin Paul. All rights reserved.
//
//
// Some notes:
// - Instead of arrays used vector+map because they are highly
//   memory efficient (through use of contiguous storage like arrays) but are
//   better for our purposes because they can be resized on the fly.
// - While unordered_map is faster when we're accessing individual elements,
//   it would be slower in this situation because it we need to deal with a
//   range of elements and additions/deletions.
// - More practically, unordered_map seems fastest when dealing with lookup
//   tables, but significantly worse when you can freely insert/delete elements
// - Since we didn't specify vector storage type, it defaults to heap and we
//   take advantage of that to quickly add and remove elements.
//
// Tested on Mac OS X 10.11.2 (Xcode+Clang) and Debian Linux 8.2 with GCC 4.9
//

#include <algorithm>  // std::*_heap
#include <iomanip>  // std::cout.precision
#include <iostream>  // std::cout
#include <iterator>  // std::iterator
#include <limits>  // std::numeric_limits
#include <map>  // std::map
#include <numeric>  // std::accumulate
#include <utility>  // std::pair
#include <vector>  // std:vector
#define MSGLENGTH 64

// Avoid ambiguity in namespaces
using std::cout;
using std::endl;
using std::vector;
using std::pair;

// Declaring everything we need, setting defaults and limits
int target_size = 0;
bool bidPrice = false;
float sellPrice = 0.00;
bool askPrice = false;
float buyPrice = std::numeric_limits<float>::max();

// Main order structure
struct Order {
  int timestamp;
  char order_id;
  char side;
  float price;
  int size;

  Order(int ts, char id, char bs, float p, int s)
      : timestamp(ts), order_id(id), side(bs), price(p), size(s) {}

  bool operator<(const Order& alt) const {
    bool result = price < alt.price;
    result = (side == 'B') ? result : ~result;
    return result;
  }

  bool operator==(const Order& alt) const {
    bool result = order_id == alt.order_id;
    return result;
  }

  bool operator==(const char& oid) const {
    bool result = order_id == oid;
    return result;
  }
};

// Our Bid/Ask queues, declared as vectors to allow for out-of-sequence
// transactions, std-based sorting and quick additions and deletions
vector<Order> bids;
vector<Order> asks;
typedef vector<Order>* vOrderBook;
typedef vector<Order>::iterator orderBookIterator;
std::map<char, vOrderBook> orderMap;
int size(int size, const Order& ord) { return size + ord.size; }

// Functions
pair<float, int32_t> calPrice(pair<float, int32_t> priceQuantity,
                              const Order& ord) {
  int32_t lhs = priceQuantity.second;
  int32_t execQuant = (lhs >= ord.size) ? ord.size : lhs;
  float priceOrder = execQuant * ord.price;
  priceQuantity.first += priceOrder;
  priceQuantity.second = (lhs - execQuant);
  return priceQuantity;
}

// Basic Operations
void addOrder(const char line[]) {
  int timestamp;
  char order_id;
  char side;
  int size;
  float price;
  char msgtype;

  sscanf(line, "%d %c %c %c %f %d", &timestamp, &msgtype, &order_id, &side,
         &price, &size);

  Order* OrderInstance = new Order(timestamp, order_id, side, price, size);

  vector<Order>* orderBookQueue;

  if (side == 'B') {
    orderBookQueue = &(bids);
  } else {
    orderBookQueue = &(asks);
  }

  orderBookQueue->push_back(*OrderInstance);
  orderBookIterator first = orderBookQueue->begin();
  orderBookIterator last = orderBookQueue->end();
  if (first != last) push_heap(first, last);
  orderMap[order_id] = orderBookQueue;
}

void reduceOrder(const char line[]) {
  int timestamp;
  char msgtype;
  char order_id;
  int size;
  sscanf(line, "%d %c %c  %d", &timestamp, &msgtype, &order_id, &size);

  vOrderBook vBook = orderMap[order_id];
  orderBookIterator iterateOrder = find(vBook->begin(), vBook->end(), order_id);

  iterateOrder->size = iterateOrder->size - size;
  if (iterateOrder->size == 0) {
    vBook->erase(iterateOrder);
    make_heap(vBook->begin(), vBook->end());
  }
}

// Price Calculation and output
void price(int timestamp) {
  cout << std::fixed;
  cout.precision(2);

  int total = accumulate(bids.begin(), bids.end(), 0, size);

  if (total >= target_size) {
    pair<float, int32_t> limitOrderBook;
    limitOrderBook.first = 0;
    limitOrderBook.second = target_size;
    limitOrderBook = accumulate(bids.begin(), bids.end(),
                                limitOrderBook, calPrice);

    if (sellPrice != limitOrderBook.first) {
      cout << timestamp << " S " << limitOrderBook.first << endl;
      sellPrice = limitOrderBook.first;
    }
    bidPrice = true;
  } else if (bidPrice) {
    cout << timestamp << " S NA" << endl;
    bidPrice = false;
    sellPrice = 0.00;
  }

  total = accumulate(asks.begin(), asks.end(), 0, size);

  if (total >= target_size) {
    pair<float, int32_t> limitOrderBook;
    limitOrderBook.first = 0;
    limitOrderBook.second = target_size;
    limitOrderBook = accumulate(asks.begin(), asks.end(),
                                limitOrderBook, calPrice);

    if (buyPrice != limitOrderBook.first) {
      cout << timestamp << " B " << limitOrderBook.first << endl;
      buyPrice = limitOrderBook.first;
    }

    askPrice = true;
    } else if (askPrice) {
    cout << timestamp << " B NA" << endl;
    askPrice = false;
    buyPrice = std::numeric_limits<float>::max();
  }
}

// Main loop
int main(int argc, const char* argv[]) {
  char line[MSGLENGTH];

  if (argc != 2) {
    cout << "Usage: " << argv[0]
         << " [TARGET-SIZE] \n"
            "Pricer reads a market data log on standard input. As the book is\n"
            "modified, Pricer prints (on standard output) the total expense\n"
            "incurred on buyig [target-size] shares (by taking as asks as\n"
            "necessary, lowest first), and the total income you would receive "
            "if\n"
            "you sold [target-size] shares (by hitting as many bids as "
            "necessary,\n"
            "highest first). Each time the income or expense changes, it "
            "prints\n"
            "the changed value.\n\n";
    return 1;
  } else {
    target_size = atoi(argv[1]);  // Has to be an integer
    while (std::cin.getline(line, MSGLENGTH)) {
      char type;
      int timestamp;
      sscanf(line, "%d %c ", &timestamp, &type);
      switch (type) {
        case 'A':
          addOrder(line);
          break;
        case 'R':
          reduceOrder(line);
          break;
        default:
          cout << "No data in last command" << type << endl;
      }
      price(timestamp);
    }
  }
  return 0;
}
