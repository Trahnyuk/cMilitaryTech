#include "AmmoDatabase.h"
#include "MissionInput.h"
#include "Simulator.h"
#include "TargetDatabase.h"

#include <iomanip>
#include <iostream>

int main() {
    try {
        const cMiltech::MissionInput input("input.txt");
        const cMiltech::AmmoDatabase ammoDb;

        const auto ammoOpt = ammoDb.find(input.getAmmoName());
        if (!ammoOpt.has_value()) {
            std::cerr << "Unknown ammo type: " << input.getAmmoName() << '\n';
            return 1;
        }

        const cMiltech::TargetDatabase targets("targets.txt");

        cMiltech::Simulator sim(input, ammoOpt.value(), targets);
        const auto log = sim.run();

        log.writeToFile("simulation.txt");

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Simulation complete. Steps recorded: " << log.size() << '\n';

        if (log.hasDrop()) {
            const auto& d = log.drop();
            std::cout << "Drop: t=" << d.time << "s"
                      << "  target=" << d.targetIndex
                      << "  drone=(" << d.dronePos.x << ", " << d.dronePos.y << ")"
                      << "  dir="    << d.direction  << " rad\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
