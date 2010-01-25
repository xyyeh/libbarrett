/*
 * cartesian_hold.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: dc
 */

#include <iostream>
#include <string>

#include <unistd.h>  // usleep

#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <libconfig.h>
#include <Eigen/Geometry>

#include <barrett/exception.h>
#include <barrett/detail/debug.h>
#include <barrett/thread/abstract/mutex.h>
#include <barrett/math.h>
#include <barrett/units.h>
#include <barrett/log.h>
#include <barrett/systems.h>
#include <barrett/wam.h>


//namespace log = barrett::log;
namespace math = barrett::math;
namespace systems = barrett::systems;
namespace units = barrett::units;
using barrett::Wam;
using systems::connect;
using systems::reconnect;
using systems::disconnect;


using boost::bind;
using boost::ref;


const size_t DOF = 7;
const double T_s = 0.002;


void waitForEnter() {
	static std::string line;
	std::getline(std::cin, line);
}

int main() {
	barrett::installExceptionHandler();  // give us pretty stack traces when things die


	struct config_t config;
	char filename[] = "/etc/wam/wamg.config";
	config_init(&config);
	syslog(LOG_ERR,"Open '%s' ...",filename);
	int err = config_read_file(&config,filename);
	if (err != CONFIG_TRUE) {
		syslog(LOG_ERR,"libconfig error: %s, line %d\n",
		config_error_text(&config), config_error_line(&config));
		config_destroy(&config);
		/* Failure */
		return -1;
	}
	config_setting_t* wamconfig = config_lookup(&config, "wam");
	systems::KinematicsBase<DOF> kin(config_setting_get_member(wamconfig, "kinematics"));
	config_destroy(&config);


	systems::RealTimeExecutionManager rtem;
	systems::System::defaultExecutionManager = &rtem;


	Wam<DOF> wam;

	systems::ToolPosition<DOF> toolPos;
	systems::ToolForceToJointTorques<DOF> tf2jt;
	systems::PIDController<units::CartesianPosition> pid;

	systems::ToolOrientation<DOF> toolOrient;
	systems::ToolOrientationController<DOF> toolOrientController;

	systems::Summer<Wam<DOF>::jt_type, 2> jtSum;


	math::Array<3> tmp;
	tmp.assign(2e3);
	pid.setKp(tmp);
	tmp.assign(2e1);
	pid.setKd(tmp);
	tmp.assign(1e2);
	pid.setControlSignalLimit(tmp);


	connect(wam.jpOutput, kin.jpInput);
	connect(wam.jvOutput, kin.jvInput);
	connect(kin.kinOutput, toolPos.kinInput);
	connect(kin.kinOutput, tf2jt.kinInput);
	connect(kin.kinOutput, toolOrient.kinInput);
	connect(kin.kinOutput, toolOrientController.kinInput);

	connect(toolPos.output, pid.feedbackInput);
	connect(pid.controlOutput, tf2jt.input);
	connect(tf2jt.output, jtSum.getInput(0));

	connect(toolOrient.output, toolOrientController.feedbackInput);
	connect(toolOrientController.controlOutput, jtSum.getInput(1));

	connect(jtSum.output, wam.input);


	// tie inputs together for zero torque
	connect(toolPos.output, pid.referenceInput);
	connect(toolOrient.output, toolOrientController.referenceInput);



	// start the main loop!
	rtem.start();

	std::cout << "Enter to gravity compensate.\n";
	waitForEnter();
	wam.gravityCompensate();

	std::cout << "Enter to hold Cartesian position.\n";
	waitForEnter();
	systems::ExposedOutput<units::CartesianPosition> setPointLoc;
	systems::ExposedOutput<Eigen::Quaterniond> setPointOrient;
	{
		SCOPED_LOCK(rtem.getMutex());
		setPointLoc.setValue(pid.feedbackInput.getValue());
		setPointOrient.setValue(toolOrientController.feedbackInput.getValue());
	}
	reconnect(setPointLoc.output, pid.referenceInput);
	reconnect(setPointOrient.output, toolOrientController.referenceInput);


	std::cout << "Enter to move home.\n";
	waitForEnter();
	reconnect(toolPos.output, pid.referenceInput);  // zero torque
	reconnect(toolOrient.output, toolOrientController.referenceInput);

	wam.moveHome();
	while ( !wam.moveIsDone() ) {
		usleep(static_cast<int>(1e5));
	}

	wam.gravityCompensate(false);
	wam.idle();

	std::cout << "Shift-idle, then press enter.\n";
	waitForEnter();
	rtem.stop();

	return 0;
}