
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int BLOCK = 8;          // bits per group
const int MAX_BITS = 512;     // longest input accepted

struct BitNode {
    int      bit;             // 0 or 1
    BitNode* prev;
    BitNode* next;

    explicit BitNode(int bit) : bit(bit), prev(nullptr), next(nullptr) {}
};


class BinaryNumber {
private:
    BitNode* head;   // MSB
    BitNode* tail;   // LSB
    int      size;   // number of bits

    // Add a bit at the LSB end. O(1). 
    void pushBack(int bit) {
        BitNode* node = new BitNode(bit);
        if (tail == nullptr) {
            head = tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
        ++size;
    }

    // Add a bit at the MSB end. O(1). 
    void pushFront(int bit) {
        BitNode* node = new BitNode(bit);
        if (head == nullptr) {
            head = tail = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }
        ++size;
    }

    // Remove the MSB node. O(1). 
    void popFront() {
        if (head == nullptr) return;
        BitNode* old = head;
        head = head->next;
        if (head != nullptr) head->prev = nullptr;
        else                 tail = nullptr;
        delete old;
        --size;
    }


    void clear() {
        while (head != nullptr) popFront();
    }

    // Add leading zeros until the size is a multiple of 8. 
    void padToBlock() {
        while (size % BLOCK != 0) pushFront(0);
    }

    // Multiply by 2  growing by a block only if needed. 
    void shiftLeft() {
        pushBack(0);
        if (head->bit == 0) popFront();   // top bit was 0: size stays the same
        else                padToBlock(); // top bit was 1: need a new 8-bit block
    }

    // Remove redundant all-zero leading blocks 
    void trim() {
        while (size > BLOCK) {
            const BitNode* p = head;
            for (int i = 0; i < BLOCK; ++i, p = p->next)
                if (p->bit == 1) return;
            for (int i = 0; i < BLOCK; ++i) popFront();
        }
    }

    void swapWith(BinaryNumber& other) {
        swap(head, other.head);
        swap(tail, other.tail);
        swap(size, other.size);
    }

public:
    BinaryNumber() : head(nullptr), tail(nullptr), size(0) {}

    // Deep copy, so results never share nodes with their operands.
    BinaryNumber(const BinaryNumber& other) : head(nullptr), tail(nullptr), size(0) {
        for (const BitNode* p = other.head; p != nullptr; p = p->next)
            pushBack(p->bit);
    }

    BinaryNumber& operator=(BinaryNumber other) {   // copy-and-swap
        swapWith(other);
        return *this;
    }

    ~BinaryNumber() { clear(); }

    bool isEmpty() const { return head == nullptr; }

    
    void store(const string& bits) {
        clear();
        for (size_t i = 0; i < bits.size(); ++i)
            pushBack(bits[i] - '0');
        padToBlock();
    }

    BinaryNumber onesComplement() const {
        BinaryNumber result(*this);
        for (BitNode* p = result.head; p != nullptr; p = p->next)
            p->bit ^= 1;
        return result;
    }

    BinaryNumber twosComplement() const {
        BinaryNumber result = onesComplement();
        BinaryNumber one;
        for (int i = 0; i < size - 1; ++i) one.pushBack(0);
        one.pushBack(1);                       // same width, value 1
        return add(result, one, false);        // carry out of the top is dropped
    }

    
    static BinaryNumber add(const BinaryNumber& a, const BinaryNumber& b, bool allowGrow) {
        BinaryNumber result;
        const BitNode* pa = a.tail;
        const BitNode* pb = b.tail;
        int carry = 0;

        while (pa != nullptr || pb != nullptr) {
            int sum = carry;
            if (pa != nullptr) { sum += pa->bit; pa = pa->prev; }
            if (pb != nullptr) { sum += pb->bit; pb = pb->prev; }
            result.pushFront(sum % 2);
            carry = sum / 2;
        }
        if (carry == 1 && allowGrow) result.pushFront(1);
        result.padToBlock();
        return result;
    }

    
    static BinaryNumber multiply(const BinaryNumber& a, const BinaryNumber& b) {
        BinaryNumber product;
        product.store("0");                    // 00000000
        BinaryNumber shifted(a);

        for (const BitNode* p = b.tail; p != nullptr; p = p->prev) {
            if (p->bit == 1) product = add(product, shifted, true);
            if (p->prev != nullptr) shifted.shiftLeft();
        }
        product.trim();
        return product;
    }

    
    string toDecimal() const {
        vector<int> digits(1, 0);              
        for (const BitNode* p = head; p != nullptr; p = p->next) {
            int carry = p->bit;
            for (size_t i = 0; i < digits.size(); ++i) {
                int v = digits[i] * 2 + carry;
                digits[i] = v % 10;
                carry = v / 10;
            }
            if (carry > 0) digits.push_back(carry);
        }
        string out;
        for (size_t i = digits.size(); i > 0; --i)
            out += static_cast<char>('0' + digits[i - 1]);
        return out;
    }

    // Print the number in 8-bit groups
    void display() const {
        int count = 0;
        for (const BitNode* p = head; p != nullptr; p = p->next) {
            cout << p->bit;
            if (++count % BLOCK == 0 && p->next != nullptr) cout << ' ';
        }
    }
};

// input helpers 

// Read an int 
int readIntInRange(const string& prompt, int lo, int hi) {
    while (true) {
        cout << prompt;
        int value;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (value >= lo && value <= hi) return value;
            cout << "Invalid value. Please enter a number between "
                 << lo << " and " << hi << ".\n";
        } else {
            if (cin.eof()) exit(0);            // input closed, stop cleanly
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input, enter a number.\n";
        }
    }
}

