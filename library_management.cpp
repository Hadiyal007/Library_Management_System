#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <map>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Book {
    int id;
    string title;
    string author;
    bool issued;

    Book(int _id = 0, string _title = "", string _author = "", bool _issued = false)
        : id(_id), title(_title), author(_author), issued(_issued) {}
};

class Library {
    vector<Book> books;
    const string filename;
    const string historyfile;

public:
    Library(const string& fname = "library.csv", const string& hfile = "history.csv")
        : filename(fname), historyfile(hfile) {
        loadBooks();
    }

    void loadBooks() {
        books.clear();
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idstr, title, author, issuedstr;
            getline(ss, idstr, ',');
            getline(ss, title, ',');
            getline(ss, author, ',');
            getline(ss, issuedstr, ',');
            int id = stoi(idstr);
            bool issued = (issuedstr == "1");
            books.emplace_back(id, title, author, issued);
        }
    }

    void saveBooks() {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << " for writing." << endl;
            return;
        }
        for (const auto& b : books) {
            file << b.id << "," << b.title << "," << b.author << "," << (b.issued ? "1" : "0") << "\n";
        }
    }

    void logHistory(int id, const string& action) {
        ofstream file(historyfile, ios::app);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << historyfile << " for logging." << endl;
            return;
        }
        file << id << "," << action << "\n";
    }

    bool idExists(int id) const {
        for (const auto& b : books) {
            if (b.id == id) return true;
        }
        return false;
    }

    bool addBook(const Book& b) {
        if (b.title.empty() || b.author.empty() || b.id <= 0) return false;
        if (idExists(b.id)) return false;
        books.push_back(b);
        saveBooks();
        return true;
    }

    // Only update non-empty fields; blank means don't change
    bool updateBook(int id, const string& title, const string& author) {
        for (auto& b : books) {
            if (b.id == id) {
                if (!title.empty()) b.title = title;
                if (!author.empty()) b.author = author;
                saveBooks();
                return true;
            }
        }
        return false;
    }

    bool deleteBook(int id) {
        for (auto it = books.begin(); it != books.end(); ++it) {
            if (it->id == id) {
                books.erase(it);
                saveBooks();
                return true;
            }
        }
        return false;
    }

    Book* findBook(int id) {
        for (auto& b : books) if (b.id == id) return &b;
        return nullptr;
    }

    vector<Book> searchBooks(const string& term) {
        vector<Book> result;
        string termLower = term;
        transform(termLower.begin(), termLower.end(), termLower.begin(), ::tolower);
        for (const auto& b : books) {
            string titleLower = b.title, authorLower = b.author;
            transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            transform(authorLower.begin(), authorLower.end(), authorLower.begin(), ::tolower);
            if (titleLower.find(termLower) != string::npos ||
                authorLower.find(termLower) != string::npos ||
                to_string(b.id) == term)
                result.push_back(b);
        }
        return result;
    }

    vector<Book> getAllBooks() const { return books; }
    vector<Book> getIssuedBooks() const {
        vector<Book> res;
        for (const auto& b : books) if (b.issued) res.push_back(b);
        return res;
    }
    vector<Book> getAvailableBooks() const {
        vector<Book> res;
        for (const auto& b : books) if (!b.issued) res.push_back(b);
        return res;
    }

    // Statistics by author
    map<string, int> getStatsByAuthor() const {
        map<string, int> authorCount;
        for (const auto& b : books) {
            authorCount[b.author]++;
        }
        return authorCount;
    }

    // Statistics by title
    map<string, int> getStatsByTitle() const {
        map<string, int> titleCount;
        for (const auto& b : books) {
            titleCount[b.title]++;
        }
        return titleCount;
    }
};

void clearScreen() {
#ifdef _WIN32
    system("CLS");
#else
    system("clear");
#endif
}

