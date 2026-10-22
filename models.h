// models.h - SHARED by everyone (classes: User, Item, Report, LostReport, FoundReport)
#ifndef MODELS_H
#define MODELS_H
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <cctype>
using namespace std;

// ---------- small helpers ----------
inline string toLower(string s) { for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]); return s; }
inline string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
// remove the file delimiter so records never break
inline string clean(string s) { s = trim(s); for (size_t i = 0; i < s.size(); i++) if (s[i] == '|') s[i] = '/'; return s; }
inline vector<string> split(const string& s, char d) {
    vector<string> out; string cur;
    for (size_t i = 0; i < s.size(); i++) { if (s[i] == d) { out.push_back(cur); cur.clear(); } else cur += s[i]; }
    out.push_back(cur);
    return out;
}
inline bool containsText(const string& hay, const string& needle) { return toLower(hay).find(toLower(needle)) != string::npos; }
inline string hashPassword(const string& p) { return to_string(hash<string>()(p)); }

// ---------- console input helpers ----------
inline string readLine(const string& prompt) { string s; cout << prompt; getline(cin, s); return trim(s); }
inline int readInt(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty() && s.find_first_not_of("0123456789") == string::npos && s.size() < 9) return stoi(s);
        cout << "  Please enter a valid number.\n";
        if (!cin) return 0;
    }
}

// ---------- status ----------
enum Status { OPEN = 0, MATCHED = 1, RETURNED = 2 };
inline string statusText(int s) { return s == OPEN ? "OPEN" : (s == MATCHED ? "MATCHED" : "RETURNED"); }

// ---------- User ----------
class User {
private:
    string name, email, passwordHash, role;   // encapsulation: all private
public:
    User() {}
    User(const string& n, const string& e, const string& hash, const string& r = "student")
        : name(n), email(toLower(e)), passwordHash(hash), role(r) {}
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getRole() const { return role; }
    string getHash() const { return passwordHash; }
    bool checkPassword(const string& p) const { return passwordHash == hashPassword(p); }
    bool isAdmin() const { return role == "admin"; }
};

// ---------- Item ----------
class Item {
private:
    string name, category, location, description, secret;  // secret = hidden detail used for verification
public:
    Item() {}
    Item(const string& n, const string& c, const string& l, const string& d, const string& s = "")
        : name(clean(n)), category(clean(c)), location(clean(l)), description(clean(d)), secret(clean(s)) {}
    string getName() const { return name; }
    string getCategory() const { return category; }
    string getLocation() const { return location; }
    string getDescription() const { return description; }
    string getSecret() const { return secret; }
};

// ---------- Report (base) ----------
class Report {
protected:
    int id;
    Item item;
    string reporterEmail;
    int status;
public:
    Report(int i, const Item& it, const string& email, int st = OPEN)
        : id(i), item(it), reporterEmail(toLower(email)), status(st) {}
    virtual ~Report() {}
    virtual string type() const = 0;     // "LOST" or "FOUND"
    int getId() const { return id; }
    const Item& getItem() const { return item; }
    string getReporterEmail() const { return reporterEmail; }
    int getStatus() const { return status; }
    void setStatus(int s) { status = s; }
    string toRecord() const {
        return type() + "|" + to_string(id) + "|" + reporterEmail + "|" + to_string(status) + "|" +
               item.getName() + "|" + item.getCategory() + "|" + item.getLocation() + "|" +
               item.getDescription() + "|" + item.getSecret();
    }
    void display() const {
        cout << "[" << type() << " #" << id << "] " << item.getName() << " | " << item.getCategory()
             << " | " << item.getLocation() << " | " << statusText(status) << "\n     "
             << item.getDescription() << "\n";
    }
};

// inheritance: LostReport and FoundReport inherit from Report
class LostReport : public Report {
public:
    LostReport(int i, const Item& it, const string& email, int st = OPEN) : Report(i, it, email, st) {}
    string type() const { return "LOST"; }
};
class FoundReport : public Report {
public:
    FoundReport(int i, const Item& it, const string& email, int st = OPEN) : Report(i, it, email, st) {}
    string type() const { return "FOUND"; }
};

// ---------- Claim request (goes into the queue) ----------
struct ClaimRequest {
    int claimId;
    int lostId;
    int foundId;
    string claimantEmail;
    string answer;     // claimant's answer to the verification question
};

#endif
