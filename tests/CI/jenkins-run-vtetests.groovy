import hudson.model.*
import hudson.utils.*

/**
 * Copyright (C) 2014 Eaton

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s): Tomas Halman <TomasHalman@eaton.com>
 *
 * Description: Runs the "Test plan". Necessary jobs first (like compiling),
 *              later on functional/integration tests. If one of necessary jobs
 *              fails, Test plan fails immediatelly too. If one of functional/integration
 *              tests fails, test plan prints warning and is marked as unstable
 *              using "Console output parsing" plugin.
 */

def doJob(name,params) {
    job = Hudson.instance.getJob(name);
    if( job.isDisabled() ) return 0;
    fut = job.scheduleBuild2(0, new Cause.UpstreamCause(build), new ParametersAction(params) );
    fut.waitForStart();
    while( ! ( fut.isDone() || fut.isCancelled() ) ) {
        sleep(1000);
    }
    return job.getLastBuild();
}


def buildmachine = build.buildVariableResolver.resolve("BUILDMACHINE");
def fork = build.buildVariableResolver.resolve("FORK");
def branch = build.buildVariableResolver.resolve("BRANCH");

println "BRANCH=$branch"
println "FORK=$fork"
println "BUILDMACHINE=$buildmachine"

def jobParams = [
  new StringParameterValue('FORK', fork),
  new StringParameterValue('BRANCH', branch),
  new StringParameterValue('BUILDMACHINE', buildmachine),
]


// Running other tests, it make sense to continue if some of them fails
for(
    jobName in [
        "vtetest_restapi",
        "vtetest_rackpower",
    ]
){
    println "=== Starting $jobName ===";
    lastbuild = doJob(jobName, jobParams);
    if( lastbuild == 0 ) {
        println "Job $jobName is disabled, skipped";
    } else {
        result = lastbuild.getResult();
            if ( result == Result.SUCCESS ) {
            print result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for details";
        } else {
            println "WARNING: " + jobName + " result is " + result.toString();
            println "see " + lastbuild.getAbsoluteUrl() + "  for failed build";
            println "or  " + lastbuild.getAbsoluteUrl() + "console for console output.";
        }
    }
}
