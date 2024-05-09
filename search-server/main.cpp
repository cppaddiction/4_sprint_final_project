#include "search_server.h"

using namespace std;

void TestAddDocs() {
    SearchServer server;
    server.SetStopWords("in the"s);
    server.AddDocument(10, ""s, DocumentStatus::ACTUAL, {});
    ASSERT_EQUAL(server.GetDocumentCount(), 1);
    server.AddDocument(12, "cat in the city"s, DocumentStatus::ACTUAL, {});
    ASSERT_EQUAL(server.GetDocumentCount(), 2);
    server.AddDocument(5, "in the"s, DocumentStatus::REMOVED, {0, 1});
    ASSERT_EQUAL(server.GetDocumentCount(), 3);
}
 
void TestStopWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

void TestMinusWords1() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};    
    const int doc_idd = 43;
    const string contentt = "catt inn thee cityy"s;
    const vector<int> ratingss = {1, 2, 3, 4};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_idd, contentt, DocumentStatus::ACTUAL, ratingss);
        ASSERT_EQUAL(server.FindTopDocuments("-in thee"s).size(), 1);
    }
}

void TestMinusWords2() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    const int doc_idd = 43;
    const string contentt = "catt inn thee cityy"s;
    const vector<int> ratingss = {1, 2, 3, 4};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_idd, contentt, DocumentStatus::ACTUAL, ratingss);
        ASSERT_EQUAL(server.FindTopDocuments("-in -thee f"s).size(), 0);
    }
}

void TestMinusWords3() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.SetStopWords("in the"s);
        ASSERT_EQUAL(server.FindTopDocuments("cat in"s).size(), 1);
        ASSERT_EQUAL(server.FindTopDocuments("cat -in"s).size(), 1);
        ASSERT_EQUAL(server.FindTopDocuments("cat -city"s).size(), 0);
    }
}

void TestMatch1() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    const int doc_idd = 43;
    const string contentt = "catt inn thee cityy"s;
    const vector<int> ratingss = {1, 2, 3, 4};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_idd, contentt, DocumentStatus::ACTUAL, ratingss);
        const auto res1=server.MatchDocument("cat city", 42);
        const auto res2=server.MatchDocument("inn cityy", 43);
        DocumentStatus s1, s2;
        vector<string> v1, v2;
        tie(v1, s1)=res1;
        tie(v2, s2)=res2;
        ASSERT(v1[0]=="cat"&&v1[1]=="city");
        ASSERT(v2[1]=="inn"&&v2[0]=="cityy");
    }
}

void TestMatch2() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3}; 
    SearchServer server;
    server.SetStopWords("in the"s);
    server.AddDocument(doc_id, content, DocumentStatus::REMOVED, ratings); 
    tuple<vector<string>, DocumentStatus> search_result;
    search_result = server.MatchDocument("cat in the car"s, doc_id);
    ASSERT_EQUAL(get<0>(search_result).size(), 1);
    ASSERT_EQUAL(get<0>(search_result)[0], "cat");
    search_result = server.MatchDocument("cat in the car -city"s, doc_id);
    ASSERT(get<0>(search_result).empty());
}

void TestRel1() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    const int doc_idd = 43;
    const string contentt = "catt inn thee cityy"s;
    const vector<int> ratingss = {1, 2, 3, 4};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_idd, contentt, DocumentStatus::ACTUAL, ratingss);
        const auto res=server.FindTopDocuments("in thee catt"s);
        ASSERT(res[0].relevance>res[1].relevance);
    }
}

void TestRel2() {
    SearchServer server;
    server.SetStopWords("in the"s);
    ASSERT(server.FindTopDocuments("cat in the car"s).empty());
    server.AddDocument(0, "black cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(1, "black dog in the car"s, DocumentStatus::ACTUAL, {4, 5, 6});
    server.AddDocument(2, "gray elephant in the car"s, DocumentStatus::ACTUAL, {3, 5, 3});
    ASSERT(server.FindTopDocuments("parrot on the shoulder"s).empty()); 
    vector<Document> result = server.FindTopDocuments("gray cat in the car"s);
    bool rel_sort_is_correct = true;
    for (size_t i = 0; i < result.size() - 1; ++i) {
        if ((abs(result[i].relevance - result[i + 1].relevance) > 10e-6)
            && (result[i].relevance < result[i + 1].relevance)) {
                rel_sort_is_correct = false;
                break;
            }
    }
    ASSERT(rel_sort_is_correct); 
}

void TestRel3() {
    SearchServer server;
    server.AddDocument(0, "cat"s, DocumentStatus::ACTUAL, {});
    ASSERT_EQUAL(server.FindTopDocuments("cat"s).size(), 1);
    ASSERT_EQUAL(server.FindTopDocuments("cat"s)[0].relevance, 0.0); 
    server.AddDocument(1, "gray dog"s, DocumentStatus::ACTUAL, {});    
    ASSERT_EQUAL(server.FindTopDocuments("dog"s).size(), 1);
    double exp_relevance = log (2. / 1) * (1. / 2);
    ASSERT_EQUAL(server.FindTopDocuments("dog"s)[0].relevance, exp_relevance);
}

void TestRate1() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    const int doc_idd = 43;
    const string contentt = "catt inn thee cityy"s;
    const vector<int> ratingss = {1, 2, 3, 6};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_idd, contentt, DocumentStatus::ACTUAL, ratingss);
        const auto res=server.FindTopDocuments("in thee catt"s);
        ASSERT_EQUAL(res[0].rating, 3);
        ASSERT_EQUAL(res[1].rating, 2);
    }
}

