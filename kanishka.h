// kanishka.h - KANISHKA RAWAT: data structures, reports, search, matching, stack
#ifndef KANISHKA_H
#define KANISHKA_H
#include "models.h"
#include <algorithm>
#include <set>

// ================= 1. Singly linked list of reports =================
struct Node {
    Report* data;
    Node* next;
    Node(Report* d) : data(d), next(nullptr) {}
};

class ReportList {
private:
    Node* head;
    int count;
    ReportList(const ReportList&);              // no copying (owns pointers)
    ReportList& operator=(const ReportList&);
public:
    ReportList() : head(nullptr), count(0) {}
    ~ReportList() { clear(); }

    void insert(Report* r) {                    // insert at end
        Node* n = new Node(r);
        if (!head) head = n;
        else { Node* t = head; while (t->next) t = t->next; t->next = n; }
        count++;
    }
    bool remove(int id) {                       // delete by id
        Node* cur = head; Node* prev = nullptr;
        while (cur && cur->data->getId() != id) { prev = cur; cur = cur->next; }
        if (!cur) return false;
        if (prev) prev->next = cur->next; else head = cur->next;
        delete cur->data; delete cur;
        count--;
        return true;
    }
    Report* find(int id) const {
        for (Node* t = head; t; t = t->next) if (t->data->getId() == id) return t->data;
        return nullptr;
    }
    int size() const { return count; }
    Node* begin() const { return head; }
    void clear() {
        while (head) { Node* t = head; head = head->next; delete t->data; delete t; }
        count = 0;
    }
    void displayAll(const string& typeFilter = "") const {
        bool any = false;
        for (Node* t = head; t; t = t->next)
            if (typeFilter.empty() || t->data->type() == typeFilter) { t->data->display(); any = true; }
        if (!any) cout << "  (nothing to show)\n";
    }
};

// ================= 2. Search (case-insensitive, partial match) =================
inline vector<Report*> searchByName(const ReportList& l, const string& q) {
    vector<Report*> r;
    for (Node* t = l.begin(); t; t = t->next) if (containsText(t->data->getItem().getName(), q)) r.push_back(t->data);
    return r;
}
inline vector<Report*> searchByCategory(const ReportList& l, const string& q) {
    vector<Report*> r;
    for (Node* t = l.begin(); t; t = t->next) if (containsText(t->data->getItem().getCategory(), q)) r.push_back(t->data);
    return r;
}
inline vector<Report*> searchByLocation(const ReportList& l, const string& q) {
    vector<Report*> r;
    for (Node* t = l.begin(); t; t = t->next) if (containsText(t->data->getItem().getLocation(), q)) r.push_back(t->data);
    return r;
}
// combined search: leave a field empty to ignore it
inline vector<Report*> searchAll(const ReportList& l, const string& name, const string& cat, const string& loc) {
    vector<Report*> r;
    for (Node* t = l.begin(); t; t = t->next) {
        const Item& it = t->data->getItem();
        if (!name.empty() && !containsText(it.getName(), name)) continue;
        if (!cat.empty()  && !containsText(it.getCategory(), cat)) continue;
        if (!loc.empty()  && !containsText(it.getLocation(), loc)) continue;
        r.push_back(t->data);
    }
    return r;
}

// ================= 3. Lost <-> Found matching =================
// Step 1: compare category and location. Step 2: compare keywords (name + description).
inline set<string> keywords(const string& text) {
    static const char* stop[] = {"the", "and", "with", "has", "have", "from", "near", "this", "that", "was", "for", "a", "of"};
    set<string> out; string w;
    string t = toLower(text) + " ";
    for (size_t i = 0; i < t.size(); i++) {
        if (isalnum((unsigned char)t[i])) w += t[i];
        else {
            if (w.size() >= 3) {
                bool isStop = false;
                for (size_t k = 0; k < sizeof(stop) / sizeof(stop[0]); k++) if (w == stop[k]) isStop = true;
                if (!isStop) out.insert(w);
            }
            w.clear();
        }
    }
    return out;
}
inline int matchScore(const Report& lost, const Report& found) {
    const Item& a = lost.getItem(); const Item& b = found.getItem();
    int score = 0;
    if (toLower(a.getCategory()) == toLower(b.getCategory())) score += 3;
    if (containsText(a.getLocation(), b.getLocation()) || containsText(b.getLocation(), a.getLocation())) score += 2;
    set<string> ka = keywords(a.getName() + " " + a.getDescription());
    set<string> kb = keywords(b.getName() + " " + b.getDescription());
    int common = 0;
    for (set<string>::iterator it = ka.begin(); it != ka.end(); ++it) if (kb.count(*it)) common++;
    score += (common > 4 ? 4 : common);
    return score;
}
struct MatchResult { Report* found; int score; };
inline bool betterMatch(const MatchResult& x, const MatchResult& y) { return x.score > y.score; }