void printBook(const Book& b) {
    cout << "-----------------------------------------\n";
    cout << "ID: " << b.id << "\nTitle: " << b.title
         << "\nAuthor: " << b.author
         << "\nStatus: " << (b.issued ? "Issued" : "Available") << "\n";
}

string getNonEmptyInput(const string& prompt) {
    string input;
    do {
        cout << prompt;
        getline(cin, input);
        if (input.empty()) cout << "Input cannot be empty. Please try again.\n";
    } while (input.empty());
    return input;
}

int main() {
    Library lib;
    int choice;

    while (true) {
        clearScreen();
        cout << "\n===== Library Management System =====\n";
        cout << "1. Add Book\n";
        cout << "2. Update Book\n";
        cout << "3. Delete Book\n";
        cout << "4. Search Book\n";
        cout << "5. Issue Book\n";
        cout << "6. Return Book\n";
        cout << "7. Show All Books\n";
        cout << "8. Show Issued Books\n";
        cout << "9. Show Available Books\n";
        cout << "10. Show Statistics\n";
        cout << "11. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number.\n";
            continue;
        }

        if (choice == 1) {
            clearScreen();
            string title, author;
            int baseId, quantity;

            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            title = getNonEmptyInput("Enter Title: ");
            author = getNonEmptyInput("Enter Author: ");

            // Ensure starting ID is unique
            while (true) {
                cout << "Enter starting Book ID (integer > 0): ";
                if (!(cin >> baseId) || baseId <= 0) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid ID!\n";
                    continue;
                }
                if (lib.idExists(baseId)) {
                    cout << "This starting ID (" << baseId << ") is already occupied. Please enter a new starting ID.\n";
                    continue;
                }
                break;
            }
            cout << "How many copies of this book do you want to add? ";
            if (!(cin >> quantity) || quantity < 1) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid quantity!\n";
                cout << "\nPress Enter to continue...";
                cin.ignore();
                continue;
            }
            clearScreen();

            bool allAdded = true;
            for (int i = 0; i < quantity; ++i) {
                int currId = baseId + i;
                if (lib.idExists(currId)) {
                    cout << "Failed to add copy with ID: " << currId << " (ID already exists)\n";
                    allAdded = false;
                    continue;
                }
                Book b(currId, title, author, false);
                if (lib.addBook(b)) {
                    cout << "Added copy with ID: " << b.id << "\n";
                } else {
                    cout << "Failed to add copy with ID: " << b.id << " (invalid input)\n";
                    allAdded = false;
                }
            }
            if (allAdded) {
                cout << "All copies added successfully!\n";
            } else {
                cout << "Some copies may not have been added due to duplicate IDs or invalid input.\n";
            }
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 2) {
            clearScreen();
            int id;
            cout << "Enter Book ID to update: ";
            cin >> id;
            cin.ignore();

            // Accept blank input for don't change
            cout << "Enter new Title (leave blank to keep unchanged): ";
            string newTitle;
            getline(cin, newTitle);

            cout << "Enter new Author (leave blank to keep unchanged): ";
            string newAuthor;
            getline(cin, newAuthor);

            clearScreen();
            if (lib.updateBook(id, newTitle, newAuthor)) {
                cout << "Book updated successfully!\n";
            } else {
                cout << "Book ID not found.\n";
            }
            cout << "\nPress Enter to continue...";
            cin.get();
        } else if (choice == 3) {
            clearScreen();
            int id;
            cout << "Enter Book ID to delete: ";
            cin >> id;
            clearScreen();
            if (lib.deleteBook(id)) {
                cout << "Book deleted successfully!\n";
            } else {
                cout << "Book ID not found.\n";
            }
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 4) {
            clearScreen();
            cin.ignore();
            cout << "Enter search term (Title/Author/ID): ";
            string term;
            getline(cin, term);
            clearScreen();
            auto found = lib.searchBooks(term);
            if (found.empty()) {
                cout << "No matching book found.\n";
            } else {
                for (const auto& b : found) printBook(b);
            }
            cout << "\nPress Enter to continue...";
            cin.get();
        } else if (choice == 5) {
            clearScreen();
            // Show all available books before issuing
            auto availableBooks = lib.getAvailableBooks();
            if (availableBooks.empty()) {
                cout << "No books are available for issuing.\n";
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                continue;
            }
            cout << "Available Books:\n";
            cout << left << setw(6) << "ID" << setw(25) << "Title" << setw(20) << "Author" << "\n";
            cout << "--------------------------------------------------------------\n";
            for (const auto& b : availableBooks) {
                cout << left << setw(6) << b.id
                     << setw(25) << b.title
                     << setw(20) << b.author << "\n";
            }
            cout << "--------------------------------------------------------------\n";
            int id;
            cout << "Enter Book ID to issue: ";
            cin >> id;
            clearScreen();
            Book* b = lib.findBook(id);
            if (!b) {
                cout << "Book ID not found.\n";
            } else if (b->issued) {
                cout << "Book already issued.\n";
            } else {
                b->issued = true;
                lib.saveBooks();
                lib.logHistory(id, "Issued");
                cout << "Book issued successfully!\n";
            }
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 6) {
            clearScreen();
            int id;
            cout << "Enter Book ID to return: ";
            cin >> id;
            clearScreen();
            Book* b = lib.findBook(id);
            if (!b) {
                cout << "Book ID not found.\n";
            } else if (!b->issued) {
                cout << "Book is not issued.\n";
            } else {
                b->issued = false;
                lib.saveBooks();
                lib.logHistory(id, "Returned");
                cout << "Book returned successfully!\n";
            }
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 7) {
            clearScreen();
            cout << "\n====== Library Books ======\n";
            cout << left << setw(6) << "ID" << setw(25) << "Title" << setw(20) << "Author" << "Status\n";
            cout << "--------------------------------------------------------------\n";
            for (const auto& b : lib.getAllBooks()) {
                cout << left << setw(6) << b.id
                     << setw(25) << b.title
                     << setw(20) << b.author
                     << (b.issued ? "Issued" : "Available") << "\n";
            }
            cout << "--------------------------------------------------------------\n";
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 8) {
            clearScreen();
            cout << "\n====== Issued Books ======\n";
            auto issued = lib.getIssuedBooks();
            if (issued.empty()) cout << "No books are currently issued.\n";
            for (const auto& b : issued)
                cout << "ID: " << b.id << " | " << b.title << " by " << b.author << "\n";
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 9) {
            clearScreen();
            cout << "\n====== Available Books ======\n";
            auto available = lib.getAvailableBooks();
            if (available.empty()) cout << "No books are currently available.\n";
            for (const auto& b : available)
                cout << "ID: " << b.id << " | " << b.title << " by " << b.author << "\n";
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 10) {
            clearScreen();
            int total = lib.getAllBooks().size();
            int issued = lib.getIssuedBooks().size();
            int available = lib.getAvailableBooks().size();
            cout << "\n====== Library Statistics ======\n";
            cout << "Total books: " << total << "\n";
            cout << "Issued books: " << issued << "\n";
            cout << "Available books: " << available << "\n";

            // Statistics by author
            cout << "\n--- Books by Author ---\n";
            auto authorStats = lib.getStatsByAuthor();
            for (const auto& entry : authorStats) {
                cout << setw(22) << entry.first << " : " << entry.second << "\n";
            }

            // Statistics by title
            cout << "\n--- Books by Title ---\n";
            auto titleStats = lib.getStatsByTitle();
            for (const auto& entry : titleStats) {
                cout << setw(22) << entry.first << " : " << entry.second << "\n";
            }

            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        } else if (choice == 11) {
            clearScreen();
            cout << "Exiting... Goodbye!\n";
            break;
        } else {
            clearScreen();
            cout << "Invalid choice.\n";
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        }
    }
    return 0;
}