void TestRate2() {
    SearchServer server;
    server.AddDocument(0, "cat"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(1, "dog"s, DocumentStatus::ACTUAL, {2, 5});
    server.AddDocument(2, "parrot", DocumentStatus::ACTUAL, {2, -9, 2});
    map<int, int> exp_res = {{0, 0}, {1, 3}, {2, -1}};
    bool rating_is_correct = true;
    for (const auto& [id, _, rating] : server.FindTopDocuments("cat dog parrot")) {
        if (rating != exp_res[id]) {
            rating_is_correct = false;
        }
    }    
    ASSERT(rating_is_correct);
}

void TestPredicate1() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    const int doc_idd = 43;
    const string contentt = "catt inn thee cityy"s;
    const vector<int> ratingss = {1, 2, 3, 6};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_idd, contentt, DocumentStatus::IRRELEVANT, ratingss);
        const auto res=server.FindTopDocuments("in thee catt"s, [](int document_id, DocumentStatus status, int rating) {return status == DocumentStatus::IRRELEVANT;});
        ASSERT(res.size()==1&&res[0].rating==3);
    }
}

void TestPredicate2() {
    SearchServer server;
    server.AddDocument(0, "cat"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(1, "dog"s, DocumentStatus::ACTUAL, {2, 5});
    server.AddDocument(2, "parrot", DocumentStatus::ACTUAL, {2, -9, 2});
    vector<Document> search_result = server.FindTopDocuments("cat dog parrot"s, [](int document_id, DocumentStatus doc_status, int rating) {return rating < 0;});
    ASSERT_EQUAL(search_result.size(), 1);
    ASSERT_EQUAL(search_result[0].id, 2);  
    search_result = server.FindTopDocuments("cat dog parrot"s, [](int document_id, DocumentStatus doc_status, int rating) {return document_id == 0;});
    ASSERT_EQUAL(search_result.size(), 1);
    ASSERT_EQUAL(search_result[0].id, 0);
    search_result = server.FindTopDocuments("cat dog parrot"s, [](int document_id, DocumentStatus doc_status, int rating) {return rating == 10;});
    ASSERT(search_result.empty());    
}

void TestDocsStatus() {
    SearchServer server;
    server.AddDocument(0, "cat"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(1, "dog"s, DocumentStatus::BANNED, {});
    server.AddDocument(2, "parrot", DocumentStatus::IRRELEVANT, {});
    vector<Document> search_result = server.FindTopDocuments("cat dog parrot"s, DocumentStatus::REMOVED);    
    ASSERT(search_result.empty()); 
    search_result = server.FindTopDocuments("cat dog parrot"s, DocumentStatus::IRRELEVANT);    
    ASSERT_EQUAL(search_result.size(), 1);
    ASSERT_EQUAL(search_result[0].id, 2);
}

void TestSearchServer() {
    RUN_TEST(TestAddDocs);
    RUN_TEST(TestStopWords);
    RUN_TEST(TestMinusWords1);
    RUN_TEST(TestMinusWords2);
    RUN_TEST(TestMinusWords3);
    RUN_TEST(TestMatch1);
    RUN_TEST(TestMatch2);
    RUN_TEST(TestRel1);
    RUN_TEST(TestRel2);
    RUN_TEST(TestRel3);
    RUN_TEST(TestRate1);
    RUN_TEST(TestRate2);
    RUN_TEST(TestPredicate1);
    RUN_TEST(TestPredicate2);
    RUN_TEST(TestDocsStatus);
}

int main() {
    TestSearchServer();
    cout << "Search server testing finished"s << endl;
}