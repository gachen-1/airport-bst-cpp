// Airport Code Lookup Service with a Binary Search Tree
// EECE 2560 / EECE 7205 Mini Project. Data: OpenFlights.org (Open Database License).

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#define IS_TTY _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define IS_TTY isatty(fileno(stdin))
#endif

#include "Airport.h"
#include "AirportBST.h"

using namespace std;

// ======================================================================== helpers

static string toUpper(string s)
{
    int n = (int)s.size();
    for (int i = 0; i < n; i++)
        if (s[i] >= 'a' && s[i] <= 'z') s[i] = (char)(s[i] - 'a' + 'A');
    return s;
}

static string trim(const string& s)
{
    int b = 0, e = (int)s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) b++;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) e--;
    return s.substr(b, e - b);
}

static vector<string> words(const string& line)
{
    vector<string> out;
    istringstream in(line);
    string w;
    while (in >> w) out.push_back(w);
    return out;
}

static string fixed1(double x)
{
    ostringstream out;
    out << fixed << setprecision(1) << x;
    return out.str();
}

// Merge sort of airports by IATA code (our own, stable, Theta(n log n)).
static void mergeSortAirports(vector<Airport>& a, vector<Airport>& tmp, int lo, int hi)
{
    if (hi - lo < 1) return;
    int mid = lo + (hi - lo) / 2;
    mergeSortAirports(a, tmp, lo, mid);
    mergeSortAirports(a, tmp, mid + 1, hi);
    int i = lo, j = mid + 1;
    int n = hi - lo + 1;
    for (int k = 0; k < n; k++)
    {
        if (j > hi || (i <= mid && !(a[j].iata < a[i].iata))) tmp[lo + k] = a[i++];
        else tmp[lo + k] = a[j++];
    }
    for (int k = 0; k < n; k++) a[lo + k] = tmp[lo + k];
}

static vector<Airport> sortedByCode(const vector<Airport>& records)
{
    vector<Airport> a = records;
    vector<Airport> tmp(a.size());
    if (!a.empty()) mergeSortAirports(a, tmp, 0, (int)a.size() - 1);
    return a;
}

// Merge sort of plain strings (used to count distinct countries).
static void mergeSortStrings(vector<string>& a, vector<string>& tmp, int lo, int hi)
{
    if (hi - lo < 1) return;
    int mid = lo + (hi - lo) / 2;
    mergeSortStrings(a, tmp, lo, mid);
    mergeSortStrings(a, tmp, mid + 1, hi);
    int i = lo, j = mid + 1;
    int n = hi - lo + 1;
    for (int k = 0; k < n; k++)
    {
        if (j > hi || (i <= mid && !(a[j] < a[i]))) tmp[lo + k] = a[i++];
        else tmp[lo + k] = a[j++];
    }
    for (int k = 0; k < n; k++) a[lo + k] = tmp[lo + k];
}

