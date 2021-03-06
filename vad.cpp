/* 
 * File:   vad.cpp
 * Author: rain
 * 
 * Created on 30 марта 2014 г., 14:13
 */

#include "vad.h"

VAD::VAD() {
    mMFCC = nullptr;
    mSVM = nullptr;
}

//VAD::VAD(const VAD& orig) {
//}

VAD::~VAD() {
    if (mMFCC != nullptr)
        delete mMFCC;
    
    if (mSVM != nullptr)
        delete mSVM;
}

std::vector< std::string > VAD::listDir(std::string path) {
    std::vector< std::string > files;
    struct dirent *ent;
    DIR *dir;
    if ((dir = opendir (path.data())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type != DT_DIR) {
                std::string filename = path + ent->d_name;
                files.push_back(filename);
            }
        }
        closedir (dir);
    }
    
    return files;
}

void VAD::initializeMFCC(int interval, int filtersCount, int mfccCount) {
    if (mMFCC != nullptr)
        delete mMFCC;
    
    mMFCC = new MFCC(interval, filtersCount, mfccCount);
//    mMFCC->load("tests/fixtures/mike.wav");
//    mMFCC->trifBank();
//    mMFCC->extract();
}
void VAD::initializeSVM(int num_features) {
    if (mSVM != nullptr)
        delete mSVM;
    
    mSVM = new SVM(num_features);
}