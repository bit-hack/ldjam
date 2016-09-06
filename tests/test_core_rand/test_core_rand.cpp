#include <array>
#include <cstdio>
#include <vector>
#include <memory>
#include "../../framework_core/random.h"

struct csv_t {

    csv_t(const char * path = nullptr)
        : fd_(nullptr)
    {
        if (path) {
            open(path);
        }
    }

    ~csv_t() {
        if (fd_) {
            fclose(fd_);
            fd_ = nullptr;
        }
    }

    bool open(const char * path) {
#if defined(_MSC_VER)
        fopen_s(&fd_, path, "w");
#else
        fd_ = fopen(path, "w");
#endif
        return fd_ ? true : false;
    }

    void plot(const float x, const float y) {
        if (fd_) {
            fprintf(fd_, "%f,%f\n", x, y);
        }
    }

    FILE * fd_;
};

struct histogram_t {

    std::vector<float> value_;
    float min_, max_;

    histogram_t()
        : min_(0.f)
        , max_(0.f)
    {
    }

    void push(float x) {
        if (value_.empty()) {
            min_ = max_ = x;
        }
        else {
            min_ = (x<min_) ? x : min_;
            max_ = (x>max_) ? x : max_;
        }
        value_.push_back(x);
    }

    void finish(const char * path) {
        const float range = 1.f / (max_-min_);
        std::array<int32_t, 100> bucket;
        bucket.fill(0);
        for (float v : value_) {
            int32_t index = int32_t(((v-min_) * range) * bucket.size());
            if (index>=bucket.size()) index = 0;
            if (index<0) index = 0;
            ++bucket[index];
        }
        csv_t csv(path);
        for (int32_t i = 0; i<bucket.size(); ++i) {
            float x = min_ + ((float(i) / bucket.size()) / range);
            float y = (float(bucket[i])/float(value_.size())) * 100.f;
            csv.plot(x, y);
        }
    }
};

bool test_randfu() {
    random_t rand(1234);
    std::unique_ptr<histogram_t> hist(new histogram_t);
    for (int i = 0; i<1024 * 100; ++i) {
        hist->push(rand.randfu());
    }
    hist->finish("random_randfu.csv");
    return true;
}

bool test_randfs() {
    random_t rand(1234);
    std::unique_ptr<histogram_t> hist(new histogram_t);
    for (int i = 0; i<1024 * 100; ++i) {
        hist->push(rand.randfs());
    }
    hist->finish("random_randfs.csv");
    return true;
}

bool test_gaussian() {
    random_t rand(1234);
    std::unique_ptr<histogram_t> hist(new histogram_t);
    for (int i = 0; i<1024 * 100; ++i) {
        hist->push(rand.gaussian());
    }
    hist->finish("random_gaussian.csv");
    return true;
}

bool test_triangle() {
    random_t rand(1234);
    std::unique_ptr<histogram_t> hist(new histogram_t);
    for (int i = 0; i<1024 * 100; ++i) {
        hist->push(rand.trandfs());
    }
    hist->finish("random_triangle.csv");
    return true;
}

bool test_pinch() {
    random_t rand(1234);
    std::unique_ptr<histogram_t> hist(new histogram_t);
    for (int i = 0; i<1024 * 100; ++i) {
        hist->push(rand.pinch());
    }
    hist->finish("random_pinch.csv");
    return true;
}

int main() {
    test_randfu();
    test_randfs();
    test_gaussian();
    test_triangle();
    test_pinch();
    return 0;
}
