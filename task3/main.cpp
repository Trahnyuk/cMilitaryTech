#include "AmmoDatabase.h"
#include "MissionInput.h"
#include "Simulator.h"
#include "TargetDatabase.h"
#include "LoggingMacros.h"

#include <iomanip>
#include <iostream>
#include <cstring>

int main() {
    try {
        LOG("=== Drone Simulation ===");
        
        // Load configuration from JSON (using C-strings)
        const cMiltech::MissionInput input("config.json");
        
        // Load ammunition database from JSON (dynamic array with new[])
        const cMiltech::AmmoDatabase ammoDb("ammo.json");

        // Find ammo (using C-string comparison)
        const cMiltech::AmmoParameters* ammo = ammoDb.find(input.getAmmoName());
        if (!ammo) {
            std::cerr << "Unknown ammo type: " << input.getAmmoName() << '\n';
            return 1;
        }
        LOG("Selected ammo: " << ammo->getName());

        // Load target database from JSON (2D array with new[][])
        const cMiltech::TargetDatabase targets("targets.json");
        LOG("Targets loaded: " << targets.targetCount() << " targets, " 
            << targets.timeSteps() << " time steps");

        //// Run simulation
        cMiltech::Simulator sim(input, *ammo, targets);
        
        //LOG("Running simulation...");
        const auto log = sim.run();

        // Write output in JSON format (Homework 3 requirement)
        log.writeToFile("simulation.json");

        // Display summary
        std::cout << "\n=== Simulation Complete ===\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Steps recorded: " << log.size() << '\n';

        if (log.hasDrop()) {
            const auto& d = log.drop();
            std::cout << "\nDrop Event:\n";
            std::cout << "  Time:      " << d.time << " s\n";
            std::cout << "  Target:    " << d.targetIndex << '\n';
            std::cout << "  Position:  (" << d.dronePos.x << ", " << d.dronePos.y << ")\n";
            std::cout << "  Direction: " << d.direction << " rad\n";
        } else {
            std::cout << "\nNo drop occurred (max steps reached)\n";
        }
        
        std::cout << "\nOutput files:\n";
        std::cout << "  simulation.json\n";

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
