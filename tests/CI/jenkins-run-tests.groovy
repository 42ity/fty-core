import hudson.model.*
import hudson.utils.*

/**
 * Copyright (C) 2014 Eaton
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
    \brief   Runs the "Test plan"
    \author  Tomas Halman <TomasHalman@Eaton.com>
    \details Runs the "Test plan". Necessary jobs first (like compiling),
             later on functional/integration tests. If one of necessary jobs
             fails, Test plan fails immediatelly too. If one of functional/integration
             tests fails, test plan prints warning and is marked as unstable
             using "Console output parsing" plugin.
*/

def doJob(name,params) {
    job = Hudson.instance.getJob(name);
    if( job.isDisabled() ) return 0;
    fut = job.scheduleBuild2(0, new Cause.UpstreamCause(build), new ParametersAction(params) );
    fut.waitForStart();
    println "* See " + job.getLastBuild().getAbsoluteUrl() + "consoleFull for console output.";
    while( ! ( fut.isDone() || fut.isCancelled() ) ) {
        sleep(1000);
    }
    println "* Job duration: " + job.getLastCompletedBuild().getDurationString();
    result = job.getLastCompletedBuild();
}


def buildmachine = build.buildVariableResolver.resolve("BUILDMACHINE");
def fork = build.buildVariableResolver.resolve("FORK");
def branch = build.buildVariableResolver.resolve("BRANCH");
def buildsubdir = build.buildVariableResolver.resolve("BUILDSUBDIR");
def SKIP_MAKE_CHECK_GITIGNORE = build.buildVariableResolver.resolve("SKIP_MAKE_CHECK_GITIGNORE");
def CITEST_QUICKFAIL = build.buildVariableResolver.resolve("CITEST_QUICKFAIL");
def SUT_WEB_SCHEMA = build.buildVariableResolver.resolve("SUT_WEB_SCHEMA");

println "BRANCH=$branch"
println "FORK=$fork"
println "BUILDMACHINE=$buildmachine"
println "BUILDSUBDIR=$buildsubdir"
println "SKIP_MAKE_CHECK_GITIGNORE=$SKIP_MAKE_CHECK_GITIGNORE"
println "CITEST_QUICKFAIL=$CITEST_QUICKFAIL"
println "SUT_WEB_SCHEMA=$SUT_WEB_SCHEMA"

def jobParams = [
  new StringParameterValue('FORK', fork),
  new StringParameterValue('BRANCH', branch),
  new StringParameterValue('BUILDMACHINE', buildmachine),
  new StringParameterValue('BUILDSUBDIR', buildsubdir),
  new StringParameterValue('SKIP_MAKE_CHECK_GITIGNORE', SKIP_MAKE_CHECK_GITIGNORE),
  new StringParameterValue('CITEST_QUICKFAIL', CITEST_QUICKFAIL),
  new StringParameterValue('SUT_WEB_SCHEMA', SUT_WEB_SCHEMA),
]

// Running critical jobs, no sense to continue on error
for(
    jobName in [
        "test_make_check",
        "test_deploy_database"
    ]
){
    println "=== Starting $jobName ===";
    lastbuild = doJob(jobName, jobParams);
    if( lastbuild == 0 ) {
        println "SKIPPED: Job $jobName is disabled";
    } else {
        result = lastbuild.getResult();
        if ( result == Result.SUCCESS ) {
            print result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for details";
        } else  if ( result == Result.UNSTABLE ) {
            print "WARNING: " + jobName + " result is " + result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for failed build details";
        } else  if ( result == Result.ABORTED ) {
            print "ERROR: " + jobName + " result is " + result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for failed build details";
            throw new Exception("Job $jobName was aborted");
        } else {
            print "ERROR: " + jobName + " result is " + result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for failed build details";
            throw new Exception("Job $jobName failed");
        }
    }
}

// Running other tests, it make sense to continue if some of them fails
for(
    jobName in [
        "test_compilation_warnings",
        "start_bios",
        "test_db_tests",
        "test_restapi",
        "test_netcfg",
        "test_NUT",
        "test_rackpower",
        "test_libbiosapi",
        "stop_bios"
    ]
){
    println "=== Starting $jobName ===";
    lastbuild = doJob(jobName, jobParams);
    if( lastbuild == 0 ) {
        println "SKIPPED: Job $jobName is disabled, skipped";
    } else {
        result = lastbuild.getResult();
        if ( result == Result.SUCCESS ) {
            print result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for details";
        } else  if ( result == Result.ABORTED ) {
            print "ERROR: " + jobName + " result is " + result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + " for failed build details";
            throw new Exception("Job $jobName was aborted");
        } else {
            print "WARNING: " + jobName + " result is " + result.toString();
            println ", see " + lastbuild.getAbsoluteUrl() + "  for failed build details";
        }
    }
}

println ""
println "=== Wrapping up the umbrella build:"
println "* Build scheduled: " + build.getTimestamp().getTime().toString();
println "* Build duration: " + build.getDurationString() + " (finishing now)";
println ""
