import hudson.model.*
import hudson.utils.*

/**
 * Copyright (C) 2014 - 2020 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*! \file    jenkins-run-vtetests.groovy
    \brief   starts the VM reset jobs for all hypervisors
    \author  Tomas Halman <TomasHalman@Eaton.com>
*/

def startJob(name,params) {
    job = Hudson.instance.getJob(name);
    job.scheduleBuild2(0, new Cause.UpstreamCause(build), new ParametersAction(params) );
    sleep(1000);
}

def waitForJob(name) {
    jobs = Hudson.instance.getJob(name);
    while( job.isInQueue() || job.isBuilding() ) {
        sleep(1000);
    }
}

// Start another jobs to reset machines
for( vm in ["mirabox-test.roz.lab.etn.com", "debian-test.roz.lab.etn.com", "test-debian-0", "test-debian-1", "test-debian-2", "test-debian-3"]) {
    println "reseting $vm";
    startJob(
        "reset_virtual_machine",
        [ new StringParameterValue('VIRTUALMACHINE', vm), ]
    );
    waitForJob("reset_virtual_machine");
}
