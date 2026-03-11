#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

void replaceStringWithVector(vector<string>& mainVec, const string& toFind,string& replacementVec)                    
{
    auto it = find(mainVec.begin(), mainVec.end(), toFind);
    if (it != mainVec.end()) 
    {
        it = mainVec.erase(it);
        mainVec.insert(it, replacementVec);
    }
}

struct Production 
{
    string lhs;
    vector<string> rhs; // Each rule is a sequence of symbols (terminals or non-terminals)
};

class CFG {
private:
    map<string, Production> rules;
    string startSymbol;

public:
    CFG(const string& start) : startSymbol(start) {}

    void addRule(const string& lhs, const vector<string>& alternatives) 
    {
        rules[lhs] = { lhs, alternatives };
    }

    string generateString(int maxDepth = 5) 
    {
        map<string,Production> checked;
        vector<string> symbols = {startSymbol}; 
        return derive(symbols, checked, maxDepth);
    }

private:
    string derive(vector<string>& symbols, map<string,Production>& checked, int depth)
    {
        //srand(time(NULL));
        if (depth < 0)
        {   
            for (string& sym : symbols)
            {
                if (isNonTerminal(sym)) return "E"; // failed derivation
            }
            string result;
            for (string& sym : symbols) result += sym;
            return result; // case: To be Cecked;
        }

        int flag = 1;
        vector<string> toDerive;
        string result;
        for(string& sys: symbols)
        {
            if(isNonTerminal(sys)) 
            { 
                flag = 0;
                depth--;
                toDerive.push_back(sys);
            }
        }

        if(flag == 1) // case where no non-terminals in symbols
        {
            for(string& sys: symbols)
            {
                result += sys;
            }
            return result;
        }

        // Try to build the result symbol by symbol
        string test;
        
        for(string& sym : toDerive)
        {
            sos:
            test = rules[sym].rhs[rand() % rules[sym].rhs.size()];
            if(test == "" && depth > 0)
            {
                goto sos;
            }
        
            //check if test is present in checked.
            auto it = find(checked[sym].rhs.begin(), checked[sym].rhs.end(), test);
            if (it != checked[sym].rhs.end()) 
            {
                // test(Production rule is in checked)
                goto sos;
            }

            vector<string> NewSymobls;
            for(int i=0;i<test.length();i++)
            {
                NewSymobls.push_back(string(1,test[i]));
            }
            string str = derive(NewSymobls, checked, depth);
            if(str == "E")
            {
                checked[sym].lhs = sym;
                checked[sym].rhs.push_back(test);
                goto sos;
            }
            else
            {
                //cout << "it is: " << str << endl; 
                replaceStringWithVector(symbols, sym, str);
                continue;
            }
        }

        for (string& sym : symbols) result += sym;
        return result;
    }

    bool isNonTerminal(const string& sym) 
    {
        return rules.find(sym) != rules.end();
    }

    friend ostream& operator<<(ostream& os, const CFG& p);
    friend void writeGrammarArrayToFile(const string& filename);
    friend void readGrammarArrayFromFile(const string& filename);
};

ostream& operator<<(ostream& os, const CFG& p) 
{
    for(const auto& [symbol, prod_rule] : p.rules)
    {
        cout << "Non-Terminal : " << symbol << endl;
        for(int i=0;i<prod_rule.rhs.size(); i++)
        {
            // string str = "";
            // for(int j=0;j<prod_rule.rhs[i].size();j++)
            // {
            //     str += prod_rule.rhs[i][j];
            // }
            cout << "Rule: " << prod_rule.rhs[i] << endl;
        }
    }
    return os;
}

vector<CFG> cfg_arr;

void writeGrammarArrayToFile(const string& filename) 
{
    ofstream outFile(filename);

    for (const auto& grammar : cfg_arr) {
        outFile << "START " << grammar.startSymbol << "\n";
        outFile << "RULES " << grammar.rules.size() << "\n";
        for (const auto& [key, prod] : grammar.rules) {
            outFile << key << " -> " << prod.lhs;
            for (const auto& sym : prod.rhs) {
                if(sym == "")
                {
                    outFile << " empty";
                }
                else
                {
                    outFile << " " << sym;
                }   
            }
            outFile << "\n";
        }
        outFile << "END\n";
    }

    outFile.close();
}

