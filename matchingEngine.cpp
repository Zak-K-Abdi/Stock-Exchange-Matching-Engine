#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

/*
Order book structure
unordered_map<pair<uint16_t, std::string>, float>
                   firmID, symbol,    price.

vector ID holds the id number of every firm that has placed an order.
*/


class MatchingEngine
{
private:
    std::map<std::pair<uint16_t, std::string>, float> buyBook;
    std::map<std::pair<uint16_t, std::string>, float> sellBook;
    std::unordered_map<uint16_t, std::vector<int>> orders; //Holds the live orders and the fills, cannot hold price as that is float.
    std::unordered_map<uint16_t, float> amountPaid; //Holds the total amount paid.
    std::vector<uint16_t> ID;
    std::unordered_set<uint16_t> seen;
public:
    MatchingEngine() {};

    bool inBuy(uint16_t firmId, std::string symbol); //Checks if the firmId and symbol is already in the buyBook/sellBook.
    bool inSell(uint16_t firmId, std::string symbol);

    void cancelOrder(uint16_t firmId, std::string symbol);
    void newOrder(uint16_t firmId, std::string symbol, char side, float price);
    void modifyOrder(uint16_t firmId, std::string symbol, float price);

    void printBuy();
    void printSell();
    void printID() {std::cout << "FIRM IDs" << '\n'; for (int i : ID) std::cout << i << '\n';}
    void liveOrders();
    void output();

    void matchOrder(uint16_t firmId, std::string symbol, char side);
};

bool MatchingEngine::inBuy(uint16_t firmId, std::string symbol)
{
    if (buyBook.find({firmId, symbol}) != buyBook.end()) return true;

    return false;
}

bool MatchingEngine::inSell(uint16_t firmId, std::string symbol)
{
    if (sellBook.find({firmId, symbol}) != sellBook.end()) return true;

    return false;
}

void MatchingEngine::cancelOrder(uint16_t firmId, std::string symbol)
{
    if (inBuy(firmId, symbol))
    {
        buyBook.erase({firmId, symbol});
    }

    else if (inSell(firmId, symbol))
    {
        sellBook.erase({firmId, symbol});
    }
}

//Can only have one {firmId, symbol} between both maps.
//If one is already in either of the maps then do nothing.
void MatchingEngine::newOrder(uint16_t firmId, std::string symbol, char side, float price)
{
    if (side == 'B')
    {
        if (!inBuy(firmId, symbol) && !inSell(firmId, symbol))
        {
            buyBook[{firmId, symbol}] = price;
            if (seen.find(firmId) == seen.end())
            {
                ID.push_back(firmId);
                seen.insert(firmId);
            }
            if (orders.find(firmId) == orders.end() && amountPaid.find(firmId) == amountPaid.end())
            {
                orders[firmId] = {0, 0};
                amountPaid[firmId] = 0;
            }
            matchOrder(firmId, symbol, side);
        }
    }

    else if (side == 'S')
    {
        if (!inSell(firmId, symbol) && !inBuy(firmId, symbol))
        {
            sellBook[{firmId, symbol}] = price;
            orders[firmId] = {0, 0};
            amountPaid[firmId] = 0;
            if (seen.find(firmId) == seen.end())
            {
                ID.push_back(firmId);
                seen.insert(firmId);
            }

            if (orders.find(firmId) == orders.end() && amountPaid.find(firmId) == amountPaid.end())
            {
                orders[firmId] = {0, 0};
                amountPaid[firmId] = 0;
            }

            matchOrder(firmId, symbol, side);
        }
    }
}

//Change the price of the order.
//Using inBuy and inSell to find
//which book the order is in.
void MatchingEngine::modifyOrder(uint16_t firmId, std::string symbol, float price)
{
    if (inBuy(firmId, symbol))
    {
        buyBook.erase({firmId, symbol});
        buyBook[{firmId, symbol}] = price;
        matchOrder(firmId, symbol, 'B');
    }
    

    else if (inSell(firmId, symbol))
    {
        sellBook.erase({firmId, symbol});
        sellBook[{firmId, symbol}] = price;
        matchOrder(firmId, symbol, 'S');
    }
}