// Read a string of 0s and 1s spaces allowed, 
string readBinaryString(const string& prompt) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) exit(0);      // input closed, stop cleanly

        string bits;
        bool valid = true;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == ' ' || c == '\t' || c == '\r') continue;
            if (c != '0' && c != '1') { valid = false; break; }
            bits += c;
        }
        if (!valid)
            cout << "Only the digits 0 and 1 are allowed. Please try again.\n";
        else if (bits.empty())
            cout << "Input cannot be empty. Please try again.\n";
        else if (static_cast<int>(bits.size()) > MAX_BITS)
            cout << "Too long. Enter at most " << MAX_BITS << " bits.\n";
        else
            return bits;
    }
}

//* Ask which stored number to use it Retuurns 1 or 2. 
int askWhich() {
    return readIntInRange("Which number? (1 = A, 2 = B): ", 1, 2);
}

void showMenu() {
    cout << "\n===== Binary Arithmetic (DLL) =====\n"
         << "1. Store Binary Number\n"
         << "2. 1's Complement\n"
         << "3. 2's Complement\n"
         << "4. Binary Addition (A + B)\n"
         << "5. Binary Multiplication (A x B)\n"
         << "6. Conversion to Decimal\n"
         << "7. Display Stored Numbers\n"
         << "0. Exit\n";
}

// Print a stored number
void showStored(const char* label, const BinaryNumber& n) {
    cout << label << " = ";
    if (n.isEmpty()) cout << "(not stored)";
    else             n.display();
    cout << "\n";
}

int main() {
    BinaryNumber A, B;
    int choice;

    do {
        showMenu();
        choice = readIntInRange("Choice (0-7): ", 0, 7);

        // Addition and multiplication need both numbers stored.
        if ((choice == 4 || choice == 5) && (A.isEmpty() || B.isEmpty())) {
            cout << "Store both A and B first (option 1).\n";
            continue;
        }

        switch (choice) {
        case 1: {
            int which = askWhich();
            string bits = readBinaryString("Enter binary number (0s and 1s): ");
            (which == 1 ? A : B).store(bits);
            cout << "Stored as " << (which == 1 ? "A" : "B") << " = ";
            (which == 1 ? A : B).display();
            cout << "\n";
            break;
        }
        case 2:
        case 3:
        case 6: {
            int which = askWhich();
            const BinaryNumber& n = (which == 1) ? A : B;
            if (n.isEmpty()) {
                cout << "That number has not been stored yet (option 1).\n";
                break;
            }
            const char* name = (which == 1) ? "A" : "B";
            if (choice == 2) {
                cout << "1's complement of " << name << " = ";
                n.onesComplement().display();
                cout << "\n";
            } else if (choice == 3) {
                cout << "2's complement of " << name << " = ";
                n.twosComplement().display();
                cout << "\n";
            } else {
                cout << name << " = ";
                n.display();
                cout << "  ->  decimal " << n.toDecimal() << "\n";
            }
            break;
        }
        case 4: {
            BinaryNumber sum = BinaryNumber::add(A, B, true);
            cout << "A + B = ";
            sum.display();
            cout << "  (decimal " << sum.toDecimal() << ")\n";
            break;
        }
        case 5: {
            BinaryNumber product = BinaryNumber::multiply(A, B);
            cout << "A x B = ";
            product.display();
            cout << "  (decimal " << product.toDecimal() << ")\n";
            break;
        }
        case 7:
            showStored("A", A);
            showStored("B", B);
            break;
        case 0:
            cout << "Goodbye.\n";
            break;
        }
    } while (choice != 0);

    return 0;   // destructors free all nodes
}