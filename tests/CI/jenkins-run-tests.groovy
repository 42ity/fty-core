import hudson.model.*
import hudson.utils.*

def startJob(name,params) {
    job = Hudson.instance.getJob(name);
    job.scheduleBuild2(0, new Cause.UpstreamCause(build), new ParametersAction(params) );
    sleep(1000);
}

def waitForJob(name) {
    job = Hudson.instance.getJob(name);
    while( job.isInQueue() || job.isBuilding() ) {
        sleep(1000);
    }
    return job.getLastBuild().getResult();
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

// Running critical jobs, no sense to continue on error
for( jobName in [ "test_make_check", "test_deploy_database" ]){
    println "=== Starting $jobName ===";
    startJob(jobName, jobParams);
    result = waitForJob(jobName);
    if ( result == Result.SUCCESS ) {
        println result.toString()
    } else  if ( result == Result.UNSTABLE ) {
        println "WARNING: " + jobName + " result is " + result.toString();
    } else {
        println "ERROR: " + jobName + " result is " + result.toString();
        throw new Exception("Job $jobName failed");
    }
}

// Running other tests, it make sense to continue if some of them fails
for( jobName in [ "start_bios","test_netmon", "test_restapi", "stop_bios"]){
    println "=== Starting $jobName ===";
    startJob(jobName, jobParams);
    result = waitForJob(jobName);
    if ( result == Result.SUCCESS ) {
        println result.toString()
    } else {
        println "WARNING: " + jobName + " result is " + result.toString();
    }
}