void MatchingEngine::printBuy()
{
    std::cout << "BUYBOOK" << '\n';
    for (auto b : buyBook)
    {
        std::cout << b.first.first << " " << b.first.second << " " << b.second << '\n';
    }
}

void MatchingEngine::printSell()
{
    std::cout << "SELLBOOK" << '\n';
    for (auto s : sellBook)
    {
        std::cout << s.first.first << " " << s.first.second << " " << s.second << '\n';
    }
}
/*Range based for loop iterating over the books did not work,
instead I created a vector to hold every firmId, if the firmId and symbol
are in the opposite book and satisfy the price condition (buy price >= sell price)
match the orders and remove both of them from the order books.
*/
void MatchingEngine::matchOrder(uint16_t firmId, std::string symbol, char side)
{
    if (side == 'B')
    {
        float price = buyBook[{firmId, symbol}];
        for (int i = 0; i < ID.size(); i++)
        {
            if (sellBook.find({ID[i], symbol}) != sellBook.end() && sellBook[{ID[i], symbol}] <= price)
            {
                orders[firmId][1]++;
                orders[ID[i]][1]++;
                amountPaid[firmId] -= sellBook[{ID[i], symbol}];
                amountPaid[ID[i]] += sellBook[{ID[i], symbol}];

                sellBook.erase({ID[i], symbol});
                buyBook.erase({firmId, symbol});
            }
        }
    }

    else if (side == 'S')
    {
        float price = sellBook[{firmId, symbol}];
        for (int i = 0; i < ID.size(); i++)
        {
            if (buyBook.find({ID[i], symbol}) != buyBook.end() && buyBook[{ID[i], symbol}] >= price)
            {
                orders[firmId][1]++;
                orders[ID[i]][1]++;
                amountPaid[firmId] += price;
                amountPaid[ID[i]] -= price;

                buyBook.erase({ID[i], symbol});
                sellBook.erase({firmId, symbol});
            }
        }
    }
}

void MatchingEngine::liveOrders()
{
    for (int i = 0; i < ID.size(); i++)
    {
        for (auto b : buyBook)
        {
            if (ID[i] == b.first.first) orders[ID[i]][0]++;
        }

        for (auto s : sellBook)
        {
            if (ID[i] == s.first.first) orders[ID[i]][0]++;
        }
    }
}

void MatchingEngine::output()
{
    std::cout << "FINAL OUTPUT" << '\n';
    std::sort(ID.begin(), ID.end());
    
    for (int i = 0; i < ID.size(); i++)
    {
        for (auto o : orders)
        {
            if (ID[i] == o.first)
            {
                std::cout << ID[i] << " " << o.second[0] << " " << o.second[1] << " " << amountPaid[ID[i]] << '\n';
                break;
            }
        }
    }
}


int main()
{
    uint16_t N = 0;
    std::cin >> N;

    uint16_t firmId;
    std::string symbol;
    char orderType;
    char side;
    float price;

    MatchingEngine book;

    for (int i = 0; i < N; i++)
    {
        std::cin >> orderType >> firmId >> symbol;

        switch (orderType)
        {
            case 'N':
            {
                std::cout << "NEW ORDER - ENTER THE SIDE AND THEN THE PRICE" << '\n';
                std::cin >> side >> price;
                book.newOrder(firmId, symbol, side, price);
                break;
            }

            case 'M':
            {
                std::cout << "MODIFY ORDER - ENTER THE PRICE" << '\n';
                std::cin >> price;
                book.modifyOrder(firmId, symbol, price);
                break;
            }

            case 'C':
            {
                std::cout << "ORDER CANCELLED" << '\n';
                book.cancelOrder(firmId, symbol);
                break;
            }

            default:
            {
                break;
            }
        }
    }

    book.printBuy();
    book.printSell();
    book.printID();
    book.liveOrders();
    book.output();
    return 0;
}