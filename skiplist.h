/* ************************************************************************
> File Name:     skipList.h
> Author:        JasonLaw
> Created Time:  2022-01-02
> Description:   
 ************************************************************************/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;    // 互斥锁
std::string delimiter = ":";

// 类模板实现跳表节点node
template<typename K, typename V>
class Node {

public:

    Node()  {}

    Node(K k, V v, int level) : key_(k), value_(v), node_level_(level) {
        // level + 1 数组索引从0 -> level
        forward_ = new Node<K, V>*[level + 1];
        
        // 初始化
        memset(forward_, 0, sizeof(Node<K, V>*) * (level + 1));
    }

    ~Node() {
        delete[] forward_;
    }

    K GetKey() const;

    V GetValue() const;

    void SetValue(V);

    Node<K, V> **forward_;

    int node_level_;

private:

    K key_;
    V value_;
};

template<typename K, typename V>
K Node<K, V>::GetKey() const {
    return key_;
};

template<typename K, typename V>
V Node<K, V>::GetValue() const {
    return value_;
};

template<typename K, typename V>
void Node<K, V>::SetValue(V value) {
    this->value_ = value;
};

// 跳表类
template<typename K, typename V>
class SkipList {

public:

    SkipList(int level) : max_level_(level), skip_list_level_(0), element_count_(0) {
        K key;
        V value;
        header_ = new Node<K, V>(key, value, max_level_);
    }
    ~SkipList() {
        if (file_writer_.is_open()) {
            file_writer_.close();
        }
        if (file_reader_.is_open()) {
            file_reader_.close();
        }
        delete header_;
    }

    int GetRandomLevel();
    Node<K, V>* CreateNode(K, V, int);
    int InsertElement(K, V);
    void DisplayList();
    bool SearchElement(K);
    void DeleteElement(K);
    void DumpFile();
    void LoadFile();
    size_t size();

private:
    void GetKeyValueFromString(const std::string& str, std::string* key, std::string* value);
    bool IsValidString(const std::string& str);

private:
    // 跳表最大高度
    int max_level_;
    
    // 跳表目前高度
    int skip_list_level_;

    // 头结点指针
    Node<K, V>* header_;

    // 文件操作符
    std::ofstream file_writer_;
    std::ifstream file_reader_;

    // 结点数量
    int element_count_;

};

// 创建节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::CreateNode(const K k, const V v, int level) {
    return new Node<K, V>(k, v, level);
};

// 将给定的键值对插入跳表
// 返回 0: 插入成功
// 返回 1: 插入失败，元素已存在
template<typename K, typename V>
int SkipList<K, V>::InsertElement(const K key, const V value) {
    
    mtx.lock();
    Node<K, V>* current = this->header_;

    // 
    Node<K, V>* update[max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));

    // 从跳表顶层开始遍历
    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward_[i] != nullptr && current->forward_[i]->GetKey() < key) {
            current = current->forward_[i];
        }
        update[i] = current;
    }

    // current指向实际插入位置
    current = current->forward_[0];

    // 判断current结点的键是否与目标结点键相同
    if (current != nullptr && current->GetKey() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // current为nullptr则已达到当前层的末端
    // urrent的key不等于目标节点的key，则需要在update[0]和current之间插入新节点
    if (current == nullptr || current->GetKey() != key) {
        // 生成新结点的索引层数
        int random_level = GetRandomLevel();

        // 新结点的索引层数大于跳表的当前层数，则更新跳表头结点
        if (random_level > skip_list_level_) {
            for (int i = skip_list_level_ + 1; i < random_level + 1; i++) {
                update[i] = header_;
            }
            skip_list_level_ = random_level;
        }

        // 创建新结点
        Node<K, V>* inserted_node = CreateNode(key, value, random_level);

        // 插入结点
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward_[i] = update[i]->forward_[i];
            update[i]->forward_[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        element_count_++;
    }
    mtx.unlock();
    return 0;
}

// 展示跳表
template<typename K, typename V>
void SkipList<K, V>::DisplayList() {
    std::cout << "\n*****Skip List*****"<<"\n"; 
    
    for (int i = 0; i <= skip_list_level_; i++) {
        Node<K, V> *node = this->header_->forward_[i]; 
        
        std::cout << "Level " << i << ": ";
        
        while (node != nullptr) {
            std::cout << node->GetKey() << ":" << node->GetValue() << ";";
            node = node->forward_[i];
        }
        
        std::cout << std::endl;
    }
};

// 存储数据到文件
template<typename K, typename V>
void SkipList<K, V>::DumpFile() {
    std::cout << "dump_file-----------------" << std::endl;

    file_writer_.open(STORE_FILE);
    Node<K, V>* node = this->header_->forward_[0];

    while (node != nullptr) {
        file_writer_ << node->GetKey() << ":" << node->GetValue() << "\n";
        std::cout << node->GetKey() << ":" << node->GetValue() << "\n";
        node = node->forward_[0];
    }

    file_writer_.flush();
    file_writer_.close();

    return;
};

// 从本地文件加载数据
template<typename K, typename V>
void SkipList<K, V>::LoadFile() {
    file_reader_.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();

    while (getline(file_reader_, line)) {
        GetKeyValueFromString(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        InsertElement(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    file_reader_.close();
};

// 跳表结点个数
template<typename K, typename V> 
size_t SkipList<K, V>::size() { 
    return element_count_;
};

// 
template<typename K, typename V>
void SkipList<K, V>::GetKeyValueFromString(const std::string& str, std::string* key, std::string* value) {
    if(!IsValidString(str)) {
        return;
    }

    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());

    return;
};

// 
template<typename K, typename V>
bool SkipList<K, V>::IsValidString(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
};

// 删除跳表中的元素
template<typename K, typename V> 
void SkipList<K, V>::DeleteElement(K key) {
    mtx.lock();
    Node<K, V>* current = this->header_;
    Node<K, V>* update[max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));

    // 
    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward_[i] != nullptr && current->forward_[i]->GetKey() < key) {
            current = current->forward_[i];
        }
        update[i] = current;
    }

    current = current->forward_[0];
    if (current != nullptr && current->GetKey() == key) {
        // 
        for (int i = 0; i <= skip_list_level_; i++) {
            // 
            if (update[i]->forward_[i] != current) {
                break;
            }

            update[i]->forward_[i] = current->forward_[i];
        }

        // 移除没有元素的层
        while (skip_list_level_ > 0 && header_->forward_[skip_list_level_] == 0) {
            skip_list_level_--;
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        element_count_--;
    }

    mtx.unlock();
    return;
};

// 搜索结点
template<typename K, typename V> 
bool SkipList<K, V>::SearchElement(K key) {
    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = this->header_;

    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward_[i] && current->forward_[i]->GetKey() < key) {
            current = current->forward_[i];
        }
    }

    current = current->forward_[0];

    if (current != nullptr && current->GetKey() == key) {
        std::cout << "Found key: " << key << ", value: " << current->GetValue() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
};

// 生成结点的层数
template<typename K, typename V>
int SkipList<K, V>::GetRandomLevel() {
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < max_level_) ? k : max_level_;
    return k;
};