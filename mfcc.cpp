#include "mfcc.h"

//public:
MFCC::MFCC(int duration, int filtersCount, int mfccCount) {
    initialize(duration, filtersCount, mfccCount);
}

MFCC::~MFCC() {
    if (mStream != nullptr)
        delete mStream;
}

void MFCC::load(std::string filename)
{
    if (mStream != nullptr)
        delete mStream;
    
    mStream = new Signal();
    mStream->loadFromFile(filename);
    mFftSize = mStream->getSampleRate() * mDuration / 1000.0;

    std::cout << "file info:" << std::endl;
    std::cout << "file name: " << filename << std::endl;
    std::cout << "channels: " << mStream->getChannelCount() << std::endl;
    std::cout << "sample rate: " << mStream->getSampleRate() << std::endl;
    std::cout << "sample count: " << mStream->getSampleCount()  << std::endl;
}

vector2d MFCC::extract()
{
    int i, k;
    auto P = dft();
    auto H = trifBank();
    vector2d ret, X;
    
    for(auto p_n : P) {
        std::vector<double> vec;
        for(auto h_i : H) {
            double sum = 0;
            for(k = 0; k < p_n.size(); k++) {
                sum += p_n[k] * h_i[k];
            }
            vec.push_back(log(sum));
        }
        X.push_back(vec);
    }
    
    return X;
//    return ret;
}

//protected:
double MFCC::hzToMel(double hz)
{
    return (1127.0 * log( 1.0 + hz / 700.0 ));
}

double MFCC::melToHz(double mel)
{
    return (700.0 * (exp( mel / 1127.0) - 1.0 ));
}

vector2d MFCC::dft() {
    fftw_complex *in, *out;
    fftw_plan plan;
    int i, j;
    double* val = new double;
    vector2d ret;
    
    in =  Malloc(fftw_complex, mFftSize);
    out = Malloc(fftw_complex, mFftSize);
    plan = fftw_plan_dft_1d(mFftSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    
    mStream->rewind();
    i = 0;
    while( mStream->readNext(val) != 0 ) {
        in[i][0] = *val;
        in[i][1] = 0;
        i++;
        if (i == mFftSize) {
            i = 0;
            fftw_execute(plan);
            std::vector< double > line;
            
            for(j = 0; j < mFftSize/2; j++) {
                line.push_back( std::abs(std::complex<double>(out[j][0], out[j][1])) );
            }
            ret.push_back(line);
        }
    }
    /* if i not zero need fill in and fftw_execute */
    if (i != 0) {
        for(;i < mFftSize;i++) {
            in[i][0] = 0;
            in[i][1] = 0;
        }
        fftw_execute(plan);
        std::vector< double > line;

        for(j = 0; j < mFftSize/2; j++) {
            line.push_back( std::abs(std::complex<double>(out[j][0], out[j][1])) );
        }
        ret.push_back(line);
    }
    mStream->rewind();
    
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    
    return ret;
}

vector2d MFCC::trifBank() {
    float fLow = hzToMel(0);
    float fHigh = hzToMel(mStream->getSampleRate() / 2.);
    int M = mFftSize / 2;
    std::vector< double > C;
    float tmp, interval = 1000.0 / mDuration;
    float len = (fHigh - fLow) / (mFiltersCount + 1.);
    int i,k;
    vector2d ret;
    
    for(i = 0; i < mFiltersCount + 1; i++) {
        C.push_back( melToHz(fLow + (i+1) * len) );
    }
    
    for(i = 0; i < mFiltersCount; i++) {
        std::vector<double> vec;
        for(k = 0; k < M; k++) {
            tmp = k * interval;
            if (C[i] <= tmp && tmp <= C[i+1]) {
                vec.push_back( (tmp - C[i]) / (C[i+1] - C[i]) );
            } else if (C[i+1] <= tmp && tmp <= C[i+2]) {
                vec.push_back( (C[i+2] - tmp) / (C[i+2] - C[i+1]) );
            } else {
                vec.push_back(0.);
            }
        }
        ret.push_back( vec );
    }
    return ret;
}

void MFCC::initialize(int duration, int filtersCount, int mfccCount)
{
    mFiltersCount = filtersCount;
    mMfccCount    = mfccCount;
    mDuration     = duration;
    mStream       = nullptr;
    mFftSize      = 0;
}

//private:
