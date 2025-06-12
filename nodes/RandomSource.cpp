#include <chrono>
#include <iostream>
#include <random>
#include <thread>

using namespace std;
int main(int argc, char** argv) {
    int rate = 1000; // samples/sec
    if (argc>1) rate = std::stoi(argv[1]);
    mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<float> dist(0,1);

    auto period = std::chrono::microseconds(1000000 / rate);
    while (true) {
        float sample = dist(gen);
        cout.write(reinterpret_cast<char*>(&sample), sizeof(sample));
        cout.flush();
        this_thread::sleep_for(period);
    }
    return 0;
}
