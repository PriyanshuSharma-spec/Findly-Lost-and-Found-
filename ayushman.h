#ifndef AYUSHMAN_H
#define AYUSHMAN_H

#include "models.h"
#include "kanishka.h"
#include "priyanshu.h"

class AuthManager {
private:
    User users[MAX_USERS];
    int userCount;

public:
    AuthManager() {
        userCount = 0;
    }

    static bool isCollegeEmail(string email) {
        email = toLower(trim(email));
        string domain = "@gehu.ac.in";

        if (email.size() <= domain.size()) {
            return false;
        }
        if (email.substr(email.size() - domain.size()) != domain) {
            return false;
        }

        string localPart = email.substr(0, email.size() - domain.size());
        if (localPart.find('@') != string::npos || localPart.find(' ') != string::npos) {
            return false;
        }
        return true;
    }

    void load() {
        userCount = FileStorage::loadUsers(users, MAX_USERS);
        if (userCount == 0) {
            users[0] = User("Admin", "admin@gehu.ac.in", hashPassword("admin123"), "admin");
            userCount = 1;
            FileStorage::saveUsers(users, userCount);
        }
    }

    bool registerUser(string name, string email, string password, string& msg) {
        if (trim(name) == "") {
            msg = "Name cannot be empty.";
            return false;
        }
        if (!isCollegeEmail(email)) {
            msg = "Only college email IDs (@gehu.ac.in) are allowed.";
            return false;
        }
        if (password.size() < 6) {
            msg = "Password must be at least 6 characters.";
            return false;
        }
        if (userCount >= MAX_USERS) {
            msg = "User limit reached.";
            return false;
        }
        for (int i = 0; i < userCount; i++) {
            if (users[i].getEmail() == toLower(trim(email))) {
                msg = "This email is already registered.";
                return false;
            }
        }

        users[userCount] = User(cleanText(name), trim(email), hashPassword(password), "student");
        userCount++;
        FileStorage::saveUsers(users, userCount);
        msg = "Registration successful.";
        return true;
    }

    bool login(string email, string password, User& loggedIn, string& msg) {
        if (!isCollegeEmail(email)) {
            msg = "Only college email IDs are allowed.";
            return false;
        }
        for (int i = 0; i < userCount; i++) {
            if (users[i].getEmail() == toLower(trim(email))) {
                if (users[i].checkPassword(password)) {
                    loggedIn = users[i];
                    msg = "Login successful.";
                    return true;
                }
                msg = "Wrong password.";
                return false;
            }
        }
        msg = "No account with this email.";
        return false;
    }

    int getUserCount() const { return userCount; }
    User getUser(int index) const { return users[index]; }
};

class AdminPanel {
public:
    static void viewUsers(const AuthManager& auth) {
        for (int i = 0; i < auth.getUserCount(); i++) {
            User u = auth.getUser(i);
            cout << "  " << u.getName() << " <" << u.getEmail() << "> [" << u.getRole() << "]\n";
        }
    }

    static bool processNextClaim(ClaimQueue& queue, ReportList& list, ActivityStack& activity, string& msg) {
        ClaimRequest c;
        if (!queue.dequeue(c)) {
            msg = "No pending claims.";
            return false;
        }

        string result;
        bool ok = Verifier::verifyClaim(c, list, result);
        msg = "Claim #" + to_string(c.claimId) + ": " + result;

        if (ok) {
            activity.push("Item returned (claim #" + to_string(c.claimId) + ")");
        } else {
            activity.push("Claim rejected (claim #" + to_string(c.claimId) + ")");
        }
        return true;
    }

    static bool verifyByQuestions(ReportList& list, ActivityStack& activity, string& msg) {
        int lostId = readInt("Lost report id: ");
        int foundId = readInt("Found report id: ");

        Report* lost = list.find(lostId);
        Report* found = list.find(foundId);

        if (lost == NULL || found == NULL || lost->getType() != "LOST" || found->getType() != "FOUND") {
            msg = "Invalid report ids.";
            return false;
        }
        if (lost->getStatus() == RETURNED || found->getStatus() == RETURNED) {
            msg = "Item was already returned.";
            return false;
        }

        cout << "\nFound item: " << found->getItem().getName() << " (" << found->getItem().getCategory() << ")\n";
        cout << "Ask the person these questions and type what they answer.\n\n";

        string email = readLine("1. What is your college email? ");
        string location = readLine("2. Where did you lose it? ");
        string description = readLine("3. Describe the item (colour, brand, marks): ");
        string hidden = readLine("4. What is the hidden detail of the item? ");

        bool emailOk = (toLower(trim(email)) == lost->getReporterEmail());

        bool locationOk = false;
        if (location != "") {
            locationOk = containsText(found->getItem().getLocation(), location) ||
                         containsText(location, found->getItem().getLocation());
        }

        string answerWords[MAX_WORDS];
        string itemWords[MAX_WORDS];
        int answerCount = getKeywords(description, answerWords);
        int itemCount = getKeywords(found->getItem().getName() + " " + found->getItem().getDescription(), itemWords);
        bool descriptionOk = false;
        for (int i = 0; i < answerCount; i++) {
            for (int j = 0; j < itemCount; j++) {
                if (answerWords[i] == itemWords[j]) {
                    descriptionOk = true;
                }
            }
        }

        string expected = toLower(trim(found->getItem().getSecretDetail()));
        bool hiddenOk = (expected != "" && toLower(trim(hidden)) == expected);

        cout << "\n--- Result ---\n";
        cout << "Email is the owner of the lost report: " << (emailOk ? "YES" : "NO") << "\n";
        cout << "Location matches: " << (locationOk ? "YES" : "NO") << "\n";
        cout << "Description matches: " << (descriptionOk ? "YES" : "NO") << "\n";
        cout << "Hidden detail matches: " << (hiddenOk ? "YES" : "NO") << "\n";

        if (!emailOk || !hiddenOk || (!locationOk && !descriptionOk)) {
            msg = "Verification failed. Item not returned.";
            activity.push("Verification failed for lost #" + to_string(lostId) + " and found #" + to_string(foundId));
            return false;
        }

        string confirm = readLine("\nAll good. Return the item to this person? (y/n): ");
        if (toLower(confirm) != "y") {
            msg = "Return cancelled by admin.";
            return false;
        }

        lost->setStatus(RETURNED);
        found->setStatus(RETURNED);
        activity.push("Item returned after questions (lost #" + to_string(lostId) + ", found #" + to_string(foundId) + ")");
        msg = "Verification passed. Item returned to owner.";
        return true;
    }
};

#endif
