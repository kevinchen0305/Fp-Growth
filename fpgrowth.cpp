#include<iostream>
#include<sstream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<map>
#include<unordered_map>
#include<time.h>
//#define TIME 1
#define CONDITIONAL_TREE 1
using namespace std;

struct Fptree{
    int item;
    int count;
    Fptree* parent;
    Fptree* sibling;
    vector<Fptree*> children;
    Fptree(int x, int y) : item(x), count(y) {}
    Fptree() {}
};

class fp_growth{
    public:
        fp_growth(int item_num, int min_sup, int trans_num) :n(item_num), minsup(min_sup), transaction(trans_num)
        {

        }

        void FPGrowth(const string &filename){
            vector<int> sorted_one_itemset = generate_frequent_one_itemset(filename);

            Fptree* Root = new Fptree();
            Root->count = -1;
            Root->item = -1;
            Root->parent = NULL;
            Root->sibling = NULL;

            construct_FPtree(Root, filename, sorted_one_itemset);
            
            #ifdef CONDITIONAL_TREE
                cout << "---------------------------" << endl;
                cout << "Header Table" << endl;
                for(auto it:HeaderTable){
                    cout << it.first << " ";
                    Fptree* tmp = it.second;
                    while(tmp != NULL){
                        cout << tmp->item << ":" << tmp->count << " ";
                        tmp = tmp->sibling;
                    }
                    cout << endl;
                }
                cout << "---------------------------" << endl;
            #endif

            conditional_tree(Root);
        }

        void construct_FPtree(Fptree* &Root, const string &filename, vector<int> &sorted_vector){
            auto compare = [&](int a, int b) {
                return find(sorted_vector.begin(), sorted_vector.end(), a) < find(sorted_vector.begin(), sorted_vector.end(), b);
            };

            ifstream myfile;
            string line;
            myfile.open(filename);
            if(myfile.is_open()){
                while(getline(myfile, line)){
                    istringstream ss(line);
                    int num;
                    vector<int> tmp;
                    while(ss >> num){
                        auto findelement = find(sorted_vector.begin(), sorted_vector.end(), num);
                        if(findelement != sorted_vector.end()){
                            tmp.push_back(num);
                        }
                    }

                    sort(tmp.begin(), tmp.end(), compare);// sorted each transaction by 1-itemset

                    //for(int i=0; i<tmp.size(); i++){
                    //    cout << tmp[i] << " ";
                    //}
                    //cout << endl;

                    Fptree* Head = Root;
                    for(int i=0; i<tmp.size(); i++){
                        bool is_exist = 0;
                        for(int k=0; k<Head->children.size(); k++){
                            if(Head->children[k]->item == tmp[i]){
                                is_exist = 1;
                                Head->children[k]->count += 1;
                                Head = Head->children[k];
                                break;
                            }
                        }
                        if(is_exist == 0){
                            Fptree* Node = new Fptree();
                            Node->item = tmp[i];
                            Node->count = 1;
                            Node->parent = Head;
                            Node->sibling = NULL;
                            Head->children.push_back(Node);
                            add_headertable(Node);
                            Head = Head->children.back();
                        }
                    }
                }
            }
        }

        void add_headertable(Fptree* &Node){
            auto it = HeaderTable.find(Node->item);
            if(it != HeaderTable.end()){ // find!!!
                Fptree* HeaderHead = it->second;
                while(HeaderHead->sibling != NULL){
                    HeaderHead = HeaderHead->sibling;
                }
                HeaderHead->sibling = Node;
            }
            else{
                HeaderTable[Node->item] = Node;
            }
        }

        void conditional_tree(Fptree* &Root){
            for(auto it:HeaderTable){
                unordered_map<Fptree*, int> umap;
                Fptree* tmp = it.second;
                while(tmp != NULL){
                    Fptree* ToParent = tmp;
                    while(ToParent != Root){
                        umap[ToParent] += tmp->count;
                        ToParent = ToParent->parent;
                    }
                    tmp = tmp->sibling;
                }
                
                #ifdef CONDITIONAL_TREE
                    cout << "This Round Item of Header Table: " << it.first << endl;
                    cout << "NodeItem NodeCount CountAfterConditionalPattern" << endl;
                #endif
                for(auto itt:umap){
                    #ifdef CONDITIONAL_TREE
                        cout << itt.first->item << " " << itt.first->count << " " << itt.second << endl;
                    #endif
                    if(itt.first->item == it.first){
                        Fptree* Toparent = itt.first->parent;
                        vector<int> fre_vec;
                        fre_vec.push_back(it.first);
                        while(Toparent != Root){
                            if(umap[Toparent] >= minsup){
                                fre_vec.push_back(Toparent->item);
                            }
                            Toparent = Toparent->parent;
                        }

                        #ifdef CONDITIONAL_TREE
                            cout << "This Node is Header Table Node, it's Conditional Tree: ";
                            for(int i=0; i<fre_vec.size(); i++){
                                cout << fre_vec[i] << " ";
                            }
                            cout << endl;
                        #endif

                        sort(fre_vec.begin(), fre_vec.end());
                        maximal_frequent_itemset.push_back(fre_vec);
                        
                
                        sort(maximal_frequent_itemset.begin(), maximal_frequent_itemset.end(), sort_by_length);
                        vector<vector<int>>::iterator fre_ptr = maximal_frequent_itemset.begin();
                        for(int i=0; i<maximal_frequent_itemset.size(); i++){
                            for(int j=maximal_frequent_itemset.size()-1; j>i; j--){
                                if(includes(maximal_frequent_itemset[j].begin(), maximal_frequent_itemset[j].end(), maximal_frequent_itemset[i].begin(), maximal_frequent_itemset[i].end())){
                                    maximal_frequent_itemset.erase(fre_ptr+i);
                                    i--;
                                    break;
                                }
                            }
                        }
                        
                    }
                }
                #ifdef CONDITIONAL_TREE
                    cout << "--------------------------------------" << endl;
                #endif
            }
        }

