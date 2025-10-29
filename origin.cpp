#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <openssl/sha.h>
#include <vector>
#include <map>
#include <zlib.h>


using namespace std;
namespace fs = std::filesystem;

void repoInit() {
    if (fs::exists(".origin")) {
        cout << "repo already present.\n";
        return;
    }
    // created directories which symbolize the repo structure.
    fs::create_directory(".origin");
    fs::create_directory(".origin/objects");
    fs::create_directory(".origin/refs");
    fs::create_directory(".origin/logs");
    fs::create_directories(".origin/indexe");

    // file created to contain references.
    ofstream headFile(".origin/head");
    headFile << "ref: refs/main\n";
    headFile.close();

    // file where the latest commit hash will be stored.
    ofstream refFile(".origin/refs/main");
    refFile.close();
}

string reading(const string &filename) {
    ifstream ipFile(filename, ios::binary);
    if (!ipFile.is_open()) {
        cerr << "Error opening the file";
        return "";
    }
    stringstream s;
    s << ipFile.rdbuf();
    string fileContent = s.str();
    ipFile.close();
    return fileContent;
}

vector<unsigned char> compressData(const string &data) {
    uLong srcLen = data.size();
    uLong destLen = compressBound(srcLen);
    vector<unsigned char> out(destLen);

    if (compress(out.data(), &destLen, (const Bytef*)data.data(), srcLen) != Z_OK) {
        cerr << "Compression failed!\n";
        return {};
    }
    out.resize(destLen);
    return out;
}


string hashing(const string &fileContent) {

    

    string header = "blob " + to_string(fileContent.size()) + '\0';
    string finalContent = header + fileContent; // fixed variable shadowing
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(finalContent.c_str()),
         finalContent.size(), hash);

    // convert to hex in the simplest loop
    string hexHash;
    char buf[3];
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(buf, "%02x", hash[i]);
        hexHash += buf;
    }

    cout << "hash: " << hexHash << endl;

    string folder = ".origin/objects/" + hexHash.substr(0, 2);
    string filename = hexHash.substr(2);
    fs::create_directories(folder);

    auto compressed = compressData(finalContent);

    ofstream blob(folder + "/" + filename, ios::binary);
    blob.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    blob.close();
    return hexHash;
    
}
//This function create a file "Index" ans saves the info eg(test.cpp:c74e0cbcbdeefff97c8c3412fb2ca9e9b2507ae8).
void indexAdd(const string &filename , const string &hash){
    ofstream inFile(".origin/index" , ios:: app);
    if(!inFile.is_open()){
        cerr<<"Index file not opening";
    }
    inFile<<filename<<":"<<hash<<endl;
}


int main() {
    string hash;
    repoInit();
    string filename = "test.cpp";
    string content = reading(filename);
    if (!content.empty()) {
        hash = hashing(content);
    }
    indexAdd(filename,hash);
}