void readGrammarArrayFromFile(const string& filename) 
{
    cfg_arr.clear();

    ifstream inFile(filename);
    string line;

    while (getline(inFile, line)) {
        if (line.substr(0, 6) == "START ") {
            CFG g("S");
            g.startSymbol = line.substr(6);

            getline(inFile, line); // RULES n
            int ruleCount = stoi(line.substr(6));

            for (int i = 0; i < ruleCount; ++i) {
                getline(inFile, line);
                size_t arrowPos = line.find("->");
                string key = line.substr(0, arrowPos - 1);
                string rest = line.substr(arrowPos + 3);

                istringstream ss(rest);
                Production p;
                ss >> p.lhs;

                string sym;
                while (ss >> sym) {
                    if (sym == "empty") {
                        p.rhs.push_back(""); // Interpret "empty" as ""
                    } else {
                        p.rhs.push_back(sym);
                    }
                }

                g.rules[key] = p;
            }

            getline(inFile, line); // END
            cfg_arr.push_back(g);
        }
    }

    inFile.close();
    return;
}

void Add_CFGs()
{
    string init;
    int counter = 0;
    vector<string> prod_rule;
    CFG cfg("S");

    cout << "Add Rules: " << endl;
    while(1)
    {
        prod_rule.clear();
        cout << "Enter Non-Terminal : ";
        cin >> init;
        if(init == "stop")
        {
            break;
        }
        while(1)
        {
            string str;
            cout << "Product rule counter(empty for \"\") " << counter +1 << " : ";
            cin >> str;
            if(str == "next")
            {
                cfg.addRule(init, prod_rule);
                break;
            }
            if(str == "empty")
            {
                str = "";
            }
            prod_rule.push_back(str);
            counter++;
        }
    }

    cfg_arr.push_back(cfg);

    writeGrammarArrayToFile("cfgs.txt");
   
    return;
}

void View_CFGs()
{
    readGrammarArrayFromFile("cfgs.txt");
    cout << endl;
    for(int i=0;i<cfg_arr.size();i++)
    {
        cout << i+1 << ". " << cfg_arr[i] << endl;
    }

    int opt;
    cout << "Enter Index of CFG to delete(0 to exit) : ";
    cin >> opt;

    if(opt == 0)
    {
        return;
    }
    else if(opt <= cfg_arr.size())
    {
        cfg_arr.erase(cfg_arr.begin() + opt-1);
        writeGrammarArrayToFile("cfgs.txt");
    }
    else
    {
        cout << "Invalid Option. " << endl;
        return;
    }
    
}

void Admin_fun()
{
    int opt;

    sos:
    cout << "1. Add CFGs" << endl;
    cout << "2. View/Remove CFGs" << endl;
    cout << "3. Exit" << endl;
    cout << "Choose : ";

    cin >> opt;

    switch (opt)
    {
    case 1:
        Add_CFGs();
        goto sos;
        break;
    case 2:
        View_CFGs();
        goto sos;
        break;
    case 3:
        return;
        break;
    default:
        cout << "Try Again." << endl;
        goto sos;
        break;
    }
}

void type1()
{
    srand(time(NULL));
    int it = rand()%cfg_arr.size();
    
    cout << "Select the invalid string from the followiing CFG :\n" << cfg_arr[it] << endl;
    string options[4];
    for(int i=0;i<3;i++)
    {
        options[i] = cfg_arr[it].generateString(6);
    }

    try_again:
    int it2 = rand()%cfg_arr.size();
    if(it2 == it)
    {
        goto try_again;
    }

    options[4] = cfg_arr[it2].generateString(6);
    string ans = options[4];

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(options, options + 4, default_random_engine(seed));

    for(int i=0;i<4;i++)
    {
        cout << i+1 << ". " << options[i] << endl;
    }
    int opt;
    cout << "Choose: " ;
    cin >> opt; 

    if(options[opt-1] == ans)
    {
        cout << "Correct Answer!!! " << endl;
    }
    else
    {
        cout << "Better Luck Next Time!!!" << endl;
    }
    return;
}

void type2()
{

}

void type3()
{

}

void Quiz()
{
    for(int i=0; i<10;i++)
    {
        int it = rand()%3;
        switch(it)
        {
            case 1:
                type1(); // guess valid string
                break;
            case 2:
                type1(); // guess invalid string
                break;
            case 3:
                type1(); // guess derivation
                break;
        }
        continue;
    }
}

int main() 
{
    srand(time(0));
    int opt;

    readGrammarArrayFromFile("cfgs.txt");

    sos:
    cout << "1. Quiz" << endl;
    cout << "2. Admin" << endl;
    cout << "3. Exit" << endl;
    cout << "Choose : " ;

    cin >> opt;

    switch (opt)
    {
    case 1:
        Quiz();
        break;
    case 2:
        Admin_fun();
        goto sos;
        break;
    case 3:
        exit(0);
        break;
    default:
        cout << "Try Again." << endl;
        goto sos;
        break;
    }

    return 0;
}