        vector<int> generate_frequent_one_itemset(const string &filename){
            map<int, int> mp;
            ifstream myfile;
            string line;
            myfile.open(filename);
            if(myfile.is_open()){
                while(getline(myfile, line)){
                    istringstream ss(line);
                    int num;
                    while(ss >> num){
                        mp[num]++;
                    }
                }
            }
            
            vector<int> sorted_key;

            for(auto it=mp.begin(); it!=mp.end(); ){
                if(it->second < minsup){
                    it = mp.erase(it);
                }
                else{
                    sorted_key.push_back(it->first);
                    it++;
                }
            }

            sort(sorted_key.begin(), sorted_key.end(), [&](int a, int b){
                return mp[a] > mp[b];
            });

            return sorted_key;
        }

        static bool sort_by_length(vector<int> &vec1, vector<int> &vec2){
            return vec1.size() < vec2.size();
        }

        void print_maximal_frequent_itemset(){
            for(int i=0; i<maximal_frequent_itemset.size(); i++){
                for(int j=0; j<maximal_frequent_itemset[i].size(); j++){
                    cout << maximal_frequent_itemset[i][j] << " ";
                }
                cout << endl;
            }
        }


    private:
        int n;
        int minsup;
        int transaction;
        unordered_map<int, Fptree*> HeaderTable;
        vector<vector<int>> maximal_frequent_itemset;

};

int main(int argc, char *argv[])
{
    //you must input -> file.txt item_number transaction_number
    if(argc != 4){
        cout << "error input" << endl;
        cout << "you must input -> file.txt item_number transaction_number";
        exit(0);
    }
    
    int num_item = atoi(argv[2]);
    int num_transaction = atoi(argv[3]);
    int minsup;
    cout << "input minimum support count: ";
    cin >> minsup;

    fp_growth f(num_item, minsup, num_transaction);

    #ifdef TIME
        timespec beg_t, end_t;
        double MFI_duration;
    #endif 

    #ifdef TIME
        clock_gettime(CLOCK_MONOTONIC, &beg_t);
    #endif
    f.FPGrowth(argv[1]);
    #ifdef TIME
        clock_gettime(CLOCK_MONOTONIC, &end_t);
        MFI_duration = (end_t.tv_sec - beg_t.tv_sec) * 1000.0 + (end_t.tv_nsec - beg_t.tv_nsec) / 1000000.0;
    #endif
    
    cout << "Maximal Frequent Itemsets: " << endl;
    f.print_maximal_frequent_itemset();

    #ifdef TIME
        printf("%.3lf ms\n", MFI_duration);
    #endif

    return 0;
}
/*
cout << "Root: " << Root->item  << " " << Root->count << endl;
cout << Root->children[0]->item << ":" << Root->children[0]->count << endl;
cout << Root->children[1]->item << ":" << Root->children[1]->count << endl;
//cout << Root->children[2]->item << ":" << Root->children[2]->count << endl;
cout << Root->children[0]->children[0]->item << ":" << Root->children[0]->children[0]->count << endl;
//cout << Root->children[0]->children[1]->item << ":" << Root->children[0]->children[1]->count << endl;
cout << Root->children[0]->children[0]->children[0]->item << ":" << Root->children[0]->children[0]->children[0]->count << endl;
cout << Root->children[0]->children[0]->children[1]->item << ":" << Root->children[0]->children[0]->children[1]->count << endl;
cout << Root->children[0]->children[0]->children[2]->item << ":" << Root->children[0]->children[0]->children[2]->count << endl;

cout << Root->children[0]->children[0]->children[0]->children[0]->item << ":" << Root->children[0]->children[0]->children[0]->children[0]->count << endl;
cout << Root->children[0]->children[0]->children[0]->children[1]->item << ":" << Root->children[0]->children[0]->children[0]->children[1]->count << endl;

cout << Root->children[0]->children[0]->children[0]->children[0]->children[0]->item << ":" << Root->children[0]->children[0]->children[0]->children[0]->children[0]->count << endl;
cout << Root->children[0]->children[0]->children[0]->children[1]->children[0]->item << ":" << Root->children[0]->children[0]->children[0]->children[1]->children[0]->count << endl;
*/