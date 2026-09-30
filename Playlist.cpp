
#include <iostream>
#include <iomanip>
#include <climits>
#include <cstdlib>
#include <limits>
#include <string>
#include <utility>

using namespace std;

struct Song {
    int    id;
    string name;
    int    minutes;
    int    seconds;
    Song*  prev;
    Song*  next;

    Song(int id, const string& name, int minutes, int seconds)
        : id(id), name(name), minutes(minutes), seconds(seconds),
          prev(nullptr), next(nullptr) {}
};

class Playlist {
private:
    Song* head;     // first song
    Song* tail;     // last song
    Song* current;  // current song
    int   count;    // number of songs

    // Print one song on a single line. 
    static void printSong(const Song* s) {
        cout << "  [" << s->id << "] " << s->name << " ("
             << s->minutes << ":" << setfill('0') << setw(2) << s->seconds
             << setfill(' ') << ")\n";
    }

    // Find a node by ID, or nullptr if it does not exist. 
    Song* findById(int id) const {
        for (Song* p = head; p != nullptr; p = p->next)
            if (p->id == id) return p;
        return nullptr;
    }

public:
    Playlist() : head(nullptr), tail(nullptr), current(nullptr), count(0) {}

    // Copying would cause double-free of nodes, so it is disabled.
    Playlist(const Playlist&) = delete;
    Playlist& operator=(const Playlist&) = delete;

    // Free every node for no leaks.
    ~Playlist() {
        while (head != nullptr) {
            Song* next = head->next;
            delete head;
            head = next;
        }
    }

    bool isEmpty() const { return head == nullptr; }

    bool exists(int id) const { return findById(id) != nullptr; }

    
    bool addSong(int id, const string& name, int min, int sec) {
        if (findById(id) != nullptr) return false;  // IDs must be unique

        Song* node = new Song(id, name, min, sec);
        if (tail == nullptr) {          // empty list
            head = tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
        ++count;
        return true;
    }

     
    bool deleteSong(int id) {
        Song* target = findById(id);
        if (target == nullptr) return false;

        // Keep the cursor valid if we are deleting the playing song.
        if (current == target)
            current = (target->next != nullptr) ? target->next : target->prev;

        // Unlink from neighbours (handles head/tail/middle/only-node).
        if (target->prev != nullptr) target->prev->next = target->next;
        else                         head = target->next;

        if (target->next != nullptr) target->next->prev = target->prev;
        else                         tail = target->prev;

        delete target;
        --count;
        return true;
    }

    
    void displayForward() const {
        if (isEmpty()) { cout << "Playlist is empty.\n"; return; }
        cout << "Playlist (forward, " << count << " songs):\n";
        for (const Song* p = head; p != nullptr; p = p->next) printSong(p);
    }

   
    void displayBackward() const {
        if (isEmpty()) { cout << "Playlist is empty.\n"; return; }
        cout << "Playlist (backward, " << count << " songs):\n";
        for (const Song* p = tail; p != nullptr; p = p->prev) printSong(p);
    }

   
    void searchSong(int id) const {
        const Song* s = findById(id);
        if (s == nullptr) cout << "Song with ID " << id << " not found.\n";
        else { cout << "Found:\n"; printSong(s); }
    }

 
    void playNext() {
        if (isEmpty()) { cout << "Playlist is empty.\n"; return; }
        if (current == nullptr)            current = head;
        else if (current->next != nullptr) current = current->next;
        else { cout << "Already at the last song.\n"; }
        cout << "Now playing:\n";
        printSong(current);
    }

   
    void playPrevious() {
        if (isEmpty()) { cout << "Playlist is empty.\n"; return; }
        if (current == nullptr)            current = tail;
        else if (current->prev != nullptr) current = current->prev;
        else { cout << "Already at the first song.\n"; }
        cout << "Now playing:\n";
        printSong(current);
    }

    // Reverse the list in place by swapping prev/next of every node.
    void reverse() {
        if (count < 2) return;  // nothing to reverse
        Song* p = head;
        while (p != nullptr) {
            swap(p->prev, p->next);
            p = p->prev;        // old next is now in prev
        }
        swap(head, tail);
    }
};

// input helpers 


int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        if (cin.eof()) exit(0);  // input closed, stop cleanly
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input, enter a number.\n";
    }
}


int readIntInRange(const string& prompt, int lo, int hi) {
    while (true) {
        int value = readInt(prompt);
        if (value >= lo && value <= hi) return value;
        cout << "Invalid value. Please enter a number between "
             << lo << " and " << hi << ".\n";
    }
}


string readNonEmptyLine(const string& prompt) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) exit(0);  // input closed, stop cleanly
        // trim leading/trailing spaces
        size_t first = line.find_first_not_of(" \t");
        size_t last  = line.find_last_not_of(" \t");
        if (first != string::npos) return line.substr(first, last - first + 1);
        cout << "Name cannot be empty. Please try again.\n";
    }
}

void showMenu() {
    cout << "\n===== Playlist Menu =====\n"
         << "1. Add Song\n"
         << "2. Delete Song\n"
         << "3. Display Forward\n"
         << "4. Display Backward\n"
         << "5. Search Song\n"
         << "6. Play Next\n"
         << "7. Play Previous\n"
         << "8. Reverse Playlist\n"
         << "0. Exit\n";
}

int main() {
    Playlist playlist;
    int choice;

    do {
        showMenu();
        choice = readIntInRange("Choice (0-8): ", 0, 8);

        switch (choice) {
        case 1: {
            // Each field is re-asked until it is valid.
            int id;
            while (true) {
                id = readIntInRange("Song ID (positive number): ", 1, INT_MAX);
                if (!playlist.exists(id)) break;
                cout << "ID " << id << " is already used. Enter a different ID.\n";
            }
            string name = readNonEmptyLine("Song name: ");
            int min = readIntInRange("Duration minutes (0-999): ", 0, 999);
            int sec = readIntInRange("Duration seconds (0-59): ", 0, 59);
            playlist.addSong(id, name, min, sec);
            cout << "Song added.\n";
            break;
        }
        case 2: {
            int id = readIntInRange("Song ID to delete: ", 1, INT_MAX);
            cout << (playlist.deleteSong(id) ? "Song deleted.\n"
                                             : "Song not found.\n");
            break;
        }
        case 3: playlist.displayForward();  break;
        case 4: playlist.displayBackward(); break;
        case 5: playlist.searchSong(readIntInRange("Song ID to search: ", 1, INT_MAX)); break;
        case 6: playlist.playNext();     break;
        case 7: playlist.playPrevious(); break;
        case 8:
            playlist.reverse();
            cout << "Playlist reversed.\n";
            break;
        case 0: cout << "Goodbye.\n"; break;
        }
    } while (choice != 0);

    return 0;  // Playlist destructor frees all nodes
}