/*
 * zero_torque.cpp
 *
 *  Created on: Apr 5, 2010
 *      Author: dc
 */

#include <iostream>

#include <libconfig.h++>

#include <barrett/systems.h>


using namespace barrett;

const size_t DOF = 4;
const double T_s = 0.002;


int main() {
	libconfig::Config config;
	config.readFile("/etc/barrett/wam4.conf");

	systems::RealTimeExecutionManager rtem(T_s);
	systems::System::defaultExecutionManager = &rtem;


	// instantiate Systems
	systems::Wam<DOF> wam(config.lookup("wam"));


	// start the main loop!
	rtem.start();

	while (true) {
		sleep(1);
	}

	rtem.stop();
	return 0;
}
