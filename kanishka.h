#ifndef KANISHKA_H
#define KANISHKA_H

#include "models.h"
#include <fstream>

const int MAX_RESULTS = 100;
const int MAX_WORDS = 30;

struct Node {
    Report* data;
    Node* next;

    Node(Report* r) {
        data = r;
        next = NULL;
    }
};

class ReportList {
private:
    Node* head;
    int count;

public:
    ReportList() {
        head = NULL;
        count = 0;
    }

    ~ReportList() {
        clear();
    }

    void insert(Report* r) {
        Node* newNode = new Node(r);
        if (head == NULL) {
            head = newNode;
        } else {
            Node* temp = head;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
        count++;
    }

    bool removeById(int id) {
        Node* current = head;
        Node* previous = NULL;

        while (current != NULL && current->data->getId() != id) {
            previous = current;
            current = current->next;
        }

        if (current == NULL) {
            return false;
        }

        if (previous == NULL) {
            head = current->next;
        } else {
            previous->next = current->next;
        }

        delete current->data;
        delete current;
        count--;
        return true;
    }

    Report* find(int id) const {
        Node* temp = head;
        while (temp != NULL) {
            if (temp->data->getId() == id) {
                return temp->data;
            }
            temp = temp->next;
        }
        return NULL;
    }

    Node* getHead() const { return head; }
    int size() const { return count; }

    void clear() {
        while (head != NULL) {
            Node* temp = head;
            head = head->next;
            delete temp->data;
            delete temp;
        }
        count = 0;
    }

    void displayAll() const {
        if (head == NULL) {
            cout << "  Nothing to show.\n";
            return;
        }
        Node* temp = head;
        while (temp != NULL) {
            temp->data->display();
            temp = temp->next;
        }
    }
};

int searchReports(const ReportList& list, string name, string category, string location, Report* results[]) {
    int found = 0;
    Node* temp = list.getHead();

    while (temp != NULL && found < MAX_RESULTS) {
        Item it = temp->data->getItem();
        bool ok = true;

        if (name != "" && !containsText(it.getName(), name)) ok = false;
        if (category != "" && !containsText(it.getCategory(), category)) ok = false;
        if (location != "" && !containsText(it.getLocation(), location)) ok = false;

        if (ok) {
            results[found] = temp->data;
            found++;
        }
        temp = temp->next;
    }
    return found;
}

bool isStopWord(string w) {
    string stopWords[] = {"the", "and", "with", "has", "have", "from", "near", "this", "that", "was", "for"};
    for (int i = 0; i < 11; i++) {
        if (w == stopWords[i]) {
            return true;
        }
    }
    return false;
}

int getKeywords(string text, string words[]) {
    int count = 0;
    string current = "";
    text = toLower(text) + " ";

    for (size_t i = 0; i < text.size(); i++) {
        if (isalnum((unsigned char)text[i])) {
            current += text[i];
        } else {
            if (current.size() >= 3 && !isStopWord(current) && count < MAX_WORDS) {
                bool alreadyThere = false;
                for (int j = 0; j < count; j++) {
                    if (words[j] == current) {
                        alreadyThere = true;
                    }
                }
                if (!alreadyThere) {
                    words[count] = current;
                    count++;
                }
            }
            current = "";
        }
    }
    return count;
}

int matchScore(const Report& lost, const Report& found) {
    Item a = lost.getItem();
    Item b = found.getItem();
    int score = 0;

    if (toLower(a.getCategory()) == toLower(b.getCategory())) {
        score += 3;
    }

    if (containsText(a.getLocation(), b.getLocation()) || containsText(b.getLocation(), a.getLocation())) {
        score += 2;
    }

    string wordsA[MAX_WORDS];
    string wordsB[MAX_WORDS];
    int countA = getKeywords(a.getName() + " " + a.getDescription(), wordsA);
    int countB = getKeywords(b.getName() + " " + b.getDescription(), wordsB);

    int common = 0;
    for (int i = 0; i < countA; i++) {
        for (int j = 0; j < countB; j++) {
            if (wordsA[i] == wordsB[j]) {
                common++;
            }
        }
    }
    if (common > 4) {
        common = 4;
    }
    score += common;

    return score;
}

struct MatchResult {
    Report* found;
    int score;
};

int findMatches(const ReportList& list, const Report& lost, MatchResult matches[]) {
    int count = 0;
    Node* temp = list.getHead();

    while (temp != NULL && count < MAX_RESULTS) {
        Report* f = temp->data;
        if (f->getType() == "FOUND" && f->getStatus() != RETURNED) {
            bool sameCategory = toLower(lost.getItem().getCategory()) == toLower(f->getItem().getCategory());
            int score = matchScore(lost, *f);
            if (sameCategory && score >= 4) {
                matches[count].found = f;
                matches[count].score = score;
                count++;
            }
        }
        temp = temp->next;
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (matches[j].score < matches[j + 1].score) {
                MatchResult swapTemp = matches[j];
                matches[j] = matches[j + 1];
                matches[j + 1] = swapTemp;
            }
        }
    }
    return count;
}

struct ClaimNode {
    ClaimRequest data;
    ClaimNode* next;
};

class ClaimQueue {
private:
    ClaimNode* front;
    ClaimNode* rear;
    int count;

public:
    ClaimQueue() {
        front = NULL;
        rear = NULL;
        count = 0;
    }

