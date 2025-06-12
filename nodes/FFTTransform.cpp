#include <fftw3.h>
#include <iostream>
#include <vector>
#include <cmath>

int main(int argc, char** argv) {
    int N = 256;
    if (argc>1) N = std::stoi(argv[1]);

    std::vector<double> in(N);
    std::vector<fftw_complex> out(N/2+1);
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in.data(), out.data(), FFTW_MEASURE);

    while (std::cin.read(reinterpret_cast<char*>(in.data()), sizeof(double)*N)) {
        fftw_execute(plan);
        // write out magnitude as doubles
        for (int i = 0; i < N/2+1; ++i) {
            double mag = std::hypot(out[i][0], out[i][1]);
            std::cout.write(reinterpret_cast<char*>(&mag), sizeof(mag));
        }
        std::cout.flush();
    }
    fftw_destroy_plan(plan);
    return 0;
}
