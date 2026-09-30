#include <climits>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

// One person standing in the circle. 
struct Person {
    int     id;
    Person* next;   //  the last person points back to the first

    explicit Person(int id) : id(id), next(nullptr) {}
};

// Singly circular linked list of people. 
class Circle {
private:
    Person* last;   // last - next is the first node
    int     size;   // number of people still in the circle

    // Append one person at the end and keep the list circular
    void addPerson(int id) {
        Person* node = new Person(id);
        if (last == nullptr) {          // first node points to itself
            node->next = node;
        } else {
            node->next = last->next;    // new last to the first
            last->next = node;          // old last to the new last
        }
        last = node;
        ++size;
    }

public:
    Circle() : last(nullptr), size(0) {}

    // Copying would cause a double-free of nodes, so it is disabled.
    Circle(const Circle&) = delete;
    Circle& operator=(const Circle&) = delete;

    // Free every remaining node for no leakage. 
    ~Circle() {
        if (last == nullptr) return;
        Person* p = last->next;
        last->next = nullptr;           // break the circle so the loop can end
        while (p != nullptr) {
            Person* next = p->next;
            delete p;
            p = next;
        }
    }

    // Create Circle:
    void createCircle(int n) {
        for (int id = 1; id <= n; ++id) addPerson(id);
    }

    
     //Elimination Process: 
    vector<int> eliminate(int k) {
        vector<int> order;
        if (last == nullptr) return order;
        order.reserve(size);

        Person* prev = last;            // person just before curr
        Person* curr = last->next;      // person 1, where counting starts

        while (size > 1) {
            // Walk k-1 steps; % size skips full laps when k is large.
            int steps = (k - 1) % size;
            for (int i = 0; i < steps; ++i) {
                prev = curr;
                curr = curr->next;
            }

            order.push_back(curr->id);
            prev->next = curr->next;    // unlink curr from the circle
            if (curr == last) last = prev;
            delete curr;
            --size;

            curr = prev->next;          // resume counting from next person
        }
        return order;
    }

    // Display Survivor: ID of the last person left 
    int survivor() const { return (last != nullptr) ? last->id : -1; }
};

// output helpers

// print IDs in the order they were removed. 
void displayEliminated(const vector<int>& order) {
    if (order.empty()) {
        cout << "Nobody was eliminated.\n";
        return;
    }
    cout << "Elimination order: ";
    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0) cout << " -> ";
        cout << order[i];
    }
    cout << "\n";
}

// inputer helpers:

// Read an int , re-prompting until the value is valid. 
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
            if (cin.eof()) exit(0);     // input closed, stop cleanly
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input, enter a number.\n";
        }
    }
}

int main() {
    cout << "===== Josephus Problem Simulation =====\n";
    int n = readIntInRange("Number of people N (1-100000): ", 1, 100000);
    int k = readIntInRange("Step count k (1-1000000): ", 1, 1000000);

    Circle circle;
    circle.createCircle(n);
    cout << "Circle of " << n << " people created. Eliminating every k-th person (k = "
         << k << "), starting from person 1.\n\n";

    vector<int> order = circle.eliminate(k);

    displayEliminated(order);
    cout << "Survivor: person " << circle.survivor() << "\n";

    return 0;   // Circle destructor frees the survivor node
}