    ~ClaimQueue() {
        ClaimRequest temp;
        while (dequeue(temp)) {
        }
    }

    void enqueue(ClaimRequest c) {
        ClaimNode* newNode = new ClaimNode;
        newNode->data = c;
        newNode->next = NULL;

        if (rear == NULL) {
            front = newNode;
        } else {
            rear->next = newNode;
        }
        rear = newNode;
        count++;
    }

    bool dequeue(ClaimRequest& c) {
        if (front == NULL) {
            return false;
        }
        ClaimNode* temp = front;
        c = temp->data;
        front = front->next;
        if (front == NULL) {
            rear = NULL;
        }
        delete temp;
        count--;
        return true;
    }

    bool isEmpty() const { return front == NULL; }
    int size() const { return count; }

    int maxClaimId() const {
        int maxId = 0;
        ClaimNode* temp = front;
        while (temp != NULL) {
            if (temp->data.claimId > maxId) maxId = temp->data.claimId;
            temp = temp->next;
        }
        return maxId;
    }

    bool saveToFile(const string& fileName = "claims.txt") const {
        ofstream out(fileName.c_str());
        if (!out) return false;
        ClaimNode* temp = front;
        while (temp != NULL) {
            out << temp->data.claimId << "|" << temp->data.lostId << "|"
                << temp->data.foundId << "|" << cleanText(temp->data.claimantEmail)
                << "|" << cleanText(temp->data.answer) << "\n";
            temp = temp->next;
        }
        return true;
    }

    int loadFromFile(const string& fileName = "claims.txt") {
        ifstream in(fileName.c_str());
        string line;
        int maxId = 0;
        ClaimRequest c;
        while (getline(in, line)) {
            string parts[5];
            if (splitLine(line, '|', parts, 5) != 5) continue;
            c.claimId = atoi(parts[0].c_str());
            c.lostId = atoi(parts[1].c_str());
            c.foundId = atoi(parts[2].c_str());
            c.claimantEmail = parts[3];
            c.answer = parts[4];
            enqueue(c);
            if (c.claimId > maxId) maxId = c.claimId;
        }
        return maxId;
    }

    void display() const {
        if (front == NULL) {
            cout << "  No pending claims.\n";
            return;
        }
        ClaimNode* temp = front;
        while (temp != NULL) {
            cout << "  Claim #" << temp->data.claimId << " by " << temp->data.claimantEmail
                 << " (lost #" << temp->data.lostId << ", found #" << temp->data.foundId << ")\n";
            temp = temp->next;
        }
    }
};

struct ActivityNode {
    string text;
    ActivityNode* next;
};

class ActivityStack {
private:
    ActivityNode* topNode;
    int count;

public:
    ActivityStack() {
        topNode = NULL;
        count = 0;
    }

    ~ActivityStack() {
        while (!isEmpty()) {
            pop();
        }
    }

    void push(string text) {
        ActivityNode* newNode = new ActivityNode;
        newNode->text = text;
        newNode->next = topNode;
        topNode = newNode;
        count++;
    }

    string pop() {
        if (topNode == NULL) {
            return "";
        }
        ActivityNode* temp = topNode;
        string text = temp->text;
        topNode = topNode->next;
        delete temp;
        count--;
        return text;
    }

    string top() const {
        if (topNode == NULL) {
            return "";
        }
        return topNode->text;
    }

    bool isEmpty() const { return topNode == NULL; }
    int size() const { return count; }

    bool saveToFile(const string& fileName = "activity.txt") const {
        ofstream out(fileName.c_str());
        if (!out) return false;
        // Write oldest first so loading restores the same stack order.
        string entries[1000];
        int n = 0;
        ActivityNode* temp = topNode;
        while (temp != NULL && n < 1000) {
            entries[n++] = cleanText(temp->text);
            temp = temp->next;
        }
        for (int i = n - 1; i >= 0; --i) out << entries[i] << "\n";
        return true;
    }

    void loadFromFile(const string& fileName = "activity.txt") {
        ifstream in(fileName.c_str());
        string line;
        string entries[1000];
        int n = 0;
        while (getline(in, line) && n < 1000) entries[n++] = line;
        for (int i = 0; i < n; ++i) push(entries[i]);
    }

    void display(int limit) const {
        if (topNode == NULL) {
            cout << "  No activity yet.\n";
            return;
        }
        ActivityNode* temp = topNode;
        int shown = 0;
        while (temp != NULL && shown < limit) {
            cout << "  - " << temp->text << "\n";
            temp = temp->next;
            shown++;
        }
    }
};

Report* createReportFromInput(bool isLost, string email, int id) {
    if (isLost) {
        cout << "\n-- Report LOST item --\n";
    } else {
        cout << "\n-- Report FOUND item --\n";
    }

    string name = readLine("Item name: ");
    string category = readLine("Category (ID card / wallet / keys / phone / bag / other): ");
    string location = readLine("Location: ");
    string description = readLine("Description (colour, brand, marks): ");

    if (name == "" || category == "" || location == "") {
        cout << "Name, category and location are required.\n";
        return NULL;
    }

    string secret = "";
    if (!isLost) {
        secret = readLine("Hidden detail only the owner would know: ");
        if (secret == "") {
            cout << "A hidden detail is required for found items.\n";
            return NULL;
        }
    }

    Item it(name, category, location, description, secret);
    if (isLost) {
        return new LostReport(id, it, email);
    }
    return new FoundReport(id, it, email);
}

#endif
