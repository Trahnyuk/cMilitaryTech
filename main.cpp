#include "MissionInput.h"
#include "AmmoDatabase.h"
#include "BallisticCalculator.h"
#include "FireSolution.h"

#include <iostream>

int main() {
    try {
        
        const cMiltech::MissionInput input("input.txt");

        
        const cMiltech::AmmoDatabase db;
        const auto ammoResult = db.find(input.getAmmoName());

        if (!ammoResult.has_value()) {
            std::cerr << "Unknown ammo type: " << input.getAmmoName() << '\n';
            return 1;
        }

        
        const cMiltech::FireSolution solution =
            cMiltech::BallisticCalculator::compute(input, ammoResult.value());

        
        solution.print();
        solution.writeToFile("output.txt");

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
