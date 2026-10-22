#include <cstdio>
#include "models.h"
#include "kanishka.h"
#include "priyanshu.h"
#include "ayushman.h"

int passed = 0;
int failed = 0;

void check(string name, bool condition) {
    if (condition) {
        passed++;
        cout << "PASS: " << name << "\n";
    } else {
        failed++;
        cout << "FAIL: " << name << "\n";
    }
}

ClaimRequest makeClaim(int claimId, int lostId, int foundId, string email, string answer) {
    ClaimRequest c;
    c.claimId = claimId;
    c.lostId = lostId;
    c.foundId = foundId;
    c.claimantEmail = email;
    c.answer = answer;
    return c;
}

int main() {
    check("college email accepted", AuthManager::isCollegeEmail("priyanshusharma.25101281839@gehu.ac.in"));
    check("gmail rejected", !AuthManager::isCollegeEmail("someone@gmail.com"));
    check("empty name before domain rejected", !AuthManager::isCollegeEmail("@gehu.ac.in"));
    check("uppercase college email accepted", AuthManager::isCollegeEmail("A.B@GEHU.AC.IN"));

    ReportList list;
    list.insert(new LostReport(1, Item("Black Wallet", "Wallet", "Library", "black leather wallet with student ID"), "a@gehu.ac.in"));
    list.insert(new FoundReport(2, Item("Wallet", "wallet", "Library gate", "leather wallet black colour", "photo of a dog"), "b@gehu.ac.in"));
    list.insert(new FoundReport(3, Item("House Keys", "Keys", "Canteen", "three keys on a red ring", "blue keychain"), "c@gehu.ac.in"));

    check("list size is 3", list.size() == 3);
    check("find existing id", list.find(2) != NULL);
    check("find missing id", list.find(99) == NULL);
    check("remove middle node", list.removeById(3) && list.size() == 2 && list.find(3) == NULL);
    check("remove missing id fails", !list.removeById(99));
    list.insert(new FoundReport(3, Item("House Keys", "Keys", "Canteen", "three keys on a red ring", "blue keychain"), "c@gehu.ac.in"));

    Report* results[MAX_RESULTS];
    check("search by name ignores case", searchReports(list, "wALLet", "", "", results) == 2);
    check("search by category", searchReports(list, "", "keys", "", results) == 1);
    check("search by partial location", searchReports(list, "", "", "library", results) == 2);
    check("search with all three fields", searchReports(list, "wallet", "wallet", "gate", results) == 1);
    check("search with no result", searchReports(list, "laptop", "", "", results) == 0);

    MatchResult matches[MAX_RESULTS];
    int matchCount = findMatches(list, *list.find(1), matches);
    check("similar wallet is matched", matchCount == 1 && matches[0].found->getId() == 2);
    check("keys are not matched with wallet", matches[0].found->getId() != 3);

    ClaimQueue queue;
    ClaimRequest out;
    queue.enqueue(makeClaim(1, 1, 2, "a@gehu.ac.in", "x"));
    queue.enqueue(makeClaim(2, 1, 2, "a@gehu.ac.in", "y"));
    check("queue is first in first out", queue.dequeue(out) && out.claimId == 1 && queue.dequeue(out) && out.claimId == 2 && queue.isEmpty());
    check("dequeue on empty queue fails", !queue.dequeue(out));

    ActivityStack stack;
    stack.push("one");
    stack.push("two");
    check("stack is last in first out", stack.pop() == "two" && stack.pop() == "one" && stack.isEmpty());

    string msg;
    ClaimRequest wrong = makeClaim(1, 1, 2, "a@gehu.ac.in", "a cat");
    ClaimRequest stranger = makeClaim(2, 1, 2, "z@gehu.ac.in", "photo of a dog");
    ClaimRequest correct = makeClaim(3, 1, 2, "a@gehu.ac.in", "  Photo of a DOG ");

    check("wrong answer rejected", !Verifier::verifyClaim(wrong, list, msg) && list.find(2)->getStatus() == OPEN);
    check("other student rejected", !Verifier::verifyClaim(stranger, list, msg));
    check("correct answer accepted", Verifier::verifyClaim(correct, list, msg) && list.find(2)->getStatus() == RETURNED && list.find(1)->getStatus() == RETURNED);
    check("cannot return twice", !Verifier::verifyClaim(correct, list, msg));

    check("save reports to file", FileStorage::saveReports(list, "test_reports.tmp"));
    ReportList loaded;
    int maxId = FileStorage::loadReports(loaded, "test_reports.tmp");
    check("load reports from file", loaded.size() == 3 && maxId == 3 && loaded.find(2)->getStatus() == RETURNED && loaded.find(2)->getItem().getSecretDetail() == "photo of a dog");
    remove("test_reports.tmp");

    Item weird("A|B", "x", "y", "z");
    check("pipe character removed from input", weird.getName() == "A/B");

    cout << "\n" << passed << " passed, " << failed << " failed\n";
    if (failed == 0) {
        return 0;
    }
    return 1;
}