// Binary search for a code in airports sorted by code. Returns -1 if absent.
static int findInSorted(const vector<Airport>& sorted, const string& code)
{
    int lo = 0, hi = (int)sorted.size() - 1;
    while (lo <= hi)
    {
        int mid = lo + (hi - lo) / 2;
        if (sorted[mid].iata == code) return mid;
        if (sorted[mid].iata < code) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// Fisher-Yates shuffle with a fixed seed so the experiment can be repeated.
static vector<Airport> shuffled(const vector<Airport>& records, unsigned seed)
{
    vector<Airport> a = records;
    mt19937 rng(seed);
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++)
    {
        int j = i + (int)(rng() % (unsigned)(n - i));
        Airport t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
    return a;
}

// Insert airports in the given order. Returns the number of duplicates (rule I4).
// If validateEach is true, the BST property is checked after every insert.
static int buildTree(AirportBST& tree, const vector<Airport>& order, bool validateEach, bool& allValid)
{
    tree.clear();
    int duplicates = 0;
    allValid = true;
    int n = (int)order.size();
    for (int i = 0; i < n; i++)
    {
        if (!tree.insert(order[i]))
        {
            duplicates++;
            cout << "  Duplicate code (I4, not inserted): " << order[i].iata << "\n";
        }
        if (validateEach && !tree.validate()) allValid = false;
    }
    return duplicates;
}

// Read airports.dat. Returns false if the file cannot be opened.
static bool loadAirportFile(const string& path, vector<Airport>& records, int& totalLines, int& skipped)
{
    ifstream in(path.c_str());
    if (!in) return false;
    records.clear();
    totalLines = 0;
    skipped = 0;
    string line;
    while (getline(in, line))
    {
        if (trim(line).empty()) continue;
        totalLines++;
        Airport a;
        int r = Airport::parseLine(line, a);
        if (r == 1) records.push_back(a);
        else if (r == 0) skipped++;
        else totalLines--;   // header row of the CSV version is not counted as a data line
    }
    return true;
}

static void countCountries(const vector<Airport>& records, int& countries, int& unitedStates)
{
    vector<string> c;
    unitedStates = 0;
    int n = (int)records.size();
    for (int i = 0; i < n; i++)
    {
        c.push_back(records[i].country);
        if (records[i].country == "United States") unitedStates++;
    }
    vector<string> tmp(c.size());
    if (!c.empty()) mergeSortStrings(c, tmp, 0, (int)c.size() - 1);
    countries = 0;
    int m = (int)c.size();
    for (int i = 0; i < m; i++)
        if (i == 0 || c[i] != c[i - 1]) countries++;
}

static void printShape(const AirportBST& tree)
{
    cout << "Height: " << tree.height() << "  Average depth: " << fixed1(tree.averageDepth()) << "\n";
}

static void printLevels(const AirportBST& tree, int k, const string& indent)
{
    vector<vector<string>> lv = tree.levels(k);
    int n = (int)lv.size();
    for (int d = 0; d < n; d++)
    {
        cout << indent << "Level " << d << ":";
        int m = (int)lv[d].size();
        for (int i = 0; i < m; i++) cout << " " << lv[d][i];
        cout << "\n";
    }
}

static void printPath(const vector<string>& path, bool found)
{
    cout << "Path: ";
    int n = (int)path.size();
    for (int i = 0; i < n; i++)
    {
        if (i > 0) cout << " -> ";
        cout << path[i];
    }
    if (found) cout << " (comparisons: " << n << ")\n";
    else cout << (n > 0 ? " -> " : "") << "(NULL) not found (comparisons: " << n << ")\n";
}

static bool inorderIsSorted(const AirportBST& tree)
{
    vector<const Airport*> list;
    tree.inorder(list);
    int n = (int)list.size();
    for (int i = 0; i < n - 1; i++)
        if (!(list[i]->iata < list[i + 1]->iata)) return false;
    return true;
}

static bool sameShape(const AirportBST& a, const AirportBST& b)
{
    if (a.height() != b.height() || a.size() != b.size()) return false;
    vector<vector<string>> la = a.levels(-1), lb = b.levels(-1);
    int n = (int)la.size();
    for (int d = 0; d < n; d++)
    {
        if (la[d].size() != lb[d].size()) return false;
        int m = (int)la[d].size();
        for (int i = 0; i < m; i++)
            if (la[d][i] != lb[d][i]) return false;
    }
    return true;
}

static string okText(bool ok) { return ok ? "OK" : "FAILED"; }

// ======================================================================== the service

struct Service
{
    vector<Airport> records;   // airports in file order
    vector<Airport> sorted;    // the same airports sorted by code
    AirportBST tree;
    bool interactive;

    bool haveData() const
    {
        if (records.empty())
        {
            cout << "No data loaded. Use: load data/airports.dat\n";
            return false;
        }
        return true;
    }

    // ------------------------------------------------------------ load
    void cmdLoad(const string& path)
    {
        ifstream probe(path.c_str());
        if (!probe)
        {
            cout << "Cannot open " << path << "\n";
            return;
        }
        string first;
        while (getline(probe, first) && trim(first).empty()) {}
        probe.close();

        if (first.find(',') != string::npos)
        {
            // An airports.dat file: read the records and build the tree in file order.
            int total = 0, skipped = 0;
            loadAirportFile(path, records, total, skipped);
            sorted = sortedByCode(records);
            bool valid;
            int dup = buildTree(tree, records, false, valid);
            cout << "Loaded " << tree.size() << " airports (" << skipped
                 << " lines skipped: no IATA code). Duplicates: " << dup << "\n";
            printShape(tree);
            int countries, us;
            countCountries(records, countries, us);
            cout << "Lines: " << total << "  Countries: " << countries << "  United States: " << us
                 << "  Inorder sorted: " << (inorderIsSorted(tree) ? "yes" : "no") << "\n";
        }
        else
        {
            // A file written by 'save': one code per line in preorder. Rebuild the tree.
            if (!haveData()) return;
            // Records are looked up in the current tree first (it may hold added airports),
            // then in the original data.
            vector<const Airport*> current;
            tree.inorder(current);
            vector<Airport> lookup;
            int c = (int)current.size();
            for (int i = 0; i < c; i++) lookup.push_back(*current[i]);

            ifstream in(path.c_str());
            vector<Airport> order;
            int missing = 0;
            string line;
            while (getline(in, line))
            {
                string code = toUpper(trim(line));
                if (code.empty()) continue;
                int k = findInSorted(lookup, code);
                if (k >= 0)
                {
                    order.push_back(lookup[k]);
                    continue;
                }
                k = findInSorted(sorted, code);
                if (k < 0)
                {
                    missing++;
                    continue;
                }
                order.push_back(sorted[k]);
            }
            bool valid;
            int dup = buildTree(tree, order, false, valid);
            cout << "Rebuilt tree from " << path << ": " << tree.size() << " codes inserted in preorder"
                 << " (unknown codes: " << missing << ", duplicates: " << dup << ")\n";
            printShape(tree);
        }
    }

    // ------------------------------------------------------------ find
    void cmdFind(const string& code)
    {
        vector<string> path;
        const Airport* a = tree.find(code, path);
        printPath(path, a != nullptr);
        if (a != nullptr) cout << a->summary() << "\n";
    }

    void cmdMinMax()
    {
        const Airport* lo = tree.findMin();
        const Airport* hi = tree.findMax();
        if (lo == nullptr)
        {
            cout << "The tree is empty\n";
            return;
        }
        cout << "Minimum: " << lo->summary() << "\n";
        cout << "Maximum: " << hi->summary() << "\n";
    }

    // ------------------------------------------------------------ remove / add
    void cmdRemove(const string& code)
    {
        string report;
        tree.remove(code, report);
        cout << report << "  validate: " << okText(tree.validate()) << "\n";
    }

    void cmdAdd(const string& code, istream& in)
    {
        Airport a;
        a.iata = code;
        string line;
        if (interactive) cout << "  Name: ";
        getline(in, line);
        a.name = trim(line);
        if (interactive) cout << "  City: ";
        getline(in, line);
        a.city = trim(line);
        if (interactive) cout << "  Country: ";
        getline(in, line);
        a.country = trim(line);
        if (!interactive) cout << "  Name: " << a.name << "\n  City: " << a.city << "\n  Country: " << a.country << "\n";

        if (tree.insert(a)) cout << "Added " << a.summary();
        else cout << code << " is already in the tree (I4): not added";
        cout << "  validate: " << okText(tree.validate()) << "\n";
    }

    // ------------------------------------------------------------ list / save / validate
    void cmdList(int limit)
    {
        vector<const Airport*> list;
        tree.inorder(list);
        int n = (int)list.size();
        int shown = (limit > 0 && limit < n) ? limit : n;
        for (int i = 0; i < shown; i++) cout << list[i]->summary() << "\n";
        cout << "Listed " << shown << " of " << n << " airports. Inorder is sorted: "
             << (inorderIsSorted(tree) ? "yes" : "no") << "\n";
    }

    void cmdSave(const string& path)
    {
        vector<string> codes;
        tree.preorderCodes(codes);
        ofstream out(path.c_str());
        if (!out)
        {
            cout << "Cannot write " << path << "\n";
            return;
        }
        int n = (int)codes.size();
        for (int i = 0; i < n; i++) out << codes[i] << "\n";
        cout << "Saved " << n << " codes in preorder to " << path << "\n";
    }

    // ------------------------------------------------------------ range
    void cmdRange(string from, string to, bool show)
    {
        if (to < from)
        {
            string t = from;
            from = to;
            to = t;
        }
        vector<const Airport*> out;
        int visited = 0;
        tree.range(from, to, out, visited);
        if (show)
        {
            int n = (int)out.size();
            for (int i = 0; i < n; i++) cout << out[i]->summary() << "\n";
        }
        cout << out.size() << " airports, " << visited << " nodes visited\n";
    }

    // ------------------------------------------------------------ build orders
    void cmdBuild(const string& how, unsigned seed)
    {
        bool valid;
        if (how == "FILE") buildTree(tree, records, false, valid);
        else if (how == "SORTED") buildTree(tree, sorted, false, valid);
        else if (how == "SHUFFLE") buildTree(tree, shuffled(records, seed), false, valid);
        else
        {
            cout << "Usage: build file | sorted | shuffle [seed]\n";
            return;
        }
        cout << "Built " << tree.size() << " airports in " << how << " order"
             << (how == "SHUFFLE" ? " (seed " + to_string(seed) + ")" : "") << "\n";
        printShape(tree);
    }

    void cmdRebalance()
    {
        vector<const Airport*> list;
        tree.inorder(list);                      // the current keys, already sorted
        vector<Airport> s;
        int n = (int)list.size();
        for (int i = 0; i < n; i++) s.push_back(*list[i]);
        tree.buildBalanced(s);
        printShape(tree);
    }

    void cmdStats()
    {
        cout << "Airports in tree: " << tree.size() << "  Root: " << tree.rootCode() << "\n";
        printShape(tree);
        cmdMinMax();
        cout << "validate: " << okText(tree.validate()) << "\n";
    }

    // ------------------------------------------------------------ Task 6
    void cmdExperiment()
    {
        if (!haveData()) return;
        const int SEARCHES = 10000;
        const int ROUNDS = 5;
        mt19937 rng(7205);
        vector<string> targets;
        int n = (int)records.size();
        for (int i = 0; i < SEARCHES; i++) targets.push_back(records[rng() % (unsigned)n].iata);

        const char* names[4] = {"File order", "Sorted order", "Balanced (middle rule)", "Random shuffle (seed 2026)"};
        cout << "Task 6: shape experiment on " << n << " codes, " << SEARCHES << " random successful searches (time: best of " << ROUNDS << " runs)\n\n";
        cout << left << setw(28) << "Build order" << right << setw(8) << "Height" << setw(15) << "Average depth"
             << setw(16) << "Time (ms)" << setw(18) << "Avg comparisons" << "\n";
        cout << string(85, '-') << "\n";
        for (int b = 0; b < 4; b++)
        {
            AirportBST t;
            bool valid;
            if (b == 0) buildTree(t, records, false, valid);
            else if (b == 1) buildTree(t, sorted, false, valid);
            else if (b == 2) t.buildBalanced(sorted);
            else buildTree(t, shuffled(records, 2026), false, valid);

            // Time the same 10,000 searches 5 times and keep the fastest run,
            // so that a single slow run (another program, cache misses) does not distort the table.
            long long totalComparisons = 0;
            int found = 0;
            double ms = 0.0;
            for (int r = 0; r < ROUNDS; r++)
            {
                totalComparisons = 0;
                found = 0;
                auto start = chrono::steady_clock::now();
                for (int i = 0; i < SEARCHES; i++)
                {
                    int c = 0;
                    if (t.contains(targets[i], c)) found++;
                    totalComparisons += c;
                }
                auto stop = chrono::steady_clock::now();
                double run = chrono::duration<double, milli>(stop - start).count();
                if (r == 0 || run < ms) ms = run;
            }

            cout << left << setw(28) << names[b] << right << setw(8) << t.height() << setw(15)
                 << fixed1(t.averageDepth()) << setw(16) << fixed << setprecision(3) << ms << setw(18)
                 << fixed1((double)totalComparisons / SEARCHES) << "\n";
            if (found != SEARCHES) cout << "  warning: only " << found << " searches succeeded\n";
        }
    }

    // ------------------------------------------------------------ automated tests
    void check(const string& name, bool ok, int& passed, int& failed)
    {
        cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << name << "\n";
        if (ok) passed++;
        else failed++;
    }

    void cmdTest()
    {
        if (!haveData()) return;
        int passed = 0, failed = 0;
        bool valid;

        cout << "=== Task 1: load and build (file order) ===\n";
        int dup = buildTree(tree, records, true, valid);
        int countries, us;
        countCountries(records, countries, us);
        check("6072 airports loaded", (int)records.size() == 6072, passed, failed);
        check("0 duplicates", dup == 0, passed, failed);
        check("235 countries, 1251 in the United States", countries == 235 && us == 1251, passed, failed);
        check("height 177", tree.height() == 177, passed, failed);
        check("validate after every one of the 6072 inserts", valid, passed, failed);
        check("inorder list is sorted", inorderIsSorted(tree), passed, failed);

        cout << "\n=== Task 2: every search case ===\n";
        const Airport* mn = tree.findMin();
        const Airport* mx = tree.findMax();
        string smaller = (string("AAA") < mn->iata) ? "AAA" : "000";
        string larger = (mx->iata < string("ZZZ")) ? "ZZZ" : "~~~";
        string cases[6][2] = {{"root", tree.rootCode()},
                              {"leaf", tree.exampleCode(0)},
                              {"deepest node", tree.deepestCode()},
                              {"not in the tree", "XYZ"},
                              {"smaller than every key", smaller},
                              {"larger than every key", larger}};
        for (int i = 0; i < 6; i++)
        {
            vector<string> path;
            const Airport* a = tree.find(cases[i][1], path);
            cout << "find " << cases[i][1] << " (" << cases[i][0] << ")\n  ";
            printPath(path, a != nullptr);
            bool expectFound = (i < 3);
            check(cases[i][0] + (expectFound ? " is found" : " is not found"), (a != nullptr) == expectFound,
                  passed, failed);
        }
        cmdMinMax();

        cout << "\n=== Task 3: every delete case (each starts from the original tree) ===\n";
        string del[6][2] = {{"D1 leaf", ""},
                            {"D2 only a left child", ""},
                            {"D2 only a right child", ""},
                            {"D3 two children", ""},
                            {"root (two children, D3 + B6)", ""},
                            {"sample session: BOS", "BOS"}};
        for (int i = 0; i < 6; i++)
        {
            buildTree(tree, records, false, valid);
            string code = del[i][1];
            if (i < 4) code = tree.exampleCode(i);
            if (i == 4) code = tree.rootCode();
            cout << "remove " << code << "  [" << del[i][0] << "]\n";
            cout << "  Before:\n";
            printLevels(tree, 3, "    ");
            int before = tree.size();
            string report;
            bool removed = tree.remove(code, report);
            cout << "  " << report << "\n  After:\n";
            printLevels(tree, 3, "    ");
            vector<string> path;
            bool gone = (tree.find(code, path) == nullptr);
            bool ok = removed && gone && tree.size() == before - 1 && tree.validate() && inorderIsSorted(tree);
            check(del[i][0] + ": removed, size - 1, validate OK, inorder sorted", ok, passed, failed);
        }
        {
            buildTree(tree, records, false, valid);
            string report;
            int before = tree.size();
            bool removed = tree.remove("XYZ", report);
            cout << "remove XYZ  [not in the tree]\n  " << report << "\n";
            check("S1: removing a missing code changes nothing", !removed && tree.size() == before, passed, failed);
        }

        cout << "\n=== Task 3(c): add a made-up airport ===\n";
        buildTree(tree, records, false, valid);
        Airport q;
        q.iata = "QQQ";
        q.name = "Jiachen Hou International Airport";
        q.city = "Boston";
        q.country = "United States";
        bool added = tree.insert(q);
        cout << "add QQQ: " << q.summary() << "  validate: " << okText(tree.validate()) << "\n";
        cmdFind("QQQ");
        string report;
        tree.remove("QQQ", report);
        cout << report << "  validate: " << okText(tree.validate()) << "\n";
        vector<string> path;
        check("QQQ added, found, then removed", added && tree.find("QQQ", path) == nullptr && tree.validate(),
              passed, failed);

        cout << "\n=== Task 4: traversals and validation ===\n";
        buildTree(tree, records, false, valid);
        check("list (inorder) is sorted", inorderIsSorted(tree), passed, failed);
        cmdSave("preorder_test.txt");
        AirportBST reloaded;
        {
            ifstream in("preorder_test.txt");
            vector<Airport> order;
            string line;
            while (getline(in, line))
            {
                int k = findInSorted(sorted, trim(line));
                if (k >= 0) order.push_back(sorted[k]);
            }
            buildTree(reloaded, order, false, valid);
        }
        remove("preorder_test.txt");
        cout << "  original: height " << tree.height() << ", reloaded: height " << reloaded.height() << "\n";
        check("reloaded tree has the same height and level order", sameShape(tree, reloaded), passed, failed);
        check("validate (range method) on the reloaded tree", reloaded.validate(), passed, failed);

        cout << "\n=== Task 5: range query ===\n";
        vector<const Airport*> out;
        int visited = 0;
        tree.range("BOS", "BWI", out, visited);
        cout << "range BOS BWI: " << out.size() << " airports, " << visited << " nodes visited\n";
        vector<const Airport*> all;
        tree.inorder(all);
        int brute = 0;
        int n = (int)all.size();
        for (int i = 0; i < n; i++)
            if (!(all[i]->iata < string("BOS")) && !(string("BWI") < all[i]->iata)) brute++;
        check("range BOS BWI returns 113 airports", out.size() == 113, passed, failed);
        check("same count as a full inorder scan", (int)out.size() == brute, passed, failed);
        check("visits far fewer nodes than the whole tree", visited < tree.size() / 10, passed, failed);

        cout << "\n=== Task 6: rebalance ===\n";
        cmdRebalance();
        check("balanced height 12", tree.height() == 12, passed, failed);
        check("validate after rebalance", tree.validate() && tree.size() == 6072, passed, failed);

        cout << "\nTests passed: " << passed << ", failed: " << failed << "\n";
        buildTree(tree, records, false, valid);   // leave the tree in file order
    }

    void cmdHelp()
    {
        cout << "Commands:\n"
                "  load FILE            load airports.dat (build in file order) or a saved preorder file\n"
                "  find CODE            search, print the path and the airport\n"
                "  min | max            smallest and largest code\n"
                "  remove CODE          delete an airport (cases D1, D2, D3, root)\n"
                "  add CODE             insert a new airport (asks for name, city, country)\n"
                "  list [N]             all airports in code order (or the first N)\n"
                "  save FILE            write the codes in preorder\n"
                "  validate             check the BST property with the (lo, hi) method\n"
                "  range FROM TO [-q]   airports with FROM <= code <= TO (-q: count only)\n"
                "  levels [K]           level order of the top K levels (default 3)\n"
                "  build file|sorted|shuffle [seed]   rebuild in another order\n"
                "  rebalance            rebuild as a balanced tree (middle rule)\n"
                "  stats                size, height, average depth, min, max\n"
                "  experiment           Task 6 table\n"
                "  test                 run every required test case\n"
                "  help | quit\n";
    }
};

int main(int argc, char* argv[])
{
    Service s;
    s.interactive = IS_TTY;

    if (argc > 1)
    {
        cout << "> load " << argv[1] << "\n";
        s.cmdLoad(argv[1]);
    }
    else if (s.interactive)
    {
        cout << "Airport Code Lookup Service. Type 'help' for commands.\n";
    }

    string line;
    while (true)
    {
        cout << "> " << flush;
        if (!getline(cin, line)) break;
        line = trim(line);
        if (!s.interactive) cout << line << "\n";
        if (line.empty()) continue;

        vector<string> w = words(line);
        string cmd = toUpper(w[0]);
        int n = (int)w.size();

        if (cmd == "QUIT" || cmd == "EXIT") break;
        else if (cmd == "HELP") s.cmdHelp();
        else if (cmd == "LOAD" && n >= 2) s.cmdLoad(w[1]);
        else if (cmd == "FIND" && n >= 2) s.cmdFind(toUpper(w[1]));
        else if (cmd == "MIN" || cmd == "MAX") s.cmdMinMax();
        else if (cmd == "REMOVE" && n >= 2) s.cmdRemove(toUpper(w[1]));
        else if (cmd == "ADD" && n >= 2) s.cmdAdd(toUpper(w[1]), cin);
        else if (cmd == "LIST") s.cmdList(n >= 2 ? atoi(w[1].c_str()) : 0);
        else if (cmd == "SAVE" && n >= 2) s.cmdSave(w[1]);
        else if (cmd == "VALIDATE") cout << "validate: " << okText(s.tree.validate()) << "\n";
        else if (cmd == "RANGE" && n >= 3) s.cmdRange(toUpper(w[1]), toUpper(w[2]), !(n >= 4 && w[3] == "-q"));
        else if (cmd == "LEVELS") printLevels(s.tree, n >= 2 ? atoi(w[1].c_str()) : 3, "");
        else if (cmd == "BUILD" && n >= 2) s.cmdBuild(toUpper(w[1]), n >= 3 ? (unsigned)atoi(w[2].c_str()) : 2026u);
        else if (cmd == "REBALANCE") s.cmdRebalance();
        else if (cmd == "STATS") s.cmdStats();
        else if (cmd == "EXPERIMENT") s.cmdExperiment();
        else if (cmd == "TEST") s.cmdTest();
        else cout << "Unknown command. Type 'help'.\n";
    }
    return 0;
}