// returns open FOUND reports that are likely the lost item (best first)
inline vector<MatchResult> findMatches(const ReportList& l, const Report& lost) {
    vector<MatchResult> out;
    for (Node* t = l.begin(); t; t = t->next) {
        Report* f = t->data;
        if (f->type() != "FOUND" || f->getStatus() == RETURNED) continue;
        int s = matchScore(lost, *f);
        bool sameCat = toLower(lost.getItem().getCategory()) == toLower(f->getItem().getCategory());
        if (sameCat && s >= 4) { MatchResult m; m.found = f; m.score = s; out.push_back(m); }
    }
    sort(out.begin(), out.end(), betterMatch);
    return out;
}

// ================= 4. Queue of ownership-claim requests (FIFO) =================
struct ClaimNode { ClaimRequest data; ClaimNode* next; };
class ClaimQueue {
private:
    ClaimNode *front, *rear; int count;
    ClaimQueue(const ClaimQueue&);
    ClaimQueue& operator=(const ClaimQueue&);
public:
    ClaimQueue() : front(nullptr), rear(nullptr), count(0) {}
    ~ClaimQueue() { ClaimRequest tmp; while (dequeue(tmp)) {} }
    void enqueue(const ClaimRequest& c) {
        ClaimNode* n = new ClaimNode{c, nullptr};
        if (rear) rear->next = n; else front = n;
        rear = n; count++;
    }
    bool dequeue(ClaimRequest& out) {
        if (!front) return false;
        ClaimNode* t = front; out = t->data; front = front->next;
        if (!front) rear = nullptr;
        delete t; count--; return true;
    }
    bool isEmpty() const { return front == nullptr; }
    int size() const { return count; }
    void display() const {
        if (!front) { cout << "  (no pending claims)\n"; return; }
        for (ClaimNode* t = front; t; t = t->next)
            cout << "  Claim #" << t->data.claimId << " by " << t->data.claimantEmail
                 << " (lost #" << t->data.lostId << " -> found #" << t->data.foundId << ")\n";
    }
};

// ================= 5. Stack of recent activity (LIFO) =================
struct ActNode { string text; ActNode* next; };
class ActivityStack {
private:
    ActNode* topNode; int count;
    ActivityStack(const ActivityStack&);
    ActivityStack& operator=(const ActivityStack&);
public:
    ActivityStack() : topNode(nullptr), count(0) {}
    ~ActivityStack() { while (!isEmpty()) pop(); }
    void push(const string& s) { topNode = new ActNode{s, topNode}; count++; }
    string pop() {
        if (!topNode) return "";
        ActNode* t = topNode; string s = t->text; topNode = t->next; delete t; count--; return s;
    }
    string top() const { return topNode ? topNode->text : ""; }
    bool isEmpty() const { return topNode == nullptr; }
    int size() const { return count; }
    void display(int n = 5) const {   // newest first
        if (!topNode) { cout << "  (no activity yet)\n"; return; }
        int i = 0;
        for (ActNode* t = topNode; t && i < n; t = t->next, i++) cout << "  - " << t->text << "\n";
    }
};

// ================= 6. Creating a report from console input =================
inline Report* createReportFromInput(bool isLost, const string& email, int id) {
    cout << (isLost ? "\n-- Report LOST item --\n" : "\n-- Report FOUND item --\n");
    string name = readLine("Item name: ");
    string cat  = readLine("Category (ID card / wallet / keys / phone / bag / other): ");
    string loc  = readLine("Location: ");
    string desc = readLine("Description (colour, brand, marks): ");
    if (name.empty() || cat.empty() || loc.empty()) { cout << "Name, category and location are required.\n"; return nullptr; }
    string secret;
    if (!isLost) {
        secret = readLine("Hidden detail only the owner would know (used for verification): ");
        if (secret.empty()) { cout << "A hidden detail is required for found items.\n"; return nullptr; }
    }
    Item it(name, cat, loc, desc, secret);
    if (isLost) return new LostReport(id, it, email);
    return new FoundReport(id, it, email);
}

#endif
