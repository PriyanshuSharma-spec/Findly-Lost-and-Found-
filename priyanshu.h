#ifndef PRIYANSHU_H
#define PRIYANSHU_H

#include <fstream>
#include "models.h"
#include "kanishka.h"

class FileStorage {
public:
    static Report* fromRecord(string line) {
        string parts[9];
        int n = splitLine(line, '|', parts, 9);
        if (n < 9) {
            return NULL;
        }

        int id = atoi(parts[1].c_str());
        int status = atoi(parts[3].c_str());
        Item it(parts[4], parts[5], parts[6], parts[7], parts[8]);

        if (parts[0] == "LOST") {
            return new LostReport(id, it, parts[2], status);
        }
        if (parts[0] == "FOUND") {
            return new FoundReport(id, it, parts[2], status);
        }
        return NULL;
    }

    static bool saveReports(const ReportList& list, string fileName = "reports.txt") {
        ofstream out(fileName.c_str());
        if (!out) {
            return false;
        }
        Node* temp = list.getHead();
        while (temp != NULL) {
            out << temp->data->toRecord() << "\n";
            temp = temp->next;
        }
        return true;
    }

    static int loadReports(ReportList& list, string fileName = "reports.txt") {
        ifstream in(fileName.c_str());
        string line;
        int maxId = 0;

        while (getline(in, line)) {
            if (trim(line) == "") {
                continue;
            }
            Report* r = fromRecord(line);
            if (r != NULL) {
                list.insert(r);
                if (r->getId() > maxId) {
                    maxId = r->getId();
                }
            }
        }
        return maxId;
    }

    static bool saveUsers(User users[], int count, string fileName = "users.txt") {
        ofstream out(fileName.c_str());
        if (!out) {
            return false;
        }
        for (int i = 0; i < count; i++) {
            out << cleanText(users[i].getName()) << "|" << users[i].getEmail() << "|"
                << users[i].getHash() << "|" << users[i].getRole() << "\n";
        }
        return true;
    }

    static int loadUsers(User users[], int maxUsers, string fileName = "users.txt") {
        ifstream in(fileName.c_str());
        string line;
        int count = 0;

        while (getline(in, line) && count < maxUsers) {
            string parts[4];
            int n = splitLine(line, '|', parts, 4);
            if (n == 4) {
                users[count] = User(parts[0], parts[1], parts[2], parts[3]);
                count++;
            }
        }
        return count;
    }
};

#ifdef USE_MYSQL
#include <mysql/mysql.h>

class Database {
private:
    MYSQL* conn;

    string escape(string s) {
        char* buffer = new char[s.size() * 2 + 1];
        unsigned long n = mysql_real_escape_string(conn, buffer, s.c_str(), s.size());
        string result(buffer, n);
        delete[] buffer;
        return result;
    }

    bool runQuery(string query) {
        if (conn == NULL) {
            return false;
        }
        if (mysql_query(conn, query.c_str())) {
            cout << "MySQL error: " << mysql_error(conn) << "\n";
            return false;
        }
        return true;
    }

public:
    Database() {
        conn = NULL;
    }

    ~Database() {
        disconnect();
    }

    bool connect(string host, string user, string pass, string dbName) {
        conn = mysql_init(NULL);
        if (conn == NULL) {
            return false;
        }
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), dbName.c_str(), 3306, NULL, 0)) {
            cout << "MySQL connection failed: " << mysql_error(conn) << "\n";
            mysql_close(conn);
            conn = NULL;
            return false;
        }
        return true;
    }

    void disconnect() {
        if (conn != NULL) {
            mysql_close(conn);
            conn = NULL;
        }
    }

    bool isConnected() const {
        return conn != NULL;
    }

    bool saveUser(const User& u) {
        string q = "INSERT INTO users (email, name, password_hash, role) VALUES ('";
        q += escape(u.getEmail()) + "', '" + escape(u.getName()) + "', '";
        q += escape(u.getHash()) + "', '" + escape(u.getRole()) + "') ";
        q += "ON DUPLICATE KEY UPDATE name = VALUES(name)";
        return runQuery(q);
    }

    bool saveReport(const Report& r) {
        Item it = r.getItem();
        string q = "INSERT INTO reports (id, type, reporter_email, status, name, category, location, description, secret_detail) VALUES (";
        q += to_string(r.getId()) + ", '" + r.getType() + "', '" + escape(r.getReporterEmail()) + "', ";
        q += to_string(r.getStatus()) + ", '" + escape(it.getName()) + "', '" + escape(it.getCategory()) + "', '";
        q += escape(it.getLocation()) + "', '" + escape(it.getDescription()) + "', '" + escape(it.getSecretDetail()) + "') ";
        q += "ON DUPLICATE KEY UPDATE status = VALUES(status)";
        return runQuery(q);
    }

    bool syncAll(const ReportList& list) {
        bool ok = true;
        Node* temp = list.getHead();
        while (temp != NULL) {
            if (!saveReport(*temp->data)) {
                ok = false;
            }
            temp = temp->next;
        }
        return ok;
    }
};
#else
class Database {
public:
    bool connect(string, string, string, string) { return false; }
    void disconnect() {}
    bool isConnected() const { return false; }
    bool saveUser(const User&) { return false; }
    bool saveReport(const Report&) { return false; }
    bool syncAll(const ReportList&) { return false; }
};
#endif

class Verifier {
public:
    static bool verifyClaim(ClaimRequest c, ReportList& list, string& msg) {
        Report* lost = list.find(c.lostId);
        Report* found = list.find(c.foundId);

        if (lost == NULL || found == NULL || lost->getType() != "LOST" || found->getType() != "FOUND") {
            msg = "Invalid report ids in claim.";
            return false;
        }
        if (lost->getStatus() == RETURNED || found->getStatus() == RETURNED) {
            msg = "Item was already returned.";
            return false;
        }
        if (lost->getReporterEmail() != toLower(c.claimantEmail)) {
            msg = "Claimant did not file the lost report.";
            return false;
        }

        string expected = toLower(trim(found->getItem().getSecretDetail()));
        if (expected == "") {
            msg = "Found item has no hidden detail, verify in person.";
            return false;
        }
        if (toLower(trim(c.answer)) != expected) {
            msg = "Verification failed, answer does not match.";
            return false;
        }

        lost->setStatus(RETURNED);
        found->setStatus(RETURNED);
        msg = "Verification passed. Item released to owner.";
        return true;
    }
};

#endif
