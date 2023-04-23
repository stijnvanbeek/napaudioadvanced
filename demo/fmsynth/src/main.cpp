/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "audioapp.h"
#include "synthappeventhandler.h"

// Nap includes
#include <nap/logger.h>
#include <apprunner.h>

/**
 * Hello World Demo
 * refer to helloworldapp.h for a more detailed description of the application
 */
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner using the SynthAppEventHandler specialized in the use of the virtual midi keyboard
	nap::AppRunner<nap::AudioTestApp, nap::SynthAppEventHandler> app_runner(core);

	// Start
	nap::utility::ErrorState error;
	if (!app_runner.start(error))
	{
		nap::Logger::fatal("error: %s", error.toString().c_str());
		return -1;
	}

	// Return if the app ran successfully
	return app_runner.exitCode();
}
