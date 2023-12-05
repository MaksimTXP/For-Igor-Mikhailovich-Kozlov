#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

//Считываение строки
string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

//Считываение строки с номером
int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

// Разделяет строку на слова и возращет вектор слов
vector<string> SplitIntoWords(const string& text)  {

    vector<string> words;
    string word;

    for (const char& c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

struct Document {
    
    int id;
    double relevance;
};

class SearchServer {
public:

    //Конструктор ввектора стоп-слов
    void SetStopWords(const string& text) { // Принемает строку стоп-слов
  
        for (const string& word : SplitIntoWords(text)) {   
            stop_words_.insert(word);
        }
    }

    //Конструктор документа (строка, id, TF)
    void AddDocument (const int& document_id, const string& document) { // Принемает id и строку 
      
        ++document_count_;//Считает количество документов
        const vector <string> words_docoment = SplitIntoWordsNoStop(document);
        for (auto& word : words_docoment) {
            word_to_documents_freds[word][document_id] += (1./ words_docoment.size()); // Добавление tf по id
        }
        
    }

    //Возвращает топ 5 результатов запроса
    vector<Document> FindTopDocuments(const string& raw_query) const { // Принемает не обработанный запрос

        const Query query_words = ParseQuery(raw_query); // Конструктор плюс-минус слов
        vector<Document> matched_documents = FindAllDocuments(query_words);// Нахождение id и TF-IDF

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    //Плюс слова (слова без знака '-') стоп слова (слова со знаком '-')
    struct Query {

        set<string> plus_word;
        set<string> minus_word;
    };

    //Слово и TF
    map <string, map<int, double>> word_to_documents_freds;

    //Стоп-слова
    set<string> stop_words_;

    //Количество документов
    int document_count_ = 0;

    //Проверка на вхожедение стоп-слов
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    //Разбиение строки на слова без стоп-слов
    vector<string> SplitIntoWordsNoStop(const string& text) const {

        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) { //Проверка на стоп-слово
                words.push_back(word);
            }
        }
        return words;
    }

    //Разбивает строку запросов на слова без стоп-слов и создает плюс и минус слова 
    Query ParseQuery(const string& text) const {// Получает необработанный запрос

        Query query_words;

        for (string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                word = word.substr(1);
                if (!IsStopWord(word)) {//Проверка на стоп-слово
                    query_words.minus_word.insert(word);// Добавлене минус слова
                }
            }
            query_words.plus_word.insert(word);// Добавление плюс слова
        }
        return query_words;
    }

    //Возвращает id и TF-IDF
    vector<Document> FindAllDocuments(const Query& query_words) const { // Получает запрос из плюс и минус слов

        map<int, double> document_to_tf_idf;
        vector <Document> matched_documents;

       for (const auto& word : query_words.plus_word) {
            if (word_to_documents_freds.count(word) == 0) {
                continue;
            }
            for (const auto& [id, tf] : word_to_documents_freds.at(word)) {
               document_to_tf_idf [id] +=tf*(log(static_cast <double>(document_count_)/
                   word_to_documents_freds.at(word).size()));// Подсчет и добавление TF-IDF
            }

            for (const auto& word : query_words.minus_word) {
                if (word_to_documents_freds.count(word) == 0) {
                    continue;
                }
                for (const auto& [id, tf] : word_to_documents_freds.at(word)) {
                    document_to_tf_idf.erase(id);// Удаление по id минус слов
                }
            }
        }

       for (const auto& [id, tf_idf] : document_to_tf_idf) {
           matched_documents.push_back({ id,tf_idf });
       }
           
            return matched_documents;

    }
};

//Конструктор поискового сервера
SearchServer CreateSearchServer() {

    SearchServer search_server;
    search_server.SetStopWords(ReadLine());//Конструктор стоп-слов
    const int document_count = ReadLineWithNumber(); //Ввод количества документов
    //ВВод документов
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();//Ввод запроса
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}